import serial
from pynput import keyboard

# Setup serial connection to Arduino on COM6
ser = serial.Serial('COM6', 9600, timeout=1)

# Key press handler
def on_press(key):
    try:
        if key == keyboard.Key.up:
            ser.write(b"motor1 120\n")
            print("Sent: motor1 120")
        elif key == keyboard.Key.down:
            ser.write(b"motor1 60\n")
            print("Sent: motor1 60")
        elif key == keyboard.Key.left:
            ser.write(b"motor2 0\n")
            print("Sent: motor2 0")
        elif key == keyboard.Key.right:
            ser.write(b"motor2 120\n")
            print("Sent: motor2 120")
    except Exception as e:
        print(f"Error: {e}")

# Key release handler
def on_release(key):
    if key == keyboard.Key.esc:
        print("Exiting...")
        return False  # stop listener

# Start listening to keyboard
print("Use arrow keys to control motors. Press ESC to quit.")
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()

# Cleanup
ser.close()
