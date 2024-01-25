#pragma once

#include "lumenaries/http/request.hpp"

#include "esp_http_server.h"

#include <string>

// clang-format off
/*
// left to do

// done

esp_err_t httpd_resp_set_status(httpd_req_t *r, const char *status);
esp_err_t httpd_resp_set_type(httpd_req_t *r, const char *type);
esp_err_t httpd_resp_set_hdr(httpd_req_t *r, const char *field, const char *value);
esp_err_t httpd_resp_send(httpd_req_t *r, const char *buf, ssize_t buf_len);
esp_err_t httpd_resp_send_chunk(httpd_req_t *r, const char *buf, ssize_t buf_len);

// not doing

// send raw HTTP packet
int httpd_send(httpd_req_t *r, const char *buf, size_t buf_len);

// seems unnecessary since we already have set_status();
esp_err_t httpd_resp_send_err(httpd_req_t *req, httpd_err_code_t error, const char *msg);

// these just fill in the buf_len variable for httpd_resp_send() and httpd_resp_send_chunk()
static inline esp_err_t httpd_resp_sendstr(httpd_req_t *r, const char *str);
static inline esp_err_t httpd_resp_sendstr_chunk(httpd_req_t *r, const char *str);
*/
// clang-format on

namespace lumenaries::http {

class Request;

class Response {
public:
    explicit Response(Request& request);

    esp_err_t set_status(int status);

    esp_err_t set_content_type(std::string const& value);

    esp_err_t set_header(std::string const& field, std::string const& value);

    esp_err_t send(std::string const& buffer);

    esp_err_t write(std::string const& buffer);

private:
    httpd_req_t* request_;
};

} // namespace lumenaries::http
