import serial
import time
from pynput import keyboard
import threading

# Configuration
PORT = 'COM11'          # Fixed Bluetooth COM port
BAUD_RATE = 9600

# Motor control constants
DEFAULT_MOTOR1 = 93
MOTOR1_UP = 120
MOTOR1_DOWN = 60
MOTOR2_OPEN = 0
MOTOR2_CLOSE = 120

motor1_active = False
last_motor1_time = 0

# Connect to ESP32 via Bluetooth
try:
    ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
    print(f"[✓] Connected to ESP32 via Bluetooth on {PORT}")
    time.sleep(2)  # Give time for ESP32 to reset if needed
except Exception as e:
    print(f"[x] Failed to connect to {PORT}: {e}")
    exit()

# Send motor command
def send_command(motor, angle):
    global motor1_active, last_motor1_time
    command = f"{motor} {angle}\n"
    ser.write(command.encode())
    print(f"Sent: {command.strip()}")
    if motor == "motor1":
        motor1_active = True
        last_motor1_time = time.time()

# Key press handler
def on_press(key):
    try:
        if key == keyboard.Key.up:
            send_command("motor1", MOTOR1_UP)
        elif key == keyboard.Key.down:
            send_command("motor1", MOTOR1_DOWN)
        elif key == keyboard.Key.left:
            send_command("motor2", MOTOR2_OPEN)
        elif key == keyboard.Key.right:
            send_command("motor2", MOTOR2_CLOSE)
        elif key.char == 'h':
            send_command("motor2", MOTOR2_CLOSE)
        elif key.char == 'g':
            send_command("motor2", MOTOR2_OPEN)
    except AttributeError:
        pass  # Ignore special keys like Shift, Ctrl, etc.

# Auto-reset motor1 to default after 0.5s
def loop_reset_motor1():
    global motor1_active
    while True:
        if motor1_active and time.time() - last_motor1_time >= 0.5:
            send_command("motor1", DEFAULT_MOTOR1)
            motor1_active = False
        time.sleep(0.1)

# Start reset thread
reset_thread = threading.Thread(target=loop_reset_motor1, daemon=True)
reset_thread.start()

print("Use arrow keys or 'g'/'h' to control motors. Press Ctrl+C to exit.")
with keyboard.Listener(on_press=on_press) as listener:
    listener.join()
