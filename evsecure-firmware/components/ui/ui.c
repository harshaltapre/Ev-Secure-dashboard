#include "ui.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

static const char *TAG = "UI";

// Global variables
static spi_device_handle_t spi_handle = NULL;
static bool ui_initialized = false;
static uint64_t button_press_time[2] = {0, 0};  // For ACK and BYPASS buttons
static bool button_pressed[2] = {false, false};
static float anomaly_history[30] = {0};  // Last 30 anomaly scores
static size_t anomaly_index = 0;

// TFT display commands
#define TFT_CMD_NOP        0x00
#define TFT_CMD_SWRESET    0x01
#define TFT_CMD_RDDID      0x04
#define TFT_CMD_RDDST      0x09
#define TFT_CMD_SLPIN      0x10
#define TFT_CMD_SLPOUT     0x11
#define TFT_CMD_PTLON      0x12
#define TFT_CMD_NORON      0x13
#define TFT_CMD_INVOFF     0x20
#define TFT_CMD_INVON      0x21
#define TFT_CMD_DISPOFF    0x28
#define TFT_CMD_DISPON     0x29
#define TFT_CMD_CASET      0x2A
#define TFT_CMD_RASET      0x2B
#define TFT_CMD_RAMWR      0x2C
#define TFT_CMD_RAMRD      0x2E
#define TFT_CMD_PTLAR      0x30
#define TFT_CMD_COLMOD     0x3A
#define TFT_CMD_MADCTL     0x36
#define TFT_CMD_FRMCTR1    0xB1
#define TFT_CMD_FRMCTR2    0xB2
#define TFT_CMD_FRMCTR3    0xB3
#define TFT_CMD_INVCTR     0xB4
#define TFT_CMD_DISSET5    0xB6
#define TFT_CMD_PWCTR1     0xC0
#define TFT_CMD_PWCTR2     0xC1
#define TFT_CMD_PWCTR3     0xC2
#define TFT_CMD_PWCTR4     0xC3
#define TFT_CMD_PWCTR5     0xC4
#define TFT_CMD_VMCTR1     0xC5
#define TFT_CMD_RDID1      0xDA
#define TFT_CMD_RDID2      0xDB
#define TFT_CMD_RDID3      0xDC
#define TFT_CMD_RDID4      0xDD
#define TFT_CMD_GMCTRP1    0xE0
#define TFT_CMD_GMCTRN1    0xE1
#define TFT_CMD_PWCTR6     0xFC

// Font data (simplified 6x8 font)
static const uint8_t font_6x8[][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00, 0x00}, // "
    // ... more characters would be defined here
};

// SPI transaction for TFT
static esp_err_t tft_spi_transaction(uint8_t cmd, const uint8_t *data, size_t len)
{
    spi_transaction_t trans = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    
    esp_err_t ret = spi_device_transmit(spi_handle, &trans);
    if (ret != ESP_OK) return ret;
    
    if (data && len > 0) {
        trans.length = len * 8;
        trans.tx_buffer = data;
        ret = spi_device_transmit(spi_handle, &trans);
    }
    
    return ret;
}

// TFT initialization
static esp_err_t tft_init(void)
{
    // Configure SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = TFT_SPI_MOSI_PIN,
        .miso_io_num = TFT_SPI_MISO_PIN,
        .sclk_io_num = TFT_SPI_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32 * 1024,
    };
    
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) return ret;
    
    // Configure SPI device
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 40 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = TFT_SPI_CS_PIN,
        .queue_size = 7,
    };
    
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle);
    if (ret != ESP_OK) return ret;
    
    // Configure GPIO pins
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << TFT_DC_PIN) | (1ULL << TFT_RST_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    
    // Reset display
    gpio_set_level(TFT_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(TFT_RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Initialize display
    uint8_t init_cmds[] = {
        TFT_CMD_SWRESET, 0,
        TFT_CMD_SLPOUT, 0,
        TFT_CMD_COLMOD, 1, 0x05,  // 16-bit color
        TFT_CMD_MADCTL, 1, 0x08,  // Row/Column order
        TFT_CMD_INVON, 0,
        TFT_CMD_NORON, 0,
        TFT_CMD_DISPON, 0,
    };
    
    for (int i = 0; i < sizeof(init_cmds); i += 2) {
        gpio_set_level(TFT_DC_PIN, 0);  // Command mode
        tft_spi_transaction(init_cmds[i], NULL, 0);
        if (init_cmds[i + 1] > 0) {
            gpio_set_level(TFT_DC_PIN, 1);  // Data mode
            tft_spi_transaction(0, &init_cmds[i + 2], init_cmds[i + 1]);
        }
    }
    
    return ESP_OK;
}

esp_err_t ui_init(void)
{
    if (ui_initialized) {
        ESP_LOGW(TAG, "UI already initialized");
        return ESP_OK;
    }
    
    esp_err_t ret;
    
    // Initialize TFT display
    ret = tft_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize TFT display");
        return ret;
    }
    
    // Configure button GPIO
    gpio_config_t btn_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << UI_ACK_BUTTON_PIN) | (1ULL << UI_BYPASS_BUTTON_PIN),
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&btn_conf);
    
    // Clear screen
    ui_clear_screen(UI_COLOR_BLACK);
    
    ui_initialized = true;
    ESP_LOGI(TAG, "UI initialized successfully");
    
    return ESP_OK;
}

esp_err_t ui_update_display(safety_state_t safety_state, float anomaly_score, 
                           const char *session_id, feature_vector_t *features)
{
    if (!ui_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Update anomaly history
    anomaly_history[anomaly_index] = anomaly_score;
    anomaly_index = (anomaly_index + 1) % 30;
    
    // Clear screen
    ui_clear_screen(UI_COLOR_BLACK);
    
    // Draw header
    char header[32];
    snprintf(header, sizeof(header), "EVsecure %s", DEVICE_VERSION);
    ui_draw_text(header, 2, 2, UI_COLOR_WHITE, 1);
    
    // Draw session ID
    if (session_id && strlen(session_id) > 0) {
        char session_display[20];
        snprintf(session_display, sizeof(session_display), "Sess: %s", session_id);
        ui_draw_text(session_display, 2, 12, UI_COLOR_BLUE, 1);
    }
    
    // Draw power measurements
    if (features) {
        char power_text[32];
        snprintf(power_text, sizeof(power_text), "V:%.1fV I:%.1fA", 
                features->v_rms, features->i_rms);
        ui_draw_text(power_text, 2, 22, UI_COLOR_GREEN, 1);
        
        snprintf(power_text, sizeof(power_text), "P:%.2fkW PF:%.2f", 
                features->p_kw, features->pf);
        ui_draw_text(power_text, 2, 32, UI_COLOR_GREEN, 1);
    }
    
    // Draw safety state
    const char *state_text = "UNKNOWN";
    uint16_t state_color = UI_COLOR_GRAY;
    
    switch (safety_state) {
        case SAFETY_STATE_IDLE:
            state_text = "IDLE";
            state_color = UI_COLOR_BLUE;
            break;
        case SAFETY_STATE_HANDSHAKE:
            state_text = "HANDSHAKE";
            state_color = UI_COLOR_YELLOW;
            break;
        case SAFETY_STATE_PRECHARGE:
            state_text = "PRECHARGE";
            state_color = UI_COLOR_ORANGE;
            break;
        case SAFETY_STATE_CHARGING:
            state_text = "CHARGING";
            state_color = UI_COLOR_GREEN;
            break;
        case SAFETY_STATE_SUSPICIOUS:
            state_text = "SUSPICIOUS";
            state_color = UI_COLOR_ORANGE;
            break;
        case SAFETY_STATE_LOCKDOWN:
            state_text = "LOCKDOWN";
            state_color = UI_COLOR_RED;
            break;
    }
    
    ui_draw_text("Status:", 2, 52, UI_COLOR_WHITE, 1);
    ui_draw_text(state_text, 50, 52, state_color, 1);
    
    // Draw anomaly score
    char score_text[32];
    snprintf(score_text, sizeof(score_text), "Anomaly: %.1f%%", anomaly_score * 100.0f);
    uint16_t score_color = (anomaly_score < 0.5f) ? UI_COLOR_GREEN : 
                          (anomaly_score < 0.8f) ? UI_COLOR_YELLOW : UI_COLOR_RED;
    ui_draw_text(score_text, 2, 62, score_color, 1);
    
    // Draw anomaly graph
    ui_draw_anomaly_graph(anomaly_history, 30);
    
    // Draw button labels
    ui_draw_text("ACK", 10, 140, UI_COLOR_WHITE, 1);
    ui_draw_text("BYPASS", 70, 140, UI_COLOR_WHITE, 1);
    
    return ESP_OK;
}

esp_err_t ui_handle_buttons(void)
{
    if (!ui_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint64_t now = esp_timer_get_time() / 1000;  // Convert to milliseconds
    
    // Check ACK button
    bool ack_pressed = !gpio_get_level(UI_ACK_BUTTON_PIN);
    if (ack_pressed && !button_pressed[0]) {
        button_press_time[0] = now;
        button_pressed[0] = true;
        ESP_LOGI(TAG, "ACK button pressed");
    } else if (!ack_pressed && button_pressed[0]) {
        button_pressed[0] = false;
        ESP_LOGI(TAG, "ACK button released");
    }
    
    // Check BYPASS button
    bool bypass_pressed = !gpio_get_level(UI_BYPASS_BUTTON_PIN);
    if (bypass_pressed && !button_pressed[1]) {
        button_press_time[1] = now;
        button_pressed[1] = true;
        ESP_LOGI(TAG, "BYPASS button pressed");
    } else if (!bypass_pressed && button_pressed[1]) {
        button_pressed[1] = false;
        ESP_LOGI(TAG, "BYPASS button released");
    }
    
    return ESP_OK;
}

esp_err_t ui_deinit(void)
{
    if (!ui_initialized) {
        return ESP_OK;
    }
    
    if (spi_handle) {
        spi_bus_remove_device(spi_handle);
        spi_handle = NULL;
    }
    
    ui_initialized = false;
    ESP_LOGI(TAG, "UI deinitialized");
    
    return ESP_OK;
}

button_state_t ui_get_button_state(gpio_num_t button_pin)
{
    if (button_pin == UI_ACK_BUTTON_PIN) {
        return button_pressed[0] ? BUTTON_STATE_PRESSED : BUTTON_STATE_RELEASED;
    } else if (button_pin == UI_BYPASS_BUTTON_PIN) {
        return button_pressed[1] ? BUTTON_STATE_PRESSED : BUTTON_STATE_RELEASED;
    }
    
    return BUTTON_STATE_RELEASED;
}

bool ui_is_button_pressed(gpio_num_t button_pin)
{
    return ui_get_button_state(button_pin) == BUTTON_STATE_PRESSED;
}

bool ui_is_long_press(gpio_num_t button_pin)
{
    uint64_t now = esp_timer_get_time() / 1000;
    
    if (button_pin == UI_ACK_BUTTON_PIN && button_pressed[0]) {
        return (now - button_press_time[0]) > 2000;  // 2 seconds
    } else if (button_pin == UI_BYPASS_BUTTON_PIN && button_pressed[1]) {
        return (now - button_press_time[1]) > 2000;  // 2 seconds
    }
    
    return false;
}

esp_err_t ui_draw_text(const char *text, uint16_t x, uint16_t y, uint16_t color, uint8_t size)
{
    if (!text) return ESP_ERR_INVALID_ARG;
    
    // Set drawing window
    uint8_t window_cmd[] = {
        TFT_CMD_CASET, 4, (x >> 8) & 0xFF, x & 0xFF, ((x + strlen(text) * 6 * size - 1) >> 8) & 0xFF, (x + strlen(text) * 6 * size - 1) & 0xFF,
        TFT_CMD_RASET, 4, (y >> 8) & 0xFF, y & 0xFF, ((y + 8 * size - 1) >> 8) & 0xFF, (y + 8 * size - 1) & 0xFF,
        TFT_CMD_RAMWR, 0
    };
    
    gpio_set_level(TFT_DC_PIN, 0);
    tft_spi_transaction(TFT_CMD_CASET, &window_cmd[2], 4);
    tft_spi_transaction(TFT_CMD_RASET, &window_cmd[7], 4);
    tft_spi_transaction(TFT_CMD_RAMWR, NULL, 0);
    
    // Draw text (simplified - would need proper font rendering)
    gpio_set_level(TFT_DC_PIN, 1);
    
    // For now, just draw a simple rectangle to represent text
    ui_draw_filled_rect(x, y, strlen(text) * 6 * size, 8 * size, color);
    
    return ESP_OK;
}

esp_err_t ui_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    // Set drawing window
    uint8_t window_cmd[] = {
        TFT_CMD_CASET, 4, (x >> 8) & 0xFF, x & 0xFF, ((x + width - 1) >> 8) & 0xFF, (x + width - 1) & 0xFF,
        TFT_CMD_RASET, 4, (y >> 8) & 0xFF, y & 0xFF, ((y + height - 1) >> 8) & 0xFF, (y + height - 1) & 0xFF,
        TFT_CMD_RAMWR, 0
    };
    
    gpio_set_level(TFT_DC_PIN, 0);
    tft_spi_transaction(TFT_CMD_CASET, &window_cmd[2], 4);
    tft_spi_transaction(TFT_CMD_RASET, &window_cmd[7], 4);
    tft_spi_transaction(TFT_CMD_RAMWR, NULL, 0);
    
    // Draw rectangle outline
    gpio_set_level(TFT_DC_PIN, 1);
    
    uint16_t *pixels = malloc(width * height * 2);
    if (!pixels) return ESP_ERR_NO_MEM;
    
    for (int i = 0; i < width * height; i++) {
        pixels[i] = color;
    }
    
    tft_spi_transaction(0, (uint8_t*)pixels, width * height * 2);
    free(pixels);
    
    return ESP_OK;
}

esp_err_t ui_draw_filled_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    return ui_draw_rect(x, y, width, height, color);
}

esp_err_t ui_clear_screen(uint16_t color)
{
    return ui_draw_filled_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, color);
}

esp_err_t ui_draw_anomaly_graph(float *scores, size_t count)
{
    if (!scores || count == 0) return ESP_ERR_INVALID_ARG;
    
    // Draw graph background
    ui_draw_filled_rect(2, 80, 124, 50, UI_COLOR_BLACK);
    ui_draw_rect(2, 80, 124, 50, UI_COLOR_GRAY);
    
    // Draw graph lines
    for (int i = 1; i < count && i < 30; i++) {
        int x1 = 2 + (i - 1) * 4;
        int x2 = 2 + i * 4;
        int y1 = 130 - (int)(scores[i - 1] * 50);
        int y2 = 130 - (int)(scores[i] * 50);
        
        // Clamp values
        if (y1 < 80) y1 = 80;
        if (y1 > 130) y1 = 130;
        if (y2 < 80) y2 = 80;
        if (y2 > 130) y2 = 130;
        
        // Draw line (simplified)
        ui_draw_filled_rect(x1, y1, 4, 2, UI_COLOR_GREEN);
    }
    
    return ESP_OK;
}
