import SAB_brain_07 as sb
import turtle
import time

def test_visualization():
    print("Testing standard wedge...")
    sb.draw_wedge(0, 50)
    time.sleep(1)
    
    print("Testing another wedge...")
    sb.draw_wedge(45, 50)
    time.sleep(1)
    
    print("Testing HIGHLIGHTED wedge (red)...")
    # Redrawing the first wedge in red
    sb.draw_wedge(0, 50, override_color="red")
    
    print("Test complete. Check window.")
    turtle.done()

if __name__ == "__main__":
    test_visualization()
