import serial
import keyboard
import time

ser = serial.Serial('COM5', 9600)
time.sleep(2)

# Map keys to movement directions
key_map = {
    'up': 'front',
    'down': 'back',
    'left': 'left',
    'right': 'right'
}

# Track key state to avoid flooding
key_states = {key: False for key in key_map}

print("Hold arrow keys to move. Press ESC to quit.")

while True:
    for key, direction in key_map.items():
        if keyboard.is_pressed(key):
            if not key_states[key]:
                ser.write((f"{direction} start\n").encode())
                key_states[key] = True
        else:
            if key_states[key]:
                ser.write((f"{direction} stop\n").encode())
                key_states[key] = False

    if keyboard.is_pressed('esc'):
        break

ser.close()
