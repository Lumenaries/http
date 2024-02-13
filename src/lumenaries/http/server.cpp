#include "lumenaries/http/server.hpp"

#include "lumenaries/http/endpoint.hpp"
#include "lumenaries/http/handler.hpp"
#include "lumenaries/http/request.hpp"
#include "lumenaries/http/web_handler.hpp"

#include "esp_log.h"

#include <unistd.h> // close()

namespace lumenaries::http {

Server::Server() : _onOpen(NULL), _onClose(NULL)
{
    maxRequestBodySize = CONFIG_LUM_HTTP_MAX_REQUEST_BODY_SIZE;
    maxUploadSize = CONFIG_LUM_HTTP_MAX_UPLOAD_SIZE;

    defaultEndpoint = new Endpoint(this, HTTP_GET, "");
    onNotFound(Server::defaultNotFoundHandler);

    // for a regular server
    config = HTTPD_DEFAULT_CONFIG();
    config.open_fn = Server::openCallback;
    config.close_fn = Server::closeCallback;
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.global_user_ctx = this;
    config.global_user_ctx_free_fn = destroy;
    config.max_uri_handlers = 20;

#ifdef ENABLE_ASYNC
    // It is advisable that httpd_config_t->max_open_sockets >
    // MAX_ASYNC_REQUESTS Why? This leaves at least one socket still available
    // to handle quick synchronous requests. Otherwise, all the sockets will get
    // taken by the long async handlers, and your server will no longer be
    // responsive.
    config.max_open_sockets = ASYNC_WORKER_COUNT + 1;
    config.lru_purge_enable = true;
#endif
}

Server::~Server()
{
    for (auto* client : _clients)
        delete (client);
    _clients.clear();

    for (auto* endpoint : _endpoints)
        delete (endpoint);
    _endpoints.clear();

    for (auto* handler : _handlers)
        delete (handler);
    _handlers.clear();

    delete defaultEndpoint;
}

void Server::destroy(void* ctx)
{
    Server* temp = (Server*)ctx;
    delete temp;
}

esp_err_t Server::listen(uint16_t port)
{
    this->_use_ssl = false;
    this->config.server_port = port;

    return this->_start();
}

esp_err_t Server::_start()
{
    esp_err_t ret;

#ifdef ENABLE_ASYNC
    // start workers
    start_async_req_workers();
#endif

    // fire it up.
    ret = _startServer();
    if (ret != ESP_OK) {
        ESP_LOGE(lib_tag, "Server start failed (%s)", esp_err_to_name(ret));
        return ret;
    }

    // Register handler
    ret = httpd_register_err_handler(
        server, HTTPD_404_NOT_FOUND, Server::notFoundHandler
    );
    if (ret != ESP_OK)
        ESP_LOGE(lib_tag, "Add 404 handler failed (%s)", esp_err_to_name(ret));

    return ret;
}

esp_err_t Server::_startServer()
{
    return httpd_start(&this->server, &this->config);
}

void Server::stop()
{
    httpd_stop(this->server);
}

Handler& Server::addHandler(Handler* handler)
{
    _handlers.push_back(handler);
    return *handler;
}

void Server::removeHandler(Handler* handler)
{
    _handlers.remove(handler);
}

Endpoint* Server::on(const char* uri)
{
    return on(uri, HTTP_GET);
}

Endpoint* Server::on(const char* uri, http_method method)
{
    WebHandler* handler = new WebHandler();

    return on(uri, method, handler);
}

Endpoint* Server::on(const char* uri, Handler* handler)
{
    return on(uri, HTTP_GET, handler);
}

Endpoint* Server::on(const char* uri, http_method method, Handler* handler)
{
    // make our endpoint
    Endpoint* endpoint = new Endpoint(this, method, uri);

    // set our handler
    endpoint->setHandler(handler);

    // URI handler structure
    httpd_uri_t my_uri{
        .uri = uri,
        .method = method,
        .handler = Endpoint::requestCallback,
        .user_ctx = endpoint,
    };

    // Register endpoint with ESP-IDF server
    esp_err_t ret = httpd_register_uri_handler(this->server, &my_uri);
    if (ret != ESP_OK)
        ESP_LOGE(lib_tag, "Add endpoint failed (%s)", esp_err_to_name(ret));

    // save it for later
    _endpoints.push_back(endpoint);

    return endpoint;
}

Endpoint* Server::on(const char* uri, HttpRequestCallback fn)
{
    return on(uri, HTTP_GET, fn);
}

Endpoint*
Server::on(const char* uri, http_method method, HttpRequestCallback fn)
{
    // these basic requests need a basic web handler
    WebHandler* handler = new WebHandler();
    handler->onRequest(fn);

    return on(uri, method, handler);
}

void Server::onNotFound(HttpRequestCallback fn)
{
    WebHandler* handler = new WebHandler();
    handler->onRequest(fn);

    this->defaultEndpoint->setHandler(handler);
}

esp_err_t Server::notFoundHandler(httpd_req_t* req, httpd_err_code_t err)
{
    Server* server = (Server*)httpd_get_global_user_ctx(req->handle);
    Request request(server, req);

    // loop through our global handlers and see if anyone wants it
    for (auto* handler : server->_handlers) {
        // are we capable of handling this?
        if (handler->filter(&request) && handler->canHandle(&request)) {
            // check our credentials
            if (handler->needsAuthentication(&request))
                return handler->authenticate(&request);
            else
                return handler->handleRequest(&request);
        }
    }

    // nothing found, give it to our defaultEndpoint
    Handler* handler = server->defaultEndpoint->handler();
    if (handler->filter(&request) && handler->canHandle(&request))
        return handler->handleRequest(&request);

    // not sure how we got this far.
    return ESP_ERR_HTTPD_INVALID_REQ;
}

esp_err_t Server::defaultNotFoundHandler(Request* request)
{
    request->reply(404, "text/html", "That URI does not exist.");

    return ESP_OK;
}

void Server::onOpen(ClientCallback handler)
{
    this->_onOpen = handler;
}

esp_err_t Server::openCallback(httpd_handle_t hd, int sockfd)
{
    ESP_LOGI(lib_tag, "New client connected %d", sockfd);

    // get our global server reference
    Server* server = (Server*)httpd_get_global_user_ctx(hd);

    // lookup our client
    Client* client = server->getClient(sockfd);
    if (client == NULL) {
        client = new Client(hd, sockfd);
        server->addClient(client);
    }

    // user callback
    if (server->_onOpen != NULL)
        server->_onOpen(client);

    return ESP_OK;
}

void Server::onClose(ClientCallback handler)
{
    this->_onClose = handler;
}

void Server::closeCallback(httpd_handle_t hd, int sockfd)
{
    ESP_LOGI(lib_tag, "Client disconnected %d", sockfd);

    Server* server = (Server*)httpd_get_global_user_ctx(hd);

    // lookup our client
    Client* client = server->getClient(sockfd);
    if (client != NULL) {
        // give our handlers a chance to handle a disconnect first
        for (Endpoint* endpoint : server->_endpoints) {
            Handler* handler = endpoint->handler();
            handler->checkForClosedClient(client);
        }

        // do we have a callback attached?
        if (server->_onClose != NULL)
            server->_onClose(client);

        // remove it from our list
        server->removeClient(client);
    } else
        ESP_LOGE(lib_tag, "No client record %d", sockfd);

    // finally close it out.
    close(sockfd);
}

void Server::addClient(Client* client)
{
    _clients.push_back(client);
}

void Server::removeClient(Client* client)
{
    _clients.remove(client);
    delete client;
}

Client* Server::getClient(int socket)
{
    for (Client* client : _clients)
        if (client->socket() == socket)
            return client;

    return NULL;
}

Client* Server::getClient(httpd_req_t* req)
{
    return getClient(httpd_req_to_sockfd(req));
}

bool Server::hasClient(int socket)
{
    return getClient(socket) != NULL;
}

const std::list<Client*>& Server::getClientList()
{
    return _clients;
}

std::string urlDecode(const char* encoded)
{
    size_t length = strlen(encoded);
    char* decoded = (char*)malloc(length + 1);
    if (!decoded) {
        return "";
    }

    size_t i, j = 0;
    for (i = 0; i < length; ++i) {
        if (encoded[i] == '%' && isxdigit(encoded[i + 1]) &&
            isxdigit(encoded[i + 2])) {
            // Valid percent-encoded sequence
            int hex;
            sscanf(encoded + i + 1, "%2x", &hex);
            decoded[j++] = (char)hex;
            i += 2; // Skip the two hexadecimal characters
        } else if (encoded[i] == '+') {
            // Convert '+' to space
            decoded[j++] = ' ';
        } else {
            // Copy other characters as they are
            decoded[j++] = encoded[i];
        }
    }

    decoded[j] = '\0'; // Null-terminate the decoded string

    std::string output(decoded);
    free(decoded);

    return output;
}

} // namespace lumenaries::http
