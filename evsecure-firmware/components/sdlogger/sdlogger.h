#ifndef SDLOGGER_H
#define SDLOGGER_H

#include "esp_err.h"
#include "evsecure_config.h"
#include <time.h>

// Log file structure
typedef struct {
    char filename[32];
    size_t size;
    time_t created;
    bool uploaded;
} log_file_info_t;

// Alert structure for API upload
typedef struct {
    alert_level_t level;
    float score;
    uint64_t timestamp;
    char session_id[32];
} alert_t;

// Function declarations
esp_err_t sdlogger_init(void);
esp_err_t sdlogger_log_feature(feature_vector_t *feature);
esp_err_t sdlogger_log_alert(alert_t *alert);
esp_err_t sdlogger_upload_pending_logs(void);
esp_err_t sdlogger_get_log_files(log_file_info_t *files, size_t max_files, size_t *count);
esp_err_t sdlogger_rotate_log_file(void);
esp_err_t sdlogger_deinit(void);

// API upload functions
esp_err_t sdlogger_upload_features_batch(const char *filename);
esp_err_t sdlogger_upload_alert_immediate(alert_t *alert);

#endif // SDLOGGER_H
