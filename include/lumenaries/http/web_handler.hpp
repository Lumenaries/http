#pragma once

#include "lumenaries/http/handler.hpp"

namespace lumenaries::http {

/*
 * HANDLER :: Can be attached to any endpoint or as a generic request handler.
 */

class WebHandler : public Handler {
protected:
    HttpRequestCallback _requestCallback;
    ClientCallback _onOpen;
    ClientCallback _onClose;

public:
    WebHandler();
    ~WebHandler();

    virtual bool canHandle(Request* request) override;
    virtual esp_err_t handleRequest(Request* request) override;
    WebHandler* onRequest(HttpRequestCallback fn);

    virtual void openCallback(Client* client);
    virtual void closeCallback(Client* client);

    WebHandler* onOpen(ClientCallback fn);
    WebHandler* onClose(ClientCallback fn);
};

} // namespace lumenaries::http
