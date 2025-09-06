idf_component_register(
    SRCS "ocpp_proxy.c"
    INCLUDE_DIRS "."
    REQUIRES esp_websocket_client json
)
