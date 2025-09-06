#include "sdlogger.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "esp_vfs.h"
#include "driver/sdspi_host.h"
#include "driver/sdmmc_host.h"
#include "driver/gpio.h"
#include "sdmmc_cmd.h"
#include "cJSON.h"
#include "esp_http_client.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

static const char *TAG = "SDLOGGER";

// Global variables
static sdmmc_card_t *card = NULL;
static char current_log_filename[32];
static FILE *current_log_file = NULL;
static size_t current_log_size = 0;
static bool sdlogger_initialized = false;

// HTTP client event handler
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP Client Error");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP Client Connected");
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP Client Finished");
            break;
        default:
            break;
    }
    return ESP_OK;
}

esp_err_t sdlogger_init(void)
{
    if (sdlogger_initialized) {
        ESP_LOGW(TAG, "SD Logger already initialized");
        return ESP_OK;
    }
    
    esp_err_t ret;
    
    // Configure SD card SPI
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_SPI_CS_PIN;
    slot_config.host_id = host.slot;
    
    // Configure SPI bus
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_SPI_MOSI_PIN,
        .miso_io_num = SD_SPI_MISO_PIN,
        .sclk_io_num = SD_SPI_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus");
        return ret;
    }
    
    // Mount SD card
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card");
        return ret;
    }
    
    // Create logs directory
    mkdir("/sdcard/logs", 0755);
    
    // Generate initial log filename
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    snprintf(current_log_filename, sizeof(current_log_filename), 
             "/sdcard/logs/features_%04d%02d%02d_%02d%02d%02d.csv",
             tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    
    // Open log file
    current_log_file = fopen(current_log_filename, "w");
    if (!current_log_file) {
        ESP_LOGE(TAG, "Failed to open log file: %s", current_log_filename);
        return ESP_ERR_NO_MEM;
    }
    
    // Write CSV header
    fprintf(current_log_file, "timestamp,device_id,session_id,v_rms,i_rms,p_kw,pf,thd_v,thd_i,dvdt,didt,ocpp_rate,remote_stop_cnt,malformed,out_of_seq,fw_ok,tamper,temp_c\n");
    fflush(current_log_file);
    
    sdlogger_initialized = true;
    ESP_LOGI(TAG, "SD Logger initialized successfully");
    ESP_LOGI(TAG, "Log file: %s", current_log_filename);
    
    return ESP_OK;
}

esp_err_t sdlogger_log_feature(feature_vector_t *feature)
{
    if (!sdlogger_initialized || !current_log_file) {
        ESP_LOGE(TAG, "SD Logger not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!feature) {
        ESP_LOGE(TAG, "Invalid feature pointer");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Check if we need to rotate the log file
    if (current_log_size > MAX_LOG_FILE_SIZE_BYTES) {
        sdlogger_rotate_log_file();
    }
    
    // Get current timestamp
    uint64_t timestamp = esp_timer_get_time() / 1000000;  // Convert to seconds
    
    // Write CSV line
    fprintf(current_log_file, "%llu,%s,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%u,%u,%u,%d,%d,%.1f\n",
            timestamp, DEVICE_ID, "session_placeholder", 
            feature->v_rms, feature->i_rms, feature->p_kw, feature->pf,
            feature->thd_v, feature->thd_i, feature->dvdt, feature->didt,
            feature->ocpp_rate, feature->remote_stop_cnt, feature->malformed,
            feature->out_of_seq, feature->fw_ok ? 1 : 0, feature->tamper ? 1 : 0,
            feature->temp_c);
    
    fflush(current_log_file);
    
    // Update file size
    current_log_size = ftell(current_log_file);
    
    return ESP_OK;
}

esp_err_t sdlogger_log_alert(alert_t *alert)
{
    if (!sdlogger_initialized) {
        ESP_LOGE(TAG, "SD Logger not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!alert) {
        ESP_LOGE(TAG, "Invalid alert pointer");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Create alert log filename
    char alert_filename[64];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    snprintf(alert_filename, sizeof(alert_filename), 
             "/sdcard/logs/alert_%04d%02d%02d_%02d%02d%02d.json",
             tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    
    // Create JSON alert
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "device_id", DEVICE_ID);
    cJSON_AddStringToObject(json, "session_id", alert->session_id);
    cJSON_AddNumberToObject(json, "timestamp", alert->timestamp);
    cJSON_AddNumberToObject(json, "level", alert->level);
    cJSON_AddNumberToObject(json, "score", alert->score);
    
    // Write to file
    FILE *alert_file = fopen(alert_filename, "w");
    if (alert_file) {
        char *json_str = cJSON_Print(json);
        fprintf(alert_file, "%s\n", json_str);
        fclose(alert_file);
        free(json_str);
    }
    
    cJSON_Delete(json);
    
    // Upload alert immediately
    sdlogger_upload_alert_immediate(alert);
    
    return ESP_OK;
}

esp_err_t sdlogger_upload_pending_logs(void)
{
    if (!sdlogger_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Get list of log files
    log_file_info_t files[MAX_LOG_FILES];
    size_t file_count;
    esp_err_t ret = sdlogger_get_log_files(files, MAX_LOG_FILES, &file_count);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Upload each file that hasn't been uploaded
    for (size_t i = 0; i < file_count; i++) {
        if (!files[i].uploaded) {
            ret = sdlogger_upload_features_batch(files[i].filename);
            if (ret == ESP_OK) {
                // Mark as uploaded (in a real implementation, you'd update a flag)
                ESP_LOGI(TAG, "Uploaded log file: %s", files[i].filename);
            }
        }
    }
    
    return ESP_OK;
}

esp_err_t sdlogger_upload_features_batch(const char *filename)
{
    if (!filename) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Read the CSV file
    FILE *file = fopen(filename, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for upload: %s", filename);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Create JSON array for features
    cJSON *features_array = cJSON_CreateArray();
    char line[512];
    
    // Skip header line
    fgets(line, sizeof(line), file);
    
    // Read each line and convert to JSON
    while (fgets(line, sizeof(line), file)) {
        cJSON *feature = cJSON_CreateObject();
        
        // Parse CSV line (simplified parsing)
        char *token = strtok(line, ",");
        if (token) {
            cJSON_AddNumberToObject(feature, "ts", atoll(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddStringToObject(feature, "device_id", token);
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddStringToObject(feature, "session_id", token);
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "v_rms", atof(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "i_rms", atof(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "p_kw", atof(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "pf", atof(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "thd_v", atof(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "thd_i", atof(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "dvdt", atof(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "didt", atof(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "ocpp_rate", atof(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "remote_stop_cnt", atoi(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "malformed", atoi(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "out_of_seq", atoi(token));
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddBoolToObject(feature, "fw_ok", atoi(token) != 0);
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddBoolToObject(feature, "tamper", atoi(token) != 0);
            token = strtok(NULL, ",");
        }
        if (token) {
            cJSON_AddNumberToObject(feature, "temp_c", atof(token));
        }
        
        cJSON_AddItemToArray(features_array, feature);
    }
    
    fclose(file);
    
    // Create the complete JSON payload
    cJSON *payload = cJSON_CreateObject();
    cJSON_AddItemToObject(payload, "features", features_array);
    
    char *json_str = cJSON_Print(payload);
    
    // Configure HTTP client
    esp_http_client_config_t config = {
        .url = DASHBOARD_API_URL "/ingest/features",
        .timeout_ms = API_TIMEOUT_MS,
        .event_handler = http_event_handler,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Authorization", "ApiKey " API_KEY);
    esp_http_client_set_data(client, json_str, strlen(json_str));
    
    esp_err_t ret = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
    
    if (ret == ESP_OK && status_code == 200) {
        ESP_LOGI(TAG, "Features uploaded successfully");
    } else {
        ESP_LOGE(TAG, "Features upload failed: %s, status: %d", esp_err_to_name(ret), status_code);
    }
    
    esp_http_client_cleanup(client);
    cJSON_Delete(payload);
    free(json_str);
    
    return (ret == ESP_OK && status_code == 200) ? ESP_OK : ESP_FAIL;
}

esp_err_t sdlogger_upload_alert_immediate(alert_t *alert)
{
    if (!alert) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Create JSON alert
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "device_id", DEVICE_ID);
    cJSON_AddStringToObject(json, "session_id", alert->session_id);
    cJSON_AddNumberToObject(json, "timestamp", alert->timestamp);
    cJSON_AddNumberToObject(json, "level", alert->level);
    cJSON_AddNumberToObject(json, "score", alert->score);
    
    char *json_str = cJSON_Print(json);
    
    // Configure HTTP client
    esp_http_client_config_t config = {
        .url = DASHBOARD_API_URL "/ingest/alerts",
        .timeout_ms = API_TIMEOUT_MS,
        .event_handler = http_event_handler,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Authorization", "ApiKey " API_KEY);
    esp_http_client_set_data(client, json_str, strlen(json_str));
    
    esp_err_t ret = esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
    
    if (ret == ESP_OK && status_code == 200) {
        ESP_LOGI(TAG, "Alert uploaded successfully");
    } else {
        ESP_LOGE(TAG, "Alert upload failed: %s, status: %d", esp_err_to_name(ret), status_code);
    }
    
    esp_http_client_cleanup(client);
    cJSON_Delete(json);
    free(json_str);
    
    return (ret == ESP_OK && status_code == 200) ? ESP_OK : ESP_FAIL;
}

esp_err_t sdlogger_get_log_files(log_file_info_t *files, size_t max_files, size_t *count)
{
    if (!files || !count) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *count = 0;
    
    // In a real implementation, you would scan the /sdcard/logs directory
    // and populate the files array with information about each log file
    // For now, we'll return a placeholder
    
    return ESP_OK;
}

esp_err_t sdlogger_rotate_log_file(void)
{
    if (!sdlogger_initialized || !current_log_file) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Close current file
    fclose(current_log_file);
    current_log_file = NULL;
    
    // Generate new filename
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    snprintf(current_log_filename, sizeof(current_log_filename), 
             "/sdcard/logs/features_%04d%02d%02d_%02d%02d%02d.csv",
             tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    
    // Open new file
    current_log_file = fopen(current_log_filename, "w");
    if (!current_log_file) {
        ESP_LOGE(TAG, "Failed to open new log file: %s", current_log_filename);
        return ESP_ERR_NO_MEM;
    }
    
    // Write CSV header
    fprintf(current_log_file, "timestamp,device_id,session_id,v_rms,i_rms,p_kw,pf,thd_v,thd_i,dvdt,didt,ocpp_rate,remote_stop_cnt,malformed,out_of_seq,fw_ok,tamper,temp_c\n");
    fflush(current_log_file);
    
    current_log_size = 0;
    
    ESP_LOGI(TAG, "Log file rotated to: %s", current_log_filename);
    
    return ESP_OK;
}

esp_err_t sdlogger_deinit(void)
{
    if (!sdlogger_initialized) {
        return ESP_OK;
    }
    
    if (current_log_file) {
        fclose(current_log_file);
        current_log_file = NULL;
    }
    
    // Unmount SD card
    if (card) {
        esp_vfs_fat_sdmmc_unmount();
        card = NULL;
    }
    
    sdlogger_initialized = false;
    ESP_LOGI(TAG, "SD Logger deinitialized");
    
    return ESP_OK;
}
