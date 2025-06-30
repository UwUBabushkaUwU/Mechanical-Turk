import cv2
import numpy as np

def detect_red_corners(image):
    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

    # Define red color range (2 ranges for red hue wraparound)
    lower_red1 = np.array([0, 100, 50])
    upper_red1 = np.array([12, 255, 255])

    lower_red2 = np.array([168, 100, 50])
    upper_red2 = np.array([180, 255, 255])

    mask1 = cv2.inRange(hsv, lower_red1, upper_red1)
    mask2 = cv2.inRange(hsv, lower_red2, upper_red2)
    red_mask = cv2.bitwise_or(mask1, mask2)

    # Find contours in red mask
    contours, _ = cv2.findContours(red_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # Filter by area and sort largest first
    filtered = [cnt for cnt in contours if cv2.contourArea(cnt) > 25]  # adjust 50 if needed
    filtered = sorted(filtered, key=cv2.contourArea, reverse=True)[:4]

    if len(filtered) != 4:
        raise ValueError(f"Expected 4 red corners, found {len(filtered)}")

    # Compute centers
    centers = []
    for cnt in filtered:
        M = cv2.moments(cnt)
        if M["m00"] != 0:
            cx = int(M["m10"] / M["m00"])
            cy = int(M["m01"] / M["m00"])
            centers.append((cx, cy))

    # Sort corners: top-left, top-right, bottom-left, bottom-right
    centers = sorted(centers, key=lambda c: (c[1], c[0]))  # sort by y, then x
    top_two = sorted(centers[:2], key=lambda c: c[0])      # sort top row by x
    bottom_two = sorted(centers[2:], key=lambda c: c[0])   # sort bottom row by x
    return np.array([top_two[0], top_two[1], bottom_two[1], bottom_two[0]], dtype="float32")


def sort_corners(pts):
    # Convert to array
    pts = np.array(pts, dtype=np.float32)

    # Sum and difference to identify corners
    s = pts.sum(axis=1)
    diff = np.diff(pts, axis=1)

    top_left = pts[np.argmin(s)]
    bottom_right = pts[np.argmax(s)]
    top_right = pts[np.argmin(diff)]
    bottom_left = pts[np.argmax(diff)]

    return np.array([top_left, top_right, bottom_right, bottom_left], dtype=np.float32)

def warp_board(image, corners, output_size=(500, 500)):
    dst = np.array([
        [0, 0],
        [output_size[0] - 1, 0],
        [output_size[0] - 1, output_size[1] - 1],
        [0, output_size[1] - 1]
    ], dtype=np.float32)

    matrix = cv2.getPerspectiveTransform(corners, dst)
    warped = cv2.warpPerspective(image, matrix, output_size)
    return warped

# Main code
def give_chessboard(image):
    corners = detect_red_corners(image)
    warped = warp_board(image, corners)
    return warped

