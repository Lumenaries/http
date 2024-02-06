#pragma once

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
 * You should only ever interact with this object in the context of an HTTP
 * request callback function.
 *
 * \note
 * - If you need to save any information from the corresponding Request object
 * for future use, you should do so before you calling Response::send() or
 * Response::write();
 *
 * Usage example:
 * \code
 * namespace http = lumenaries::http;
 * auto callback = [](http::Request const& request, http::Response& response) {
 *     response.set_header("Cache", "no-cache");
 *     response.send("<h1>Hello world!</h1>");
 * };
 * \endcode
 *
 * \sa Request
 */
class Response {
public:
    /** \brief Creates a response object.
     * \param request The `esp_http_server` request being responded to.
     */
    explicit Response(httpd_req_t* request);

    /** \brief Set the status of the response.
     *
     * This sets the status of the HTTP response to the specified value. The
     * status text will be filled in based on the status code. default value is
     * "200".
     *
     * \note
     *  - This function only sets the status. The status
     *    isn't sent out until one of the send functions are executed.
     *
     * \param status The response status to be set.
     *
     * \return
     *  - ESP_OK   : On success
     *  - ESP_ERR_HTTPD_INVALID_REQ : Invalid request pointer
     *
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

    /** \brief Set a field in the response header.
     *
     * This sets any additional fields that need to be sent in the response
     * header.
     *
     * \note
     *  - This function only sets header fields. The header fields
     *    aren't sent out until one of the send functions are executed.
     *  - The maximum allowed number of additional headers is limited to
     *        value of `max_resp_headers` in config structure.
     *
     *  \todo Allow configuration of `max_resp_headers` in config structure
     *
     * \param field The header field to be set.
     * \param value The value of the field to be set.
     *
     * \return
     *  - ESP_OK : On successfully appending new header
     *  - ESP_ERR_HTTPD_RESP_HDR    : Total additional headers exceed max
     *    allowed
     *  - ESP_ERR_HTTPD_INVALID_REQ : Invalid request pointer

     */
    esp_err_t set_header(std::string const& field, std::string const& value);

    /** \brief Send a complete HTTP response.
     *
     * This will send an HTTP response to the request.
     * This assumes that you have the entire response ready in a single
     * buffer. If you wish to send a response in incremental chunks use
     * Response::write() instead.
     *
     * If no status code and no content type were set, by default this
     * will send status code: 200 and content type: "text/html".
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
     *
     *  \sa Response::write()
     */
    esp_err_t send(std::string const& buffer);

    /**
     * \brief Send a chunked HTTP response.
     *
     * This will send an HTTP response to the request using chunked-encoding.
     * This API will use chunked-encoding and send the response in the form of
     * chunks. If you have the entire response contained in a single buffer,
     * please use Response::send() instead.
     *
     * If no status code and no content type were set, by default this
     * will send status code: 200 and content type: "text/html".
     *
     * \note
     *  - Once this function is called, all request headers are purged, so
     *    request headers need be copied if they are required later.
     *  - When you are finished sending all your chunks, you must call
     *    this function without any arguments to signify the end of the message.
     *  - Once this function is called with no arguments, the request has been
     *    responded to and no additional data can be sent.
     *
     * \param buffer The response data to be sent
     *
     * \return
     *  - ESP_OK : On successfully sending the response packet chunk
     *  - ESP_ERR_HTTPD_RESP_HDR    : Essential headers are too large for
     * internal buffer
     *  - ESP_ERR_HTTPD_RESP_SEND   : Error in raw send
     *  - ESP_ERR_HTTPD_INVALID_REQ : Invalid request pointer
     *
     *
     * Usage example:
     * \code
     * ...
     * // `response` has type `Response` and is declared elsewhere
     * response.write("<h1>Initial response</h1>");
     * response.write("<h2>Next response</h2>");
     * response.write("<h3>Last response</h3>");
     * response.write(); // Signify the end of our response.
     * ...
     * \endcode
     *
     *  \sa Response::send()
     */
    esp_err_t write(std::string const& buffer = "");

    /** \brief Get a pointer to the underlying request struct.
     *
     * Should only be used if you need to interact directly with the underlying
     * C API.
     */
    [[nodiscard]] httpd_req_t* get_idf_request() const;

private:
    httpd_req_t* idf_request_;
};

} // namespace lumenaries::http
