#pragma once

#include "lumenaries/http/core.hpp"

#include "esp_http_server.h"

namespace lumenaries::http {

/*
 * Client :: Generic wrapper around the ESP-IDF socket
 */

class Client {
public:
    Client(httpd_handle_t server, int socket);
    ~Client();

    // no idea if this is the right way to do it or not, but lets see.
    // pointer to our derived class (eg. WebSocketConnection)
    void* _friend;

    bool isNew = false;

    bool operator==(Client& rhs) const
    {
        return _socket == rhs.socket();
    }

    httpd_handle_t server();
    int socket();
    esp_err_t close();

    /*
    IPAddress localIP();
    IPAddress remoteIP();
    */

protected:
    httpd_handle_t _server;
    int _socket;
};

} // namespace lumenaries::http
