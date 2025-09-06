#ifndef SECURITY_H
#define SECURITY_H

#include "esp_err.h"
#include "evsecure_config.h"
#include "esp_ota_ops.h"
#include "mbedtls/sha256.h"

// Security functions
esp_err_t security_init(void);
bool security_check_firmware_integrity(void);
esp_err_t security_verify_ota_update(const void *data, size_t len);
esp_err_t security_get_api_key(char *key, size_t len);
esp_err_t security_store_api_key(const char *key);
esp_err_t security_deinit(void);

// Secure element functions (ATECC608A)
esp_err_t security_atecc608a_init(void);
esp_err_t security_atecc608a_read_serial(uint8_t *serial, size_t len);
esp_err_t security_atecc608a_sign_data(const uint8_t *data, size_t len, uint8_t *signature);
esp_err_t security_atecc608a_verify_signature(const uint8_t *data, size_t len, 
                                            const uint8_t *signature, const uint8_t *public_key);

// Hash functions
esp_err_t security_calculate_firmware_hash(uint8_t *hash, size_t len);
esp_err_t security_verify_hash(const uint8_t *data, size_t len, const uint8_t *expected_hash);

#endif // SECURITY_H
