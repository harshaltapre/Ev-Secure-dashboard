idf_component_register(
    SRCS "tflite_micro.c" "model_data.c"
    INCLUDE_DIRS "."
    REQUIRES esp_common
)
