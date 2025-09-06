# ESP32-S3 Quick Dump Commands

## Prerequisites
```bash
pip install esptool
```

## Basic Commands

### 1. Check if ESP32-S3 is connected
```bash
esptool.py --chip esp32s3 chip_id
```

### 2. Get MAC address
```bash
esptool.py --chip esp32s3 --port COM3 read_mac
```

### 3. Dump entire flash (8MB)
```bash
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x800000 full_dump.bin
```

## Partition Dumps

### 4. Dump partition table
```bash
esptool.py --chip esp32s3 --port COM3 read_flash 0x8000 0x1000 partition_table.bin
```

### 5. Dump bootloader
```bash
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x8000 bootloader.bin
```

### 6. Dump NVS (configuration)
```bash
esptool.py --chip esp32s3 --port COM3 read_flash 0x9000 0x6000 nvs.bin
```

### 7. Dump main application
```bash
esptool.py --chip esp32s3 --port COM3 read_flash 0x10000 0x200000 app.bin
```

### 8. Dump OTA partitions
```bash
esptool.py --chip esp32s3 --port COM3 read_flash 0x210000 0x200000 ota_0.bin
esptool.py --chip esp32s3 --port COM3 read_flash 0x410000 0x200000 ota_1.bin
```

### 9. Dump storage partition
```bash
esptool.py --chip esp32s3 --port COM3 read_flash 0x610000 0x1F0000 storage.bin
```

## Security Dumps

### 10. Dump eFuse
```bash
esptool.py --chip esp32s3 --port COM3 read_efuse efuse.bin
```

### 11. Test PSRAM access
```bash
esptool.py --chip esp32s3 --port COM3 read_flash 0x3f500000 0x1000 psram_test.bin
```

## Analysis Commands

### 12. Extract strings from binary
```bash
strings app.bin > strings.txt
```

### 13. Hex dump (first 1KB)
```bash
hexdump -C app.bin | head -20
```

### 14. Check file size
```bash
ls -la *.bin
```

## Using the Python Tool

### 15. Dump all partitions
```bash
python esp32s3_dump_tool.py --port COM3 --mode partitions
```

### 16. Dump everything
```bash
python esp32s3_dump_tool.py --port COM3 --mode full
```

### 17. Dump custom range
```bash
python esp32s3_dump_tool.py --port COM3 --mode custom --start 0x10000 --size 0x10000 --name custom_dump
```

## Troubleshooting

### Port not found
```bash
# List available ports
esptool.py --chip esp32s3 chip_id
```

### Permission denied (Linux/Mac)
```bash
sudo usermod -a -G dialout $USER
# Then logout and login again
```

### Chip not responding
```bash
# Hold boot button while connecting
# Or try different baud rate
esptool.py --chip esp32s3 --port COM3 --baud 115200 read_flash 0x0 0x1000 test.bin
```

## Memory Map Reference

| Address Range | Size | Description |
|---------------|------|-------------|
| 0x000000-0x008000 | 32KB | Bootloader |
| 0x008000-0x009000 | 4KB | Partition Table |
| 0x009000-0x00F000 | 24KB | NVS |
| 0x010000-0x210000 | 2MB | Factory App |
| 0x210000-0x410000 | 2MB | OTA_0 |
| 0x410000-0x610000 | 2MB | OTA_1 |
| 0x610000-0x800000 | 1.9MB | Storage |

## Common Use Cases

### Backup before update
```bash
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x800000 backup_$(date +%Y%m%d).bin
```

### Debug specific partition
```bash
esptool.py --chip esp32s3 --port COM3 read_flash 0x9000 0x6000 nvs_debug.bin
```

### Security audit
```bash
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x800000 security_audit.bin
```

## Notes
- Replace `COM3` with your actual port
- Flash size is typically 8MB (0x800000 bytes)
- Dumped data may be encrypted if flash encryption is enabled
- PSRAM is 8MB at address 0x3f500000
