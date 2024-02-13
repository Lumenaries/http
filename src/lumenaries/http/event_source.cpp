#include "lumenaries/http/event_source.hpp"

#include "esp_log.h"

#include <string>

namespace lumenaries::http {

/*****************************************/
// EventSource - Handler
/*****************************************/

EventSource::EventSource() : Handler(), _onOpen(NULL), _onClose(NULL) {}

EventSource::~EventSource() {}

EventSourceClient* EventSource::getClient(int socket)
{
    Client* client = Handler::getClient(socket);

    if (client == NULL)
        return NULL;

    return (EventSourceClient*)client->_friend;
}

EventSourceClient* EventSource::getClient(Client* client)
{
    return getClient(client->socket());
}

esp_err_t EventSource::handleRequest(Request* request)
{
    // start our open ended HTTP response
    EventSourceResponse response(request);
    esp_err_t err = response.send();

    // lookup our client
    Client* client = checkForNewClient(request->client());
    if (client->isNew) {
        // did we get our last id?
        if (request->hasHeader("Last-Event-ID")) {
            EventSourceClient* buddy = getClient(client);
            buddy->_lastId = atoi(request->header("Last-Event-ID").c_str());
        }

        // let our handler know.
        openCallback(client);
    }

    return err;
}

EventSource* EventSource::onOpen(EventSourceClientCallback fn)
{
    _onOpen = fn;
    return this;
}

EventSource* EventSource::onClose(EventSourceClientCallback fn)
{
    _onClose = fn;
    return this;
}

void EventSource::addClient(Client* client)
{
    client->_friend = new EventSourceClient(client);
    Handler::addClient(client);
}

void EventSource::removeClient(Client* client)
{
    Handler::removeClient(client);
    delete (EventSourceClient*)client->_friend;
    client->_friend = NULL;
}

void EventSource::openCallback(Client* client)
{
    EventSourceClient* buddy = getClient(client);
    if (buddy == NULL) {
        return;
    }

    if (_onOpen != NULL)
        _onOpen(buddy);
}

void EventSource::closeCallback(Client* client)
{
    EventSourceClient* buddy = getClient(client);
    if (buddy == NULL) {
        return;
    }

    if (_onClose != NULL)
        _onClose(getClient(buddy));
}

void EventSource::send(
    const char* message,
    const char* event,
    uint32_t id,
    uint32_t reconnect
)
{
    std::string ev = generateEventMessage(message, event, id, reconnect);
    for (Client* c : _clients) {
        ((EventSourceClient*)c->_friend)->sendEvent(ev.c_str());
    }
}

/*****************************************/
// EventSourceClient
/*****************************************/

EventSourceClient::EventSourceClient(Client* client)
    : Client(client->server(), client->socket()), _lastId(0)
{
}

EventSourceClient::~EventSourceClient() {}

void EventSourceClient::send(
    const char* message,
    const char* event,
    uint32_t id,
    uint32_t reconnect
)
{
    std::string ev = generateEventMessage(message, event, id, reconnect);
    sendEvent(ev.c_str());
}

void EventSourceClient::sendEvent(const char* event)
{
    int result;
    do {
        result = httpd_socket_send(
            this->server(), this->socket(), event, strlen(event), 0
        );
    } while (result == HTTPD_SOCK_ERR_TIMEOUT);

    // if (result < 0)
    // error log here
}

/*****************************************/
// EventSourceResponse
/*****************************************/

EventSourceResponse::EventSourceResponse(Request* request) : Response(request)
{
}

esp_err_t EventSourceResponse::send()
{

    // build our main header
    std::string out{"HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/event-stream\r\n"
                    "Cache-Control: no-cache\r\n"
                    "Connection: keep-alive\r\n"};

    // get our global headers out of the way first
    for (HTTPHeader header : DefaultHeaders::Instance().getHeaders()) {
        out += std::string{header.field};
        out += header.value;
        out += "\r\n";
    }

    // separator
    out += "\r\n";

    int result;
    do {
        result = httpd_send(_request->request(), out.c_str(), out.length());
    } while (result == HTTPD_SOCK_ERR_TIMEOUT);

    if (result < 0)
        ESP_LOGE(
            lib_tag, "EventSource send failed with %s", esp_err_to_name(result)
        );

    if (result > 0)
        return ESP_OK;
    else
        return ESP_ERR_HTTPD_RESP_SEND;
}

/*****************************************/
// Event Message Generator
/*****************************************/

std::string generateEventMessage(
    const char* message,
    const char* event,
    uint32_t id,
    uint32_t reconnect
)
{
    std::string ev = "";

    if (reconnect) {
        ev += "retry: ";
        ev += std::to_string(reconnect);
        ev += "\r\n";
    }

    if (id) {
        ev += "id: ";
        ev += std::to_string(id);
        ev += "\r\n";
    }

    if (event != NULL) {
        ev += "event: ";
        ev += std::string(event);
        ev += "\r\n";
    }

    if (message != NULL) {
        ev += "data: ";
        ev += std::string(message);
        ev += "\r\n";
    }
    ev += "\r\n";

    return ev;
}
} // namespace lumenaries::http
