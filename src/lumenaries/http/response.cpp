#include "lumenaries/http/response.hpp"

#include <format>

namespace lumenaries::http {
namespace {

std::string status_to_str(int status)
{
    switch (status) {
    case 101:
        return "101 Switching Protocol";
    case 200:
        return "200 OK";
    case 201:
        return "201 Created";
    case 202:
        return "202 Accepted";
    case 204:
        return "204 No Content";
    case 206:
        return "206 Partial Content";
    case 300:
        return "300 Multiple Choices";
    case 301:
        return "301 Moved Permanently";
    case 302:
        return "302 Found";
    case 303:
        return "303 See Other";
    case 304:
        return "304 Not Modified";
    case 307:
        return "307 Moved Temporarily";
    case 400:
        return "400 Bad Request";
    case 401:
        return "401 Unauthorized";
    case 403:
        return "403 Forbidden";
    case 404:
        return "404 Not Found";
    case 413:
        return "413 Request Entity too Large";
    case 416:
        return "416 Requested Range Not Satisfiable";
    case 501:
        return "501 Not Implemented";
    case 502:
        return "502 Bad Gateway";
    case 503:
        return "503 Service Unavailable";
    case 505:
        return "505 HTTP Version Not Supported";
    case 500:
        return "500 Internal Server Error";
    default:
        return std::format("{} Unknown", status);
    }
}

} // namespace

Response::Response(Request& request) : request_{request.request_} {}

esp_err_t Response::set_status(int status)
{
    return httpd_resp_set_status(request_, status_to_str(status).c_str());
}

esp_err_t Response::set_content_type(std::string const& value)
{
    return httpd_resp_set_type(request_, value.c_str());
}

esp_err_t
Response::set_header(std::string const& field, std::string const& value)
{
    return httpd_resp_set_hdr(request_, field.c_str(), value.c_str());
}

esp_err_t Response::send(std::string const& buffer)
{
    return httpd_resp_send(request_, buffer.c_str(), buffer.length());
}

esp_err_t Response::write(std::string const& buffer)
{
    return httpd_resp_send_chunk(request_, buffer.c_str(), buffer.length());
}

} // namespace lumenaries::http
