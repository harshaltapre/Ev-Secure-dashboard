/*
 * EV-Secure ESP32-S3 Complete System
 * Author: AI Assistant
 * Date: 2024
 * 
 * Complete Arduino-compatible ESP32-S3 project for EV charging station security monitoring
 * 
 * Features:
 * - Voltage and Current Sensor Integration (I²C/ADC)
 * - 1.8" TFT Display (SPI) for real-time status
 * - SD Card Module (SPI) for data logging
 * - Relay/Contactor Control (GPIO) for power cutoff
 * - Machine Learning Model (TensorFlow Lite Micro) for threat detection
 * - Wi-Fi Communication with Dashboard API
 * - Real-time anomaly detection and alerting
 * - Remote control capabilities
 * 
 * Hardware Requirements:
 * - ESP32-S3 Development Board
 * - ACS712 Current Sensor (30A) or INA226 Energy Meter
 * - ZMPT101B Voltage Sensor or Voltage Divider
 * - 1.8" TFT Display (ST7735/ST7789)
 * - MicroSD Card Module
 * - Relay Module for Contactor Control
 * - Status LED and Buzzer
 * 
 * Pin Configuration (ESP32-S3):
 * - Current Sensor: GPIO1 (ADC1_CH0)
 * - Voltage Sensor: GPIO2 (ADC1_CH1)
 * - SD Card: MOSI=GPIO11, MISO=GPIO13, SCK=GPIO12, CS=GPIO10
 * - TFT Display: MOSI=GPIO35, MISO=GPIO37, SCK=GPIO36, CS=GPIO34, DC=GPIO14, RST=GPIO15
 * - Relay Control: GPIO18
 * - Status LED: GPIO2
 * - Buzzer: GPIO4
 * - Emergency Stop Button: GPIO16
 */

#include "EV_Secure_Config.h"
#include "SensorManager.h"
#include "DisplayManager.h"
#include "SDLogger.h"
#include "APIManager.h"
#include "RelayController.h"
#include "MLModel.h"

// Global Variables
SensorData currentSensorData;
MLPrediction mlResult;
bool isCharging = false;
bool emergencyStop = false;
bool threatDetected = false;
unsigned long lastDataTransmission = 0;
unsigned long lastMLInference = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long sessionStartTime = 0;
String sessionId = "";

// System State
enum SystemState {
  STATE_IDLE,
  STATE_HANDSHAKE,
  STATE_CHARGING,
  STATE_SUSPICIOUS,
  STATE_LOCKDOWN,
  STATE_ERROR
};

SystemState currentState = STATE_IDLE;

// Function Prototypes
void setupSystem();
void initializePeripherals();
void generateSessionId();
void updateSystemState(SystemState newState);
void handleEmergencyStop();
void processMLInference();
void updateDisplay();
void logToSD();
void sendToDashboard();
void checkDashboardCommands();
void handleThreatDetection();
void controlRelay(bool enable);

void setup() {
  Serial.begin(115200);
  Serial.println("EV-Secure ESP32-S3 System Starting...");
  Serial.println("Version: 1.0.0");
  Serial.println("Device ID: " + String(DEVICE_ID));
  
  // Initialize system
  setupSystem();
  
  // Initialize all peripherals
  initializePeripherals();
  
  // Generate initial session ID
  generateSessionId();
  
  Serial.println("EV-Secure System Initialized Successfully!");
  Serial.println("Monitoring charging station for threats...");
  
  // Set initial state
  updateSystemState(STATE_IDLE);
}

void loop() {
  unsigned long currentTime = millis();
  
  // Read sensor data continuously
  readSensors();
  
  // Run ML inference every 1 second
  if (currentTime - lastMLInference >= 1000) {
    processMLInference();
    lastMLInference = currentTime;
  }
  
  // Update display every 500ms
  if (currentTime - lastDisplayUpdate >= 500) {
    updateDisplay();
    lastDisplayUpdate = currentTime;
  }
  
  // Log to SD card every 5 seconds
  if (currentTime % 5000 == 0) {
    logToSD();
  }
  
  // Send data to dashboard every 2 seconds
  if (currentTime - lastDataTransmission >= 2000) {
    sendToDashboard();
    checkDashboardCommands();
    lastDataTransmission = currentTime;
  }
  
  // Check for emergency stop button
  handleEmergencyStop();
  
  // Handle threat detection
  if (threatDetected) {
    handleThreatDetection();
  }
  
  // Small delay to prevent watchdog issues
  delay(10);
}

void setupSystem() {
  // Initialize GPIO pins
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);
  pinMode(RELAY_CONTROL_PIN, OUTPUT);
  
  // Set initial states
  digitalWrite(STATUS_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY_CONTROL_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  
  // Initialize WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected Successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed!");
    updateSystemState(STATE_ERROR);
  }
}

void initializePeripherals() {
  Serial.println("Initializing Peripherals...");
  
  // Initialize sensors
  if (SensorManager::init()) {
    Serial.println("✓ Sensors initialized");
  } else {
    Serial.println("✗ Sensor initialization failed");
    updateSystemState(STATE_ERROR);
  }
  
  // Initialize TFT display
  if (DisplayManager::init()) {
    Serial.println("✓ TFT Display initialized");
    DisplayManager::showStartupScreen();
  } else {
    Serial.println("✗ TFT Display initialization failed");
  }
  
  // Initialize SD card
  if (SDLogger::init()) {
    Serial.println("✓ SD Card initialized");
    SDLogger::logSystemEvent("System initialized");
  } else {
    Serial.println("✗ SD Card initialization failed");
  }
  
  // Initialize relay controller
  if (RelayController::init()) {
    Serial.println("✓ Relay Controller initialized");
  } else {
    Serial.println("✗ Relay Controller initialization failed");
  }
  
  // Initialize ML model
  if (MLModel::init()) {
    Serial.println("✓ ML Model loaded");
  } else {
    Serial.println("✗ ML Model initialization failed");
  }
  
  Serial.println("Peripheral initialization complete!");
}

void readSensors() {
  // Read current sensor
  currentSensorData.current = SensorManager::readCurrent();
  
  // Read voltage sensor
  currentSensorData.voltage = SensorManager::readVoltage();
  
  // Calculate power
  currentSensorData.power = currentSensorData.current * currentSensorData.voltage;
  
  // Calculate frequency (simplified)
  currentSensorData.frequency = SensorManager::readFrequency();
  
  // Read temperature
  currentSensorData.temperature = SensorManager::readTemperature();
  
  // Check if charging has started/stopped
  bool wasCharging = isCharging;
  isCharging = (abs(currentSensorData.current) > CHARGING_THRESHOLD);
  
  if (isCharging && !wasCharging) {
    // Charging started
    generateSessionId();
    updateSystemState(STATE_HANDSHAKE);
    Serial.println("Charging session started - ID: " + sessionId);
    SDLogger::logSystemEvent("Charging started - Session: " + sessionId);
  } else if (!isCharging && wasCharging) {
    // Charging stopped
    updateSystemState(STATE_IDLE);
    Serial.println("Charging session ended");
    SDLogger::logSystemEvent("Charging ended - Session: " + sessionId);
  }
}

void processMLInference() {
  if (!isCharging) {
    mlResult.prediction = 0; // Normal when not charging
    mlResult.confidence = 1.0;
    return;
  }
  
  // Prepare input features for ML model
  float inputFeatures[INPUT_FEATURES] = {
    currentSensorData.current,
    currentSensorData.voltage,
    currentSensorData.power,
    currentSensorData.frequency,
    currentSensorData.temperature,
    (float)currentState
  };
  
  // Run ML inference
  if (MLModel::runInference(inputFeatures, &mlResult)) {
    // Check if threat detected
    threatDetected = (mlResult.prediction > THREAT_THRESHOLD);
    
    if (threatDetected) {
      Serial.println("THREAT DETECTED! Confidence: " + String(mlResult.confidence));
      SDLogger::logThreatDetection(mlResult);
    }
  } else {
    Serial.println("ML inference failed");
    mlResult.prediction = 0;
    mlResult.confidence = 0.0;
  }
}

void updateDisplay() {
  DisplayManager::updateDisplay(
    currentSensorData,
    mlResult,
    currentState,
    isCharging,
    threatDetected,
    sessionId
  );
}

void logToSD() {
  SDLogger::logSensorData(currentSensorData);
  SDLogger::logMLPrediction(mlResult);
  SDLogger::logSystemState(currentState);
}

void sendToDashboard() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected - cannot send data");
    return;
  }
  
  // Create JSON payload
  DynamicJsonDocument doc(1024);
  doc["device_id"] = DEVICE_ID;
  doc["session_id"] = sessionId;
  doc["timestamp"] = millis();
  doc["state"] = currentState;
  doc["is_charging"] = isCharging;
  doc["threat_detected"] = threatDetected;
  
  // Sensor data
  JsonObject sensors = doc.createNestedObject("sensors");
  sensors["current"] = currentSensorData.current;
  sensors["voltage"] = currentSensorData.voltage;
  sensors["power"] = currentSensorData.power;
  sensors["frequency"] = currentSensorData.frequency;
  sensors["temperature"] = currentSensorData.temperature;
  
  // ML prediction
  JsonObject ml = doc.createNestedObject("ml_prediction");
  ml["prediction"] = mlResult.prediction;
  ml["confidence"] = mlResult.confidence;
  ml["threat_level"] = threatDetected ? "HIGH" : "NORMAL";
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send to dashboard
  if (APIManager::sendData(jsonString)) {
    Serial.println("Data sent to dashboard successfully");
  } else {
    Serial.println("Failed to send data to dashboard");
  }
}

void checkDashboardCommands() {
  String command = APIManager::getCommand();
  
  if (command.length() > 0) {
    Serial.println("Received command: " + command);
    SDLogger::logSystemEvent("Command received: " + command);
    
    if (command == "STOP" || command == "EMERGENCY_STOP") {
      emergencyStop = true;
      updateSystemState(STATE_LOCKDOWN);
      controlRelay(false); // Turn off relay
      Serial.println("EMERGENCY STOP COMMAND RECEIVED!");
    } else if (command == "START") {
      emergencyStop = false;
      updateSystemState(STATE_IDLE);
      controlRelay(true); // Turn on relay
      Serial.println("START COMMAND RECEIVED!");
    } else if (command == "RESET") {
      ESP.restart();
    }
  }
}

void handleEmergencyStop() {
  if (digitalRead(EMERGENCY_STOP_PIN) == LOW) {
    if (!emergencyStop) {
      emergencyStop = true;
      updateSystemState(STATE_LOCKDOWN);
      controlRelay(false);
      Serial.println("EMERGENCY STOP BUTTON PRESSED!");
      SDLogger::logSystemEvent("Emergency stop button pressed");
      
      // Sound buzzer
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
}

void handleThreatDetection() {
  if (threatDetected) {
    if (currentState != STATE_LOCKDOWN) {
      updateSystemState(STATE_SUSPICIOUS);
      
      // Sound buzzer
      digitalWrite(BUZZER_PIN, HIGH);
      delay(500);
      digitalWrite(BUZZER_PIN, LOW);
      
      // Send alert to dashboard
      APIManager::sendAlert("THREAT_DETECTED", "Suspicious charging pattern detected");
      
      // If confidence is very high, go to lockdown
      if (mlResult.confidence > CRITICAL_THRESHOLD) {
        updateSystemState(STATE_LOCKDOWN);
        controlRelay(false);
        Serial.println("CRITICAL THREAT - SYSTEM LOCKDOWN!");
      }
    }
  }
}

void controlRelay(bool enable) {
  if (enable) {
    digitalWrite(RELAY_CONTROL_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);
    Serial.println("Relay ON - Power enabled");
  } else {
    digitalWrite(RELAY_CONTROL_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
    Serial.println("Relay OFF - Power disabled");
  }
}

void generateSessionId() {
  sessionId = "SESS_" + String(millis()) + "_" + String(random(1000, 9999));
  sessionStartTime = millis();
}

void updateSystemState(SystemState newState) {
  if (currentState != newState) {
    SystemState oldState = currentState;
    currentState = newState;
    
    Serial.println("State change: " + String(oldState) + " -> " + String(newState));
    SDLogger::logSystemEvent("State change: " + String(oldState) + " -> " + String(newState));
    
    // Update status LED based on state
    switch (currentState) {
      case STATE_IDLE:
        digitalWrite(STATUS_LED_PIN, LOW);
        break;
      case STATE_HANDSHAKE:
        // Blink slowly
        break;
      case STATE_CHARGING:
        digitalWrite(STATUS_LED_PIN, HIGH);
        break;
      case STATE_SUSPICIOUS:
        // Blink rapidly
        break;
      case STATE_LOCKDOWN:
        // Solid red (if RGB LED)
        break;
      case STATE_ERROR:
        // Blink very fast
        break;
    }
  }
}
