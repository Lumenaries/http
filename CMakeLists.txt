set(SRC_DIR src)
set(INCLUDE_DIR include)

idf_component_register(SRCS
                           "${SRC_DIR}/server.cpp"
                           "${SRC_DIR}/request.cpp"
                           "${SRC_DIR}/response.cpp"
                           "${SRC_DIR}/endpoint.cpp"
                       INCLUDE_DIRS "${INCLUDE_DIR}"
                       REQUIRES esp_http_server)
