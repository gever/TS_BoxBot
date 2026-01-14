import asyncio
from unittest.mock import AsyncMock, MagicMock

# Import the module to test
import SAB_brain_07

async def test_get_distance():
    print("Testing get_distance averaging logic...")
    
    # Mock log to avoid spam
    SAB_brain_07.log = MagicMock()
    
    # Mock sleep to run fast
    asyncio.sleep = AsyncMock()
    
    # helper to mock websocket response
    async def mock_send(websocket, data):
        return None # we don't care about send
        
    SAB_brain_07.send_command = AsyncMock()
    
    # Case 1: 5 good readings (100, 102, 98, 105, 100) -> sorted: 98, 100, 100, 102, 105 -> drop 98, 105 -> avg(100, 100, 102) = 100.66
    print("\nCase 1: 5 good readings (100, 102, 98, 105, 100)")
    SAB_brain_07.send_command.side_effect = [
       {"distance": 100},
       {"distance": 102},
       {"distance": 98},
       {"distance": 105},
       {"distance": 100}
    ]
    
    d = await SAB_brain_07.get_distance(None)
    print(f"Result: {d:.2f} (Expected ~100.67)")
    assert 100.6 <= d <= 100.7
    
    # Case 2: 3 readings, 2 errors (-1, 50, -1, 55, 60) -> valid: 50, 55, 60 -> sorted: 50, 55, 60 -> drop 50, 60 -> avg(55) = 55
    print("\nCase 2: 3 valid readings, 2 errors (-1, 50, -1, 55, 60)")
    SAB_brain_07.send_command.side_effect = [
       {"distance": -1},
       {"distance": 50},
       {"distance": -1},
       {"distance": 55},
       {"distance": 60}
    ]
    d = await SAB_brain_07.get_distance(None)
    print(f"Result: {d:.2f} (Expected 55.0)")
    assert d == 55.0

    # Case 3: Less than 3 valid (10, -1, -1, -1, -1) -> valid: 10 -> avg(10) = 10
    print("\nCase 3: 1 valid reading (10, -1, -1, -1, -1)")
    SAB_brain_07.send_command.side_effect = [
       {"distance": 10},
       {"distance": -1},
       {"distance": -1},
       {"distance": -1},
       {"distance": -1}
    ]
    d = await SAB_brain_07.get_distance(None)
    print(f"Result: {d:.2f} (Expected 10.0)")
    assert d == 10.0
    
    print("\nAll tests passed!")

if __name__ == "__main__":
    asyncio.run(test_get_distance())
