import serial
import time
from pynput import keyboard

# Set the correct serial port (check in Arduino IDE > Tools > Port)
SERIAL_PORT = 'COM6'  # Replace with your actual port
BAUD_RATE = 9600

# Open serial connection
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Allow time for Arduino to reset

# Key state tracking
pressed = set()

# Key press handler
def on_press(key):
    if key in pressed:
        return
    pressed.add(key)

    try:
        if key == keyboard.Key.up:
            ser.write(b'front start\n')
        elif key == keyboard.Key.down:
            ser.write(b'back start\n')
        elif key == keyboard.Key.left:
            ser.write(b'left start\n')
        elif key == keyboard.Key.right:
            ser.write(b'right start\n')
    except Exception as e:
        print(f"Error sending command: {e}")

# Key release handler
def on_release(key):
    try:
        if key in [keyboard.Key.up, keyboard.Key.down, keyboard.Key.left, keyboard.Key.right]:
            ser.write(b'front stop\n')  # Stop all on key release (simple fallback)
            pressed.discard(key)
        elif key == keyboard.Key.esc:
            # Exit on ESC key
            ser.write(b'front stop\n')
            ser.close()
            return False
    except Exception as e:
        print(f"Error releasing key: {e}")
        return False

# Listen for keyboard input
print("Use arrow keys to move. Press ESC to quit.")
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()
