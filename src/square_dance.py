import requests
import time
import sys

# Configuration
ROBOT_IP = "192.168.4.1" # Default Access Point IP. Change to actual IP if connected via WiFi.
BASE_URL = f"http://{ROBOT_IP}"
DEBUG = False

def log(msg, end="\n"):
    """Prints message only if DEBUG is True."""
    if DEBUG:
        print(msg, end=end, flush=True)

def send_command(endpoint, value=None):
    """Sends a command to the robot."""
    if value is not None:
        url = f"{BASE_URL}/{endpoint}?val={value}"
        log(f"Sending: {endpoint} {value}")
    else:
        url = f"{BASE_URL}/{endpoint}"
        log(f"Sending: {endpoint}")
        
    try:
        response = requests.get(url, timeout=5)
        response.raise_for_status()
        log(f"Response: {response.text.strip()}")
        return response
    except requests.exceptions.RequestException as e:
        print(f"Error sending command: {e}")
        sys.exit(1)

def get_distance():
    """Gets the current distance reading."""
    print("Checking distance...", end="", flush=True)
    try:
        response = send_command("distance")
        data = response.json()
        dist = data.get("distance", -1)
        print(f" {dist}cm")
        return dist
    except Exception as e:
        print(f" Failed to get distance: {e}")
        return -1

def wait_until_idle():
    """Polls /busy until the robot is done moving."""
    url = f"{BASE_URL}/busy"
    log("Waiting for robot...", end="")
    while True:
        try:
            response = requests.get(url, timeout=5)
            response.raise_for_status()
            data = response.json()
            
            if not data.get("busy", False):
                log(" Done!")
                break
            
            log(".", end="")
            time.sleep(0.25) # Poll max 4 times per second
        except requests.exceptions.RequestException as e:
            print(f"\nError polling status: {e}")
            # Don't exit here, might be a temporary glitch, but maybe safe to break or retry
            time.sleep(1)

def main():
    print(f"Starting Square Dance on {ROBOT_IP}...")
    
    # Side length in mm (or whatever unit the robot uses, likely cm or mm based on code)
    # create logical square: Move 4 times, Turn 4 times
    side_length = 200 # assumption: 20cm or 200mm
    turn_angle = 90
    
    for i in range(4):
        print(f"\n--- Side {i+1} ---")
        
        # Move Forward
        print(f"Moving forward {side_length}...")
        send_command("move", side_length)
        wait_until_idle()
        
        # Check Distance
        get_distance()
        
        # Turn 90 degrees
        print(f"Turning {turn_angle} degrees...")
        send_command("turn", turn_angle)
        wait_until_idle()
        
    print("\nSquare Dance Complete!")

if __name__ == "__main__":
    main()
