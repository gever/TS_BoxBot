import requests
import time

# ESP32 server base URL
base_url = 'http://boxbot.home'

# List of commands with parameters
commands = [
    ('/move', {'arg': '100'}),  # Move forward
    ('/turn', {'arg': '-45'}),  # Turn left
    ('/move', {'arg': '50'}),   # Move forward
    ('/turn', {'arg': '45'}),   # Turn right
    ('/stop', {})               # Stop
]

def send_command(command, params):
    url = f"{base_url}{command}"
    start_time = time.time()
    response = requests.get(url, params=params)
    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"Command {command} with params {params} took {elapsed_time:.2f}s, Response: {response.text}")
    return elapsed_time

def main():
    total_time = 0
    for command, params in commands:
        response_time = send_command(command, params)
        total_time += response_time
        time.sleep(1)  # Wait a second between commands to ensure complete processing

    print(f"Total time for sequence: {total_time:.2f}s")

if __name__ == "__main__":
    main()
