import time
import pyautogui
import serial

# CONFIG
SERIAL_PORT = 'COM6'     # Update this to your actual COM port
BAUD_RATE = 9600

def wait_for_lichess():
    print("You have 10 seconds to open Lichess and focus the board...")
    time.sleep(10)
    print("Ready to send moves.\n")

def send_move_uci(from_square, to_square):
    pyautogui.press('enter')                      # Activate UCI input box
    time.sleep(0.2)
    pyautogui.typewrite(from_square)
    pyautogui.press('enter')
    time.sleep(0.2)
    pyautogui.typewrite(to_square)
    pyautogui.press('enter')
    print(f"Sent move: {from_square.upper()} to {to_square.upper()}\n")

def main():
    wait_for_lichess()

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Connected to {SERIAL_PORT}")
    except Exception as e:
        print(f"Error opening serial port: {e}")
        return

    first = ""
    second = ""

    while True:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if not line:
                continue

            print(f"Arduino says: {line}")

            if line.startswith("first_activated_square:"):
                first = line.split(":")[1].strip().lower()
            elif line.startswith("second_activated_square:"):
                second = line.split(":")[1].strip().lower()

                if first and second:
                    send_move_uci(first, second)
                    first = ""
                    second = ""
        except KeyboardInterrupt:
            print("\nExiting.")
            break
        except Exception as e:
            print(f"Error: {e}")

if __name__ == "__main__":
    main()
