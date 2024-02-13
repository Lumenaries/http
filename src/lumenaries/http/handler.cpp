#include "lumenaries/http/handler.hpp"

namespace lumenaries::http {

Handler::Handler()
    : _filter(NULL), _server(NULL), _username(""), _password(""),
      _method(DIGEST_AUTH), _realm(""), _authFailMsg("")
{
}

Handler::~Handler()
{
    // actual Client deletion handled by Server
    // for (Client *client : _clients)
    //   delete(client);
    _clients.clear();
}

Handler* Handler::setFilter(RequestFilterFunction fn)
{
    _filter = fn;
    return this;
}

bool Handler::filter(Request* request)
{
    return _filter == NULL || _filter(request);
}

Handler* Handler::setAuthentication(
    const char* username,
    const char* password,
    HTTPAuthMethod method,
    const char* realm,
    const char* authFailMsg
)
{
    _username = std::string(username);
    _password = std::string(password);
    _method = method;
    _realm = std::string(realm);
    _authFailMsg = std::string(authFailMsg);
    return this;
};

bool Handler::needsAuthentication(Request* request)
{
    return (_username != "" && _password != "") &&
           !request->authenticate(_username.c_str(), _password.c_str());
}

esp_err_t Handler::authenticate(Request* request)
{
    return request->requestAuthentication(
        _method, _realm.c_str(), _authFailMsg.c_str()
    );
}

Client* Handler::checkForNewClient(Client* client)
{
    Client* c = Handler::getClient(client);
    if (c == NULL) {
        c = client;
        addClient(c);
        c->isNew = true;
    } else
        c->isNew = false;

    return c;
}

void Handler::checkForClosedClient(Client* client)
{
    if (hasClient(client)) {
        closeCallback(client);
        removeClient(client);
    }
}

void Handler::addClient(Client* client)
{
    _clients.push_back(client);
}

void Handler::removeClient(Client* client)
{
    _clients.remove(client);
}

Client* Handler::getClient(int socket)
{
    // make sure the server has it too.
    if (!_server->hasClient(socket))
        return NULL;

    // what about us?
    for (Client* client : _clients)
        if (client->socket() == socket)
            return client;

    // nothing found.
    return NULL;
}

Client* Handler::getClient(Client* client)
{
    return Handler::getClient(client->socket());
}

bool Handler::hasClient(Client* socket)
{
    return Handler::getClient(socket) != NULL;
}

const std::list<Client*>& Handler::getClientList()
{
    return _clients;
}

} // namespace lumenaries::http
