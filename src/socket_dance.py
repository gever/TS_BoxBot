import asyncio
import websockets
import json
import sys
import time

# Configuration
ROBOT_IP = "10.100.100.130" # Default Access Point IP
ROBOT_PORT = 81
URI = f"ws://{ROBOT_IP}:{ROBOT_PORT}/"
DEBUG = False

def log(msg, end="\n"):
    if DEBUG:
        print(msg, end=end, flush=True)

async def send_command(websocket, cmd, val=None):
    payload = {"cmd": cmd}
    if val is not None:
        payload["val"] = val
    
    log(f"Sending: {payload}")
    await websocket.send(json.dumps(payload))
    
    response = await websocket.recv()
    log(f"Response: {response}")
    return json.loads(response)

async def wait_until_idle(websocket):
    log("Waiting for robot...", end="")
    while True:
        await websocket.send(json.dumps({"cmd": "busy"}))
        response = await websocket.recv()
        data = json.loads(response)
        
        # Check if the response actually contains the "busy" key
        # If we get an ACK from a previous command delayed, we might need to retry or handle it
        if "busy" in data:
            if not data["busy"]:
                log(" Done!")
                break
        
        log(".", end="")
        await asyncio.sleep(0.25)

async def main():
    print(f"Starting Socket Dance on {URI}...")
    
    try:
        async with websockets.connect(URI) as websocket:
            print("Connected!")
            
            side_length = 10
            turn_angle = 90
            
            for i in range(4):
                print(f"\n--- Side {i+1} ---")
                
                # Move Forward
                print(f"Moving forward {side_length}...")
                await send_command(websocket, "move", side_length)
                await wait_until_idle(websocket)
                
                # Check Distance
                print("Checking distance...", end="", flush=True)
                resp = await send_command(websocket, "distance")
                dist = resp.get("distance", -1)
                print(f" {dist}cm")

                # Turn 90 degrees
                print(f"Turning {turn_angle} degrees...")
                await send_command(websocket, "turn", turn_angle)
                await wait_until_idle(websocket)
                
            print("\nSquare Dance Complete!")
            
    except Exception as e:
        print(f"Connection failed: {e}")

if __name__ == "__main__":
    asyncio.run(main())
