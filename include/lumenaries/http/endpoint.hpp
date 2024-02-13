#pragma once

#include "lumenaries/http/async_worker.hpp"
#include "lumenaries/http/server.hpp"

#include <string>

namespace lumenaries::http {

class Handler;

class Endpoint {
public:
    Endpoint();
    Endpoint(Server* server, http_method method, const char* uri);

    Endpoint* setHandler(Handler* handler);
    Handler* handler();

    Endpoint* setFilter(RequestFilterFunction fn);
    Endpoint* setAuthentication(
        const char* username,
        const char* password,
        HTTPAuthMethod method = BASIC_AUTH,
        const char* realm = "",
        const char* authFailMsg = ""
    );

    std::string uri();

    static esp_err_t requestCallback(httpd_req_t* req);

private:
    Server* _server;
    std::string _uri;
    http_method _method;
    Handler* _handler;

    friend Server;
};

} // namespace lumenaries::http
