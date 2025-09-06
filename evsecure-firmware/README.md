# EVsecure-device Firmware

A complete ESP32-S3 firmware project for EV charger security monitoring and anomaly detection.

## Overview

EVsecure-device is an on-device security agent that monitors EV charging sessions for potential security threats and anomalies. It combines rule-based detection with machine learning (autoencoder) to identify suspicious behavior and can automatically control charging operations.

### Key Features

- **Real-time Power Monitoring**: Voltage, current, power factor, and THD measurements
- **OCPP Protocol Monitoring**: WebSocket-based OCPP message analysis
- **Machine Learning Anomaly Detection**: TFLite-Micro autoencoder for pattern recognition
- **Safety State Machine**: IDLE → HANDSHAKE → PRECHARGE → CHARGING → SUSPICIOUS → LOCKDOWN
- **Contactor Control**: Automatic charging interruption on critical anomalies
- **Local Logging**: MicroSD card storage with CSV and JSON formats
- **Remote Dashboard Integration**: HTTPS API for real-time monitoring
- **TFT Display UI**: 1.8" SPI display with status and manual controls
- **Secure Boot & Flash Encryption**: Hardware security features
- **OTA Updates**: Signed firmware updates with rollback capability

## Hardware Requirements

### ESP32-S3 DevKit
- ESP32-S3 with PSRAM (8-16MB)
- WiFi and Bluetooth
- Multiple SPI and I2C interfaces

### Sensors & Peripherals

#### Energy Meter (Choose One)
- **INA226/INA228** (I2C) - Recommended for simplicity
- **ATM90E32** (SPI) - Higher precision, more complex

#### Current Amplifier
- **INA240** (I2C) - High-side current sensing

#### Display & Controls
- **1.8" TFT Display** (SPI) - Status and controls
- **2x Push Buttons** - ACK and BYPASS functions

#### Storage & Communication
- **MicroSD Card** (SPI) - Local data logging
- **WiFi** (onboard) - Remote communication

#### Security & Safety
- **Contactor Driver** - Charging control
- **Tamper Switch** - Physical security
- **Temperature Sensor** - DS18B20 or TMP117
- **ATECC608A** (optional) - Secure element

## Pin Configuration

### I2C Bus (INA226 + INA240 + ATECC608A)
```
SDA: GPIO 21
SCL: GPIO 22
```

### SPI Bus 1 (MicroSD)
```
MOSI: GPIO 35
MISO: GPIO 37
SCLK: GPIO 36
CS:   GPIO 34
```

### SPI Bus 2 (TFT Display)
```
MOSI: GPIO 11
MISO: GPIO 13
SCLK: GPIO 12
CS:   GPIO 10
DC:   GPIO 14
RST:  GPIO 15
```

### Control & Safety
```
Contactor Control: GPIO 18
Contactor Feedback: GPIO 19
Tamper Switch: GPIO 20
Temperature Sensor: GPIO 23
ACK Button: GPIO 16
BYPASS Button: GPIO 17
```

## Building the Firmware

### Prerequisites

1. **ESP-IDF v5.0+**
   ```bash
   git clone --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   ./install.sh
   source export.sh
   ```

2. **Python Dependencies**
   ```bash
   pip install tensorflow pandas numpy scikit-learn matplotlib requests
   ```

### Build Steps

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd evsecure-firmware
   ```

2. **Configure the project**
   ```bash
   idf.py set-target esp32s3
   idf.py menuconfig
   ```

3. **Build the firmware**
   ```bash
   idf.py build
   ```

4. **Flash the device**
   ```bash
   idf.py flash
   ```

5. **Monitor output**
   ```bash
   idf.py monitor
   ```

### Configuration

Edit `main/evsecure_config.h` to configure:
- WiFi credentials
- API endpoints and keys
- Pin assignments
- Threshold values
- Device ID

## Machine Learning Model Training

### Generate Synthetic Data
```bash
python generate_synthetic_data.py --output sessions.csv --sessions 100 --anomaly-ratio 0.1
```

### Train Autoencoder Model
```bash
python train_autoencoder.py --data sessions.csv --output model.tflite --epochs 100
```

### Convert to C Array
The training script automatically generates `model_data.h` and `model_data.c` files.

## API Integration

### Dashboard API Endpoints

#### POST /ingest/features
Upload feature data from the device.

**Request Body:**
```json
{
  "ts": 1693400000,
  "device_id": "evsec-esp32-001",
  "session_id": "abc123",
  "v_rms": 232.5,
  "i_rms": 18.4,
  "p_kw": 4.1,
  "pf": 0.96,
  "thd_v": 2.3,
  "thd_i": 3.8,
  "dvdt": 1.2,
  "didt": 2.6,
  "ocpp": {
    "rate": 4.2,
    "remote_stop_cnt": 2,
    "malformed": 0,
    "out_of_seq": 1
  },
  "fw_hash_ok": true,
  "tamper": false,
  "temp_c": 42.1
}
```

**Headers:**
```
Content-Type: application/json
Authorization: ApiKey your_api_key_here
```

#### POST /ingest/alerts
Upload alert notifications.

**Request Body:**
```json
{
  "device_id": "evsec-esp32-001",
  "session_id": "abc123",
  "timestamp": 1693400000,
  "level": "WARNING",
  "score": 0.75
}
```

### Authentication
Use API key authentication with the header:
```
Authorization: ApiKey your_api_key_here
```

## Testing

### Test Harness
Use the provided test harness to simulate the device:

```bash
python test_harness.py --api-url https://your-dashboard.com --api-key your_key
```

### Attack Scenarios
The test harness supports various attack scenarios:
- `remote_stop` - Burst of remote stop commands
- `malformed` - Malformed OCPP messages
- `thd` - THD spike with reduced OCPP rate
- `tamper` - Physical tampering detection
- `firmware` - Firmware integrity issues

### Example Usage
```bash
# Start normal operation
python test_harness.py --api-url https://dashboard.example.com --api-key abc123

# Automatically trigger an attack after 10 seconds
python test_harness.py --api-url https://dashboard.example.com --api-key abc123 --auto-attack remote_stop
```

## Safety State Machine

The device operates through these states:

1. **IDLE** - Initial state, waiting for session
2. **HANDSHAKE** - OCPP connection established
3. **PRECHARGE** - Pre-charging checks
4. **CHARGING** - Normal charging operation
5. **SUSPICIOUS** - Anomaly detected, reduced current
6. **LOCKDOWN** - Critical anomaly, contactor opened

### Anomaly Detection

#### Rule-based Detection
- Remote stop burst (>3 in 60s): +0.6 score
- Malformed/out-of-sequence messages (>2 in 30s): +0.4 score
- THD spike with reduced OCPP rate: +0.5 score
- Tamper detection or firmware corruption: Immediate critical (1.0)

#### Machine Learning
- Autoencoder reconstruction error
- Normalized to [0,1] range
- Combined with rule-based score

#### Decision Logic
- Score < 0.5: Continue charging
- Score 0.5-0.8: Warning, reduce current to 70%
- Score ≥ 0.8: Critical, open contactor

## Security Features

### Secure Boot
Enable secure boot to prevent unauthorized firmware:
```bash
idf.py secure-boot
```

### Flash Encryption
Encrypt firmware to protect intellectual property:
```bash
idf.py flash-encryption
```

### Secure Element (ATECC608A)
Optional ATECC608A integration for:
- Secure key storage
- Digital signatures
- Hardware-based authentication

## OTA Updates

### Signed Updates
1. Sign the firmware with your private key
2. Upload to OTA server
3. Device verifies signature before update
4. Automatic rollback on failure

### Update Process
```bash
# Build and sign firmware
idf.py build
idf.py sign-firmware

# Upload to OTA server
# Device will automatically download and install
```

## Troubleshooting

### Common Issues

1. **WiFi Connection Failed**
   - Check credentials in `evsecure_config.h`
   - Verify network availability
   - Check signal strength

2. **Sensor Communication Errors**
   - Verify I2C/SPI connections
   - Check power supply
   - Confirm sensor addresses

3. **SD Card Issues**
   - Format card as FAT32
   - Check card compatibility
   - Verify SPI connections

4. **API Communication Errors**
   - Check API endpoint URL
   - Verify API key
   - Check network connectivity

### Debug Output
Enable debug logging:
```c
#define CONFIG_LOG_DEFAULT_LEVEL_DEBUG
```

Monitor serial output:
```bash
idf.py monitor
```

## Performance Optimization

### Memory Usage
- Model size: <100KB
- Inference time: <10ms
- PSRAM utilization: ~2MB

### Power Consumption
- Active mode: ~200mA
- Sleep mode: ~10mA
- Deep sleep: ~5μA

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Support

For support and questions:
- Create an issue on GitHub
- Check the documentation
- Review the troubleshooting guide

## Acknowledgments

- ESP-IDF team for the excellent framework
- TensorFlow team for TFLite-Micro
- OCPP community for protocol specifications
