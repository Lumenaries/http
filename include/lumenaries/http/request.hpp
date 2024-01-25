#pragma once

#include "lumenaries/http/response.hpp"

#include "esp_http_server.h"

namespace lumenaries::http {

class Response;

class Request {
public:
private:
    httpd_req_t* request_;

    friend Response;
};

} // namespace lumenaries::http
