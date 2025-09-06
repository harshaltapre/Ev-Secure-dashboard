idf_component_register(
    SRCS "sdlogger.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_vfs_fat esp_http_client json
)
