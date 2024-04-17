# Firmware Flashing Instructions for ESP32

This document provides detailed instructions on how to flash the firmware onto the ESP32 device using the provided `flash_firmware.sh` script. Follow these steps carefully to ensure successful firmware installation.

## Prerequisites

Before you begin, ensure that you have the following prerequisites installed on your system:

- **Python 3.x**: Make sure Python 3 and pip (Python's package manager) are installed. Python is required to run the `esptool.py` script (this is a Python utility for communicating with the ROM bootloader in Espressif ESP8266 & ESP32 chips.), which is used for flashing the firmware.
(check if python installed:  `python3 --version`, if not, `sudo apt update`, `sudu apt install python3`)

1. Extract the Package: Unzip release_package.zip into a directory on your computer.

2. Navigate to the Script Directory: Open a terminal and change to the directory containing the extracted files.

3. Connect the ESP32: Ensure that the ESP32 device is connected to your computer via a USB-to-serial adapter. Check the serial port name assigned to your device (e.g., /dev/ttyUSB0 on Linux or COM3 on Windows).   On mac, to look for port, command `ls /dev/tty.*`,  look for something that resembles /dev/tty.SLAB_USBtoUART or /dev/tty.usbserial-0001., e.g. 

`chmod +x flash_firmware.sh`

`./flash_firmware.sh /dev/ttyUSB0`  --- linux, ubuntu
`./flash_firmware.sh COM3` --- windows
`./flash_firmware.sh /dev/tty.usbserial-0001` --- mac


4. Once the script completes, the ESP32 should be running the new firmware.

5. Run boxbot, with battery pack connected
    I.  choose wifi, e.g boxbot**
    II.  connect to http://boxbot.home (make sure you specify http://)
    III.  test out the commands in the browser