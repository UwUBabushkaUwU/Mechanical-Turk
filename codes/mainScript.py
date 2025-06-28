import chess
import pygame
import serial
import time
import msvcrt  # For Windows keyboard input

# --- Serial Config ---
SERIAL_PORT = "COM6"
BAUD_RATE = 9600
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Allow Arduino to reset

# --- Chess Board Setup ---
board = chess.Board()
move_stack = []

promotion_stash = {
    'q': 'i8',
    'r': 'i7',
    'n': 'i6',
    'b': 'i5'
}

def get_piece_char(piece: chess.Piece):
    if piece is None:
        return 'P'
    return piece.symbol().upper() if piece.color == chess.WHITE else piece.symbol().lower()

def convert_move(move: chess.Move):
    uci = move.uci()
    from_sq = uci[:2]
    to_sq = uci[2:4]
    piece = board.piece_at(move.from_square)
    captured = board.piece_at(move.to_square)
    promo_piece = move.promotion

    converted = []

    if captured:
        captured_piece = get_piece_char(captured)
        converted.append(f"{captured_piece}{to_sq}h0")

    if promo_piece:
        promo_char = chess.Piece(promo_piece, board.turn).symbol()
        promo_char = promo_char.lower() if board.turn == chess.BLACK else promo_char.upper()
        stash_square = promotion_stash[promo_char.lower()]
        converted.append(f"{promo_char}{stash_square}{to_sq}")
    else:
        piece_char = get_piece_char(piece)
        converted.append(f"{piece_char}{from_sq}{to_sq}")

    return converted

def send_to_arduino(command_list):
    for command in command_list:
        ser.write((command + "\n").encode())
        print(f"Sent: {command}")
        wait_for_ack()

def wait_for_ack():
    while True:
        if ser.in_waiting:
            line = ser.readline().decode().strip()
            print(f"Arduino: {line}")
            if "Move over" in line:
                break

def draw_board(screen, board, font):
    colors = [pygame.Color("burlywood1"), pygame.Color("saddlebrown")]
    square_size = 60
    screen.fill((0, 0, 0))

    for i in range(8):
        for j in range(8):
            color = colors[(i + j) % 2]
            rect = pygame.Rect(j*square_size, (7-i)*square_size, square_size, square_size)
            pygame.draw.rect(screen, color, rect)

            square_index = chess.square(j, i)
            piece = board.piece_at(square_index)
            if piece:
                label = font.render(piece.symbol(), True, pygame.Color("black"))
                screen.blit(label, (j*square_size + 20, (7-i)*square_size + 10))

    pygame.display.flip()

def get_typed_input(prompt, screen, board, font):
    user_input = ''
    print(prompt, end='', flush=True)
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                ser.close()
                exit()

        if msvcrt.kbhit():
            ch = msvcrt.getwch()
            if ch == '\r':  # Enter key
                print()
                return user_input.strip()
            elif ch == '\x08':  # Backspace
                user_input = user_input[:-1]
                print('\r' + prompt + user_input + ' ', end='', flush=True)
            else:
                user_input += ch
                print(ch, end='', flush=True)

        draw_board(screen, board, font)
        pygame.time.delay(10)

def main():
    pygame.init()
    screen = pygame.display.set_mode((480, 480))
    pygame.display.set_caption("Chess Robot Tracker")
    font = pygame.font.SysFont(None, 36)
    draw_board(screen, board, font)

    while not board.is_game_over():
        user_input = get_typed_input("Enter move (e.g., e2e4, Nf3, exd5, e8=Q), or 'undo', 'reset': ", screen, board, font)

        if user_input.lower() == "undo":
            if len(move_stack) >= 1:
                board.pop()
                move_stack.pop()
                print("Last move undone.")
                draw_board(screen, board, font)
            else:
                print("No moves to undo.")
            continue

        elif user_input.lower() == "reset":
            board.reset()
            move_stack.clear()
            print("Game reset.")
            draw_board(screen, board, font)
            continue

        try:
            move = board.parse_san(user_input)
        except ValueError:
            print("Invalid move format.")
            continue

        converted = convert_move(move)
        send_to_arduino(converted)

        board.push(move)
        move_stack.append(move)
        draw_board(screen, board, font)

    print("Game over:", board.result())
    pygame.quit()
    ser.close()

if __name__ == "__main__":
    main()
