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
 * Pin Configuration (ESP32-S3) - Updated per Hardware Wiring Guide:
 * - Current Sensor: GPIO1 (ADC1_CH0)
 * - Voltage Sensor: GPIO2 (ADC1_CH1)
 * - Temperature Sensor: GPIO3 (OneWire)
 * - SD Card: MOSI=GPIO11, MISO=GPIO13, SCK=GPIO12, CS=GPIO10
 * - TFT Display: MOSI=GPIO35, MISO=not used, SCK=GPIO36, CS=GPIO10, DC=GPIO14, RST=GPIO15, BL=GPIO5
 * - RTC Module: SDA=GPIO21, SCL=GPIO47 (I2C)
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
#include "EnhancedMLModel.h"
#include "AdvancedThreatDetection.h"

// Global Variables
SensorData currentSensorData;
MLPrediction mlResult;
EnhancedMLPrediction enhancedMLResult;  // Enhanced ML prediction
bool isCharging = false;
bool emergencyStop = false;
bool threatDetected = false;
unsigned long lastDataTransmission = 0;
unsigned long lastMLInference = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long sessionStartTime = 0;
String sessionId = "";

// System State (defined in EV_Secure_Config.h)
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
void reconnectWiFi();
void scanWiFiNetworks();

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
  
  // Show startup screen immediately so display is never blank
  if (DisplayManager::isInitialized()) {
    DisplayManager::showStartupScreen();
  }
  
  Serial.println("EV-Secure System Initialized Successfully!");
  Serial.println("Monitoring charging station for threats...");
  
  // Set initial state
  updateSystemState(STATE_IDLE);
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check WiFi connection health
  static unsigned long lastWiFiCheck = 0;
  if (currentTime - lastWiFiCheck > 30000) { // Check every 30 seconds
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost - attempting reconnection...");
      reconnectWiFi();
    }
    lastWiFiCheck = currentTime;
  }
  
  // Read sensor data continuously
  readSensors();

  // Update state to CHARGING if current detected
  if (isCharging && currentState != STATE_CHARGING) {
    updateSystemState(STATE_CHARGING);
  }
  
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
  
  // Scan for available WiFi networks first
  scanWiFiNetworks();
  
  // Initialize WiFi with improved connection logic
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  Serial.print("Password: ");
  Serial.println(WIFI_PASSWORD);
  
  // Disconnect any existing connection
  WiFi.disconnect(true);
  delay(1000);
  
  // Set WiFi mode
  WiFi.mode(WIFI_STA);
  
  // Begin connection (non-blocking). We'll monitor and reconnect in loop()
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Started WiFi connection in background.");
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
    SDLogger::logSystemEvent(String("System initialized"));
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
  
  // Initialize Enhanced ML model
  if (EnhancedMLModel::init()) {
    Serial.println("✓ Enhanced ML Model loaded");
  } else {
    Serial.println("✗ Enhanced ML Model initialization failed");
  }
  
  // Initialize Advanced Threat Detection
  if (AdvancedThreatDetection::init()) {
    Serial.println("✓ Advanced Threat Detection initialized");
  } else {
    Serial.println("✗ Advanced Threat Detection initialization failed");
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
    SDLogger::logSystemEvent(String("Charging started - Session: " + sessionId));
  } else if (!isCharging && wasCharging) {
    // Charging stopped
    updateSystemState(STATE_IDLE);
    Serial.println("Charging session ended");
    SDLogger::logSystemEvent(String("Charging ended - Session: " + sessionId));
  }
}

void processMLInference() {
  if (!isCharging) {
    mlResult.prediction = 0; // Normal when not charging
    mlResult.confidence = 1.0;
    enhancedMLResult.prediction = 0;
    enhancedMLResult.confidence = 1.0;
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
  
  // Run standard ML inference
  if (MLModel::runInference(inputFeatures, &mlResult)) {
    // Run Enhanced ML inference for advanced threat detection
    enhancedMLResult = EnhancedMLModel::predictAdvanced(currentSensorData);
    
    // Use the higher threat score between standard and enhanced ML
    float finalThreatScore = max(mlResult.prediction, enhancedMLResult.prediction);
    float finalConfidence = (mlResult.confidence + enhancedMLResult.confidence) / 2.0;
    
    // Check if threat detected using enhanced analysis
    bool standardThreat = (mlResult.prediction > THREAT_THRESHOLD);
    bool enhancedThreat = (enhancedMLResult.prediction > THREAT_THRESHOLD);
    threatDetected = standardThreat || enhancedThreat;
    
    if (threatDetected) {
      Serial.println("THREAT DETECTED! Standard: " + String(mlResult.prediction) + 
                     ", Enhanced: " + String(enhancedMLResult.prediction) + 
                     ", Confidence: " + String(finalConfidence));
      Serial.println("Attack Type: " + AdvancedThreatDetection::getAttackDescription(enhancedMLResult.attack_type));
      SDLogger::logThreatDetection(mlResult);
      
      // Log enhanced threat details
      String enhancedDetails = "Enhanced ML - Attack: " + AdvancedThreatDetection::getAttackDescription(enhancedMLResult.attack_type) +
                              ", Confidence: " + String(enhancedMLResult.confidence) +
                              ", Uncertainty: " + String(enhancedMLResult.uncertainty);
      SDLogger::logAlert("ENHANCED_THREAT", enhancedDetails);
    }
  } else {
    Serial.println("ML inference failed");
    mlResult.prediction = 0;
    mlResult.confidence = 0.0;
    enhancedMLResult.prediction = 0;
    enhancedMLResult.confidence = 0.0;
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
    Serial.println("WiFi not connected - attempting reconnection...");
    reconnectWiFi();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi reconnection failed - cannot send data");
      return;
    }
  }
  
  // Create JSON payload (match Next.js API schema exactly)
  DynamicJsonDocument doc(1024);
  doc["device_id"] = DEVICE_ID;
  doc["session_id"] = sessionId;
  doc["timestamp"] = millis();
  doc["state"] = currentState;
  doc["is_charging"] = isCharging;
  doc["threat_detected"] = threatDetected;

  // Sensor data (as required by API)
  JsonObject sensors = doc.createNestedObject("sensor_data");
  sensors["current"] = currentSensorData.current;
  sensors["voltage"] = currentSensorData.voltage;
  sensors["power"] = currentSensorData.power;
  sensors["frequency"] = currentSensorData.frequency;
  sensors["temperature"] = currentSensorData.temperature;
  sensors["timestamp"] = currentSensorData.timestamp;

  // Additional system data
  JsonObject system = doc.createNestedObject("system_data");
  system["wifi_rssi"] = WiFi.RSSI();
  system["uptime"] = millis();
  system["free_heap"] = ESP.getFreeHeap();
  system["cpu_freq"] = ESP.getCpuFreqMHz();

  // ML prediction data (enhanced)
  JsonObject ml = doc.createNestedObject("ml_prediction");
  ml["standard_prediction"] = mlResult.prediction;
  ml["standard_confidence"] = mlResult.confidence;
  ml["enhanced_prediction"] = enhancedMLResult.prediction;
  ml["enhanced_confidence"] = enhancedMLResult.confidence;
  ml["enhanced_uncertainty"] = enhancedMLResult.uncertainty;
  ml["attack_type"] = enhancedMLResult.attack_type;
  ml["attack_confidence"] = enhancedMLResult.attack_confidence;
  ml["is_anomaly"] = enhancedMLResult.is_anomaly;
  ml["threat_level"] = threatDetected ? "HIGH" : "NORMAL";
  ml["timestamp"] = mlResult.timestamp;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Ensure API manager is initialized and server reachable
  static bool apiInitDone = false;
  if (!apiInitDone) {
    apiInitDone = APIManager::init();
    if (apiInitDone) {
      Serial.println("API Manager initialized successfully");
    } else {
      Serial.println("API Manager initialization failed");
      return;
    }
  }

  // Send to dashboard
  if (APIManager::sendData(jsonString)) {
    Serial.println("Data sent to dashboard successfully");
    Serial.println("JSON Payload: " + jsonString);
  } else {
    Serial.println("Failed to send data to dashboard");
    Serial.println("Last API Error: " + APIManager::getLastError());
  }
}

void checkDashboardCommands() {
  String commandJson = APIManager::getCommand();
  
  if (commandJson.length() > 0) {
    Serial.println("Received command JSON: " + commandJson);
    SDLogger::logSystemEvent(String("Command received: " + commandJson));
    
    // Parse JSON command
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, commandJson);
    
    if (error) {
      Serial.println("Failed to parse command JSON: " + String(error.c_str()));
      return;
    }
    
    String command = doc["command"];
    String commandId = doc["id"];
    
    Serial.println("Processing command: " + command + " (ID: " + commandId + ")");
    
    if (command == "STOP" || command == "EMERGENCY_STOP") {
      emergencyStop = true;
      updateSystemState(STATE_LOCKDOWN);
      controlRelay(false); // Turn off relay
      Serial.println("EMERGENCY STOP COMMAND RECEIVED!");
      APIManager::sendAlert("COMMAND_EXECUTED", "Emergency stop command executed");
    } else if (command == "START") {
      emergencyStop = false;
      updateSystemState(STATE_IDLE);
      controlRelay(true); // Turn on relay
      Serial.println("START COMMAND RECEIVED!");
      APIManager::sendAlert("COMMAND_EXECUTED", "Start command executed");
    } else if (command == "RESET") {
      Serial.println("RESET COMMAND RECEIVED!");
      APIManager::sendAlert("COMMAND_EXECUTED", "Reset command executed");
      delay(1000); // Give time for alert to be sent
      ESP.restart();
    } else if (command == "CALIBRATE") {
      Serial.println("CALIBRATE COMMAND RECEIVED!");
      SensorManager::calibrateSensors();
      APIManager::sendAlert("COMMAND_EXECUTED", "Calibration command executed");
    } else {
      Serial.println("Unknown command: " + command);
      APIManager::sendAlert("COMMAND_ERROR", "Unknown command: " + command);
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
      SDLogger::logSystemEvent(String("Emergency stop button pressed"));
      
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
      
      // Send enhanced alert to dashboard
      String alertDetails = "Standard ML: " + String(mlResult.prediction) + 
                           ", Enhanced ML: " + String(enhancedMLResult.prediction) +
                           ", Attack: " + AdvancedThreatDetection::getAttackDescription(enhancedMLResult.attack_type);
      APIManager::sendAlert("ADVANCED_THREAT_DETECTED", alertDetails);
      
      // Enhanced threat evaluation
      float combinedConfidence = (mlResult.confidence + enhancedMLResult.confidence) / 2.0;
      bool criticalThreat = (mlResult.confidence > CRITICAL_THRESHOLD) || 
                           (enhancedMLResult.confidence > CRITICAL_THRESHOLD) ||
                           (enhancedMLResult.attack_type == ATTACK_PHYSICAL_TAMPERING) ||
                           (enhancedMLResult.attack_type == ATTACK_LOAD_DUMPING);
      
      if (criticalThreat) {
        updateSystemState(STATE_LOCKDOWN);
        controlRelay(false);
        Serial.println("CRITICAL THREAT DETECTED - SYSTEM LOCKDOWN!");
        Serial.println("Attack Type: " + AdvancedThreatDetection::getAttackDescription(enhancedMLResult.attack_type));
        
        // Send critical alert
        APIManager::sendAlert("CRITICAL_THREAT_LOCKDOWN", 
                             "Critical threat detected - System locked down. Attack: " + 
                             AdvancedThreatDetection::getAttackDescription(enhancedMLResult.attack_type));
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
    SDLogger::logSystemEvent(String("State change: " + String(oldState) + " -> " + String(newState)));
    
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

void reconnectWiFi() {
  Serial.println("Attempting WiFi reconnection...");
  
  // Disconnect and reconnect
  WiFi.disconnect();
  delay(200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Non-blocking: allow loop to continue; we'll check status on next loop
}

void scanWiFiNetworks() {
  Serial.println("Scanning for available WiFi networks...");
  Serial.println("This may take a few seconds...");
  
  // Set WiFi mode to scan
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  // Scan for networks
  int n = WiFi.scanNetworks();
  
  if (n == 0) {
    Serial.println("No networks found!");
  } else {
    Serial.print("Found ");
    Serial.print(n);
    Serial.println(" networks:");
    Serial.println("----------------------------------------");
    
    for (int i = 0; i < n; i++) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(" dBm) ");
      
      // Check encryption type
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:
          Serial.println("OPEN");
          break;
        case WIFI_AUTH_WEP:
          Serial.println("WEP");
          break;
        case WIFI_AUTH_WPA_PSK:
          Serial.println("WPA");
          break;
        case WIFI_AUTH_WPA2_PSK:
          Serial.println("WPA2");
          break;
        case WIFI_AUTH_WPA_WPA2_PSK:
          Serial.println("WPA/WPA2");
          break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
          Serial.println("WPA2-Enterprise");
          break;
        case WIFI_AUTH_WPA3_PSK:
          Serial.println("WPA3");
          break;
        default:
          Serial.println("Unknown");
          break;
      }
      
      // Highlight if this is our target network
      if (WiFi.SSID(i) == WIFI_SSID) {
        Serial.println("  *** THIS IS YOUR TARGET NETWORK ***");
      }
    }
    Serial.println("----------------------------------------");
  }
  
  // Check if our target network was found
  bool targetFound = false;
  for (int i = 0; i < n; i++) {
    if (WiFi.SSID(i) == WIFI_SSID) {
      targetFound = true;
      Serial.print("Target network '");
      Serial.print(WIFI_SSID);
      Serial.print("' found with signal strength: ");
      Serial.print(WiFi.RSSI(i));
      Serial.println(" dBm");
      
      if (WiFi.RSSI(i) < -80) {
        Serial.println("WARNING: Signal strength is very weak!");
      }
      break;
    }
  }
  
  if (!targetFound) {
    Serial.print("ERROR: Target network '");
    Serial.print(WIFI_SSID);
    Serial.println("' NOT FOUND!");
    Serial.println("Please check:");
    Serial.println("1. Network name (SSID) is correct");
    Serial.println("2. Network is broadcasting");
    Serial.println("3. Network is 2.4GHz (ESP32-S3 doesn't support 5GHz)");
    Serial.println("4. You're within range of the network");
  }
  
  Serial.println();
}
