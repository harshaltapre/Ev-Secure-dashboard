#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_websocket_client.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_ota_ops.h"
#include "esp_http_server.h"
#include "cJSON.h"

#include "evsecure_config.h"
#include "ina226.h"
#include "tflite_micro.h"
#include "sdlogger.h"
#include "ocpp_proxy.h"
#include "ui.h"
#include "security.h"

static const char *TAG = "EVSECURE_MAIN";

// Global variables and queues
static QueueHandle_t feature_queue;
static QueueHandle_t alert_queue;
static QueueHandle_t ocpp_queue;
static SemaphoreHandle_t safety_mutex;
static EventGroupHandle_t system_events;
static bool system_initialized = false;
static safety_state_t current_safety_state = SAFETY_STATE_IDLE;
static float current_anomaly_score = 0.0f;
static char current_session_id[32] = {0};
static uint32_t session_counter = 0;

// Event group bits
#define WIFI_CONNECTED_BIT BIT0
#define SENSOR_READY_BIT BIT1
#define ML_MODEL_READY_BIT BIT2
#define STORAGE_READY_BIT BIT3
#define OCPP_CONNECTED_BIT BIT4

// Function prototypes
static void wifi_init_sta(void);
static void power_sense_task(void *pvParameters);
static void ocpp_monitor_task(void *pvParameters);
static void ml_anomaly_task(void *pvParameters);
static void safety_control_task(void *pvParameters);
static void comms_task(void *pvParameters);
static void logging_task(void *pvParameters);
static void ui_task(void *pvParameters);
static esp_err_t system_init(void);
static void generate_session_id(void);
static void update_safety_state(safety_state_t new_state);

// WiFi event handler
static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "WiFi disconnected, attempting to reconnect...");
        esp_wifi_connect();
        xEventGroupClearBits(system_events, WIFI_CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(system_events, WIFI_CONNECTED_BIT);
    }
}

// WiFi initialization
static void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

// Power sensing task
static void power_sense_task(void *pvParameters)
{
    feature_vector_t feature;
    TickType_t last_wake_time = xTaskGetTickCount();
    
    ESP_LOGI(TAG, "Power sense task started");
    
    while (1) {
        // Wait for sensor to be ready
        xEventGroupWaitBits(system_events, SENSOR_READY_BIT, false, true, portMAX_DELAY);
        
        // Read sensor data
        if (ina226_read_measurements(&feature.v_rms, &feature.i_rms, &feature.p_kw, 
                                    &feature.pf, &feature.thd_v, &feature.thd_i) == ESP_OK) {
            
            // Calculate derivatives (simplified)
            static float prev_v = 0, prev_i = 0;
            feature.dvdt = (feature.v_rms - prev_v) / (FEATURE_SAMPLE_INTERVAL_MS / 1000.0f);
            feature.didt = (feature.i_rms - prev_i) / (FEATURE_SAMPLE_INTERVAL_MS / 1000.0f);
            prev_v = feature.v_rms;
            prev_i = feature.i_rms;
            
            // Read temperature
            // TODO: Implement DS18B20 temperature reading
            feature.temp_c = 25.0f; // Placeholder
            
            // Check tamper status
            feature.tamper = !gpio_get_level(TAMPER_SWITCH_PIN);
            
            // Check firmware integrity
            feature.fw_ok = security_check_firmware_integrity();
            
            // Get OCPP metrics from OCPP task
            feature.ocpp_rate = 5.0f; // Placeholder
            feature.remote_stop_cnt = 0; // Placeholder
            feature.malformed = 0; // Placeholder
            feature.out_of_seq = 0; // Placeholder
            
            // Add timestamp
            feature_vector_t *feature_ptr = malloc(sizeof(feature_vector_t));
            memcpy(feature_ptr, &feature, sizeof(feature_vector_t));
            
            // Send to ML task
            if (xQueueSend(feature_queue, &feature_ptr, 0) != pdTRUE) {
                ESP_LOGW(TAG, "Feature queue full, dropping sample");
                free(feature_ptr);
            }
        }
        
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(FEATURE_SAMPLE_INTERVAL_MS));
    }
}

// OCPP monitoring task
static void ocpp_monitor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "OCPP monitor task started");
    
    while (1) {
        // Wait for WiFi connection
        xEventGroupWaitBits(system_events, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
        
        // Connect to OCPP proxy
        if (ocpp_proxy_connect() == ESP_OK) {
            xEventGroupSetBits(system_events, OCPP_CONNECTED_BIT);
            
            // Monitor OCPP messages
            ocpp_message_t msg;
            while (ocpp_proxy_receive_message(&msg) == ESP_OK) {
                // Process OCPP message
                switch (msg.type) {
                    case OCPP_MSG_START_TRANSACTION:
                        generate_session_id();
                        update_safety_state(SAFETY_STATE_HANDSHAKE);
                        break;
                    case OCPP_MSG_REMOTE_STOP_TRANSACTION:
                        // Increment remote stop counter
                        break;
                    case OCPP_MSG_UPDATE_FIRMWARE:
                        // Handle firmware update
                        break;
                    default:
                        break;
                }
                
                // Send to ML task for analysis
                if (xQueueSend(ocpp_queue, &msg, 0) != pdTRUE) {
                    ESP_LOGW(TAG, "OCPP queue full");
                }
            }
        }
        
        // Reconnection delay
        vTaskDelay(pdMS_TO_TICKS(WEBSOCKET_RECONNECT_INTERVAL_MS));
    }
}

// ML anomaly detection task
static void ml_anomaly_task(void *pvParameters)
{
    feature_vector_t *feature_ptr;
    ocpp_message_t ocpp_msg;
    float ml_score = 0.0f;
    float rule_score = 0.0f;
    
    ESP_LOGI(TAG, "ML anomaly task started");
    
    while (1) {
        // Wait for ML model to be ready
        xEventGroupWaitBits(system_events, ML_MODEL_READY_BIT, false, true, portMAX_DELAY);
        
        // Process features
        if (xQueueReceive(feature_queue, &feature_ptr, pdMS_TO_TICKS(100)) == pdTRUE) {
            
            // Run ML inference
            if (tflite_micro_inference(feature_ptr, &ml_score) == ESP_OK) {
                
                // Calculate rule-based score
                rule_score = 0.0f;
                
                // Remote stop burst detection
                if (feature_ptr->remote_stop_cnt > REMOTE_STOP_BURST_THRESHOLD) {
                    rule_score += 0.6f;
                }
                
                // Malformed message detection
                if (feature_ptr->malformed > MALFORMED_BURST_THRESHOLD) {
                    rule_score += 0.4f;
                }
                
                // THD and OCPP rate correlation
                if (feature_ptr->thd_i > BASELINE_THD_I * THD_I_MULTIPLIER_THRESHOLD &&
                    feature_ptr->ocpp_rate < BASELINE_OCPP_RATE * OCPP_RATE_THRESHOLD) {
                    rule_score += 0.5f;
                }
                
                // Tamper or firmware integrity
                if (feature_ptr->tamper || !feature_ptr->fw_ok) {
                    rule_score = 1.0f; // Critical
                }
                
                // Calculate combined score
                current_anomaly_score = RULE_SCORE_WEIGHT * rule_score + ML_SCORE_WEIGHT * ml_score;
                
                // Create alert if threshold exceeded
                if (current_anomaly_score >= WARNING_THRESHOLD) {
                    alert_t alert = {
                        .level = (current_anomaly_score >= CRITICAL_THRESHOLD) ? ALERT_LEVEL_CRITICAL : ALERT_LEVEL_WARNING,
                        .score = current_anomaly_score,
                        .timestamp = esp_timer_get_time() / 1000000,
                        .session_id = {0}
                    };
                    strcpy(alert.session_id, current_session_id);
                    
                    if (xQueueSend(alert_queue, &alert, 0) != pdTRUE) {
                        ESP_LOGW(TAG, "Alert queue full");
                    }
                }
            }
            
            free(feature_ptr);
        }
        
        // Process OCPP messages
        if (xQueueReceive(ocpp_queue, &ocpp_msg, 0) == pdTRUE) {
            // Update OCPP metrics in feature calculation
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Safety control task
static void safety_control_task(void *pvParameters)
{
    alert_t alert;
    
    ESP_LOGI(TAG, "Safety control task started");
    
    while (1) {
        if (xQueueReceive(alert_queue, &alert, portMAX_DELAY) == pdTRUE) {
            
            xSemaphoreTake(safety_mutex, portMAX_DELAY);
            
            switch (alert.level) {
                case ALERT_LEVEL_WARNING:
                    if (current_safety_state == SAFETY_STATE_CHARGING) {
                        update_safety_state(SAFETY_STATE_SUSPICIOUS);
                        // Reduce current to 70%
                        ESP_LOGI(TAG, "Warning: Reducing current to 70%%");
                    }
                    break;
                    
                case ALERT_LEVEL_CRITICAL:
                    update_safety_state(SAFETY_STATE_LOCKDOWN);
                    // Open contactor
                    gpio_set_level(CONTACTOR_CONTROL_PIN, CONTACTOR_ACTIVE_LOW ? 0 : 1);
                    ESP_LOGE(TAG, "CRITICAL: Contactor opened, system in LOCKDOWN");
                    break;
                    
                default:
                    break;
            }
            
            xSemaphoreGive(safety_mutex);
        }
    }
}

// Communications task
static void comms_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Communications task started");
    
    while (1) {
        // Wait for WiFi connection
        xEventGroupWaitBits(system_events, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
        
        // Upload logs to dashboard
        if (sdlogger_upload_pending_logs() == ESP_OK) {
            ESP_LOGI(TAG, "Logs uploaded successfully");
        }
        
        vTaskDelay(pdMS_TO_TICKS(LOG_UPLOAD_INTERVAL_MS));
    }
}

// Logging task
static void logging_task(void *pvParameters)
{
    feature_vector_t *feature_ptr;
    
    ESP_LOGI(TAG, "Logging task started");
    
    while (1) {
        // Wait for storage to be ready
        xEventGroupWaitBits(system_events, STORAGE_READY_BIT, false, true, portMAX_DELAY);
        
        // Log features
        if (xQueueReceive(feature_queue, &feature_ptr, pdMS_TO_TICKS(1000)) == pdTRUE) {
            sdlogger_log_feature(feature_ptr);
            free(feature_ptr);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// UI task
static void ui_task(void *pvParameters)
{
    ESP_LOGI(TAG, "UI task started");
    
    while (1) {
        // Update display
        ui_update_display(current_safety_state, current_anomaly_score, 
                         current_session_id, feature_vector_t *current_features);
        
        // Handle button presses
        ui_handle_buttons();
        
        vTaskDelay(pdMS_TO_TICKS(UI_UPDATE_INTERVAL_MS));
    }
}

// System initialization
static esp_err_t system_init(void)
{
    esp_err_t ret = ESP_OK;
    
    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Create queues and semaphores
    feature_queue = xQueueCreate(10, sizeof(feature_vector_t*));
    alert_queue = xQueueCreate(5, sizeof(alert_t));
    ocpp_queue = xQueueCreate(10, sizeof(ocpp_message_t));
    safety_mutex = xSemaphoreCreateMutex();
    system_events = xEventGroupCreate();
    
    if (!feature_queue || !alert_queue || !ocpp_queue || !safety_mutex || !system_events) {
        ESP_LOGE(TAG, "Failed to create FreeRTOS objects");
        return ESP_ERR_NO_MEM;
    }
    
    // Initialize WiFi
    wifi_init_sta();
    
    // Initialize components
    if (ina226_init() == ESP_OK) {
        xEventGroupSetBits(system_events, SENSOR_READY_BIT);
    }
    
    if (tflite_micro_init() == ESP_OK) {
        xEventGroupSetBits(system_events, ML_MODEL_READY_BIT);
    }
    
    if (sdlogger_init() == ESP_OK) {
        xEventGroupSetBits(system_events, STORAGE_READY_BIT);
    }
    
    if (ui_init() == ESP_OK) {
        ESP_LOGI(TAG, "UI initialized");
    }
    
    if (security_init() == ESP_OK) {
        ESP_LOGI(TAG, "Security initialized");
    }
    
    system_initialized = true;
    return ESP_OK;
}

// Generate session ID
static void generate_session_id(void)
{
    session_counter++;
    snprintf(current_session_id, sizeof(current_session_id), "sess_%08x_%08x", 
             (unsigned int)esp_timer_get_time(), session_counter);
}

// Update safety state
static void update_safety_state(safety_state_t new_state)
{
    if (current_safety_state != new_state) {
        ESP_LOGI(TAG, "Safety state change: %d -> %d", current_safety_state, new_state);
        current_safety_state = new_state;
    }
}

// Main function
void app_main(void)
{
    ESP_LOGI(TAG, "EVsecure-device starting...");
    ESP_LOGI(TAG, "Device ID: %s", DEVICE_ID);
    ESP_LOGI(TAG, "Version: %s", DEVICE_VERSION);
    
    // Initialize system
    if (system_init() != ESP_OK) {
        ESP_LOGE(TAG, "System initialization failed");
        esp_restart();
    }
    
    // Create tasks
    xTaskCreate(power_sense_task, "power_sense", POWER_SENSE_TASK_STACK_SIZE, 
                NULL, POWER_SENSE_TASK_PRIORITY, NULL);
    xTaskCreate(ocpp_monitor_task, "ocpp_monitor", OCPP_MONITOR_TASK_STACK_SIZE, 
                NULL, OCPP_MONITOR_TASK_PRIORITY, NULL);
    xTaskCreate(ml_anomaly_task, "ml_anomaly", ML_ANOMALY_TASK_STACK_SIZE, 
                NULL, ML_ANOMALY_TASK_PRIORITY, NULL);
    xTaskCreate(safety_control_task, "safety_control", SAFETY_CONTROL_TASK_STACK_SIZE, 
                NULL, SAFETY_CONTROL_TASK_PRIORITY, NULL);
    xTaskCreate(comms_task, "comms", COMMS_TASK_STACK_SIZE, 
                NULL, COMMS_TASK_PRIORITY, NULL);
    xTaskCreate(logging_task, "logging", LOGGING_TASK_STACK_SIZE, 
                NULL, LOGGING_TASK_PRIORITY, NULL);
    xTaskCreate(ui_task, "ui", UI_TASK_STACK_SIZE, 
                NULL, UI_TASK_PRIORITY, NULL);
    
    ESP_LOGI(TAG, "EVsecure-device started successfully");
}
