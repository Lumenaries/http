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

/** \class Response http/Response lumenaries/http/Response
 *  \brief An object representing a server response to an HTTP request.
 *
 * You will typically only interact with an http::Response object when it is
 * passed as a parameter in a request callback.
 *
 * Usage example:
 * \code
 * auto handler = [](auto const& request, auto& response) {
 *     response.set_header("Cache", "no-cache");
 *     response.send("<h1>Hello world!</h1>");
 * };
 * \endcode
 *
 * \sa http::Request
 */
class Response {
public:
    /** \brief Creates a response object.
     * \param request The request being responded to.
     */
    explicit Response(httpd_req_t* request);

    /** \brief Set the response status.
     * \param status The response status to be set.
     */
    esp_err_t set_status(int status);

    /** \brief Set the content type of the response.
     *
     * This sets the Content-Type field of the response header. The
     * default value is "text/html".
     *
     * \note
     *  - This function only sets the content type. The type
     *    isn't sent out until one of the send functions are executed.
     *
     * \param value The value the field should be set to.
     *
     * \return
     *  - ESP_OK   : On success
     *  - ESP_ERR_HTTPD_INVALID_REQ : Invalid request pointer
     *
     *  \sa Response::set_header()
     */

    esp_err_t set_content_type(std::string const& value);

    /** \brief Set a field in the response header
     * \param field The header field to set.
     * \param value The value of the field to set.
     */
    esp_err_t set_header(std::string const& field, std::string const& value);

    /** \brief Send a complete HTTP response.
     *
     * This will send an HTTP response to the request.
     * This assumes that you have the entire response ready in a single
     * buffer. If you wish to send response in incremental chunks use
     * Response::write() instead.
     *
     * If no status code and content-type were set, by default this
     * will send 200 OK status code and content type as text/html.
     *
     * \note
     *  - Once this function is called, the request has been responded to and
     *    no additional data can be sent.
     *  - Once this function is called, all request headers are purged, so
     *    request headers need be copied if they are required later.
     *
     * \param buffer The response data to be sent
     *
     * \return
     *  - ESP_OK : On successfully sending the response packet
     *  - ESP_ERR_HTTPD_RESP_HDR    : Essential headers are too large for
     * internal buffer
     *  - ESP_ERR_HTTPD_RESP_SEND   : Error in raw send
     *  - ESP_ERR_HTTPD_INVALID_REQ : Invalid request
     */
    esp_err_t send(std::string const& buffer);

    /**
     * \brief Send an HTTP chunk
     *
     * This will send an HTTP response to the request using chunked-encoding.
     * This API will use chunked-encoding and send the response in the form of
     * chunks. If you have the entire response contained in a single buffer,
     * please use Response::send() instead.
     *
     * If no status code and content-type were set, by default this
     * will send 200 OK status code and content type as text/html.
     *
     * \note
     *  - When you are finished sending all your chunks, you must call
     *    this function without any arguments to signify the end of the message.
     *  - Once this function is called, all request headers are purged, so
     *    request headers need be copied if they are required later.
     *
     * \param buffer The response data to be sent
     *
     * \return
     *  - ESP_OK : On successfully sending the response packet chunk
     *  - ESP_ERR_HTTPD_RESP_HDR    : Essential headers are too large for
     * internal buffer
     *  - ESP_ERR_HTTPD_RESP_SEND   : Error in raw send
     *  - ESP_ERR_HTTPD_INVALID_REQ : Invalid request pointer
     */
    esp_err_t write(std::string const& buffer = "");

    /** \brief Get a pointer to the underlying request object.
     *
     * Should be used only if you need to interact directly with the underlying
     * C API.
     */
    [[nodiscard]] httpd_req_t* get_idf_request() const;

private:
    httpd_req_t* idf_request_;
};

} // namespace lumenaries::http
