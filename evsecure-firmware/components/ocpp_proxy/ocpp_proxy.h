#ifndef OCPP_PROXY_H
#define OCPP_PROXY_H

#include "esp_err.h"
#include "evsecure_config.h"
#include "esp_websocket_client.h"

// OCPP message structure
typedef struct {
    ocpp_msg_type_t type;
    uint64_t timestamp;
    char session_id[32];
    char payload[256];
    bool malformed;
    bool out_of_sequence;
} ocpp_message_t;

// OCPP metrics
typedef struct {
    float message_rate;
    uint32_t remote_stop_count;
    uint32_t malformed_count;
    uint32_t out_of_sequence_count;
    uint64_t last_message_time;
} ocpp_metrics_t;

// Function declarations
esp_err_t ocpp_proxy_init(void);
esp_err_t ocpp_proxy_connect(void);
esp_err_t ocpp_proxy_disconnect(void);
esp_err_t ocpp_proxy_receive_message(ocpp_message_t *message);
esp_err_t ocpp_proxy_get_metrics(ocpp_metrics_t *metrics);
esp_err_t ocpp_proxy_deinit(void);

// WebSocket event handler
esp_err_t ocpp_websocket_event_handler(esp_websocket_client_handle_t client, 
                                      esp_websocket_event_data_t *data);

#endif // OCPP_PROXY_H
