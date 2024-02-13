#include "lumenaries/http/endpoint.hpp"

#include "lumenaries/http/server.hpp"

#include <string>

namespace lumenaries::http {

Endpoint::Endpoint()
    : _server(NULL), _uri(""), _method(HTTP_GET), _handler(NULL)
{
}

Endpoint::Endpoint(Server* server, http_method method, const char* uri)
    : _server(server), _uri(uri), _method(method), _handler(NULL)
{
}

Endpoint* Endpoint::setHandler(Handler* handler)
{
    // clean up old / default handler
    if (_handler != NULL)
        delete _handler;

    // get our new pointer
    _handler = handler;

    // keep a pointer to the server
    _handler->_server = _server;

    return this;
}

Handler* Endpoint::handler()
{
    return _handler;
}

std::string Endpoint::uri()
{
    return _uri;
}

esp_err_t Endpoint::requestCallback(httpd_req_t* req)
{
#ifdef ENABLE_ASYNC
    if (is_on_async_worker_thread() == false) {
        if (submit_async_req(req, Endpoint::requestCallback) == ESP_OK) {
            return ESP_OK;
        } else {
            httpd_resp_set_status(req, "503 Busy");
            httpd_resp_sendstr(req, "No workers available. Server busy.</div>");
            return ESP_OK;
        }
    }
#endif

    Endpoint* self = (Endpoint*)req->user_ctx;
    Handler* handler = self->handler();
    Request request(self->_server, req);

    // make sure we have a handler
    if (handler != NULL) {
        if (handler->filter(&request) && handler->canHandle(&request)) {
            // check our credentials
            if (handler->needsAuthentication(&request))
                return handler->authenticate(&request);

            // pass it to our handler
            return handler->handleRequest(&request);
        }
        // pass it to our generic handlers
        else
            return Server::notFoundHandler(
                req, HTTPD_500_INTERNAL_SERVER_ERROR
            );
    } else
        return request.reply(500, "text/html", "No handler registered.");
}

Endpoint* Endpoint::setFilter(RequestFilterFunction fn)
{
    _handler->setFilter(fn);
    return this;
}

Endpoint* Endpoint::setAuthentication(
    const char* username,
    const char* password,
    HTTPAuthMethod method,
    const char* realm,
    const char* authFailMsg
)
{
    _handler->setAuthentication(username, password, method, realm, authFailMsg);
    return this;
};

} // namespace lumenaries::http
