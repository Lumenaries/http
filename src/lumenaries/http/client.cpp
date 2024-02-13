#include "lumenaries/http/client.hpp"

#include "lumenaries/http/server.hpp"

#include "lwip/sockets.h"

namespace lumenaries::http {

Client::Client(httpd_handle_t server, int socket)
    : _friend(NULL), isNew(false), _server(server), _socket(socket)
{
}

Client::~Client() {}

httpd_handle_t Client::server()
{
    return _server;
}

int Client::socket()
{
    return _socket;
}

// I'm not sure this is entirely safe to call.  I was having issues with race
// conditions when highly loaded using this.
esp_err_t Client::close()
{
    esp_err_t err = httpd_sess_trigger_close(_server, _socket);
    // HttpServer::closeCallback(_server, _socket); // call this
    // immediately so the client is taken off the list.

    return err;
}

/*
IPAddress Client::localIP()
{
    IPAddress address(0, 0, 0, 0);

    char ipstr[INET6_ADDRSTRLEN];
    struct sockaddr_in6 addr; // esp_http_server uses IPv6 addressing
    socklen_t addr_size = sizeof(addr);

    if (getsockname(_socket, (struct sockaddr*)&addr, &addr_size) < 0) {
        ESP_LOGE(lib_tag, "Error getting client IP");
        return address;
    }

    // Convert to IPv4 string
    inet_ntop(AF_INET, &addr.sin6_addr.un.u32_addr[3], ipstr, sizeof(ipstr));
    ESP_LOGI(lib_tag, "Client Local IP => %s", ipstr);
    address.fromString(ipstr);

    return address;
}

IPAddress Client::remoteIP()
{
    IPAddress address(0, 0, 0, 0);

    char ipstr[INET6_ADDRSTRLEN];
    struct sockaddr_in6 addr; // esp_http_server uses IPv6 addressing
    socklen_t addr_size = sizeof(addr);

    if (getpeername(_socket, (struct sockaddr*)&addr, &addr_size) < 0) {
        ESP_LOGE(lib_tag, "Error getting client IP");
        return address;
    }

    // Convert to IPv4 string
    inet_ntop(AF_INET, &addr.sin6_addr.un.u32_addr[3], ipstr, sizeof(ipstr));
    ESP_LOGI(lib_tag, "Client Remote IP => %s", ipstr);
    address.fromString(ipstr);

    return address;
}
*/

} // namespace lumenaries::http
