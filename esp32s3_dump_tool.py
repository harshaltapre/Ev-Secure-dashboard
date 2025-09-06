#!/usr/bin/env python3
"""
ESP32-S3 Firmware Dump Tool
A simple tool to dump firmware from ESP32-S3 devices
"""

import subprocess
import argparse
import sys
import os
from datetime import datetime

class ESP32S3Dumper:
    def __init__(self, port, chip="esp32s3"):
        self.port = port
        self.chip = chip
        self.output_dir = f"esp32s3_dump_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        
    def run_esptool(self, cmd_args):
        """Run esptool command and return result"""
        cmd = ["esptool.py", "--chip", self.chip, "--port", self.port] + cmd_args
        print(f"Running: {' '.join(cmd)}")
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            print("✓ Command executed successfully")
            return result.stdout
        except subprocess.CalledProcessError as e:
            print(f"✗ Command failed: {e}")
            print(f"Error output: {e.stderr}")
            return None
    
    def create_output_dir(self):
        """Create output directory"""
        if not os.path.exists(self.output_dir):
            os.makedirs(self.output_dir)
            print(f"Created output directory: {self.output_dir}")
    
    def get_chip_info(self):
        """Get chip information"""
        print("\n=== Getting Chip Information ===")
        return self.run_esptool(["chip_id"])
    
    def get_mac_address(self):
        """Get MAC address"""
        print("\n=== Getting MAC Address ===")
        return self.run_esptool(["read_mac"])
    
    def dump_partition_table(self):
        """Dump partition table"""
        print("\n=== Dumping Partition Table ===")
        output_file = os.path.join(self.output_dir, "partition_table.bin")
        result = self.run_esptool(["read_flash", "0x8000", "0x1000", output_file])
        if result:
            print(f"Partition table saved to: {output_file}")
        return result
    
    def dump_bootloader(self):
        """Dump bootloader"""
        print("\n=== Dumping Bootloader ===")
        output_file = os.path.join(self.output_dir, "bootloader.bin")
        result = self.run_esptool(["read_flash", "0x0", "0x8000", output_file])
        if result:
            print(f"Bootloader saved to: {output_file}")
        return result
    
    def dump_nvs(self):
        """Dump NVS partition"""
        print("\n=== Dumping NVS Partition ===")
        output_file = os.path.join(self.output_dir, "nvs.bin")
        result = self.run_esptool(["read_flash", "0x9000", "0x6000", output_file])
        if result:
            print(f"NVS partition saved to: {output_file}")
        return result
    
    def dump_factory_app(self):
        """Dump factory application"""
        print("\n=== Dumping Factory Application ===")
        output_file = os.path.join(self.output_dir, "factory_app.bin")
        result = self.run_esptool(["read_flash", "0x10000", "0x200000", output_file])
        if result:
            print(f"Factory app saved to: {output_file}")
        return result
    
    def dump_ota_partitions(self):
        """Dump OTA partitions"""
        print("\n=== Dumping OTA Partitions ===")
        
        # OTA_0 partition
        output_file_0 = os.path.join(self.output_dir, "ota_0.bin")
        result_0 = self.run_esptool(["read_flash", "0x210000", "0x200000", output_file_0])
        if result_0:
            print(f"OTA_0 partition saved to: {output_file_0}")
        
        # OTA_1 partition
        output_file_1 = os.path.join(self.output_dir, "ota_1.bin")
        result_1 = self.run_esptool(["read_flash", "0x410000", "0x200000", output_file_1])
        if result_1:
            print(f"OTA_1 partition saved to: {output_file_1}")
        
        return result_0 and result_1
    
    def dump_storage(self):
        """Dump storage partition"""
        print("\n=== Dumping Storage Partition ===")
        output_file = os.path.join(self.output_dir, "storage.bin")
        result = self.run_esptool(["read_flash", "0x610000", "0x1F0000", output_file])
        if result:
            print(f"Storage partition saved to: {output_file}")
        return result
    
    def dump_full_flash(self):
        """Dump entire flash memory"""
        print("\n=== Dumping Full Flash Memory ===")
        output_file = os.path.join(self.output_dir, "full_flash.bin")
        result = self.run_esptool(["read_flash", "0x0", "0x800000", output_file])
        if result:
            print(f"Full flash saved to: {output_file}")
        return result
    
    def dump_efuse(self):
        """Dump eFuse"""
        print("\n=== Dumping eFuse ===")
        output_file = os.path.join(self.output_dir, "efuse.bin")
        result = self.run_esptool(["read_efuse", output_file])
        if result:
            print(f"eFuse saved to: {output_file}")
        return result
    
    def test_psram(self):
        """Test PSRAM access"""
        print("\n=== Testing PSRAM Access ===")
        output_file = os.path.join(self.output_dir, "psram_test.bin")
        result = self.run_esptool(["read_flash", "0x3f500000", "0x1000", output_file])
        if result:
            print(f"PSRAM test saved to: {output_file}")
        return result
    
    def dump_psram(self):
        """Dump PSRAM (if accessible)"""
        print("\n=== Dumping PSRAM ===")
        output_file = os.path.join(self.output_dir, "psram.bin")
        result = self.run_esptool(["read_flash", "0x3f500000", "0x800000", output_file])
        if result:
            print(f"PSRAM saved to: {output_file}")
        return result
    
    def dump_custom_range(self, start_addr, size, output_name):
        """Dump custom memory range"""
        print(f"\n=== Dumping Custom Range: 0x{start_addr:x} to 0x{start_addr + size:x} ===")
        output_file = os.path.join(self.output_dir, f"{output_name}.bin")
        result = self.run_esptool(["read_flash", hex(start_addr), hex(size), output_file])
        if result:
            print(f"Custom range saved to: {output_file}")
        return result
    
    def create_summary(self):
        """Create a summary file"""
        summary_file = os.path.join(self.output_dir, "dump_summary.txt")
        with open(summary_file, 'w') as f:
            f.write("ESP32-S3 Firmware Dump Summary\n")
            f.write("=" * 40 + "\n")
            f.write(f"Dump Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"Chip: {self.chip}\n")
            f.write(f"Port: {self.port}\n")
            f.write(f"Output Directory: {self.output_dir}\n\n")
            
            f.write("Files dumped:\n")
            for file in os.listdir(self.output_dir):
                if file.endswith('.bin'):
                    file_path = os.path.join(self.output_dir, file)
                    size = os.path.getsize(file_path)
                    f.write(f"  {file}: {size} bytes\n")
        
        print(f"Summary saved to: {summary_file}")

def main():
    parser = argparse.ArgumentParser(description='ESP32-S3 Firmware Dump Tool')
    parser.add_argument('--port', required=True, help='Serial port (e.g., COM3)')
    parser.add_argument('--chip', default='esp32s3', help='Chip type (default: esp32s3)')
    parser.add_argument('--mode', choices=['full', 'partitions', 'custom'], 
                       default='partitions', help='Dump mode (default: partitions)')
    parser.add_argument('--start', help='Custom start address (hex)')
    parser.add_argument('--size', help='Custom size (hex)')
    parser.add_argument('--name', help='Custom output name')
    
    args = parser.parse_args()
    
    # Create dumper instance
    dumper = ESP32S3Dumper(args.port, args.chip)
    
    # Create output directory
    dumper.create_output_dir()
    
    # Get basic information
    dumper.get_chip_info()
    dumper.get_mac_address()
    
    if args.mode == 'full':
        # Dump everything
        dumper.dump_full_flash()
        dumper.dump_efuse()
        dumper.test_psram()
        
    elif args.mode == 'partitions':
        # Dump partitions
        dumper.dump_partition_table()
        dumper.dump_bootloader()
        dumper.dump_nvs()
        dumper.dump_factory_app()
        dumper.dump_ota_partitions()
        dumper.dump_storage()
        dumper.dump_efuse()
        
    elif args.mode == 'custom':
        # Dump custom range
        if not all([args.start, args.size, args.name]):
            print("Error: --start, --size, and --name are required for custom mode")
            sys.exit(1)
        
        start_addr = int(args.start, 16)
        size = int(args.size, 16)
        dumper.dump_custom_range(start_addr, size, args.name)
    
    # Create summary
    dumper.create_summary()
    
    print(f"\n=== Dump Complete ===")
    print(f"All files saved to: {dumper.output_dir}")
    print("You can now analyze the dumped firmware using tools like binwalk, hexdump, or strings")

if __name__ == "__main__":
    main()
