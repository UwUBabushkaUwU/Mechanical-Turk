import cv2
import numpy as np
import time
from creating_warped_image_with_claude import give_chessboard
from divide_into_squares import divide_chessboard_equally
from determine_state import classify_piece

def get_move(prev_state, curr_state):
    def index_to_square(i):
        file = chr(ord('a') + i % 8)
        rank = str(8 - i // 8)
        return file + rank

    removed = []
    added = []

    for i in range(64):
        if prev_state[i] != curr_state[i]:
            if prev_state[i] != 'empty' and curr_state[i] == 'empty':
                removed.append(i)
            elif prev_state[i] == 'empty' and curr_state[i] != 'empty':
                added.append(i)

    if len(removed) == 1 and len(added) == 1:
        from_sq = index_to_square(removed[0])
        to_sq = index_to_square(added[0])
        if prev_state[removed[0]] == curr_state[added[0]]:
            return from_sq + to_sq  # normal move
        else:
            return from_sq + 'x' + to_sq  # capture

    # Castling logic
    if len(removed) == 2 and len(added) == 2:
        # white kingside
        if sorted(removed) == [60, 63] and sorted(added) == [62, 61]:
            return 'e1g1'
        # white queenside
        if sorted(removed) == [60, 56] and sorted(added) == [58, 59]:
            return 'e1c1'
        # black kingside
        if sorted(removed) == [4, 7] and sorted(added) == [6, 5]:
            return 'e8g8'
        # black queenside
        if sorted(removed) == [0, 3] and sorted(added) == [2, 1]:
            return 'e8c8'

    # En passant (inferred from diagonal move with capture but destination square was empty before)
    if len(removed) == 1 and len(added) == 1:
        r0, a0 = removed[0], added[0]
        from_rank = 8 - r0 // 8
        to_rank = 8 - a0 // 8
        if abs((r0 % 8) - (a0 % 8)) == 1 and abs(from_rank - to_rank) == 1:
            return index_to_square(r0) + 'x' + index_to_square(a0)  # en passant style

    return "Unknown move or multiple changes"


# Load and warp the image
def return_move(image1, image2):
    warped1 = give_chessboard(image1)
    warped2 = give_chessboard(image2)

    # Divide the warped image into 64 squares
    squares1 = divide_chessboard_equally(warped1)
    squares2 = divide_chessboard_equally(warped2)

    # Classify each square
    prev_board_state = [classify_piece(square) for square in squares1]
    curr_board_state = [classify_piece(square) for square in squares2]

    # Print board state
    print("\nPrevious Board State:")
    for i in range(8):
        print(prev_board_state[i * 8:(i + 1) * 8])

    print("\nCurrent Board State:")
    for i in range(8):
        print(curr_board_state[i * 8:(i + 1) * 8])

    # Get the move
    move = get_move(prev_board_state, curr_board_state)
    return move

def is_frame_similar(frame1, frame2, threshold=120000000):
    # Convert to grayscale
    gray1 = cv2.cvtColor(frame1, cv2.COLOR_BGR2GRAY)
    gray2 = cv2.cvtColor(frame2, cv2.COLOR_BGR2GRAY)
    
    # Compute absolute difference
    diff = cv2.absdiff(gray1, gray2)
    
    # Blur to remove small noise
    blurred = cv2.GaussianBlur(diff, (5, 5), 0)
    
    # Sum the pixel intensities
    diff_score = np.sum(blurred)

    return diff_score < threshold
def count_green_pixels(frame):
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    # Define green range (tweak if needed)
    lower_green = np.array([35, 40, 40])
    upper_green = np.array([85, 255, 255])
    mask = cv2.inRange(hsv, lower_green, upper_green)
    return np.sum(mask > 0)

def capture_chess_and_return_move():
    url = "http://172.23.150.121:8080/video"
    cap = cv2.VideoCapture(url)

    if not cap.isOpened():
        print("Failed to open camera")
        return

    print("Warming up camera...")
    time.sleep(3)

    print("Capturing first image...")
    while True:
        ret, raw_frame = cap.read()
        if not ret:
            print("Camera error")
            return
        width = raw_frame.shape[1]
        cropped = raw_frame[:, :int(width * 0.7)]  # Crop right 30%
        first_frame = cv2.rotate(cropped, cv2.ROTATE_90_COUNTERCLOCKWISE)
        break

    base_green = count_green_pixels(first_frame)
    print("Base green pixel count:", base_green)

    # Wait for movement to START
    print("Waiting for movement to start...")
    green_drop_threshold = 0.95  # Drop by 10% or more triggers movement
    while True:
        ret, raw_frame = cap.read()
        if not ret:
            break
        width = raw_frame.shape[1]
        cropped = raw_frame[:, :int(width * 0.7)]
        frame = cv2.rotate(cropped, cv2.ROTATE_90_COUNTERCLOCKWISE)

        current_green = count_green_pixels(frame)
        green_ratio = current_green / base_green

        if green_ratio < green_drop_threshold:
            print(f"Movement started! Green ratio: {green_ratio:.2f}")
            break

        cv2.imshow("Camera Feed", cv2.resize(frame, (640, int(frame.shape[0]*640/frame.shape[1]))))
        if cv2.waitKey(10) & 0xFF == ord('q'):
            break

    # Wait for movement to STOP (use similarity logic)
    print("Waiting for movement to stop...")
    still_start = None
    prev_frame = frame
    while True:
        ret, raw_frame = cap.read()
        if not ret:
            break
        width = raw_frame.shape[1]
        cropped = raw_frame[:, :int(width * 0.7)]
        frame = cv2.rotate(cropped, cv2.ROTATE_90_COUNTERCLOCKWISE)

        if is_frame_similar(prev_frame, frame):
            if still_start is None:
                still_start = time.time()
            elif time.time() - still_start > 3.0:
                print("Movement stopped!")
                second_frame = frame
                break
        else:
            still_start = None

        prev_frame = frame
        cv2.imshow("Camera Feed", cv2.resize(frame, (640, int(frame.shape[0]*640/frame.shape[1]))))
        if cv2.waitKey(10) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

    move = return_move(first_frame, second_frame)
    print("Detected Move:", move)
    return move

#capture_chess_and_return_move()
img_path = r"C:\Users\Chinar Mhatre\Documents\Mechanical Turk\CV\img\captured_image.jpg"
image = cv2.imread(img_path)

chessboard = give_chessboard(image)
squares = divide_chessboard_equally(chessboard)
rev_board_state = [classify_piece(square) for square in squares]
for i in range(8):
        print(rev_board_state[i * 8:(i + 1) * 8])

# Optionally display or save the result
cv2.imshow("Warped Chessboard", chessboard)
cv2.waitKey(0)
cv2.destroyAllWindows()
