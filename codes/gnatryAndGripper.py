import serial
from pynput import keyboard
import time

# Serial Setup
SERIAL_PORT = 'COM6'   # Change if needed
BAUD_RATE = 9600

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Give time for Arduino reset

pressed = set()

def send_command(command):
    try:
        full_cmd = f"{command}\n"
        ser.write(full_cmd.encode())
        ser.flush()
        print(f"[Sent] {command}")
    except Exception as e:
        print(f"Failed to send: {e}")

def on_press(key):
    if key in pressed:
        return
    pressed.add(key)

    try:
        k = key.char.lower() if hasattr(key, 'char') and key.char else str(key).strip("'").lower()

        if k == 'w':
            send_command('front start')
        elif k == 's':
            send_command('back start')
        elif k == 'a':
            send_command('left start')
        elif k == 'd':
            send_command('right start')

        elif k == 'i':
            send_command('motor1 120')
        elif k == 'k':
            send_command('motor1 60')
        elif k == 'j':
            send_command('motor2 0')
        elif k == 'l':
            send_command('motor2 120')

    except Exception as e:
        print(f"Error: {e}")

def on_release(key):
    pressed.discard(key)

    if hasattr(key, 'char') and key.char.lower() in ['w', 'a', 's', 'd']:
        send_command('front stop')

    if key == keyboard.Key.esc:
        print("Exiting...")
        ser.close()
        return False

print("Simplified test active. Press keys to send. ESC to exit.")

with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()
