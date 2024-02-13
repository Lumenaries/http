set(SRC_DIR src)
set(INCLUDE_DIR include)

idf_component_register(SRCS
                           "${SRC_DIR}/lumenaries/http/async_worker.cpp"
                           "${SRC_DIR}/lumenaries/http/client.cpp"
                           "${SRC_DIR}/lumenaries/http/endpoint.cpp"
                           "${SRC_DIR}/lumenaries/http/event_source.cpp"
                           "${SRC_DIR}/lumenaries/http/handler.cpp"
                           "${SRC_DIR}/lumenaries/http/http_status.cpp"
                           "${SRC_DIR}/lumenaries/http/http_status.cpp"
                           "${SRC_DIR}/lumenaries/http/request.cpp"
                           "${SRC_DIR}/lumenaries/http/response.cpp"
                           "${SRC_DIR}/lumenaries/http/server.cpp"
                           "${SRC_DIR}/lumenaries/http/web_handler.cpp"
                       INCLUDE_DIRS "${INCLUDE_DIR}"
                       REQUIRES
                           esp_http_server
                           log)
