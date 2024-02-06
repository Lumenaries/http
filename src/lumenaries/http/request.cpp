#include "lumenaries/http/request.hpp"

#include <array>
#include <memory>

namespace lumenaries::http {
namespace {

constexpr size_t max_cookie_size = CONFIG_LUM_HTTP_MAX_COOKIE_SIZE;

} // namespace

Request::Request(httpd_req_t* request) : idf_request_{request} {}

std::string Request::get_header_value(std::string const& field) const
{
    auto buf_len = httpd_req_get_hdr_value_len(idf_request_, field.c_str()) + 1;

    if (buf_len < 2) {
        // No header found, return empty string
        return {};
    }

    char buf[buf_len];

    httpd_req_get_hdr_value_str(idf_request_, field.c_str(), buf, buf_len);
    return std::string{buf};
}

std::string Request::get_query_string() const
{
    auto buf_len = httpd_req_get_url_query_len(idf_request_) + 1;

    if (buf_len < 2) {
        // No query string, return empty string
        return {};
    }

    char buf[buf_len];

    httpd_req_get_url_query_str(idf_request_, buf, buf_len);
    return std::string{buf};
}

std::string Request::get_parameter_value(std::string const& name) const
{
    auto query_string = get_query_string();

    // Include space for null-terminator
    auto buf_len = httpd_req_get_url_query_len(idf_request_) + 1;

    if (buf_len < 2) {
        return {};
    }

    char buf[buf_len];

    httpd_query_key_value(query_string.c_str(), name.c_str(), buf, buf_len);
    return std::string{buf};
}

std::string Request::get_cookie_val(std::string const& name) const
{
    auto cookie_size = max_cookie_size;
    char cookie[max_cookie_size];

    httpd_req_get_cookie_val(idf_request_, name.c_str(), cookie, &cookie_size);

    return std::string{cookie};
}

httpd_req_t* Request::get_idf_request() const
{
    return idf_request_;
}

} // namespace lumenaries::http
