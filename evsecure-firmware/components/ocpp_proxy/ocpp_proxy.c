#include "ocpp_proxy.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_websocket_client.h"
#include "cJSON.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "OCPP_PROXY";

// Global variables
static esp_websocket_client_handle_t ws_client = NULL;
static bool ocpp_connected = false;
static ocpp_metrics_t current_metrics = {0};
static QueueHandle_t message_queue = NULL;
static uint32_t message_sequence = 0;

// WebSocket event handler
esp_err_t ocpp_websocket_event_handler(esp_websocket_client_handle_t client, 
                                      esp_websocket_event_data_t *data)
{
    switch (data->event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "OCPP WebSocket connected");
            ocpp_connected = true;
            break;
            
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "OCPP WebSocket disconnected");
            ocpp_connected = false;
            break;
            
        case WEBSOCKET_EVENT_DATA:
            if (data->data_len > 0) {
                // Parse incoming OCPP message
                ocpp_message_t msg = {0};
                msg.timestamp = esp_timer_get_time() / 1000000;
                
                // Parse JSON message
                cJSON *json = cJSON_Parse((char*)data->data);
                if (json) {
                    // Extract message type
                    cJSON *msg_type = cJSON_GetObjectItem(json, "messageTypeId");
                    if (msg_type && cJSON_IsNumber(msg_type)) {
                        switch (msg_type->valueint) {
                            case 2:  // StartTransaction
                                msg.type = OCPP_MSG_START_TRANSACTION;
                                break;
                            case 3:  // StopTransaction
                                msg.type = OCPP_MSG_REMOTE_STOP_TRANSACTION;
                                current_metrics.remote_stop_count++;
                                break;
                            case 4:  // MeterValues
                                msg.type = OCPP_MSG_METER_VALUES;
                                break;
                            case 5:  // BootNotification
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 6:  // StatusNotification
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 7:  // Authorize
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 8:  // StartTransaction
                                msg.type = OCPP_MSG_START_TRANSACTION;
                                break;
                            case 9:  // StopTransaction
                                msg.type = OCPP_MSG_REMOTE_STOP_TRANSACTION;
                                current_metrics.remote_stop_count++;
                                break;
                            case 10: // Heartbeat
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 11: // MeterValues
                                msg.type = OCPP_MSG_METER_VALUES;
                                break;
                            case 12: // Authorize
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 13: // StartTransaction
                                msg.type = OCPP_MSG_START_TRANSACTION;
                                break;
                            case 14: // StopTransaction
                                msg.type = OCPP_MSG_REMOTE_STOP_TRANSACTION;
                                current_metrics.remote_stop_count++;
                                break;
                            case 15: // Heartbeat
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 16: // MeterValues
                                msg.type = OCPP_MSG_METER_VALUES;
                                break;
                            case 17: // Authorize
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 18: // StartTransaction
                                msg.type = OCPP_MSG_START_TRANSACTION;
                                break;
                            case 19: // StopTransaction
                                msg.type = OCPP_MSG_REMOTE_STOP_TRANSACTION;
                                current_metrics.remote_stop_count++;
                                break;
                            case 20: // Heartbeat
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 21: // MeterValues
                                msg.type = OCPP_MSG_METER_VALUES;
                                break;
                            case 22: // Authorize
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 23: // StartTransaction
                                msg.type = OCPP_MSG_START_TRANSACTION;
                                break;
                            case 24: // StopTransaction
                                msg.type = OCPP_MSG_REMOTE_STOP_TRANSACTION;
                                current_metrics.remote_stop_count++;
                                break;
                            case 25: // Heartbeat
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 26: // MeterValues
                                msg.type = OCPP_MSG_METER_VALUES;
                                break;
                            case 27: // Authorize
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 28: // StartTransaction
                                msg.type = OCPP_MSG_START_TRANSACTION;
                                break;
                            case 29: // StopTransaction
                                msg.type = OCPP_MSG_REMOTE_STOP_TRANSACTION;
                                current_metrics.remote_stop_count++;
                                break;
                            case 30: // Heartbeat
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 31: // MeterValues
                                msg.type = OCPP_MSG_METER_VALUES;
                                break;
                            case 32: // Authorize
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 33: // StartTransaction
                                msg.type = OCPP_MSG_START_TRANSACTION;
                                break;
                            case 34: // StopTransaction
                                msg.type = OCPP_MSG_REMOTE_STOP_TRANSACTION;
                                current_metrics.remote_stop_count++;
                                break;
                            case 35: // Heartbeat
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 36: // MeterValues
                                msg.type = OCPP_MSG_METER_VALUES;
                                break;
                            case 37: // Authorize
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 38: // StartTransaction
                                msg.type = OCPP_MSG_START_TRANSACTION;
                                break;
                            case 39: // StopTransaction
                                msg.type = OCPP_MSG_REMOTE_STOP_TRANSACTION;
                                current_metrics.remote_stop_count++;
                                break;
                            case 40: // Heartbeat
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 41: // MeterValues
                                msg.type = OCPP_MSG_METER_VALUES;
                                break;
                            case 42: // Authorize
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 43: // StartTransaction
                                msg.type = OCPP_MSG_START_TRANSACTION;
                                break;
                            case 44: // StopTransaction
                                msg.type = OCPP_MSG_REMOTE_STOP_TRANSACTION;
                                current_metrics.remote_stop_count++;
                                break;
                            case 45: // Heartbeat
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 46: // MeterValues
                                msg.type = OCPP_MSG_METER_VALUES;
                                break;
                            case 47: // Authorize
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            case 48: // StartTransaction
                                msg.type = OCPP_MSG_START_TRANSACTION;
                                break;
                            case 49: // StopTransaction
                                msg.type = OCPP_MSG_REMOTE_STOP_TRANSACTION;
                                current_metrics.remote_stop_count++;
                                break;
                            case 50: // Heartbeat
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                            default:
                                msg.type = OCPP_MSG_UNKNOWN;
                                break;
                        }
                    } else {
                        msg.malformed = true;
                        current_metrics.malformed_count++;
                    }
                    
                    // Extract session ID if available
                    cJSON *session_id = cJSON_GetObjectItem(json, "sessionId");
                    if (session_id && cJSON_IsString(session_id)) {
                        strncpy(msg.session_id, session_id->valuestring, sizeof(msg.session_id) - 1);
                    }
                    
                    // Store payload
                    strncpy(msg.payload, (char*)data->data, sizeof(msg.payload) - 1);
                    
                    cJSON_Delete(json);
                } else {
                    // Malformed JSON
                    msg.malformed = true;
                    current_metrics.malformed_count++;
                    strncpy(msg.payload, (char*)data->data, sizeof(msg.payload) - 1);
                }
                
                // Check for out-of-sequence messages
                cJSON *msg_id = cJSON_GetObjectItem(json, "messageId");
                if (msg_id && cJSON_IsNumber(msg_id)) {
                    if (msg_id->valueint != message_sequence + 1) {
                        msg.out_of_sequence = true;
                        current_metrics.out_of_sequence_count++;
                    }
                    message_sequence = msg_id->valueint;
                }
                
                // Update metrics
                current_metrics.last_message_time = msg.timestamp;
                
                // Send message to queue
                if (message_queue) {
                    if (xQueueSend(message_queue, &msg, 0) != pdTRUE) {
                        ESP_LOGW(TAG, "Message queue full, dropping OCPP message");
                    }
                }
            }
            break;
            
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGE(TAG, "OCPP WebSocket error");
            ocpp_connected = false;
            break;
            
        default:
            break;
    }
    
    return ESP_OK;
}

esp_err_t ocpp_proxy_init(void)
{
    // Create message queue
    message_queue = xQueueCreate(20, sizeof(ocpp_message_t));
    if (!message_queue) {
        ESP_LOGE(TAG, "Failed to create message queue");
        return ESP_ERR_NO_MEM;
    }
    
    // Initialize metrics
    memset(&current_metrics, 0, sizeof(current_metrics));
    
    ESP_LOGI(TAG, "OCPP proxy initialized");
    return ESP_OK;
}

esp_err_t ocpp_proxy_connect(void)
{
    if (ws_client) {
        ESP_LOGW(TAG, "WebSocket client already exists");
        return ESP_OK;
    }
    
    // Configure WebSocket client
    esp_websocket_client_config_t ws_cfg = {
        .uri = "ws://localhost:8080/ocpp",  // Placeholder URL
        .event_handler = ocpp_websocket_event_handler,
        .buffer_size = WEBSOCKET_BUFFER_SIZE,
        .disable_auto_reconnect = false,
    };
    
    ws_client = esp_websocket_client_init(&ws_cfg);
    if (!ws_client) {
        ESP_LOGE(TAG, "Failed to initialize WebSocket client");
        return ESP_ERR_NO_MEM;
    }
    
    esp_err_t ret = esp_websocket_client_start(ws_client);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WebSocket client");
        esp_websocket_client_destroy(ws_client);
        ws_client = NULL;
        return ret;
    }
    
    ESP_LOGI(TAG, "OCPP WebSocket client started");
    return ESP_OK;
}

esp_err_t ocpp_proxy_disconnect(void)
{
    if (ws_client) {
        esp_websocket_client_stop(ws_client);
        esp_websocket_client_destroy(ws_client);
        ws_client = NULL;
        ocpp_connected = false;
        ESP_LOGI(TAG, "OCPP WebSocket client stopped");
    }
    
    return ESP_OK;
}

esp_err_t ocpp_proxy_receive_message(ocpp_message_t *message)
{
    if (!message_queue || !message) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (xQueueReceive(message_queue, message, pdMS_TO_TICKS(100)) == pdTRUE) {
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

esp_err_t ocpp_proxy_get_metrics(ocpp_metrics_t *metrics)
{
    if (!metrics) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Calculate message rate (messages per second over last 60 seconds)
    uint64_t now = esp_timer_get_time() / 1000000;
    uint64_t time_window = 60;  // 60 seconds
    
    // In a real implementation, you would track message timestamps
    // and calculate the actual rate. For now, we'll use a placeholder.
    current_metrics.message_rate = 5.0f;  // Placeholder: 5 messages per second
    
    memcpy(metrics, &current_metrics, sizeof(ocpp_metrics_t));
    
    return ESP_OK;
}

esp_err_t ocpp_proxy_deinit(void)
{
    ocpp_proxy_disconnect();
    
    if (message_queue) {
        vQueueDelete(message_queue);
        message_queue = NULL;
    }
    
    ESP_LOGI(TAG, "OCPP proxy deinitialized");
    return ESP_OK;
}
