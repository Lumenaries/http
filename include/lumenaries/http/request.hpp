#pragma once

#include "esp_http_server.h"

#include <string>

// clang-format off

/*
// done

// not done

int httpd_req_to_sockfd(httpd_req_t *r);
int httpd_req_recv(httpd_req_t *r, char *buf, size_t buf_len);
esp_err_t httpd_req_get_hdr_value_str(httpd_req_t *r, const char *field, char *val, size_t val_size);
esp_err_t httpd_req_get_url_query_str(httpd_req_t *r, char *buf, size_t buf_len);
esp_err_t httpd_query_key_value(const char *qry, const char *key, char *val, size_t val_size);
esp_err_t httpd_req_get_cookie_val(httpd_req_t *req, const char *cookie_name, char *val, size_t *val_size);

// not doing

// used when calling httpd_req_get_hdr_value_str()
size_t httpd_req_get_hdr_value_len(httpd_req_t *r, const char *field);

// used when calling httpd_req_get_url_query_str()
size_t httpd_req_get_url_query_len(httpd_req_t *r);

*/

// clang-format on

namespace lumenaries::http {

class Request {
public:
    explicit Request(httpd_req_t* request);

    // what to do about esp_err_t?
    [[nodiscard]] std::string get_header_value(std::string const& field) const;

    [[nodiscard]] std::string get_query_string() const;

    [[nodiscard]] std::string get_parameter_value(std::string const& name
    ) const;

    [[nodiscard]] std::string get_cookie_val(std::string const& name) const;

    [[nodiscard]] httpd_req_t* get_idf_request() const;

private:
    httpd_req_t* idf_request_;
};

} // namespace lumenaries::http
