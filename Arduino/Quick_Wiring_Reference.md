# 🔌 Quick Wiring Reference - Your Hardware

## 📋 Your Components & Connections

### 🔋 Power Connections (All to ESP32-S3)
```
3.3V → Voltage Sensor +
3.3V → Current Sensor VCC  
3.3V → RTC Module VCC
3.3V → TFT VCC
GND  → Voltage Sensor -
GND  → Current Sensor GND
GND  → RTC Module GND
GND  → TFT GND
GND  → Emergency Stop Button (one terminal)
```

### 📊 Sensor Connections
```
Voltage Sensor S    → GPIO2  (ADC1_CH1)
Current Sensor OUT  → GPIO1  (ADC1_CH0)
Temperature Sensor  → GPIO3  (OneWire)
```

### 🕐 RTC Module (I²C)
```
RTC SDA → GPIO21
RTC SCL → GPIO22
```

### 💾 SD Card (SPI)
```
SD CS   → GPIO10
SD MOSI → GPIO11
SD CLK  → GPIO12
SD MISO → GPIO13
```

### 🖥️ TFT Display (SPI)
```
TFT CS   → GPIO34
TFT SCK  → GPIO36
TFT MOSI → GPIO35
TFT MISO → GPIO37
TFT SDA  → GPIO14 (DC pin)
TFT RST  → GPIO15
TFT BL   → GPIO5 (Backlight)
```

### 🎛️ Control Connections
```
Relay Control      → GPIO18
Status LED         → GPIO2
Buzzer             → GPIO4
Emergency Stop     → GPIO16 (other terminal to GND)
```

## ⚡ Quick Test Checklist

Before uploading firmware:
- [ ] All GND connections made
- [ ] All 3.3V connections made
- [ ] Sensor signals connected to correct GPIO pins
- [ ] SPI connections correct (SD card and TFT)
- [ ] I²C connections correct (RTC)
- [ ] Control pins connected
- [ ] No short circuits
- [ ] Power supply adequate (5V, 1A minimum)

## 🚀 Upload Process

1. **Open Arduino IDE**
2. **File → Open** → Select `EV_Secure_ESP32S3_Complete.ino`
3. **Edit WiFi settings** in `EV_Secure_Config.h`:
   ```cpp
   #define WIFI_SSID "YOUR_WIFI_SSID"
   #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
   #define DASHBOARD_URL "https://your-dashboard.com/api/endpoint"
   #define API_KEY "YOUR_API_KEY_HERE"
   ```
4. **Tools → Board → ESP32S3 Dev Module**
5. **Tools → Port → [Your COM Port]**
6. **Click Upload** (→ button)

## 📱 Expected Serial Output

After successful upload, you should see:
```
EV-Secure ESP32-S3 System Starting...
Version: 1.0.0
Device ID: EV_SECURE_001
Initializing Sensor Manager...
✓ Sensors initialized
Initializing TFT Display...
✓ TFT Display initialized
Initializing SD Card Logger...
✓ SD Card initialized
Connecting to WiFi: YOUR_WIFI_SSID
WiFi Connected Successfully!
IP Address: 192.168.1.100
EV-Secure System Initialized Successfully!
Monitoring charging station for threats...
```

## 🔧 Troubleshooting

### If sensors show wrong readings:
- Check power supply (should be 3.3V)
- Verify signal connections
- Check for loose wires

### If display doesn't work:
- Verify SPI connections
- Check TFT power supply
- Ensure proper initialization

### If SD card not detected:
- Check SPI connections
- Format SD card as FAT32
- Try different SD card

### If WiFi fails:
- Check SSID and password
- Verify signal strength
- Use 2.4GHz network (not 5GHz)

---

**Your EV-Secure system is ready to protect charging infrastructure!** 🛡️
