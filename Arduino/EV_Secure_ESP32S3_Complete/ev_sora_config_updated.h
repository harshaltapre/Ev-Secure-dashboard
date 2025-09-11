/*
 * EV-Sora Configuration File - UPDATED FOR WORKING API
 * Customize these parameters according to your setup
 * 
 * IMPORTANT: Replace the placeholder values with your actual configuration
 */

#ifndef EV_SORA_CONFIG_UPDATED_H
#define EV_SORA_CONFIG_UPDATED_H

// ============================================================================
// WiFi Configuration
// ============================================================================
#define WIFI_SSID "YOUR_WIFI_SSID"                    // Replace with your WiFi name
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"            // Replace with your WiFi password
#define WIFI_TIMEOUT_MS 10000

// ============================================================================
// Dashboard API Configuration - WORKING ENDPOINTS
// ============================================================================
// Replace "your-domain.com" with your actual Next.js app domain
// For local development, use: "http://localhost:3000"
// For production, use your actual domain: "https://your-domain.com"
#define DASHBOARD_URL "http://localhost:3000"         // Change this to your actual URL
#define API_KEY "vsr_st001_abc123def456"             // Replace with your generated API key
#define API_TIMEOUT_MS 10000
#define ALERT_COOLDOWN_MS 30000  // 30 seconds between alerts

// ============================================================================
// API Endpoints - These are now working endpoints
// ============================================================================
#define API_DATA_ENDPOINT "/api/data"
#define API_COMMANDS_ENDPOINT "/api/commands"
#define API_ALERTS_ENDPOINT "/api/alerts"
#define API_STATUS_ENDPOINT "/api/status"

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
#define ACS712_OFFSET 1.65       // VCC/2 for bidirectional sensing
#define ACS712_MAX_CURRENT 30.0  // Maximum expected current

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
#define STATION_ID "ST001"                 // Unique station identifier (must match API key)
#define DEVICE_ID "EV_SORA_001"            // Device identifier
#define DEVICE_VERSION "1.0.0"             // Firmware version
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
#define SSL_ENABLED false                   // Set to false for HTTP, true for HTTPS
#define CERT_VERIFICATION false             // Set to false for development
#define API_RATE_LIMIT 10                  // Maximum API calls per minute

// ============================================================================
// HTTP Configuration
// ============================================================================
#define HTTP_OK 200
#define HTTP_CREATED 201
#define HTTP_BAD_REQUEST 400
#define HTTP_UNAUTHORIZED 401
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404
#define HTTP_INTERNAL_ERROR 500

// ============================================================================
// Rate Limiting
// ============================================================================
#define MAX_REQUESTS_PER_MINUTE 10
#define REQUEST_TIMEOUT_MS 10000
#define RETRY_ATTEMPTS 3
#define RETRY_DELAY_MS 1000

#endif // EV_SORA_CONFIG_UPDATED_H
