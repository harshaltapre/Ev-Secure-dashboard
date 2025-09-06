idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_wifi esp_event nvs_flash esp_netif esp_http_client esp_websocket_client esp_timer esp_sleep esp_ota_ops esp_http_server json ina226 tflite_micro sdlogger ocpp_proxy ui security
)
