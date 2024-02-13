#pragma once

#include "esp_http_server.h"
#include "esp_random.h"

#include <functional>
#include <list>
#include <string>

namespace lumenaries::http {

auto constexpr lib_tag = "lumenaries/http";

#ifndef MAX_COOKIE_SIZE
#define MAX_COOKIE_SIZE 512
#endif

#ifndef FILE_CHUNK_SIZE
#define FILE_CHUNK_SIZE 8 * 1024
#endif

#ifndef STREAM_CHUNK_SIZE
#define STREAM_CHUNK_SIZE 1024
#endif

#ifndef MAX_UPLOAD_SIZE
#define MAX_UPLOAD_SIZE (2048 * 1024) // 2MB
#endif

#ifndef MAX_REQUEST_BODY_SIZE
#define MAX_REQUEST_BODY_SIZE (16 * 1024) // 16K
#endif

enum HTTPAuthMethod { BASIC_AUTH, DIGEST_AUTH };

std::string urlDecode(const char* encoded);

class Server;
class Request;
class Client;

// filter function definition
typedef std::function<bool(Request* request)>
    RequestFilterFunction;

// client connect callback
typedef std::function<void(Client* client)> ClientCallback;

// callback definitions
typedef std::function<esp_err_t(Request* request)>
    HttpRequestCallback;

struct HTTPHeader {
    char* field;
    char* value;
};

class DefaultHeaders {
    std::list<HTTPHeader> _headers;

public:
    DefaultHeaders() {}

    void addHeader(const std::string& field, const std::string& value)
    {
        addHeader(field.c_str(), value.c_str());
    }

    void addHeader(const char* field, const char* value)
    {
        HTTPHeader header;

        // these are just going to stick around forever.
        header.field = (char*)malloc(strlen(field) + 1);
        header.value = (char*)malloc(strlen(value) + 1);

        strlcpy(header.field, field, strlen(field) + 1);
        strlcpy(header.value, value, strlen(value) + 1);

        _headers.push_back(header);
    }

    const std::list<HTTPHeader>& getHeaders()
    {
        return _headers;
    }

    // delete the copy constructor, singleton class
    DefaultHeaders(DefaultHeaders const&) = delete;
    DefaultHeaders& operator=(DefaultHeaders const&) = delete;

    // single static class interface
    static DefaultHeaders& Instance()
    {
        static DefaultHeaders instance;
        return instance;
    }
};

} // namespace lumenaries::http
