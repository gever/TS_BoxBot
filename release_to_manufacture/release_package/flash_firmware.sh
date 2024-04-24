#!/bin/bash

# Check if a serial port argument was provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <serial-port>"
    echo "Example: $0 /dev/ttyUSB0"
    exit 1
fi

# Determine which Python version to use
PYTHON_CMD=python3
if python --version 2>&1 | grep -q "Python 3"; then
    PYTHON_CMD=python
fi

echo "Using $PYTHON_CMD for flashing..."

# Check if esptool is installed
if ! $PYTHON_CMD -m esptool --version &> /dev/null
then
    echo "esptool is not installed. Installing esptool..."
    $PYTHON_CMD -m pip install -r requirements.txt
fi

echo "Erasing Flash..."
$PYTHON_CMD -m esptool --chip esp32 --port "$1" --baud 115200 erase_flash

echo "Flashing Firmware..."
$PYTHON_CMD -m esptool --chip esp32 --port "$1" --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x10000 firmware.bin 0x8000 partitions.bin

echo "Flashing File System..."
# use ESP-IDF parttool.py  - determine the address is 0x290000 Add command to upload SPIFFS image here
$PYTHON_CMD -m esptool --chip esp32 --port "$1" --baud 115200 write_flash 0x290000 spiffs.bin


echo "Firmware has been flashed successfully!"
