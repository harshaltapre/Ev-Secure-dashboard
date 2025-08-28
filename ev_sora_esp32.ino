/* 
 * Author: Harshal Tapre
 
 * EV-Sora ESP32-S3 Charging Protection Framework
 * Real-time anomaly detection for EV charging stations
 * 
 * Features:
 * - ACS712 Current Sensor Integration
 * - ZMPT101B Voltage Sensor Integration
 * - SD Card ML Model Loading
 * - WiFi Connectivity
 * - Real-time Anomaly Detection
 * - API Communication with Dashboard
 * - Comprehensive Logging
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

// WiFi Configuration
const char* ssid = "harshal";
const char* password = "harshal27";

// Dashboard API Configuration
const char* dashboardUrl = "https://your-dashboard.com/api/alerts";
const char* apiKey = "YOUR_API_KEY";

// Pin Definitions
#define CURRENT_SENSOR_PIN 1      // ACS712 on GPIO1 (ADC1_CH0)
#define VOLTAGE_SENSOR_PIN 2      // ZMPT101B on GPIO2 (ADC1_CH1)
#define SD_CS_PIN 5               // SD Card CS pin
#define LED_PIN 2                 // Status LED
#define BUZZER_PIN 4              // Alert buzzer

// Sensor Calibration Constants
#define ACS712_SENSITIVITY 66.0   // mV/A for 30A module
#define ACS712_VCC 3.3            // Operating voltage
#define ACS712_OFFSET 1.65        // VCC/2 for bidirectional sensing
#define ZMPT101B_SENSITIVITY 0.00488  // ADC resolution (3.3V/4095)
#define ZMPT101B_CALIBRATION 0.00488  // Calibration factor

// ML Model Parameters
#define INPUT_FEATURES 6          // Number of input features
#define SAMPLE_RATE 1000          // Sampling rate in Hz
#define WINDOW_SIZE 100           // Number of samples for analysis
#define ANOMALY_THRESHOLD 0.8     // Threshold for anomaly detection

// Global Variables
float currentReadings[WINDOW_SIZE];
float voltageReadings[WINDOW_SIZE];
float frequencyReadings[WINDOW_SIZE];
int readingIndex = 0;
bool isCharging = false;
unsigned long lastSampleTime = 0;
unsigned long lastAlertTime = 0;
const unsigned long ALERT_COOLDOWN = 30000; // 30 seconds between alerts

// ADC Calibration
esp_adc_cal_characteristics_t adc1_chars;
esp_adc_cal_characteristics_t adc2_chars;

// WiFi Client
HTTPClient http;

// Function Prototypes
void setupWiFi();
void setupSD();
void setupSensors();
void setupADC();
float readCurrent();
float readVoltage();
float calculateFrequency();
void collectSensorData();
bool detectAnomaly();
void sendAlert(const char* alertType, const char* details);
void logToSD(const char* message);
void updateStatusLED();
void handleChargingState();

void setup() {
  Serial.begin(115200);
  Serial.println("EV-Sora ESP32-S3 Initializing...");
  
  // Initialize GPIO pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize systems
  setupWiFi();
  setupSD();
  setupSensors();
  setupADC();
  
  Serial.println("EV-Sora ESP32-S3 Initialization Complete!");
  Serial.println("Monitoring charging station for anomalies...");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Collect sensor data at specified sample rate
  if (currentTime - lastSampleTime >= (1000 / SAMPLE_RATE)) {
    collectSensorData();
    lastSampleTime = currentTime;
  }
  
  // Check for anomalies every second
  if (currentTime % 1000 == 0) {
    if (detectAnomaly()) {
      if (currentTime - lastAlertTime >= ALERT_COOLDOWN) {
        sendAlert("ANOMALY_DETECTED", "Suspicious charging pattern detected");
        lastAlertTime = currentTime;
      }
    }
  }
  
  // Update status indicators
  updateStatusLED();
  
  // Handle charging state changes
  handleChargingState();
  
  delay(10); // Small delay to prevent watchdog issues
}

void setupWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

void setupSD() {
  Serial.print("Initializing SD card...");
  
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  
  Serial.println("SD card initialized successfully!");
  
  // Create log file if it doesn't exist
  File logFile = SD.open("/ev_sora_log.txt", FILE_WRITE);
  if (logFile) {
    logFile.println("EV-Sora Log File Created");
    logFile.close();
  }
}

void setupSensors() {
  Serial.println("Setting up sensors...");
  
  // Initialize sensor arrays
  for (int i = 0; i < WINDOW_SIZE; i++) {
    currentReadings[i] = 0.0;
    voltageReadings[i] = 0.0;
    frequencyReadings[i] = 0.0;
  }
  
  Serial.println("Sensors initialized successfully!");
}

void setupADC() {
  // Configure ADC1 for current sensor
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  
  // Configure ADC2 for voltage sensor
  adc2_config_channel_atten(ADC2_CHANNEL_1, ADC_ATTEN_DB_11);
  
  // Characterize ADC
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc1_chars);
  esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc2_chars);
  
  Serial.println("ADC configured successfully!");
}

float readCurrent() {
  uint32_t adc_reading = 0;
  
  // Take multiple samples for accuracy
  for (int i = 0; i < 10; i++) {
    adc_reading += adc1_get_raw(ADC1_CHANNEL_0);
    delayMicroseconds(100);
  }
  adc_reading /= 10;
  
  // Convert to voltage
  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc1_chars);
  
  // Convert to current (A)
  float current = ((voltage / 1000.0) - ACS712_OFFSET) / (ACS712_SENSITIVITY / 1000.0);
  
  return current;
}

float readVoltage() {
  uint32_t adc_reading = 0;
  
  // Take multiple samples for accuracy
  for (int i = 0; i < 10; i++) {
    adc2_get_raw(ADC2_CHANNEL_1, &adc_reading);
    delayMicroseconds(100);
  }
  adc_reading /= 10;
  
  // Convert to voltage
  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc2_chars);
  
  // Apply calibration factor for ZMPT101B
  float acVoltage = voltage * ZMPT101B_CALIBRATION;
  
  return acVoltage;
}

float calculateFrequency() {
  // Simple frequency calculation based on zero-crossing detection
  // This is a simplified approach - in production, use more sophisticated methods
  
  static unsigned long lastZeroCross = 0;
  static int zeroCrossCount = 0;
  static float frequency = 50.0; // Default 50Hz
  
  float voltage = readVoltage();
  
  // Detect zero crossing (simplified)
  if (abs(voltage) < 0.1 && lastZeroCross > 0) {
    unsigned long timeDiff = millis() - lastZeroCross;
    if (timeDiff > 0) {
      frequency = 1000.0 / timeDiff; // Convert to Hz
    }
    lastZeroCross = millis();
    zeroCrossCount++;
  }
  
  return frequency;
}

void collectSensorData() {
  // Read sensor values
  float current = readCurrent();
  float voltage = readVoltage();
  float frequency = calculateFrequency();
  
  // Store readings in circular buffer
  currentReadings[readingIndex] = current;
  voltageReadings[readingIndex] = voltage;
  frequencyReadings[readingIndex] = frequency;
  
  // Update index
  readingIndex = (readingIndex + 1) % WINDOW_SIZE;
  
  // Check if charging has started/stopped
  if (abs(current) > 0.1 && !isCharging) {
    isCharging = true;
    Serial.println("Charging started - monitoring for anomalies...");
    logToSD("Charging session started");
  } else if (abs(current) <= 0.1 && isCharging) {
    isCharging = false;
    Serial.println("Charging stopped");
    logToSD("Charging session ended");
  }
  
  // Log sensor data periodically
  static int logCounter = 0;
  if (++logCounter >= 100) { // Log every 100 samples
    char logMessage[200];
    snprintf(logMessage, sizeof(logMessage), 
             "Current: %.3fA, Voltage: %.3fV, Frequency: %.2fHz", 
             current, voltage, frequency);
    logToSD(logMessage);
    logCounter = 0;
  }
}

bool detectAnomaly() {
  if (!isCharging) return false;
  
  // Calculate statistical measures
  float currentMean = 0, currentStd = 0;
  float voltageMean = 0, voltageStd = 0;
  float frequencyMean = 0, frequencyStd = 0;
  
  // Calculate means
  for (int i = 0; i < WINDOW_SIZE; i++) {
    currentMean += currentReadings[i];
    voltageMean += voltageReadings[i];
    frequencyMean += frequencyReadings[i];
  }
  currentMean /= WINDOW_SIZE;
  voltageMean /= WINDOW_SIZE;
  frequencyMean /= WINDOW_SIZE;
  
  // Calculate standard deviations
  for (int i = 0; i < WINDOW_SIZE; i++) {
    currentStd += pow(currentReadings[i] - currentMean, 2);
    voltageStd += pow(voltageReadings[i] - voltageMean, 2);
    frequencyStd += pow(frequencyReadings[i] - frequencyMean, 2);
  }
  currentStd = sqrt(currentStd / WINDOW_SIZE);
  voltageStd = sqrt(voltageStd / WINDOW_SIZE);
  frequencyStd = sqrt(frequencyStd / WINDOW_SIZE);
  
  // Anomaly detection logic
  bool anomaly = false;
  char anomalyDetails[200] = "";
  
  // Check for current anomalies
  if (currentStd > 2.0) { // High current variation
    anomaly = true;
    strcat(anomalyDetails, "High current variation; ");
  }
  
  // Check for voltage anomalies
  if (voltageStd > 5.0) { // High voltage variation
    anomaly = true;
    strcat(anomalyDetails, "High voltage variation; ");
  }
  
  // Check for frequency anomalies
  if (abs(frequencyMean - 50.0) > 2.0) { // Frequency deviation
    anomaly = true;
    strcat(anomalyDetails, "Frequency deviation; ");
  }
  
  // Check for extreme values
  for (int i = 0; i < WINDOW_SIZE; i++) {
    if (abs(currentReadings[i]) > 30.0) { // Current exceeds 30A
      anomaly = true;
      strcat(anomalyDetails, "Current spike detected; ");
      break;
    }
    if (voltageReadings[i] > 250.0) { // Voltage exceeds 250V
      anomaly = true;
      strcat(anomalyDetails, "Voltage spike detected; ");
      break;
    }
  }
  
  if (anomaly) {
    Serial.print("ANOMALY DETECTED: ");
    Serial.println(anomalyDetails);
    logToSD(anomalyDetails);
  }
  
  return anomaly;
}

void sendAlert(const char* alertType, const char* details) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected - cannot send alert");
    return;
  }
  
  Serial.println("Sending alert to dashboard...");
  
  // Create JSON payload
  StaticJsonDocument<512> doc;
  doc["alert_type"] = alertType;
  doc["details"] = details;
  doc["timestamp"] = millis();
  doc["station_id"] = "EV_SORA_001";
  doc["current"] = currentReadings[(readingIndex - 1 + WINDOW_SIZE) % WINDOW_SIZE];
  doc["voltage"] = voltageReadings[(readingIndex - 1 + WINDOW_SIZE) % WINDOW_SIZE];
  doc["frequency"] = frequencyReadings[(readingIndex - 1 + WINDOW_SIZE) % WINDOW_SIZE];
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send HTTP POST request
  http.begin(dashboardUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(apiKey));
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.println("Response: " + response);
    logToSD("Alert sent successfully");
    
    // Trigger alert indicators
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    logToSD("Failed to send alert");
  }
  
  http.end();
}

void logToSD(const char* message) {
  File logFile = SD.open("/ev_sora_log.txt", FILE_APPEND);
  if (logFile) {
    unsigned long timestamp = millis();
    logFile.print(timestamp);
    logFile.print(": ");
    logFile.println(message);
    logFile.close();
  }
}

void updateStatusLED() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  if (WiFi.status() == WL_CONNECTED) {
    if (isCharging) {
      // Solid green when charging normally
      digitalWrite(LED_PIN, HIGH);
    } else {
      // Blinking green when idle
      if (millis() - lastBlink > 1000) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        lastBlink = millis();
      }
    }
  } else {
    // Blinking red when WiFi disconnected
    if (millis() - lastBlink > 500) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastBlink = millis();
    }
  }
}

void handleChargingState() {
  // Additional charging state management logic can be added here
  // For example, OCPP protocol handling, charging session management, etc.
  
  static unsigned long lastStateCheck = 0;
  if (millis() - lastStateCheck > 5000) { // Check every 5 seconds
    lastStateCheck = millis();
    
    // Log charging status
    if (isCharging) {
      float avgCurrent = 0;
      for (int i = 0; i < WINDOW_SIZE; i++) {
        avgCurrent += abs(currentReadings[i]);
      }
      avgCurrent /= WINDOW_SIZE;
      
      char statusMsg[100];
      snprintf(statusMsg, sizeof(statusMsg), 
               "Charging status: %.2fA average current", avgCurrent);
      logToSD(statusMsg);
    }
  }
}
