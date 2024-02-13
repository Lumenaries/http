#include "lumenaries/http/web_handler.hpp"

#include "esp_log.h"

namespace lumenaries::http {

WebHandler::WebHandler()
    : Handler(), _requestCallback(NULL), _onOpen(NULL), _onClose(NULL)
{
}
WebHandler::~WebHandler() {}

bool WebHandler::canHandle(Request* request)
{
    return true;
}

esp_err_t WebHandler::handleRequest(Request* request)
{
    // lookup our client
    Client* client = checkForNewClient(request->client());
    if (client->isNew)
        openCallback(client);

    /* Request body cannot be larger than a limit */
    if (request->contentLength() > request->server()->maxRequestBodySize) {
        ESP_LOGE(
            PH_TAG,
            "Request body too large : %d bytes",
            request->contentLength()
        );

        /* Respond with 400 Bad Request */
        char error[60];
        sprintf(
            error,
            "Request body must be less than %lu bytes!",
            request->server()->maxRequestBodySize
        );
        httpd_resp_send_err(request->request(), HTTPD_400_BAD_REQUEST, error);

        /* Return failure to close underlying connection else the incoming file
         * content will keep the socket busy */
        return ESP_FAIL;
    }

    // get our body loaded up.
    esp_err_t err = request->loadBody();
    if (err != ESP_OK)
        return err;

    // load our params in.
    request->loadParams();

    // okay, pass on to our callback.
    if (this->_requestCallback != NULL)
        err = this->_requestCallback(request);

    return err;
}

WebHandler* WebHandler::onRequest(HttpRequestCallback fn)
{
    _requestCallback = fn;
    return this;
}

void WebHandler::openCallback(Client* client)
{
    if (_onOpen != NULL)
        _onOpen(client);
}

void WebHandler::closeCallback(Client* client)
{
    if (_onClose != NULL)
        _onClose(getClient(client));
}

WebHandler* WebHandler::onOpen(ClientCallback fn)
{
    _onOpen = fn;
    return this;
}

WebHandler* WebHandler::onClose(ClientCallback fn)
{
    _onClose = fn;
    return this;
}

} // namespace lumenaries::http
