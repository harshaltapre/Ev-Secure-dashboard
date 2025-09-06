idf_component_register(
    SRCS "security.c"
    INCLUDE_DIRS "."
    REQUIRES driver nvs_flash mbedtls esp_ota_ops
)
