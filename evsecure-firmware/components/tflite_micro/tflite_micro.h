#ifndef TFLITE_MICRO_H
#define TFLITE_MICRO_H

#include "esp_err.h"
#include "evsecure_config.h"

// TFLite Micro model structure
typedef struct {
    const unsigned char* model_data;
    size_t model_size;
    void* interpreter;
    void* input_tensor;
    void* output_tensor;
    void* arena;
    size_t arena_size;
} tflite_micro_model_t;

// Function declarations
esp_err_t tflite_micro_init(void);
esp_err_t tflite_micro_inference(feature_vector_t *features, float *reconstruction_error);
esp_err_t tflite_micro_deinit(void);
size_t tflite_micro_get_model_size(void);
esp_err_t tflite_micro_load_model(const unsigned char* model_data, size_t model_size);

// Model data (placeholder - will be replaced by actual model)
extern const unsigned char model_data[];
extern const size_t model_data_size;

#endif // TFLITE_MICRO_H
