import cv2
import numpy as np

def divide_chessboard_equally(image):
    h, w = image.shape[:2]
    square_height = h // 8
    square_width = w // 8

    squares = []

    for row in range(8):
        for col in range(8):
            y1 = row * square_height
            y2 = (row + 1) * square_height if row < 7 else h  # include edge pixels
            x1 = col * square_width
            x2 = (col + 1) * square_width if col < 7 else w
            square = image[y1:y2, x1:x2]
            squares.append(square)

    return squares
