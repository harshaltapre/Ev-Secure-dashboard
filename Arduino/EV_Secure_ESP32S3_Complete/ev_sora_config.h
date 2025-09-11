/*
 * EV-Sora Configuration File
 * Customize these parameters according to your setup
 */

#ifndef EV_SORA_CONFIG_H
#define EV_SORA_CONFIG_H

// ============================================================================
// WiFi Configuration
// ============================================================================
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define WIFI_TIMEOUT_MS 10000

// ============================================================================
// Dashboard API Configuration
// ============================================================================
#define DASHBOARD_URL "https://your-dashboard.com/api/alerts"
#define API_KEY "YOUR_API_KEY"
#define API_TIMEOUT_MS 10000
#define ALERT_COOLDOWN_MS 30000  // 30 seconds between alerts

// ============================================================================
// Hardware Pin Configuration
// ============================================================================
// Sensor Pins
#define CURRENT_SENSOR_PIN 1      // ACS712 on GPIO1 (ADC1_CH0)
#define VOLTAGE_SENSOR_PIN 2      // ZMPT101B on GPIO2 (ADC1_CH1)
#define SD_CS_PIN 5               // SD Card CS pin
#define LED_PIN 2                 // Status LED
#define BUZZER_PIN 4              // Alert buzzer

// ============================================================================
// Sensor Calibration Constants
// ============================================================================
// ACS712 Current Sensor (30A Module)
#define ACS712_SENSITIVITY 66.0   // mV/A for 30A module
#define ACS712_VCC 3.3            // Operating voltage
#define ACS712_OFFSET 1.65        // VCC/2 for bidirectional sensing
#define ACS712_MAX_CURRENT 30.0   // Maximum expected current

// ZMPT101B Voltage Sensor
#define ZMPT101B_SENSITIVITY 0.00488  // ADC resolution (3.3V/4095)
#define ZMPT101B_CALIBRATION 0.00488  // Calibration factor
#define ZMPT101B_MAX_VOLTAGE 250.0    // Maximum expected voltage

// ============================================================================
// ML Model Parameters
// ============================================================================
#define INPUT_FEATURES 6          // Number of input features
#define SAMPLE_RATE 1000          // Sampling rate in Hz
#define WINDOW_SIZE 100           // Number of samples for analysis
#define ANOMALY_THRESHOLD 0.8     // Threshold for anomaly detection

// ============================================================================
// Anomaly Detection Thresholds
// ============================================================================
// Current Anomalies
#define CURRENT_VARIATION_THRESHOLD 2.0    // High current variation threshold
#define CURRENT_SPIKE_THRESHOLD 30.0       // Current spike threshold

// Voltage Anomalies
#define VOLTAGE_VARIATION_THRESHOLD 5.0    // High voltage variation threshold
#define VOLTAGE_SPIKE_THRESHOLD 250.0      // Voltage spike threshold

// Frequency Anomalies
#define FREQUENCY_DEVIATION_THRESHOLD 2.0  // Frequency deviation threshold
#define NOMINAL_FREQUENCY 50.0             // Nominal frequency (50Hz for EU, 60Hz for US)

// ============================================================================
// System Configuration
// ============================================================================
#define STATION_ID "EV_SORA_001"           // Unique station identifier
#define LOG_INTERVAL 100                   // Log every N samples
#define STATUS_CHECK_INTERVAL 5000         // Status check interval (ms)
#define ADC_SAMPLES 10                     // Number of ADC samples for averaging
#define ADC_DELAY_MICROS 100               // Delay between ADC samples

// ============================================================================
// Debug Configuration
// ============================================================================
#define DEBUG_MODE true                     // Enable debug output
#define SERIAL_BAUD_RATE 115200            // Serial communication baud rate
#define LOG_LEVEL 2                        // 0=Errors only, 1=Warnings, 2=Info, 3=Debug

// ============================================================================
// OCPP Protocol Configuration (Future Enhancement)
// ============================================================================
#define OCPP_ENABLED false                 // Enable OCPP protocol support
#define OCPP_SERVER_URL "ws://your-ocpp-server.com"
#define OCPP_CHARGE_POINT_ID "EV_SORA_001"

// ============================================================================
// Security Configuration
// ============================================================================
#define SSL_ENABLED true                   // Enable SSL/TLS for secure communication
#define CERT_VERIFICATION true             // Verify SSL certificates
#define API_RATE_LIMIT 10                 // Maximum API calls per minute

#endif // EV_SORA_CONFIG_H
