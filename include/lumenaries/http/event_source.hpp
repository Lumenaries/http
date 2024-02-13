#pragma once

#include "lumenaries/http/client.hpp"
#include "lumenaries/http/core.hpp"
#include "lumenaries/http/handler.hpp"
#include "lumenaries/http/response.hpp"

namespace lumenaries::http {

class EventSource;
class EventSourceResponse;
class EventSourceClient;
class Response;

typedef std::function<void(EventSourceClient* client)>
    EventSourceClientCallback;

class EventSourceClient : public Client {
public:
    EventSourceClient(Client* client);
    ~EventSourceClient();

    uint32_t lastId() const
    {
        return _lastId;
    }
    void send(
        const char* message,
        const char* event = NULL,
        uint32_t id = 0,
        uint32_t reconnect = 0
    );
    void sendEvent(const char* event);

protected:
    uint32_t _lastId;

    friend EventSource;
};

class EventSource : public Handler {
public:
    EventSource();
    ~EventSource();

    EventSourceClient* getClient(int socket) override;
    EventSourceClient* getClient(Client* client) override;
    void addClient(Client* client) override;
    void removeClient(Client* client) override;
    void openCallback(Client* client) override;
    void closeCallback(Client* client) override;

    EventSource* onOpen(EventSourceClientCallback fn);
    EventSource* onClose(EventSourceClientCallback fn);

    esp_err_t handleRequest(Request* request) override final;

    void send(
        const char* message,
        const char* event = NULL,
        uint32_t id = 0,
        uint32_t reconnect = 0
    );

private:
    EventSourceClientCallback _onOpen;
    EventSourceClientCallback _onClose;
};

class EventSourceResponse : public Response {
public:
    EventSourceResponse(Request* request);
    esp_err_t send() override;
};

std::string generateEventMessage(
    const char* message,
    const char* event,
    uint32_t id,
    uint32_t reconnect
);
} // namespace lumenaries::http
