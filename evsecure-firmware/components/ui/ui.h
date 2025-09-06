#ifndef UI_H
#define UI_H

#include "esp_err.h"
#include "evsecure_config.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

// UI colors (RGB565 format)
#define UI_COLOR_BLACK     0x0000
#define UI_COLOR_WHITE     0xFFFF
#define UI_COLOR_RED       0xF800
#define UI_COLOR_GREEN     0x07E0
#define UI_COLOR_BLUE      0x001F
#define UI_COLOR_YELLOW    0xFFE0
#define UI_COLOR_ORANGE    0xFD20
#define UI_COLOR_GRAY      0x8410

// Display dimensions
#define TFT_WIDTH          128
#define TFT_HEIGHT         160
#define TFT_ROTATION       0

// Button states
typedef enum {
    BUTTON_STATE_RELEASED = 0,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_LONG_PRESS
} button_state_t;

// UI layout
typedef struct {
    char session_id[32];
    float v_rms;
    float i_rms;
    float p_kw;
    float anomaly_score;
    safety_state_t safety_state;
    bool ack_button_pressed;
    bool bypass_button_pressed;
} ui_data_t;

// Function declarations
esp_err_t ui_init(void);
esp_err_t ui_update_display(safety_state_t safety_state, float anomaly_score, 
                           const char *session_id, feature_vector_t *features);
esp_err_t ui_handle_buttons(void);
esp_err_t ui_deinit(void);

// Button functions
button_state_t ui_get_button_state(gpio_num_t button_pin);
bool ui_is_button_pressed(gpio_num_t button_pin);
bool ui_is_long_press(gpio_num_t button_pin);

// Display functions
esp_err_t ui_draw_text(const char *text, uint16_t x, uint16_t y, uint16_t color, uint8_t size);
esp_err_t ui_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
esp_err_t ui_draw_filled_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
esp_err_t ui_clear_screen(uint16_t color);
esp_err_t ui_draw_anomaly_graph(float *scores, size_t count);

#endif // UI_H
