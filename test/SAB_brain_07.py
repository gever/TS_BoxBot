import asyncio
import websockets
import json
import sys
import time
import turtle
import traceback

# Configuration
ROBOT_IP = "10.100.100.130" # Default Access Point IP
ROBOT_PORT = 81
URI = f"ws://{ROBOT_IP}:{ROBOT_PORT}/"
DEBUG = False
SCAN_ANGLE = 360/20

# --- Visualization Setup ---
screen = turtle.Screen()
screen.title("Robot LiDAR Scan Visualization")
screen.bgcolor("black")
screen.setup(width=600, height=600)

t = turtle.Turtle()
t.speed(0) # Fastest drawing speed
t.shape("triangle")
t.color("lime") # Radar green
t.pencolor("lime")

def draw_wedge(angle, distance, override_color=None):
    """Draws a filled triangle representing the distance at a specific angle."""
    
    if distance == -1:
        fill_color = "orange"
        draw_dist = 200 # Fixed size for error readings
    elif distance > 0:
        fill_color = "darkgreen"
        draw_dist = distance * 2 # Scale: 1cm = 2 pixels
    else:
        return # Should not happen based on current logic but good safety

    t.setheading(90 - angle) # Center axis of the wedge
    
    if override_color:
        t.fillcolor(override_color)
    else:
        t.fillcolor(fill_color)
    t.begin_fill()
    t.penup()
    t.goto(0, 0)
    t.pendown()
    
    # Right edge of the wedge
    t.setheading(90 - (angle - SCAN_ANGLE/2))
    t.forward(draw_dist)
    
    # Left edge of the wedge
    t.setheading(90 - (angle + SCAN_ANGLE/2))
    t.goto(0,0) # Back to center to close the triangle properly
    
    # Wait, simpler triangle: Center -> Right Point -> Left Point -> Center
    # Re-doing coordinates to be precise
    
    t.penup()
    t.goto(0,0)
    t.pendown()
    
    # 1. Go to Right Point
    t.setheading(90 - (angle - SCAN_ANGLE/2))
    t.forward(draw_dist)
    right_pos = t.pos()
    
    # 2. Go to Left Point
    t.penup()
    t.goto(0,0)
    t.setheading(90 - (angle + SCAN_ANGLE/2))
    t.pendown()
    t.forward(draw_dist)
    left_pos = t.pos()
    
    # 3. Connect them
    t.goto(right_pos)
    t.goto(0,0)
    
    t.end_fill()


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

async def get_distance(websocket):
    # Check Distance
    # Take 5 samples, throw out high/low, average the rest
    samples = []
    log("Checking distance...", end="")
    
    for _ in range(5):
        try:
            resp = await send_command(websocket, "distance")
            dist = resp.get("distance", -1)
            if dist != -1:
                samples.append(dist)
        except Exception as e:
            log(f"Err: {e}")
        
        # Small delay between pinging to let sensor reset if needed
        await asyncio.sleep(0.2)
    
    # Filter and average
    final_dist = -1
    
    if not samples:
        log(" No valid readings.")
        return -1
        
    if len(samples) < 3:
        # Just average what we have
        final_dist = sum(samples) / len(samples)
    else:
        # Sort, remove min and max
        samples.sort()
        # Remove smallest
        samples.pop(0)
        # Remove largest
        samples.pop()
        
        final_dist = sum(samples) / len(samples)

    log(f" Samples: {samples} -> Avg: {final_dist:.1f}cm")
    return final_dist


async def scan(websocket):
    mem = []
    steps = int(360 / SCAN_ANGLE)
    t.clear() # Clear the map for a new scan
    
    print("Starting scan...")
    for step in range(steps):
        current_angle = step * SCAN_ANGLE
        d = await get_distance(websocket)
        mem.append(d)

        # TODO: check for d=-1 (bad scan)
        
        # Visualize the reading immediately
        draw_wedge(current_angle, d)
        
        # Move the robot
        # Move the robot
        await send_command(websocket, "turn", SCAN_ANGLE)
        await wait_until_idle(websocket)
    
    return mem

async def main():
    print(f"Starting Socket Dance on {URI}...")
    
    while True:
        try:
            async with websockets.connect(URI) as websocket:
                print("Connected!")
                
                while True:
                    readings = await scan(websocket)
                
                    # Find the path with the most room
                    if readings:
                        farthest = max(readings)
                        target_idx = readings.index(farthest)
                        target_angle = target_idx * SCAN_ANGLE
                        
                        print(f"Moving toward clear path: {farthest}cm at {target_angle}°")
                        
                        # Highlight the chosen path
                        draw_wedge(target_angle, farthest, override_color="red")
                        
                        
                        # Turn to face the path and move
                        # Note: The robot already finished a 360, so we turn relative to 'front'
                        await send_command(websocket, "turn", target_angle)
                        await wait_until_idle(websocket)
                        await send_command(websocket, "move", min(farthest / 2, 40))
                        await wait_until_idle(websocket)

        except (websockets.exceptions.ConnectionClosedError, ConnectionResetError, OSError) as e:
            print(f"Connection lost: {e}")
            print("Reconnecting in 2 seconds...")
            await asyncio.sleep(2)
        except Exception as e:
            print(f"Unexpected error: {e}")
            traceback.print_exc()
            await asyncio.sleep(2)

if __name__ == "__main__":
    asyncio.run(main())
