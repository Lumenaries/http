#pragma once

#include "lumenaries/http/client.hpp"
#include "lumenaries/http/core.hpp"
#include "lumenaries/http/response.hpp"
#include "lumenaries/http/server.hpp"
#include "lumenaries/http/web_parameter.hpp"

#include "esp_http_server.h"

#include <map>
#include <string>

namespace lumenaries::http {

typedef std::map<std::string, std::string> SessionData;

enum Disposition { NONE, INLINE, ATTACHMENT, FORM_DATA };

struct ContentDisposition {
    Disposition disposition;
    std::string filename;
    std::string name;
};

class Request {
public:
    Request(Server* server, httpd_req_t* req);
    virtual ~Request();

    void* _tempObject;

    Server* server();
    httpd_req_t* request();
    virtual Client* client();

    bool isMultipart();
    esp_err_t loadBody();

    const std::string header(const char* name);
    bool hasHeader(const char* name);

    static void freeSession(void* ctx);
    bool hasSessionKey(const std::string& key);
    const std::string getSessionKey(const std::string& key);
    void setSessionKey(const std::string& key, const std::string& value);

    bool hasCookie(const char* key);
    const std::string getCookie(const char* key);

    http_method
    method(); // returns the HTTP method used as enum value (eg. HTTP_GET)
    const std::string
    methodStr(); // returns the HTTP method used as a string (eg. "GET")
    const std::string
    path(); // returns the request path (eg /page?foo=bar returns "/page")
    const std::string& uri(); // returns the full request uri (eg /page?foo=bar)
    const std::string& query(
    ); // returns the request query data (eg /page?foo=bar returns "foo=bar")
    const std::string
    host(); // returns the requested host (request to http://psychic.local/foo
            // will return "psychic.local")
    const std::string contentType(); // returns the Content-Type header value
    size_t contentLength();          // returns the Content-Length header value
    const std::string& body();       // returns the body of the request
    const ContentDisposition getContentDisposition();

    const std::string& queryString()
    {
        return query();
    } // compatability function.  same as query()
    const std::string& url()
    {
        return uri();
    } // compatability function.  same as uri()

    void loadParams();
    WebParameter* addParam(WebParameter* param);
    WebParameter* addParam(
        const std::string& name,
        const std::string& value,
        bool decode = true
    );
    bool hasParam(const char* key);
    WebParameter* getParam(const char* name);

    const std::string getFilename();

    bool authenticate(const char* username, const char* password);
    esp_err_t requestAuthentication(
        HTTPAuthMethod mode,
        const char* realm,
        const char* authFailMsg
    );

    esp_err_t redirect(const char* url);
    esp_err_t reply(int code);
    esp_err_t reply(const char* content);
    esp_err_t reply(int code, const char* contentType, const char* content);

protected:
    Server* _server;
    httpd_req_t* _req;
    SessionData* _session;
    Client* _client;

    http_method _method;
    std::string _uri;
    std::string _query;
    std::string _body;

    std::list<WebParameter*> _params;

    void _addParams(const std::string& params);
    void _parseGETParams();
    void _parsePOSTParams();

    const std::string _extractParam(
        const std::string& authReq,
        const std::string& param,
        const char delimit
    );
    const std::string _getRandomHexString();

    friend Server;
};

} // namespace lumenaries::http
