#pragma once

namespace lumenaries::http {

bool http_informational(int code);
bool http_success(int code);
bool http_redirection(int code);
bool http_client_error(int code);
bool http_server_error(int code);
bool http_failure(int code);
const char* http_status_group(int code);
const char* http_status_reason(int code);

} // namespace lumenaries::http
