#pragma once

#include "lumenaries/http/core.hpp"
#include "lumenaries/http/request.hpp"

#include <string>

namespace lumenaries::http {

class Endpoint;
class Server;

/*
 * HANDLER :: Can be attached to any endpoint or as a generic request handler.
 */

class Handler {
public:
    Handler();
    virtual ~Handler();

    Handler* setFilter(RequestFilterFunction fn);
    bool filter(Request* request);

    Handler* setAuthentication(
        const char* username,
        const char* password,
        HTTPAuthMethod method = BASIC_AUTH,
        const char* realm = "",
        const char* authFailMsg = ""
    );
    bool needsAuthentication(Request* request);
    esp_err_t authenticate(Request* request);

    virtual bool isWebSocket()
    {
        return false;
    };

    Client* checkForNewClient(Client* client);
    void checkForClosedClient(Client* client);

    virtual void addClient(Client* client);
    virtual void removeClient(Client* client);
    virtual Client* getClient(int socket);
    virtual Client* getClient(Client* client);
    virtual void openCallback(Client* client){};
    virtual void closeCallback(Client* client){};

    bool hasClient(Client* client);
    int count()
    {
        return _clients.size();
    };
    const std::list<Client*>& getClientList();

    // derived classes must implement these functions
    virtual bool canHandle(Request* request)
    {
        return true;
    };
    virtual esp_err_t handleRequest(Request* request) = 0;

protected:
    RequestFilterFunction _filter;
    Server* _server;

    std::string _username;
    std::string _password;
    HTTPAuthMethod _method;
    std::string _realm;
    std::string _authFailMsg;

    std::list<Client*> _clients;

    friend Endpoint;
};

} // namespace lumenaries::http
