set(SRC_DIR src)
set(INCLUDE_DIR include)

idf_component_register(SRCS
                           "${SRC_DIR}/lumenaries/http/request.cpp"
                           "${SRC_DIR}/lumenaries/http/response.cpp"
                       INCLUDE_DIRS "${INCLUDE_DIR}"
                       REQUIRES esp_http_server)
