import cv2
import numpy as np

def classify_piece(square):
    # Step 1: Remove square edges or background using edge masking
    height = square.shape[0]
    lower_part = square[int(height * 0):]  # Crop from 40% height to bottom

    # Optional: continue with processing
    processed_square = mask_low_edge_regions(lower_part)
    # Step 2: Check purple dominance (BGR: 255, 0, 255)
    purple_mask = cv2.inRange(processed_square, (255, 0, 255), (255, 0, 255))
    cv2.imshow("Purple Mask", purple_mask)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

    purple_ratio = np.sum(purple_mask > 0) / (square.shape[0] * square.shape[1])
    if purple_ratio > 0.3:
        return 'empty'

    # Step 3: Convert to HSV
    hsv = cv2.cvtColor(processed_square, cv2.COLOR_BGR2HSV)

    # Step 4: Define color thresholds
    lower_cream = np.array([30, 20, 50])
    upper_cream = np.array([45, 30, 140])

    lower_black = np.array([0, 0, 0])
    upper_black = np.array([180, 70, 60])

    # Step 5: Apply masks
    cream_mask = cv2.inRange(hsv, lower_cream, upper_cream)
    black_mask = cv2.inRange(hsv, lower_black, upper_black)

    # Step 6: Calculate color ratios
    total_pixels = square.shape[0] * square.shape[1]
    cream_ratio = np.sum(cream_mask > 0) / total_pixels
    black_ratio = np.sum(black_mask > 0) / total_pixels

    # Step 7: Classification logic
    if black_ratio > 0.2:
        return 'black'
    else:
        return 'white'

def mask_low_edge_regions(square):
    gray = cv2.cvtColor(square, cv2.COLOR_BGR2GRAY)
    blurred = gray.copy()

    # Use Sobel to detect edges
    grad_x = cv2.Sobel(blurred, cv2.CV_64F, 1, 0, ksize=1)
    grad_y = cv2.Sobel(blurred, cv2.CV_64F, 0, 1, ksize=1)
    gradient_magnitude = cv2.magnitude(grad_x, grad_y)

    # Normalize and threshold to get low-edge areas
    norm_grad = cv2.normalize(gradient_magnitude, None, 0, 255, cv2.NORM_MINMAX)
    norm_grad = norm_grad.astype(np.uint8)
    low_edge_mask = cv2.inRange(norm_grad, 0, 8)

    # Find contours of low-edge regions
    contours, _ = cv2.findContours(low_edge_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    if contours:
        # Get the largest one (by area)
        largest_contour = max(contours, key=cv2.contourArea)
        largest_mask = np.zeros_like(low_edge_mask)
        cv2.drawContours(largest_mask, [largest_contour], -1, 255, thickness=cv2.FILLED)

        # Make a purple image
        purple = np.full(square.shape, (255, 0, 255), dtype=np.uint8)

        # Composite only the largest low-edge region as purple
        result = np.where(largest_mask[:, :, np.newaxis] == 255, purple, square)
        return result

    # No low-edge region detected: return original square
    return square
