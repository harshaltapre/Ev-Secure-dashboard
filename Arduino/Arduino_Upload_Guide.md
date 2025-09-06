# Arduino IDE Upload Guide for EV-Secure ESP32-S3

## 📁 Project Structure
Your Arduino project should have this structure:

```
Ardiuno/
├── EV_Secure_ESP32S3_Complete.ino    ← Main sketch (opens this file)
├── EV_Secure_Config.h                 ← Configuration
├── MLModel.h                          ← ML model library
├── SensorManager.h                    ← Sensor management
├── DisplayManager.h                   ← TFT display control
├── SDLogger.h                         ← SD card logging
├── APIManager.h                       ← API communication
├── RelayController.h                  ← Relay control
└── Arduino_Upload_Guide.md            ← This guide
```

## 🚀 Step-by-Step Upload Process

### Step 1: Install ESP32 Arduino Core
1. Open Arduino IDE
2. Go to `File → Preferences`
3. Add this URL to "Additional Boards Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to `Tools → Board → Boards Manager`
5. Search for "ESP32" and install "ESP32 by Espressif Systems"

### Step 2: Install Required Libraries
Go to `Tools → Manage Libraries` and install:
- WiFi
- HTTPClient
- SD
- SPI
- ArduinoJson
- Adafruit_GFX
- Adafruit_ST7735
- OneWire
- DallasTemperature

### Step 3: Open Main Sketch
1. Go to `File → Open`
2. Navigate to your `Ardiuno` folder
3. Select `EV_Secure_ESP32S3_Complete.ino`
4. Arduino IDE will automatically load all `.h` files

### Step 4: Configure Board Settings
1. `Tools → Board → ESP32 Arduino → ESP32S3 Dev Module`
2. `Tools → Port → [Select your COM port]`
3. `Tools → Upload Speed → 921600` (or 115200 if having issues)
4. `Tools → CPU Frequency → 240MHz (WiFi/BT)`
5. `Tools → Flash Size → 4MB (32Mb)`
6. `Tools → Partition Scheme → Default 4MB with spiffs`

### Step 5: Configure Your Settings
Before uploading, edit `EV_Secure_Config.h`:

```cpp
// WiFi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// API Configuration  
#define DASHBOARD_URL "https://your-dashboard.com/api/endpoint"
#define API_KEY "YOUR_API_KEY_HERE"
```

### Step 6: Upload Firmware
1. Click **Upload** button (→) or press `Ctrl+U`
2. Wait for compilation to complete
3. Watch for "Connecting..." message
4. Press and hold **BOOT** button on ESP32-S3 when prompted
5. Release **BOOT** button when upload starts
6. Wait for "Done uploading" message

## 🔧 Troubleshooting Upload Issues

### Common Problems:

1. **"Failed to connect to ESP32-S3"**
   - Press and hold BOOT button during upload
   - Check USB cable (use data cable, not charging cable)
   - Try different USB port
   - Install ESP32-S3 drivers

2. **"Compilation error"**
   - Check all libraries are installed
   - Verify ESP32 Arduino Core is installed
   - Check for syntax errors in code

3. **"WiFi connection failed"**
   - Verify WiFi credentials in config
   - Check WiFi signal strength
   - Ensure 2.4GHz network (ESP32-S3 doesn't support 5GHz)

4. **"SD card initialization failed"**
   - Check SD card wiring
   - Format SD card as FAT32
   - Try different SD card

## 📊 What Happens During Upload

1. **Compilation Phase**:
   - Arduino IDE compiles all `.ino` and `.h` files
   - Creates binary firmware file
   - Checks for syntax errors

2. **Upload Phase**:
   - Connects to ESP32-S3 via USB
   - Erases flash memory
   - Writes new firmware
   - Verifies upload

3. **Boot Phase**:
   - ESP32-S3 boots with new firmware
   - Initializes all peripherals
   - Connects to WiFi
   - Starts monitoring

## 🔍 Monitoring Upload Progress

Watch the Arduino IDE console for:
```
Sketch uses 1234567 bytes (45%) of program storage space.
Global variables use 45678 bytes (13%) of dynamic memory.
Connecting........_____
Chip is ESP32-S3
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
Crystal is 40MHz
MAC: AA:BB:CC:DD:EE:FF
Uploading stub...
Running stub...
Stub running...
Configuring flash size...
Flash will be erased from 0x00001000 to 0x00005fff...
Flash will be erased from 0x00008000 to 0x00008fff...
Flash will be erased from 0x0000e000 to 0x0000ffff...
Flash will be erased from 0x00010000 to 0x0016ffff...
Compressed 1234567 bytes to 456789...
Writing at 0x00001000... (100%)
Wrote 1234567 bytes (456789 compressed) at 0x00001000 in 12.3 seconds (effective 100.5 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
Done uploading.
```

## 📱 Post-Upload Verification

1. **Open Serial Monitor** (`Tools → Serial Monitor`)
2. Set baud rate to `115200`
3. Press **RESET** button on ESP32-S3
4. You should see:
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

## 🛠️ Development Tips

1. **Use Serial Monitor** for debugging
2. **Check Serial output** for error messages
3. **Test individual components** before full integration
4. **Keep backup** of working firmware
5. **Use version control** (Git) for code management

## 📋 Pre-Upload Checklist

- [ ] ESP32 Arduino Core installed
- [ ] All required libraries installed
- [ ] WiFi credentials configured
- [ ] API endpoint configured
- [ ] Hardware properly wired
- [ ] SD card inserted and formatted
- [ ] USB cable connected
- [ ] Correct COM port selected
- [ ] Correct board selected (ESP32S3 Dev Module)

## 🚨 Important Notes

- **Always backup** your working code before making changes
- **Test thoroughly** before deployment
- **Follow safety procedures** when working with high voltage
- **Keep firmware updated** for security patches
- **Monitor system logs** for any issues

---

**Your EV-Secure ESP32-S3 system is now ready to protect charging infrastructure!**
