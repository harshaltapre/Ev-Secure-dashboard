 # ESP32-S3 Firmware Dump Guide

## Overview
This guide covers various methods to dump firmware from an ESP32-S3 device, including flash memory, PSRAM, and specific partitions.

## Prerequisites
- ESP-IDF installed and configured
- USB cable connected to ESP32-S3
- Serial port access (COM port on Windows)
- Python with esptool installed: `pip install esptool`

## Method 1: Dump Entire Flash Memory

### Using esptool (Recommended)
```bash
# Dump entire flash to binary file
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x800000 flash_dump.bin

# Dump with verification
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x800000 flash_dump.bin --verify
```

### Using ESP-IDF
```bash
# Set target
idf.py set-target esp32s3

# Dump flash
idf.py flash read_flash 0x0 0x800000 flash_dump.bin
```

## Method 2: Dump Specific Partitions

### List Partitions First
```bash
# View partition table
esptool.py --chip esp32s3 --port COM3 read_flash 0x8000 0x2000 partition_table.bin
```

### Dump Individual Partitions
```bash
# Dump bootloader (0x0 to 0x8000)
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x8000 bootloader.bin

# Dump partition table (0x8000 to 0x9000)
esptool.py --chip esp32s3 --port COM3 read_flash 0x8000 0x1000 partition_table.bin

# Dump NVS partition (0x9000 to 0xf000)
esptool.py --chip esp32s3 --port COM3 read_flash 0x9000 0x6000 nvs.bin

# Dump factory app (0x10000 to 0x210000)
esptool.py --chip esp32s3 --port COM3 read_flash 0x10000 0x200000 factory_app.bin

# Dump OTA partitions
esptool.py --chip esp32s3 --port COM3 read_flash 0x210000 0x200000 ota_0.bin
esptool.py --chip esp32s3 --port COM3 read_flash 0x410000 0x200000 ota_1.bin

# Dump storage partition (0x610000 to 0x800000)
esptool.py --chip esp32s3 --port COM3 read_flash 0x610000 0x1F0000 storage.bin
```

## Method 3: Dump PSRAM (if enabled)

### Check PSRAM Status
```bash
# Check if PSRAM is enabled
esptool.py --chip esp32s3 --port COM3 read_flash 0x3f500000 0x1000 psram_test.bin
```

### Dump PSRAM Contents
```bash
# Dump PSRAM (if accessible)
esptool.py --chip esp32s3 --port COM3 read_flash 0x3f500000 0x800000 psram_dump.bin
```

## Method 4: Dump with Encryption Handling

### If Flash Encryption is Enabled
```bash
# Dump encrypted flash
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x800000 encrypted_flash.bin

# To decrypt, you need the flash encryption key
# This is stored in eFuse and cannot be read directly
# You can only decrypt if you have the key from when encryption was enabled
```

## Method 5: Interactive Dump Script

### Create a Python Script
```python
#!/usr/bin/env python3
import esptool
import argparse

def dump_esp32s3_firmware(port, output_file, start_addr=0x0, size=0x800000):
    """Dump ESP32-S3 firmware from specified address range"""
    
    cmd = [
        '--chip', 'esp32s3',
        '--port', port,
        'read_flash',
        hex(start_addr),
        hex(size),
        output_file
    ]
    
    esptool.main(cmd)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Dump ESP32-S3 firmware')
    parser.add_argument('--port', required=True, help='Serial port (e.g., COM3)')
    parser.add_argument('--output', required=True, help='Output file name')
    parser.add_argument('--start', default='0x0', help='Start address (default: 0x0)')
    parser.add_argument('--size', default='0x800000', help='Size to dump (default: 0x800000)')
    
    args = parser.parse_args()
    
    start_addr = int(args.start, 16)
    size = int(args.size, 16)
    
    dump_esp32s3_firmware(args.port, args.output, start_addr, size)
    print(f"Firmware dumped to {args.output}")
```

## Method 6: Dump Specific Memory Regions

### Dump eFuse
```bash
# Dump eFuse (read-only)
esptool.py --chip esp32s3 --port COM3 read_efuse efuse_dump.bin
```

### Dump MAC Address
```bash
# Read MAC address
esptool.py --chip esp32s3 --port COM3 read_mac
```

## Method 7: Analyze Dumped Firmware

### Using Binwalk
```bash
# Install binwalk
pip install binwalk

# Analyze dumped firmware
binwalk flash_dump.bin

# Extract files from firmware
binwalk -e flash_dump.bin
```

### Using Ghidra/IDA Pro
1. Load the dumped binary
2. Set architecture to Xtensa (ESP32)
3. Analyze for patterns and functions

## Method 8: Dump Running Firmware (Advanced)

### Using GDB/OpenOCD
```bash
# Start OpenOCD
openocd -f board/esp32s3-builtin.cfg

# Connect GDB
xtensa-esp32s3-elf-gdb
(gdb) target remote :3333
(gdb) dump binary memory running_firmware.bin 0x40000000 0x40080000
```

## Troubleshooting

### Common Issues

1. **Port Not Found**
   ```bash
   # List available ports
   esptool.py --chip esp32s3 chip_id
   ```

2. **Permission Denied**
   ```bash
   # On Linux/Mac, add user to dialout group
   sudo usermod -a -G dialout $USER
   ```

3. **Chip Not Responding**
   ```bash
   # Hold boot button while connecting
   # Or try different baud rates
   esptool.py --chip esp32s3 --port COM3 --baud 115200 read_flash 0x0 0x1000 test.bin
   ```

4. **Flash Encryption**
   - If flash is encrypted, dumped data will be encrypted
   - Need original encryption key to decrypt
   - Cannot recover key from eFuse if Secure Boot is enabled

## Security Considerations

### What You Can Dump
- **Unencrypted firmware**: Full binary analysis possible
- **Encrypted firmware**: Encrypted, cannot be analyzed without key
- **eFuse**: Read-only, contains security settings
- **PSRAM**: Runtime data, may contain sensitive information

### What You Cannot Dump
- **Flash encryption keys**: Stored in eFuse, protected
- **Secure Boot keys**: Protected by hardware
- **Private keys**: If stored in secure element (ATECC608A)

## Use Cases

### 1. Firmware Analysis
```bash
# Dump for reverse engineering
esptool.py --chip esp32s3 --port COM3 read_flash 0x10000 0x200000 app_firmware.bin
```

### 2. Backup Before Update
```bash
# Backup current firmware
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x800000 backup.bin
```

### 3. Debugging
```bash
# Dump specific partition for debugging
esptool.py --chip esp32s3 --port COM3 read_flash 0x9000 0x6000 nvs_debug.bin
```

### 4. Security Audit
```bash
# Dump for security analysis
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x800000 security_audit.bin
```

## Example Workflow

### Complete Firmware Analysis
```bash
# 1. Identify chip and port
esptool.py --chip esp32s3 chip_id

# 2. Dump partition table
esptool.py --chip esp32s3 --port COM3 read_flash 0x8000 0x1000 partition_table.bin

# 3. Dump main application
esptool.py --chip esp32s3 --port COM3 read_flash 0x10000 0x200000 main_app.bin

# 4. Dump NVS (configuration)
esptool.py --chip esp32s3 --port COM3 read_flash 0x9000 0x6000 nvs_config.bin

# 5. Analyze with binwalk
binwalk main_app.bin

# 6. Extract strings
strings main_app.bin > strings.txt
```

## Notes

- **Flash Size**: ESP32-S3 typically has 8MB flash (0x800000 bytes)
- **Partition Layout**: Varies by project, check partition table first
- **Encryption**: If enabled, dumped data will be encrypted
- **Secure Boot**: If enabled, prevents unauthorized firmware execution
- **PSRAM**: 8MB available on ESP32-S3, may contain runtime data

## Tools Required

- **esptool**: Primary tool for ESP32 communication
- **ESP-IDF**: Development framework with additional tools
- **binwalk**: Firmware analysis tool
- **hexdump**: Binary analysis (built-in on most systems)
- **strings**: Extract ASCII strings from binary

## References

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [esptool Documentation](https://github.com/espressif/esptool)
- [ESP32-S3 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
