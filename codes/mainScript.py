from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
import time
import serial
import chess

# === SETTINGS ===
USER_DATA_DIR = r"C:\Users\Chinar Mhatre\AppData\Local\Google\Chrome\User Data"  # <-- CHANGE THIS
SERIAL_PORT = "COM6"
BAUD_RATE = 9600

# === Arduino Setup ===
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
print("Connecting to Arduino...")
time.sleep(2)

# === Board Setup ===
board = chess.Board()
seen_moves = []

promotion_stash = {
    'q': 'i8',
    'r': 'i7',
    'n': 'i6',
    'b': 'i5'
}

def get_piece_char(board, move):
    piece = board.piece_at(move.from_square)
    return piece.symbol().upper() if piece.color == chess.WHITE else piece.symbol().lower()

def convert_move(board, move):
    from_sq = chess.square_name(move.from_square)
    to_sq = chess.square_name(move.to_square)
    captured = board.piece_at(move.to_square)
    promo = move.promotion

    commands = []
    if captured:
        captured_char = captured.symbol().upper()
        commands.append(f"{captured_char}{to_sq}h0")

    if promo:
        promo_char = chess.Piece(promo, board.turn).symbol().upper()
        stash = promotion_stash[promo_char.lower()]
        commands.append(f"{promo_char}{stash}{to_sq}")
    else:
        piece_char = get_piece_char(board, move)
        commands.append(f"{piece_char}{from_sq}{to_sq}")
    return commands

def wait_for_ack():
    while True:
        if ser.in_waiting:
            line = ser.readline().decode().strip()
            if "Move over" in line:
                break

def send_to_arduino(commands):
    for cmd in commands:
        print(f"Sending: {cmd}")
        ser.write((cmd + "\n").encode())
        wait_for_ack()

# === Browser Setup ===
options = Options()
options.add_argument(f"--user-data-dir={USER_DATA_DIR}")
options.add_argument("--profile-directory=Default")  # or "Profile 1", etc.
options.add_argument("--start-maximized")
options.add_argument("--no-sandbox") 

driver = webdriver.Chrome(options=options)
driver.get("https://lichess.org")

input("Open a Lichess game as WHITE and press Enter...")

# === Main Loop ===
def get_lichess_moves(driver):
    try:
        elements = driver.find_elements(By.TAG_NAME, 'kwdb')  # <== updated here
        moves = [el.text.strip() for el in elements if el.text.strip()]
        print("Current moves on page:", moves)
        return moves
    except Exception as e:
        print("Error reading moves:", e)
        return []

print("Watching opponent's (Black's) moves...")

while True:
    try:
        all_moves = get_lichess_moves(driver)
        if len(all_moves) > len(seen_moves):
            new_moves = all_moves[len(seen_moves):]

            for san in new_moves:
                try:
                    move = board.parse_san(san)
                except Exception as e:
                    print(f"Could not parse move '{san}':", e)
                    continue

                if board.turn == chess.BLACK:  # Opponent's move
                    print(f"Opponent move detected: {san}")
                    cmds = convert_move(board, move)
                    send_to_arduino(cmds)

                board.push(move)
                seen_moves.append(san)

        time.sleep(2)

    except KeyboardInterrupt:
        print("Exiting...")
        driver.quit()
        ser.close()
        break
