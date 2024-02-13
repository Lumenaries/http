#pragma once

#include "lumenaries/http/client.hpp"
#include "lumenaries/http/core.hpp"
#include "lumenaries/http/handler.hpp"

namespace lumenaries::http {

class Endpoint;
class Handler;
class StaticFileHandler;

class Server {
public:
    Server();
    virtual ~Server();

    // esp-idf specific stuff
    httpd_handle_t server;
    httpd_config_t config;

    // some limits on what we will accept
    unsigned long maxUploadSize;
    unsigned long maxRequestBodySize;

    Endpoint* defaultEndpoint;

    static void destroy(void* ctx);

    esp_err_t listen(uint16_t port);

    virtual void stop();

    Handler& addHandler(Handler* handler);
    void removeHandler(Handler* handler);

    void addClient(Client* client);
    void removeClient(Client* client);
    Client* getClient(int socket);
    Client* getClient(httpd_req_t* req);
    bool hasClient(int socket);
    int count()
    {
        return _clients.size();
    };
    const std::list<Client*>& getClientList();

    Endpoint* on(const char* uri);
    Endpoint* on(const char* uri, http_method method);
    Endpoint* on(const char* uri, Handler* handler);
    Endpoint* on(const char* uri, http_method method, Handler* handler);
    Endpoint* on(const char* uri, HttpRequestCallback onRequest);
    Endpoint*
    on(const char* uri, http_method method, HttpRequestCallback onRequest);

    static esp_err_t notFoundHandler(httpd_req_t* req, httpd_err_code_t err);
    static esp_err_t defaultNotFoundHandler(Request* request);
    void onNotFound(HttpRequestCallback fn);

    void onOpen(ClientCallback handler);
    void onClose(ClientCallback handler);
    static esp_err_t openCallback(httpd_handle_t hd, int sockfd);
    static void closeCallback(httpd_handle_t hd, int sockfd);

protected:
    bool _use_ssl = false;
    std::list<Endpoint*> _endpoints;
    std::list<Handler*> _handlers;
    std::list<Client*> _clients;

    ClientCallback _onOpen;
    ClientCallback _onClose;

    esp_err_t _start();
    virtual esp_err_t _startServer();
};

} // namespace lumenaries::http
