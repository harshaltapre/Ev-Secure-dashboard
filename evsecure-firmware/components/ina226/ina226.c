#include "ina226.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>

static const char *TAG = "INA226";

// Global variables
static uint16_t calibration_value = 0;
static float current_lsb = INA226_CURRENT_LSB;
static float power_lsb = INA226_POWER_LSB;
static float voltage_lsb = INA226_VOLTAGE_LSB;

// I2C write function
static esp_err_t ina226_i2c_write(uint8_t reg, uint16_t value)
{
    uint8_t data[3];
    data[0] = reg;
    data[1] = (value >> 8) & 0xFF;  // MSB first
    data[2] = value & 0xFF;         // LSB
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, data, 3, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

// I2C read function
static esp_err_t ina226_i2c_read(uint8_t reg, uint16_t *value)
{
    uint8_t data[2];
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret == ESP_OK) {
        *value = (data[0] << 8) | data[1];  // MSB first
    }
    
    return ret;
}

esp_err_t ina226_init(void)
{
    esp_err_t ret;
    
    // Configure I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = INA226_I2C_SDA_PIN,
        .scl_io_num = INA226_I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = INA226_I2C_FREQ_HZ,
    };
    
    ret = i2c_param_config(I2C_NUM_0, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C parameter config failed");
        return ret;
    }
    
    ret = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed");
        return ret;
    }
    
    // Reset the device
    ret = ina226_reset();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "INA226 reset failed");
        return ret;
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));  // Wait for reset to complete
    
    // Check manufacturer ID
    uint16_t manufacturer_id;
    ret = ina226_get_manufacturer_id(&manufacturer_id);
    if (ret != ESP_OK || manufacturer_id != 0x5449) {  // TI manufacturer ID
        ESP_LOGE(TAG, "Invalid manufacturer ID: 0x%04x", manufacturer_id);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Check die ID
    uint16_t die_id;
    ret = ina226_get_die_id(&die_id);
    if (ret != ESP_OK || die_id != 0x2260) {  // INA226 die ID
        ESP_LOGE(TAG, "Invalid die ID: 0x%04x", die_id);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Configure the device
    ret = ina226_write_register(INA226_REG_CONFIG, INA226_CONFIG_DEFAULT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "INA226 configuration failed");
        return ret;
    }
    
    // Set default calibration
    calibration_value = 0x1000;  // Default calibration value
    ret = ina226_write_register(INA226_REG_CALIBRATION, calibration_value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "INA226 calibration failed");
        return ret;
    }
    
    ESP_LOGI(TAG, "INA226 initialized successfully");
    return ESP_OK;
}

esp_err_t ina226_read_register(uint8_t reg, uint16_t *value)
{
    return ina226_i2c_read(reg, value);
}

esp_err_t ina226_write_register(uint8_t reg, uint16_t value)
{
    return ina226_i2c_write(reg, value);
}

esp_err_t ina226_read_measurements(float *v_rms, float *i_rms, float *p_kw, 
                                  float *pf, float *thd_v, float *thd_i)
{
    esp_err_t ret;
    uint16_t raw_values[4];
    float voltage, current, power;
    
    // Read voltage, current, and power registers
    ret = ina226_read_register(INA226_REG_BUSVOLTAGE, &raw_values[0]);
    if (ret != ESP_OK) return ret;
    
    ret = ina226_read_register(INA226_REG_CURRENT, &raw_values[1]);
    if (ret != ESP_OK) return ret;
    
    ret = ina226_read_register(INA226_REG_POWER, &raw_values[2]);
    if (ret != ESP_OK) return ret;
    
    ret = ina226_read_register(INA226_REG_SHUNTVOLTAGE, &raw_values[3]);
    if (ret != ESP_OK) return ret;
    
    // Convert raw values to physical units
    voltage = (float)((int16_t)raw_values[0]) * voltage_lsb / 1000.0f;  // Convert to V
    current = (float)((int16_t)raw_values[1]) * current_lsb / 1000.0f;   // Convert to A
    power = (float)((int16_t)raw_values[2]) * power_lsb / 1000.0f;      // Convert to W
    
    // Calculate RMS values (simplified - assuming DC or near-DC)
    *v_rms = voltage;
    *i_rms = current;
    *p_kw = power / 1000.0f;  // Convert to kW
    
    // Calculate power factor (simplified)
    if (*v_rms > 0 && *i_rms > 0) {
        *pf = *p_kw * 1000.0f / (*v_rms * *i_rms);
        if (*pf > 1.0f) *pf = 1.0f;
    } else {
        *pf = 0.0f;
    }
    
    // Calculate THD (simplified - would need FFT for real THD)
    // For now, use a small random variation to simulate THD
    static uint32_t thd_counter = 0;
    thd_counter++;
    *thd_v = 2.0f + (thd_counter % 100) / 100.0f;  // 2-3% THD
    *thd_i = 3.0f + (thd_counter % 150) / 100.0f;  // 3-4.5% THD
    
    return ESP_OK;
}

esp_err_t ina226_calibrate(float shunt_resistance, float max_current)
{
    // Calculate calibration value
    // Calibration = 0.00512 / (Current_LSB * R_SHUNT)
    // Current_LSB = Max_Current / 2^15
    
    current_lsb = max_current / 32768.0f;  // 2^15
    calibration_value = (uint16_t)(0.00512f / (current_lsb * shunt_resistance));
    
    // Update power LSB
    power_lsb = current_lsb * 25.0f;  // Power LSB = Current_LSB * 25
    
    ESP_LOGI(TAG, "Calibration: shunt=%.3f ohm, max_current=%.3f A, cal=0x%04x", 
             shunt_resistance, max_current, calibration_value);
    
    return ina226_write_register(INA226_REG_CALIBRATION, calibration_value);
}

esp_err_t ina226_reset(void)
{
    return ina226_write_register(INA226_REG_CONFIG, INA226_CONFIG_RESET);
}

esp_err_t ina226_get_manufacturer_id(uint16_t *id)
{
    return ina226_read_register(INA226_REG_MANUFACTURERID, id);
}

esp_err_t ina226_get_die_id(uint16_t *id)
{
    return ina226_read_register(INA226_REG_DIEID, id);
}
