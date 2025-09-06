#ifndef MODEL_DATA_H
#define MODEL_DATA_H

#include <stddef.h>

// Placeholder model data - this would be replaced by actual TFLite model
// This is a simple autoencoder structure for demonstration
extern const unsigned char model_data[];
extern const size_t model_data_size;

// Model metadata
#define MODEL_INPUT_SIZE 15
#define MODEL_OUTPUT_SIZE 1
#define MODEL_ARENA_SIZE 32768

#endif // MODEL_DATA_H
