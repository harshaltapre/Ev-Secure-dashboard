#include "tflite_micro.h"
#include "model_data.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <string.h>

static const char *TAG = "TFLITE_MICRO";

// Global model instance
static tflite_micro_model_t model_instance = {0};
static bool model_initialized = false;

// Feature normalization parameters (would be learned from training data)
static const float feature_means[15] = {
    230.0f,  // v_rms
    15.0f,   // i_rms
    3.5f,    // p_kw
    0.95f,   // pf
    2.5f,    // thd_v
    3.5f,    // thd_i
    0.0f,    // dvdt
    0.0f,    // didt
    5.0f,    // ocpp_rate
    0.0f,    // remote_stop_cnt
    0.0f,    // malformed
    0.0f,    // out_of_seq
    1.0f,    // fw_ok
    0.0f,    // tamper
    25.0f    // temp_c
};

static const float feature_stds[15] = {
    20.0f,   // v_rms
    5.0f,    // i_rms
    1.5f,    // p_kw
    0.05f,   // pf
    1.0f,    // thd_v
    1.5f,    // thd_i
    10.0f,   // dvdt
    5.0f,    // didt
    2.0f,    // ocpp_rate
    1.0f,    // remote_stop_cnt
    1.0f,    // malformed
    1.0f,    // out_of_seq
    0.0f,    // fw_ok
    0.0f,    // tamper
    10.0f    // temp_c
};

esp_err_t tflite_micro_init(void)
{
    if (model_initialized) {
        ESP_LOGW(TAG, "TFLite Micro already initialized");
        return ESP_OK;
    }
    
    // Allocate memory for model arena
    model_instance.arena = malloc(TFLITE_ARENA_SIZE);
    if (!model_instance.arena) {
        ESP_LOGE(TAG, "Failed to allocate model arena");
        return ESP_ERR_NO_MEM;
    }
    
    model_instance.arena_size = TFLITE_ARENA_SIZE;
    
    // Load model data
    esp_err_t ret = tflite_micro_load_model(model_data, model_data_size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to load model");
        free(model_instance.arena);
        return ret;
    }
    
    model_initialized = true;
    ESP_LOGI(TAG, "TFLite Micro initialized successfully");
    ESP_LOGI(TAG, "Model size: %zu bytes", model_data_size);
    
    return ESP_OK;
}

esp_err_t tflite_micro_inference(feature_vector_t *features, float *reconstruction_error)
{
    if (!model_initialized) {
        ESP_LOGE(TAG, "TFLite Micro not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!features || !reconstruction_error) {
        ESP_LOGE(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Convert features to input array
    float input_features[15];
    input_features[0] = features->v_rms;
    input_features[1] = features->i_rms;
    input_features[2] = features->p_kw;
    input_features[3] = features->pf;
    input_features[4] = features->thd_v;
    input_features[5] = features->thd_i;
    input_features[6] = features->dvdt;
    input_features[7] = features->didt;
    input_features[8] = features->ocpp_rate;
    input_features[9] = (float)features->remote_stop_cnt;
    input_features[10] = (float)features->malformed;
    input_features[11] = (float)features->out_of_seq;
    input_features[12] = features->fw_ok ? 1.0f : 0.0f;
    input_features[13] = features->tamper ? 1.0f : 0.0f;
    input_features[14] = features->temp_c;
    
    // Normalize features
    float normalized_features[15];
    for (int i = 0; i < 15; i++) {
        normalized_features[i] = (input_features[i] - feature_means[i]) / feature_stds[i];
    }
    
    // Simulate autoencoder inference
    // In a real implementation, this would call the actual TFLite Micro interpreter
    // For now, we'll simulate the behavior with a simple reconstruction error calculation
    
    // Simulate encoding (simplified)
    float encoded[8];  // Assume 8-dimensional latent space
    for (int i = 0; i < 8; i++) {
        encoded[i] = 0.0f;
        for (int j = 0; j < 15; j++) {
            // Simple linear encoding (placeholder)
            encoded[i] += normalized_features[j] * (0.1f + i * 0.05f + j * 0.01f);
        }
    }
    
    // Simulate decoding (simplified)
    float decoded[15];
    for (int i = 0; i < 15; i++) {
        decoded[i] = 0.0f;
        for (int j = 0; j < 8; j++) {
            // Simple linear decoding (placeholder)
            decoded[i] += encoded[j] * (0.1f + i * 0.05f + j * 0.01f);
        }
    }
    
    // Calculate reconstruction error
    float mse = 0.0f;
    for (int i = 0; i < 15; i++) {
        float diff = normalized_features[i] - decoded[i];
        mse += diff * diff;
    }
    mse /= 15.0f;
    
    // Convert to normalized error (0-1 range)
    *reconstruction_error = fminf(mse / 2.0f, 1.0f);  // Normalize to 0-1
    
    ESP_LOGD(TAG, "Inference completed, reconstruction error: %.4f", *reconstruction_error);
    
    return ESP_OK;
}

esp_err_t tflite_micro_deinit(void)
{
    if (!model_initialized) {
        return ESP_OK;
    }
    
    if (model_instance.arena) {
        free(model_instance.arena);
        model_instance.arena = NULL;
    }
    
    model_initialized = false;
    ESP_LOGI(TAG, "TFLite Micro deinitialized");
    
    return ESP_OK;
}

size_t tflite_micro_get_model_size(void)
{
    return model_data_size;
}

esp_err_t tflite_micro_load_model(const unsigned char* model_data, size_t model_size)
{
    if (!model_data || model_size == 0) {
        ESP_LOGE(TAG, "Invalid model data");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Store model data
    model_instance.model_data = model_data;
    model_instance.model_size = model_size;
    
    // In a real implementation, this would:
    // 1. Parse the TFLite model structure
    // 2. Set up the interpreter
    // 3. Allocate tensors
    // 4. Set up the model graph
    
    ESP_LOGI(TAG, "Model loaded successfully, size: %zu bytes", model_size);
    
    return ESP_OK;
}
