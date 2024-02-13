#include "lumenaries/http/request.hpp"

#include "lumenaries/http/http_status.hpp"
#include "lumenaries/http/server.hpp"

#include "esp_log.h"

#include <string>

// #include "MD5Builder.h" // used for md5 decrypting of authorization headers
// #include <libb64/cencode.h> // used for base64 encoding

namespace lumenaries::http {

Request::Request(Server* server, httpd_req_t* req)
    : _tempObject(NULL), _server(server), _req(req), _method(HTTP_GET),
      _query(""), _body("")
{
    // load up our client.
    this->_client = server->getClient(req);

    // handle our session data
    if (req->sess_ctx != NULL)
        this->_session = (SessionData*)req->sess_ctx;
    else {
        this->_session = new SessionData();
        req->sess_ctx = this->_session;
    }

    // callback for freeing the session later
    req->free_ctx = this->freeSession;

    // load up some data
    this->_uri = std::string(this->_req->uri);
}

Request::~Request()
{
    // temorary user object
    if (_tempObject != NULL)
        free(_tempObject);

    // our web parameters
    for (auto* param : _params)
        delete (param);
    _params.clear();
}

void Request::freeSession(void* ctx)
{
    if (ctx != NULL) {
        SessionData* session = (SessionData*)ctx;
        delete session;
    }
}

Server* Request::server()
{
    return _server;
}

httpd_req_t* Request::request()
{
    return _req;
}

Client* Request::client()
{
    return _client;
}

const std::string Request::getFilename()
{
    // parse the content-disposition header
    if (this->hasHeader("Content-Disposition")) {
        ContentDisposition cd = this->getContentDisposition();
        if (cd.filename != "")
            return cd.filename;
    }

    // fall back to passed in query string
    WebParameter* param = getParam("_filename");
    if (param != NULL)
        return param->name();

    // fall back to parsing it from url (useful for wildcard uploads)
    std::string uri = this->uri();
    int filenameStart = uri.find_last_of('/') + 1;
    std::string filename = uri.substr(filenameStart);
    if (filename != "")
        return filename;

    // finally, unknown.
    ESP_LOGE(PH_TAG, "Did not get a valid filename from the upload.");
    return "unknown.txt";
}

const ContentDisposition Request::getContentDisposition()
{
    ContentDisposition cd;
    std::string header = this->header("Content-Disposition");
    int start;
    int end;

    if (header.find("form-data") == 0)
        cd.disposition = FORM_DATA;
    else if (header.find("attachment") == 0)
        cd.disposition = ATTACHMENT;
    else if (header.find("inline") == 0)
        cd.disposition = INLINE;
    else
        cd.disposition = NONE;

    start = header.find("filename=");
    if (start) {
        end = header.find('"', start + 10);
        cd.filename = header.substr(start + 10, end - 1);
    }

    start = header.find("name=");
    if (start) {
        end = header.find('"', start + 6);
        cd.name = header.substr(start + 6, end - 1);
    }

    return cd;
}

esp_err_t Request::loadBody()
{
    esp_err_t err = ESP_OK;

    this->_body = std::string();

    // Get header value string length and allocate memory for length + 1, extra
    // byte for null termination
    size_t remaining = this->_req->content_len;
    char* buf = (char*)malloc(remaining + 1);

    // while loop for retries
    while (remaining > 0) {
        // read our data from the socket
        int received = httpd_req_recv(this->_req, buf, this->_req->content_len);

        // Retry if timeout occurred
        if (received == HTTPD_SOCK_ERR_TIMEOUT)
            continue;
        // bail if we got an error
        else if (received == HTTPD_SOCK_ERR_FAIL) {
            err = ESP_FAIL;
            break;
        }

        // keep track of our
        remaining -= received;
    }

    // null terminate and make our string
    buf[this->_req->content_len] = '\0';
    this->_body = std::string(buf);

    // keep track of that pesky memory
    free(buf);

    return err;
}

http_method Request::method()
{
    return (http_method)this->_req->method;
}

const std::string Request::methodStr()
{
    return std::string(http_method_str((http_method)this->_req->method));
}

const std::string Request::path()
{
    int index = _uri.find("?");
    if (index == -1)
        return _uri;
    else
        return _uri.substr(0, index);
}

const std::string& Request::uri()
{
    return this->_uri;
}

const std::string& Request::query()
{
    return this->_query;
}

// no way to get list of headers yet....
// int Request::headers()
// {
// }

const std::string Request::header(const char* name)
{
    size_t header_len = httpd_req_get_hdr_value_len(this->_req, name);

    // if we've got one, allocated it and load it
    if (header_len) {
        char header[header_len + 1];
        httpd_req_get_hdr_value_str(this->_req, name, header, sizeof(header));
        return std::string(header);
    } else
        return "";
}

bool Request::hasHeader(const char* name)
{
    return httpd_req_get_hdr_value_len(this->_req, name) > 0;
}

const std::string Request::host()
{
    return this->header("Host");
}

const std::string Request::contentType()
{
    return header("Content-Type");
}

size_t Request::contentLength()
{
    return this->_req->content_len;
}

const std::string& Request::body()
{
    return this->_body;
}

bool Request::isMultipart()
{
    const std::string& type = this->contentType();

    return (this->contentType().find("multipart/form-data") >= 0);
}

esp_err_t Request::redirect(const char* url)
{
    Response response(this);
    response.setCode(301);
    response.addHeader("Location", url);

    return response.send();
}

bool Request::hasCookie(const char* key)
{
    char cookie[MAX_COOKIE_SIZE];
    size_t cookieSize = MAX_COOKIE_SIZE;
    esp_err_t err =
        httpd_req_get_cookie_val(this->_req, key, cookie, &cookieSize);

    // did we get anything?
    if (err == ESP_OK)
        return true;
    else if (err == ESP_ERR_HTTPD_RESULT_TRUNC)
        ESP_LOGE(PH_TAG, "cookie too large (%d bytes).\n", cookieSize);

    return false;
}

const std::string Request::getCookie(const char* key)
{
    char cookie[MAX_COOKIE_SIZE];
    size_t cookieSize = MAX_COOKIE_SIZE;
    esp_err_t err =
        httpd_req_get_cookie_val(this->_req, key, cookie, &cookieSize);

    // did we get anything?
    if (err == ESP_OK)
        return std::string(cookie);
    else
        return "";
}

void Request::loadParams()
{
    // did we get a query string?
    size_t query_len = httpd_req_get_url_query_len(_req);
    if (query_len) {
        char query[query_len + 1];
        httpd_req_get_url_query_str(_req, query, sizeof(query));
        _query += query;

        // parse them.
        _addParams(_query);
    }

    // did we get form data as body?
    if (this->method() == HTTP_POST &&
        this->contentType() == "application/x-www-form-urlencoded") {
        _addParams(_body);
    }
}

void Request::_addParams(const std::string& params)
{
    size_t start = 0;
    while (start < params.length()) {
        int end = params.find('&', start);
        if (end < 0)
            end = params.length();
        int equal = params.find('=', start);
        if (equal < 0 || equal > end)
            equal = end;
        std::string name = params.substr(start, equal);
        std::string value =
            equal + 1 < end ? params.substr(equal + 1, end) : std::string();
        addParam(name, value);
        start = end + 1;
    }
}

WebParameter* Request::addParam(
    const std::string& name,
    const std::string& value,
    bool decode
)
{
    if (decode)
        return addParam(
            new WebParameter(urlDecode(name.c_str()), urlDecode(value.c_str()))
        );
    else
        return addParam(new WebParameter(name, value));
}

WebParameter* Request::addParam(WebParameter* param)
{
    _params.push_back(param);
    return param;
}

bool Request::hasParam(const char* key)
{
    return getParam(key) != NULL;
}

WebParameter* Request::getParam(const char* key)
{
    for (auto* param : _params)
        if (param->name() == key)
            return param;

    return NULL;
}

bool Request::hasSessionKey(const std::string& key)
{
    return this->_session->find(key) != this->_session->end();
}

const std::string Request::getSessionKey(const std::string& key)
{
    auto it = this->_session->find(key);
    if (it != this->_session->end())
        return it->second;
    else
        return "";
}

void Request::setSessionKey(const std::string& key, const std::string& value)
{
    this->_session->insert(std::pair<std::string, std::string>(key, value));
}

static const std::string md5str(const std::string& in)
{
    /*
    MD5Builder md5 = MD5Builder();
    md5.begin();
    md5.add(in);
    md5.calculate();
    return md5.toString();
    */
    return {};
}

bool Request::authenticate(const char* username, const char* password)
{
    /*
    if (hasHeader("Authorization")) {
        std::string authReq = header("Authorization");
        if (authReq.startsWith("Basic")) {
            authReq = authReq.substr(6);
            authReq.trim();
            char toencodeLen = strlen(username) + strlen(password) + 1;
            char* toencode = new char[toencodeLen + 1];
            if (toencode == NULL) {
                authReq = "";
                return false;
            }
            char* encoded =
                new char[base64_encode_expected_len(toencodeLen) + 1];
            if (encoded == NULL) {
                authReq = "";
                delete[] toencode;
                return false;
            }
            sprintf(toencode, "%s:%s", username, password);
            if (base64_encode_chars(toencode, toencodeLen, encoded) > 0 &&
                authReq.equalsConstantTime(encoded)) {
                authReq = "";
                delete[] toencode;
                delete[] encoded;
                return true;
            }
            delete[] toencode;
            delete[] encoded;
        } else if (authReq.startsWith(F("Digest"))) {
            authReq = authReq.substr(7);
            std::string _username = _extractParam(authReq, F("username=\""),
    '\"'); if (!_username.length() || _username != std::string(username)) {
                authReq = "";
                return false;
            }
            // extracting required parameters for RFC 2069 simpler Digest
            std::string _realm = _extractParam(authReq, F("realm=\""), '\"');
            std::string _nonce = _extractParam(authReq, F("nonce=\""), '\"');
            std::string _uri = _extractParam(authReq, F("uri=\""), '\"');
            std::string _resp = _extractParam(authReq, F("response=\""), '\"');
            std::string _opaque = _extractParam(authReq, F("opaque=\""), '\"');

            if ((!_realm.length()) || (!_nonce.length()) || (!_uri.length()) ||
                (!_resp.length()) || (!_opaque.length())) {
                authReq = "";
                return false;
            }
            if ((_opaque != this->getSessionKey("opaque")) ||
                (_nonce != this->getSessionKey("nonce")) ||
                (_realm != this->getSessionKey("realm"))) {
                // DUMP(_opaque);
                // DUMP(this->getSessionKey("opaque"));
                // DUMP(_nonce);
                // DUMP(this->getSessionKey("nonce"));
                // DUMP(_realm);
                // DUMP(this->getSessionKey("realm"));
                authReq = "";
                return false;
            }
            // parameters for the RFC 2617 newer Digest
            std::string _nc, _cnonce;
            if (authReq.find("qop=auth") != -1 ||
                authReq.find("qop=\"auth\"") != -1) {
                _nc = _extractParam(authReq, F("nc="), ',');
                _cnonce = _extractParam(authReq, F("cnonce=\""), '\"');
            }
            std::string _H1 = md5str(
                std::string(username) + ':' + _realm + ':' +
    std::string(password)
            );
            ESP_LOGD(PH_TAG, "Hash of user:realm:pass=%s", _H1);
            std::string _H2 = "";
            if (_method == HTTP_GET) {
                _H2 = md5str(String(F("GET:")) + _uri);
            } else if (_method == HTTP_POST) {
                _H2 = md5str(String(F("POST:")) + _uri);
            } else if (_method == HTTP_PUT) {
                _H2 = md5str(String(F("PUT:")) + _uri);
            } else if (_method == HTTP_DELETE) {
                _H2 = md5str(String(F("DELETE:")) + _uri);
            } else {
                _H2 = md5str(String(F("GET:")) + _uri);
            }
            ESP_LOGD(PH_TAG, "Hash of GET:uri=%s", _H2);
            std::string _responsecheck = "";
            if (authReq.find("qop=auth") != -1 ||
                authReq.find("qop=\"auth\"") != -1) {
                _responsecheck = md5str(
                    _H1 + ':' + _nonce + ':' + _nc + ':' + _cnonce +
                    F(":auth:") + _H2
                );
            } else {
                _responsecheck = md5str(_H1 + ':' + _nonce + ':' + _H2);
            }
            ESP_LOGD(PH_TAG, "The Proper response=%s", _responsecheck);
            if (_resp == _responsecheck) {
                authReq = "";
                return true;
            }
        }
        authReq = "";
    }
    return false;
    */
    return true;
}

const std::string Request::_extractParam(
    const std::string& authReq,
    const std::string& param,
    const char delimit
)
{
    int _begin = authReq.find(param);
    if (_begin == -1)
        return "";
    return authReq.substr(
        _begin + param.length(), authReq.find(delimit, _begin + param.length())
    );
}

const std::string Request::_getRandomHexString()
{
    char buffer[33]; // buffer to hold 32 Hex Digit + /0
    int i;
    for (i = 0; i < 4; i++) {
        sprintf(buffer + (i * 8), "%08lx", esp_random());
    }
    return std::string(buffer);
}

esp_err_t Request::requestAuthentication(
    HTTPAuthMethod mode,
    const char* realm,
    const char* authFailMsg
)
{
    // what is thy realm, sire?
    if (!strcmp(realm, ""))
        this->setSessionKey("realm", "Login Required");
    else
        this->setSessionKey("realm", realm);

    Response response(this);
    std::string authStr;

    // what kind of auth?
    if (mode == BASIC_AUTH) {
        authStr = "Basic realm=\"" + this->getSessionKey("realm") + "\"";
        response.addHeader("WWW-Authenticate", authStr.c_str());
    } else {
        // only make new ones if we havent sent them yet
        if (this->getSessionKey("nonce").empty())
            this->setSessionKey("nonce", _getRandomHexString());
        if (this->getSessionKey("opaque").empty())
            this->setSessionKey("opaque", _getRandomHexString());

        authStr = "Digest realm=\"" + this->getSessionKey("realm") +
                  "\", qop=\"auth\", nonce=\"" + this->getSessionKey("nonce") +
                  "\", opaque=\"" + this->getSessionKey("opaque") + "\"";
        response.addHeader("WWW-Authenticate", authStr.c_str());
    }

    // DUMP(authStr);

    response.setCode(401);
    response.setContentType("text/html");
    response.setContent(authStr.c_str());
    return response.send();
}

esp_err_t Request::reply(int code)
{
    Response response(this);

    response.setCode(code);
    response.setContentType("text/plain");
    response.setContent(http_status_reason(code));

    return response.send();
}

esp_err_t Request::reply(const char* content)
{
    Response response(this);

    response.setCode(200);
    response.setContentType("text/html");
    response.setContent(content);

    return response.send();
}

esp_err_t Request::reply(int code, const char* contentType, const char* content)
{
    Response response(this);

    response.setCode(code);
    response.setContentType(contentType);
    response.setContent(content);

    return response.send();
}

} // namespace lumenaries::http
