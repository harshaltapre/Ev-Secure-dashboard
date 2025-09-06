#include "security.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/i2c.h"
#include "mbedtls/sha256.h"
#include "mbedtls/rsa.h"
#include "mbedtls/pk.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "SECURITY";

// Global variables
static bool security_initialized = false;
static bool atecc608a_available = false;
static nvs_handle_t nvs_handle = 0;

// Expected firmware hash (would be calculated during build)
static const uint8_t expected_firmware_hash[32] = {
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0
};

// ATECC608A commands
#define ATECC608A_CMD_READ           0x02
#define ATECC608A_CMD_WRITE          0x12
#define ATECC608A_CMD_SIGN           0x41
#define ATECC608A_CMD_VERIFY         0x45
#define ATECC608A_CMD_GENKEY         0x40
#define ATECC608A_CMD_RANDOM         0x46
#define ATECC608A_CMD_SHA           0x47
#define ATECC608A_CMD_LOCK           0x17
#define ATECC608A_CMD_UPDATE_EXTRA   0x20
#define ATECC608A_CMD_READ_ENC       0x03
#define ATECC608A_CMD_WRITE_ENC      0x13

esp_err_t security_init(void)
{
    if (security_initialized) {
        ESP_LOGW(TAG, "Security already initialized");
        return ESP_OK;
    }
    
    esp_err_t ret;
    
    // Open NVS handle
    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle");
        return ret;
    }
    
    // Try to initialize ATECC608A
    ret = security_atecc608a_init();
    if (ret == ESP_OK) {
        atecc608a_available = true;
        ESP_LOGI(TAG, "ATECC608A secure element initialized");
    } else {
        ESP_LOGW(TAG, "ATECC608A not available, using software security");
    }
    
    security_initialized = true;
    ESP_LOGI(TAG, "Security initialized successfully");
    
    return ESP_OK;
}

bool security_check_firmware_integrity(void)
{
    if (!security_initialized) {
        ESP_LOGE(TAG, "Security not initialized");
        return false;
    }
    
    uint8_t current_hash[32];
    esp_err_t ret = security_calculate_firmware_hash(current_hash, sizeof(current_hash));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to calculate firmware hash");
        return false;
    }
    
    // Compare with expected hash
    if (memcmp(current_hash, expected_firmware_hash, sizeof(current_hash)) != 0) {
        ESP_LOGE(TAG, "Firmware integrity check failed");
        return false;
    }
    
    ESP_LOGD(TAG, "Firmware integrity check passed");
    return true;
}

esp_err_t security_verify_ota_update(const void *data, size_t len)
{
    if (!data || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // In a real implementation, you would:
    // 1. Verify the digital signature of the OTA update
    // 2. Check the firmware version
    // 3. Validate the update against known good firmware
    
    // For now, we'll do a simple hash verification
    uint8_t update_hash[32];
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    mbedtls_sha256_update(&ctx, (const unsigned char*)data, len);
    mbedtls_sha256_finish(&ctx, update_hash);
    mbedtls_sha256_free(&ctx);
    
    ESP_LOGI(TAG, "OTA update hash calculated");
    
    // In a real implementation, you would verify this hash against a signed hash
    // For now, we'll accept any update (not secure!)
    
    return ESP_OK;
}

esp_err_t security_get_api_key(char *key, size_t len)
{
    if (!key || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (atecc608a_available) {
        // Read from secure element
        // This would be implemented with ATECC608A commands
        ESP_LOGW(TAG, "Reading API key from secure element (not implemented)");
    }
    
    // Read from NVS as fallback
    size_t required_size = 0;
    esp_err_t ret = nvs_get_str(nvs_handle, NVS_KEY_API_KEY, NULL, &required_size);
    if (ret == ESP_OK && required_size <= len) {
        ret = nvs_get_str(nvs_handle, NVS_KEY_API_KEY, key, &len);
        if (ret == ESP_OK) {
            return ESP_OK;
        }
    }
    
    // Return default key if not found
    strncpy(key, API_KEY, len - 1);
    key[len - 1] = '\0';
    
    return ESP_OK;
}

esp_err_t security_store_api_key(const char *key)
{
    if (!key) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (atecc608a_available) {
        // Store in secure element
        // This would be implemented with ATECC608A commands
        ESP_LOGW(TAG, "Storing API key in secure element (not implemented)");
    }
    
    // Store in NVS as fallback
    esp_err_t ret = nvs_set_str(nvs_handle, NVS_KEY_API_KEY, key);
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs_handle);
    }
    
    return ret;
}

esp_err_t security_deinit(void)
{
    if (!security_initialized) {
        return ESP_OK;
    }
    
    if (nvs_handle) {
        nvs_close(nvs_handle);
        nvs_handle = 0;
    }
    
    security_initialized = false;
    atecc608a_available = false;
    
    ESP_LOGI(TAG, "Security deinitialized");
    return ESP_OK;
}

esp_err_t security_atecc608a_init(void)
{
    // Configure I2C for ATECC608A
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = ATECC608A_I2C_SDA_PIN,
        .scl_io_num = ATECC608A_I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,  // 100kHz for ATECC608A
    };
    
    esp_err_t ret = i2c_param_config(I2C_NUM_1, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C for ATECC608A");
        return ret;
    }
    
    ret = i2c_driver_install(I2C_NUM_1, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2C driver for ATECC608A");
        return ret;
    }
    
    // Try to communicate with ATECC608A
    uint8_t test_cmd[] = {0x03, 0x07, 0x02, 0x00, 0x00, 0x00};  // Read command
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ATECC608A_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, test_cmd, sizeof(test_cmd), true);
    i2c_master_stop(cmd);
    
    ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ATECC608A not responding");
        return ret;
    }
    
    ESP_LOGI(TAG, "ATECC608A communication successful");
    return ESP_OK;
}

esp_err_t security_atecc608a_read_serial(uint8_t *serial, size_t len)
{
    if (!serial || len < 9) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Read serial number from ATECC608A
    uint8_t cmd[] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x09};  // Read 9 bytes from config zone
    
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (ATECC608A_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(i2c_cmd, cmd, sizeof(cmd), true);
    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (ATECC608A_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(i2c_cmd, serial, 9, I2C_MASTER_LAST_NACK);
    i2c_master_stop(i2c_cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_1, i2c_cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(i2c_cmd);
    
    return ret;
}

esp_err_t security_atecc608a_sign_data(const uint8_t *data, size_t len, uint8_t *signature)
{
    if (!data || !signature || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // This would implement the ATECC608A sign command
    // For now, we'll return a placeholder
    ESP_LOGW(TAG, "ATECC608A sign data not implemented");
    
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t security_atecc608a_verify_signature(const uint8_t *data, size_t len, 
                                            const uint8_t *signature, const uint8_t *public_key)
{
    if (!data || !signature || !public_key || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // This would implement the ATECC608A verify command
    // For now, we'll return a placeholder
    ESP_LOGW(TAG, "ATECC608A verify signature not implemented");
    
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t security_calculate_firmware_hash(uint8_t *hash, size_t len)
{
    if (!hash || len < 32) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Get current running partition
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (!running) {
        ESP_LOGE(TAG, "Failed to get running partition");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Read firmware data and calculate hash
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    
    // Read firmware in chunks
    uint8_t buffer[1024];
    size_t offset = 0;
    size_t bytes_read;
    
    while (offset < running->size) {
        size_t to_read = (running->size - offset > sizeof(buffer)) ? sizeof(buffer) : (running->size - offset);
        
        esp_err_t ret = esp_partition_read(running, offset, buffer, to_read);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read firmware at offset %zu", offset);
            mbedtls_sha256_free(&ctx);
            return ret;
        }
        
        mbedtls_sha256_update(&ctx, buffer, to_read);
        offset += to_read;
    }
    
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);
    
    ESP_LOGD(TAG, "Firmware hash calculated");
    return ESP_OK;
}

esp_err_t security_verify_hash(const uint8_t *data, size_t len, const uint8_t *expected_hash)
{
    if (!data || !expected_hash || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t calculated_hash[32];
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    mbedtls_sha256_update(&ctx, data, len);
    mbedtls_sha256_finish(&ctx, calculated_hash);
    mbedtls_sha256_free(&ctx);
    
    if (memcmp(calculated_hash, expected_hash, 32) != 0) {
        ESP_LOGE(TAG, "Hash verification failed");
        return ESP_ERR_INVALID_CRC;
    }
    
    ESP_LOGD(TAG, "Hash verification passed");
    return ESP_OK;
}
