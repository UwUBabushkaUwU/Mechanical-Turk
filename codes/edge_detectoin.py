import cv2
import numpy as np

# Load the image
image = cv2.imread("CV/img/IMG_20250602_143036170_HDR.jpg")
if image is None:
    raise ValueError("Image not loaded")

# Resize for consistency
image = cv2.resize(image, (800, int(image.shape[0] * 800 / image.shape[1])))
original = image.copy()

# Define green range in RGB
lower_green = np.array([0, 80, 0])      # Lower RGB bound for green
upper_green = np.array([80, 255, 50]) # Upper RGB bound for green

# Create a mask based on RGB range
mask_rgb = cv2.inRange(image, lower_green, upper_green)

# Optional: blur to smooth noise
mask_rgb = cv2.GaussianBlur(mask_rgb, (5, 5), 0)

# Morphological cleaning
kernel = np.ones((5, 5), np.uint8)
mask_rgb = cv2.morphologyEx(mask_rgb, cv2.MORPH_OPEN, kernel)
mask_rgb = cv2.morphologyEx(mask_rgb, cv2.MORPH_CLOSE, kernel)

# Find contours on cleaned mask
contours, _ = cv2.findContours(mask_rgb, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

# Create empty mask to draw significant contours
significant_mask = np.zeros_like(mask_rgb)
# Keep only large contours
for cnt in contours:
    area = cv2.contourArea(cnt)
    if area > 1000:
        cv2.drawContours(significant_mask, [cnt], -1, 255, -1)

cv2.imshow("Longest Vertical and Horizontal Edges", significant_mask)
cv2.waitKey(0)
cv2.destroyAllWindows()

# Assume you already have `significant_mask`, `top_right`, `bottom_left`, and `image`

# Step 1: Detect edges in the green mask
edges = cv2.Canny(significant_mask, 50, 150, apertureSize=3)

# Step 2: Hough Line Transform to detect lines
lines = cv2.HoughLinesP(edges, 1, np.pi / 180, threshold=100, minLineLength=30, maxLineGap=10)

# Step 3: Separate vertical and horizontal-ish lines
vertical_lines = []
horizontal_lines = []

for line in lines:
    x1, y1, x2, y2 = line[0]
    dx = x2 - x1
    dy = y2 - y1
    length = np.hypot(dx, dy)
    
    if length == 0:
        continue

    angle = np.degrees(np.arctan2(dy, dx))

    # Normalize angle to [0, 180)
    angle = abs(angle)
    if angle > 90:
        angle = 180 - angle

    if angle > 45:
        vertical_lines.append((line[0], length))
    else:
        horizontal_lines.append((line[0], length))

# Step 4: Get the longest line in each category
longest_vertical = max(vertical_lines, key=lambda x: x[1], default=None)
longest_horizontal = max(horizontal_lines, key=lambda x: x[1], default=None)

# Step 5: Draw them
annotated = image.copy()
if longest_vertical:
    x1, y1, x2, y2 = longest_vertical[0]
    cv2.line(annotated, (x1, y1), (x2, y2), (0, 255, 255), 4)  # Yellow for vertical

if longest_horizontal:
    x1, y1, x2, y2 = longest_horizontal[0]
    cv2.line(annotated, (x1, y1), (x2, y2), (255, 0, 255), 4)  # Magenta for horizontal

cv2.imshow("Longest Vertical and Horizontal Edges", annotated)
cv2.waitKey(0)
cv2.destroyAllWindows()
