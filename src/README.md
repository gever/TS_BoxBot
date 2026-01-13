# TS_BoxBot - ESP32 Robot Controller

This project implements a web-enabled robot controller on an ESP32. It provides a RESTful API to control motors, servos, and read sensors.

## File Descriptions

The source code is organized as follows:

- **`main.cpp`** / **`main.h`**: The core application logic.
    - Sets up WiFi (Station or AP mode).
    - Configures the Web Server and endpoints.
    - Implements the main `loop()` and command execution.
    - Handles motor stepping via interrupts.
- **`sensors.cpp`** / **`sensors.h`**: Sensor drivers.
    - Drivers for Ultrasonic distance sensor (HC-SR04).
    - Luminosity sensors.
    - Accelerometer/Gyroscope (MPU6050) support (partial).
- **`servo.cpp`** / **`servo.h`**: Servo motor control.
    - Wrapper around `ESP32Servo` library.
    - Handles initialization and movement of up to 4 servos.
- **`status.cpp`** / **`status.h`**: Visualization and Feedback.
    - Controls the OLED display (SSD1306).
    - Provides status updates and activity logs on the screen.
- **`settings.cpp`** / **`settings.h`**: Configuration Management.
    - Saves and loads settings (like calibration data) to SPIFFS.
- **`led.cpp`** / **`led.h`**: LED control helper functions.
- **`square_dance.py`**: A sample Python script demonstrating control of the robot by making it move in a square pattern.

## Web API Reference

The robot exposes a simple HTTP API. Commands are sent as GET requests. Most commands accept a single argument as the first query parameter (name is ignored, only position matters).

### Motion Control

| Endpoint | Parameter (First Arg) | Description | Example |
| :--- | :--- | :--- | :--- |
| `/move` | `distance` (int) | Move forward (positive) or backward (negative) in mm/cm. | `/move?d=200` |
| `/turn` | `angle` (int) | Turn right (positive) or left (negative) in degrees. | `/turn?a=90` |
| `/stop` | None | Stop all motors immediately. | `/stop` |

### Status & Sensors

| Endpoint | Return JSON | Description |
| :--- | :--- | :--- |
| `/busy` | `{"busy": bool}` | Returns `true` if the robot is currently executing a motion. |
| `/distance` | `{"distance": int}` | Returns ultrasonic distance in cm. |
| `/luminosity1`| `{"luminosity1": int}` | Returns value of first light sensor. |
| `/luminosity2`| `{"luminosity2": int}` | Returns value of second light sensor. |
| `/temperature`| `{"temperature": int}` | Returns system/sensor temperature. |
| `/version` | `string` | Returns firmware version (plain text). |

### Servos & LEDs

| Endpoint | Parameters | Description |
| :--- | :--- | :--- |
| `/servoGo` | `pin` (int), `angle` (int) | Move servo at `pin` to `angle`. |
| `/led` | `pin` (int), `status` (0/1) | Turn LED on/off. |

## Python Usage Examples

### Prerequisites
```bash
pip install requests
```

### Basic Control

```python
import requests
import time

ROBOT_IP = "192.168.4.1" # Adjust to match your robot's IP
BASE_URL = f"http://{ROBOT_IP}"

def send_command(endpoint, value=None):
    url = f"{BASE_URL}/{endpoint}"
    if value is not None:
        url += f"?val={value}"
    response = requests.get(url)
    return response.json()

# Move forward 100 units
send_command("move", 100)

# Wait for completion
while True:
    status = send_command("busy")
    if not status['busy']:
        break
    time.sleep(0.1)

# Check Distance
dist_response = send_command("distance")
print(f"Distance: {dist_response['distance']} cm")
```

### Motion Plan Example

See `src/square_dance.py` for a complete example of running a coordinated movement pattern.
