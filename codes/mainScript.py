import chess
import serial
import time

# Update this to your actual COM port and baud rate
SERIAL_PORT = "COM4"
BAUD_RATE = 9600

# Setup serial connection
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Give Arduino time to reset

# Initialize board
board = chess.Board()

# Dictionary for piece abbreviation
piece_codes = {
    chess.PAWN: 'p',
    chess.KNIGHT: 'n',
    chess.BISHOP: 'b',
    chess.ROOK: 'r',
    chess.QUEEN: 'q',
    chess.KING: 'k'
}

def format_move(move):
    """Return a string command for the robot based on the move."""
    from_square = chess.square_name(move.from_square)
    to_square = chess.square_name(move.to_square)
    piece = board.piece_at(move.from_square)
    target = board.piece_at(move.to_square)

    piece_char = piece_codes[piece.piece_type]

    if target:
        # Capture
        target_char = piece_codes[target.piece_type]
        return f"{piece_char}{from_square}x{target_char}{to_square}"
    else:
        return f"{piece_char}{from_square}{to_square}"

def send_to_arduino(command):
    print("Sending to Arduino:", command)
    ser.write((command + "\n").encode())

def main():
    while not board.is_game_over():
        user_move = input("Enter your move (e.g., e4, Nf3, exd5): ")

        try:
            move = board.parse_san(user_move)
        except ValueError:
            print("Invalid move. Try again.")
            continue

        command = format_move(move)
        send_to_arduino(command)
        board.push(move)

if __name__ == "__main__":
    main()
