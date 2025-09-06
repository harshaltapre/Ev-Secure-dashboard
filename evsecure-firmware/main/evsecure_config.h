#ifndef EVSECURE_CONFIG_H
#define EVSECURE_CONFIG_H

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"

// Device Configuration
#define DEVICE_ID "evsec-esp32-001"
#define DEVICE_VERSION "1.0.0"
#define FIRMWARE_HASH "placeholder_hash_here"

// WiFi Configuration
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
#define WIFI_MAXIMUM_RETRY 5

// API Configuration
#define DASHBOARD_API_URL "https://your-dashboard-api.com"
#define API_KEY "your_api_key_here"  // Replace with actual API key
#define API_TIMEOUT_MS 10000

// Pin Definitions - ESP32-S3 DevKit
// INA226/INA228 Energy Meter (I2C)
#define INA226_I2C_SDA_PIN GPIO_NUM_21
#define INA226_I2C_SCL_PIN GPIO_NUM_22
#define INA226_I2C_FREQ_HZ 400000
#define INA226_I2C_ADDR 0x40

// INA240 Current Amplifier (I2C)
#define INA240_I2C_SDA_PIN GPIO_NUM_21
#define INA240_I2C_SCL_PIN GPIO_NUM_22
#define INA240_I2C_ADDR 0x41

// MicroSD Card (SPI)
#define SD_SPI_MOSI_PIN GPIO_NUM_35
#define SD_SPI_MISO_PIN GPIO_NUM_37
#define SD_SPI_SCLK_PIN GPIO_NUM_36
#define SD_SPI_CS_PIN GPIO_NUM_34

// TFT Display (SPI)
#define TFT_SPI_MOSI_PIN GPIO_NUM_11
#define TFT_SPI_MISO_PIN GPIO_NUM_13
#define TFT_SPI_SCLK_PIN GPIO_NUM_12
#define TFT_SPI_CS_PIN GPIO_NUM_10
#define TFT_DC_PIN GPIO_NUM_14
#define TFT_RST_PIN GPIO_NUM_15

// UI Buttons
#define UI_ACK_BUTTON_PIN GPIO_NUM_16
#define UI_BYPASS_BUTTON_PIN GPIO_NUM_17

// Contactor Control
#define CONTACTOR_CONTROL_PIN GPIO_NUM_18
#define CONTACTOR_FEEDBACK_PIN GPIO_NUM_19
#define CONTACTOR_ACTIVE_LOW true

// Tamper Detection
#define TAMPER_SWITCH_PIN GPIO_NUM_20

// Temperature Sensor (DS18B20 - One Wire)
#define TEMP_SENSOR_PIN GPIO_NUM_23

// Optional ATECC608A Secure Element (I2C)
#define ATECC608A_I2C_SDA_PIN GPIO_NUM_21
#define ATECC608A_I2C_SCL_PIN GPIO_NUM_22
#define ATECC608A_I2C_ADDR 0x60

// Task Priorities
#define POWER_SENSE_TASK_PRIORITY 5
#define OCPP_MONITOR_TASK_PRIORITY 4
#define ML_ANOMALY_TASK_PRIORITY 6
#define SAFETY_CONTROL_TASK_PRIORITY 7
#define COMMS_TASK_PRIORITY 3
#define LOGGING_TASK_PRIORITY 2
#define UI_TASK_PRIORITY 1

// Task Stack Sizes
#define POWER_SENSE_TASK_STACK_SIZE 4096
#define OCPP_MONITOR_TASK_STACK_SIZE 4096
#define ML_ANOMALY_TASK_STACK_SIZE 8192
#define SAFETY_CONTROL_TASK_STACK_SIZE 4096
#define COMMS_TASK_STACK_SIZE 8192
#define LOGGING_TASK_STACK_SIZE 4096
#define UI_TASK_STACK_SIZE 4096

// Timing Configuration
#define FEATURE_SAMPLE_INTERVAL_MS 250
#define FEATURE_AGGREGATION_30S_COUNT 120
#define FEATURE_AGGREGATION_5MIN_COUNT 1200
#define LOG_UPLOAD_INTERVAL_MS 60000
#define UI_UPDATE_INTERVAL_MS 1000

// Anomaly Detection Thresholds
#define RULE_SCORE_WEIGHT 0.6f
#define ML_SCORE_WEIGHT 0.4f
#define WARNING_THRESHOLD 0.5f
#define CRITICAL_THRESHOLD 0.8f
#define CURRENT_LIMIT_WARNING_PERCENT 70

// Rule-based Detection Parameters
#define REMOTE_STOP_BURST_THRESHOLD 3
#define REMOTE_STOP_BURST_WINDOW_MS 60000
#define MALFORMED_BURST_THRESHOLD 2
#define MALFORMED_BURST_WINDOW_MS 30000
#define THD_I_MULTIPLIER_THRESHOLD 1.5f
#define OCPP_RATE_THRESHOLD 0.6f

// Baseline Values (to be calibrated)
#define BASELINE_THD_I 2.0f
#define BASELINE_OCPP_RATE 5.0f

// Safety State Machine
typedef enum {
    SAFETY_STATE_IDLE = 0,
    SAFETY_STATE_HANDSHAKE,
    SAFETY_STATE_PRECHARGE,
    SAFETY_STATE_CHARGING,
    SAFETY_STATE_SUSPICIOUS,
    SAFETY_STATE_LOCKDOWN
} safety_state_t;

// Feature Vector Structure
typedef struct {
    float v_rms;
    float i_rms;
    float p_kw;
    float pf;
    float thd_v;
    float thd_i;
    float dvdt;
    float didt;
    float ocpp_rate;
    uint32_t remote_stop_cnt;
    uint32_t malformed;
    uint32_t out_of_seq;
    bool fw_ok;
    bool tamper;
    float temp_c;
} feature_vector_t;

// OCPP Message Types
typedef enum {
    OCPP_MSG_START_TRANSACTION,
    OCPP_MSG_METER_VALUES,
    OCPP_MSG_REMOTE_STOP_TRANSACTION,
    OCPP_MSG_UPDATE_FIRMWARE,
    OCPP_MSG_UNKNOWN
} ocpp_msg_type_t;

// Alert Levels
typedef enum {
    ALERT_LEVEL_INFO = 0,
    ALERT_LEVEL_WARNING,
    ALERT_LEVEL_CRITICAL
} alert_level_t;

// Logging Configuration
#define MAX_LOG_FILE_SIZE_BYTES 1048576  // 1MB
#define MAX_LOG_FILES 10
#define LOG_BUFFER_SIZE 1024

// TFLite Model Configuration
#define TFLITE_MODEL_SIZE 50000  // Approximate size in bytes
#define TFLITE_ARENA_SIZE 32768  // Tensor arena size
#define TFLITE_INPUT_SIZE 15     // Number of features
#define TFLITE_OUTPUT_SIZE 1     // Reconstruction error

// WebSocket Configuration
#define WEBSOCKET_BUFFER_SIZE 1024
#define WEBSOCKET_RECONNECT_INTERVAL_MS 5000

// NVS Keys
#define NVS_NAMESPACE "evsecure"
#define NVS_KEY_WIFI_SSID "wifi_ssid"
#define NVS_KEY_WIFI_PASS "wifi_pass"
#define NVS_KEY_API_KEY "api_key"
#define NVS_KEY_DEVICE_ID "device_id"
#define NVS_KEY_SESSION_COUNT "session_count"

// Error Codes
#define EVSECURE_OK 0
#define EVSECURE_ERR_INIT -1
#define EVSECURE_ERR_SENSOR -2
#define EVSECURE_ERR_WIFI -3
#define EVSECURE_ERR_API -4
#define EVSECURE_ERR_ML -5
#define EVSECURE_ERR_STORAGE -6

#endif // EVSECURE_CONFIG_H
