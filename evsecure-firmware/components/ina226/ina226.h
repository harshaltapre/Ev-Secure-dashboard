#ifndef INA226_H
#define INA226_H

#include "esp_err.h"
#include "driver/i2c.h"
#include "evsecure_config.h"

// INA226 Register Addresses
#define INA226_REG_CONFIG          0x00
#define INA226_REG_SHUNTVOLTAGE    0x01
#define INA226_REG_BUSVOLTAGE      0x02
#define INA226_REG_POWER           0x03
#define INA226_REG_CURRENT         0x04
#define INA226_REG_CALIBRATION     0x05
#define INA226_REG_MASKENABLE      0x06
#define INA226_REG_POWERLIMIT      0x07
#define INA226_REG_MANUFACTURERID  0xFE
#define INA226_REG_DIEID           0xFF

// Configuration Register Bits
#define INA226_CONFIG_RESET        0x8000
#define INA226_CONFIG_AVG_MASK     0x0E00
#define INA226_CONFIG_AVG_1        0x0000
#define INA226_CONFIG_AVG_4        0x0200
#define INA226_CONFIG_AVG_16       0x0400
#define INA226_CONFIG_AVG_64       0x0600
#define INA226_CONFIG_AVG_128      0x0800
#define INA226_CONFIG_AVG_256      0x0A00
#define INA226_CONFIG_AVG_512      0x0C00
#define INA226_CONFIG_AVG_1024     0x0E00

#define INA226_CONFIG_VBUSCT_MASK  0x01C0
#define INA226_CONFIG_VBUSCT_140US 0x0000
#define INA226_CONFIG_VBUSCT_204US 0x0040
#define INA226_CONFIG_VBUSCT_332US 0x0080
#define INA226_CONFIG_VBUSCT_588US 0x00C0
#define INA226_CONFIG_VBUSCT_1100US 0x0100
#define INA226_CONFIG_VBUSCT_2116US 0x0140
#define INA226_CONFIG_VBUSCT_4156US 0x0180
#define INA226_CONFIG_VBUSCT_8244US 0x01C0

#define INA226_CONFIG_VSHCT_MASK   0x0038
#define INA226_CONFIG_VSHCT_140US  0x0000
#define INA226_CONFIG_VSHCT_204US  0x0008
#define INA226_CONFIG_VSHCT_332US  0x0010
#define INA226_CONFIG_VSHCT_588US  0x0018
#define INA226_CONFIG_VSHCT_1100US 0x0020
#define INA226_CONFIG_VSHCT_2116US 0x0028
#define INA226_CONFIG_VSHCT_4156US 0x0030
#define INA226_CONFIG_VSHCT_8244US 0x0038

#define INA226_CONFIG_MODE_MASK    0x0007
#define INA226_CONFIG_MODE_POWERDOWN 0x0000
#define INA226_CONFIG_MODE_SHUNTT 0x0001
#define INA226_CONFIG_MODE_BUSV   0x0002
#define INA226_CONFIG_MODE_CONTINUOUS 0x0007

// Default Configuration
#define INA226_CONFIG_DEFAULT (INA226_CONFIG_AVG_64 | INA226_CONFIG_VBUSCT_1100US | \
                               INA226_CONFIG_VSHCT_1100US | INA226_CONFIG_MODE_CONTINUOUS)

// Calibration Constants
#define INA226_CURRENT_LSB 0.1f  // mA per LSB
#define INA226_POWER_LSB 2.5f    // mW per LSB
#define INA226_VOLTAGE_LSB 1.25f // mV per LSB

// Function declarations
esp_err_t ina226_init(void);
esp_err_t ina226_read_register(uint8_t reg, uint16_t *value);
esp_err_t ina226_write_register(uint8_t reg, uint16_t value);
esp_err_t ina226_read_measurements(float *v_rms, float *i_rms, float *p_kw, 
                                  float *pf, float *thd_v, float *thd_i);
esp_err_t ina226_calibrate(float shunt_resistance, float max_current);
esp_err_t ina226_reset(void);
esp_err_t ina226_get_manufacturer_id(uint16_t *id);
esp_err_t ina226_get_die_id(uint16_t *id);

#endif // INA226_H
