#pragma once

#include "esp_http_server.h"

#include <string>

namespace lumenaries::http {

/** \class Request http/Request lumenaries/http/Request
 *  \brief An object representing a client HTTP request.
 *
 * You should only ever interact with this object in the context of an HTTP
 * request callback function.
 *
 * \note
 * - If you need to save any information from this object
 * for future use, you should do so before calling Response::send() or
 * Response::write() on the corresponsing Response object.
 *
 * Usage example:
 * \code
 * namespace http = lumenaries::http;
 * auto callback = [](http::Request const& request, http::Response& response) {
 *     auto accept_header = request.get_header_value("Accept");
 *     if (accept_header == "text/html") {
 *         response.set_content_type("text/html");
 *         response.send("<html>"
 *                       "  <head>"
 *                       "  </head>"
 *                       "  <body>"
 *                       "      <h1>Welcome</h1>"
 *                       "  </body>"
 *                       "</html>");
 *     }
 * };
 * \endcode
 *
 * \sa Response
 */
class Request {
public:
    /** \brief Creates a request object.
     * \param request The `esp_http_server` request being responded to.
     */
    explicit Request(httpd_req_t* request);

    /** \brief Get the value of a field in the request header
     *
     * \note
     *  - %Request header values are purged after a call to Response::send() or
     * an empty call to Response::write(). If you need to save any headers, do
     * so before calling those send functions.
     *
     * \param field The header field for which you want the value.
     *
     * \return
     *  - Value of field if it's found in the request header.
     *  - Empty if field is not found in the request header or the Request
     * object is invalid.
     */
    [[nodiscard]] std::string get_header_value(std::string const& field) const;

    /** \brief Get the full query string from the URL.
     *
     * The query string is whatever comes after a "?" character in the URL.
     *
     * \note
     *  - The components of the query string are not URLdecoded. You must apply
     *    the appropriate decoding algorithm depending on the value of the
     *    "Content-Type" header.
     *
     * Usage example:
     * \code
     * // url: http://example.com/article?id=62
     * response.get_query_string(); // returns "id=62"
     * \endcode
     *
     * \return
     *  - Value of query string if it exists.
     *  - Empty if query string does not exist or the Request object is invalid.
     */
    [[nodiscard]] std::string get_query_string() const;

    /** \brief Extract a specific value from the query string.
     *
     * This will work for strings of the type "param1=val1&param2=val2".
     *
     * \note
     *  - The components of the query string are not URLdecoded. You must apply
     *    the appropriate decoding algorithm depending on the value of the
     *    "Content-Type" header.
     *
     * \param key The key for which you want the value.
     *
     * \return
     *  - Value of parameter if it exists in the query string.
     *  - Empty if query string does not exist or the Request object is invalid.
     */
    [[nodiscard]] std::string get_parameter_value(std::string const& key) const;

    /** \brief Get the value of a cookie field from the Cookie headers.
     *
     * \note
     *  - You may need to adjust CONFIG_LUM_HTTP_MAX_COOKIE_SIZE in menuconfig
     * if you find that your cookie value is being truncated.
     *
     * \param name The name of the cookie for which you want the value.
     *
     * \return
     *  - Value of cookie if it exists in Cookie headers (possibly
     *    truncated).
     *  - Empty if the cookie does not exist or the Request object is invalid.
     *
     *
     */
    [[nodiscard]] std::string get_cookie_val(std::string const& name) const;

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
