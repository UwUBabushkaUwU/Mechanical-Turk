import cv2
import numpy as np

def draw_line_from_point(start_pt, direction_vec, color, image):
    norm = np.linalg.norm(direction_vec)
    if norm == 0:
        return
    direction = direction_vec / norm
    length = 1000
    pt1 = tuple(np.int32(start_pt))
    pt2 = tuple(np.int32(start_pt + direction * length))
    cv2.line(image, pt1, pt2, color, 2)

# Extend from top-right corner (horizontal line)

def compute_intersection(p1, d1, p2, d2):
    """
    Computes intersection of two lines:
    Line 1: p1 + t * d1
    Line 2: p2 + s * d2
    Returns the point of intersection if it exists.
    """
    A = np.array([d1, -d2]).T
    b = p2 - p1
    try:
        t_s = np.linalg.solve(A, b)
        intersection = p1 + t_s[0] * d1
        return intersection
    except np.linalg.LinAlgError:
        return None  # Lines are parallel or singular

def distance(p1, p2):
    return np.linalg.norm(p1 - p2)

def warp_green_region(image):
    print("warp_green_region received image with shape:", None if image is None else image.shape)
    
    if image is None:
        raise ValueError("Input image is None.check the caller")
    image = cv2.resize(image, (800, int(image.shape[0] * 800 / image.shape[1])))
    original = image.copy()

    # Define green range in RGB
    lower_green = np.array([0, 80, 0])
    upper_green = np.array([80, 255, 50])

    # Create a mask based on RGB range
    mask_rgb = cv2.inRange(image, lower_green, upper_green)

    # Optional: blur to smooth noise
    mask_rgb = cv2.GaussianBlur(mask_rgb, (5, 5), 0)

    # Morphological cleaning
    kernel = np.ones((5, 5), np.uint8)
    mask_rgb = cv2.morphologyEx(mask_rgb, cv2.MORPH_OPEN, kernel)
    mask_rgb = cv2.morphologyEx(mask_rgb, cv2.MORPH_CLOSE, kernel)

    # Find contours
    contours, _ = cv2.findContours(mask_rgb, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    significant_mask = np.zeros_like(mask_rgb)

    for cnt in contours:
        if cv2.contourArea(cnt) > 1000:
            cv2.drawContours(significant_mask, [cnt], -1, 255, -1)

    # --- Get extreme points ---
    ys, xs = np.where(significant_mask == 255)
    points = np.stack((xs, ys), axis=-1)

    tolerance = 10
    min_y = np.min(ys)
    top_band = points[(points[:, 1] >= min_y) & (points[:, 1] <= min_y + tolerance)]
    top_right = top_band[np.argmax(top_band[:, 0])]

    min_x = np.min(xs)
    left_band = points[(points[:, 0] >= min_x) & (points[:, 0] <= min_x + tolerance)]
    bottom_left = left_band[np.argmax(left_band[:, 1])]  # max y in left band

    # --- Detect edges and lines ---
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




    # --- Draw extended lines from detected corners ---
    output = image.copy()
    if longest_horizontal:
        print("found horizontal line")
        dx = longest_horizontal[0][2] - longest_horizontal[0][0]
        dy = longest_horizontal[0][3] - longest_horizontal[0][1]
        draw_line_from_point(top_right, np.array([-1*dx, dy]), (0, 0, 255), output)  # Red line
        draw_line_from_point(bottom_left, np.array([dx, dy]), (0, 0, 255), output)  # Red line

    # Extend from bottom-left corner (vertical line)
    if longest_vertical:
        print("found vertical line")
        dx = longest_vertical[0][2] - longest_vertical[0][0]
        dy = longest_vertical[0][3] - longest_vertical[0][1]
        draw_line_from_point(bottom_left, np.array([dx, -1*dy]), (255, 255, 0), output)  # Cyan line
        draw_line_from_point(top_right, np.array([dx, dy]), (255,255,0), output)  # Cyan line
    # Draw corner markers
    cv2.circle(output, tuple(top_right), 1, (0, 255, 0), -1)
    cv2.circle(output, tuple(bottom_left), 1, (255, 0, 0), -1)
    cv2.imshow("Debug Board", output)
    cv2.waitKey(0)  

    # Horizontal directions (red lines)
    if longest_horizontal:
        h_dx = longest_horizontal[0][2] - longest_horizontal[0][0]
        h_dy = longest_horizontal[0][3] - longest_horizontal[0][1]
        horiz_vec_top = np.array([-h_dx, h_dy])      # From top_right
        horiz_vec_bottom = np.array([h_dx, h_dy])     # From bottom_left

    # Vertical directions (cyan lines)
    if longest_vertical:
        v_dx = longest_vertical[0][2] - longest_vertical[0][0]
        v_dy = longest_vertical[0][3] - longest_vertical[0][1]
        vert_vec_bottom = np.array([v_dx, -v_dy])     # From bottom_left
        vert_vec_top = np.array([v_dx, v_dy])         # From top_right

    inter1 = compute_intersection(
        np.array(top_right), horiz_vec_top,
        np.array(bottom_left), vert_vec_bottom
    )

    inter2 = compute_intersection(
        np.array(bottom_left), horiz_vec_bottom,
        np.array(top_right), vert_vec_top
    )

    if inter1 is not None:
        cv2.circle(output, tuple(np.int32(inter1)), 5, (0, 255, 255), -1)  # Yellow
    if inter2 is not None:
        cv2.circle(output, tuple(np.int32(inter2)), 5, (255, 0, 255), -1)  # Magenta



    # Ensure points are float32 for cv2 functions
    src_pts = np.array([
        inter1,             # Top-left
        top_right,          # Top-right
        inter2,             # Bottom-right
        bottom_left         # Bottom-left
    ], dtype=np.float32)

    # Compute width and height of the new image


    width_top = distance(inter1, top_right)
    width_bottom = distance(inter2, bottom_left)
    height_left = distance(inter1, bottom_left)
    height_right = distance(top_right, inter2)

    # Final output size (max width & height)
    dst_width = int(max(width_top, width_bottom))
    dst_height = int(max(height_left, height_right))

    # Destination points for the perspective warp
    dst_pts = np.array([
        [0, 0],                         # Top-left
        [dst_width - 1, 0],             # Top-right
        [dst_width - 1, dst_height - 1],# Bottom-right
        [0, dst_height - 1]             # Bottom-left
    ], dtype=np.float32)

    # Perspective transform
    M = cv2.getPerspectiveTransform(src_pts, dst_pts)
    warped = cv2.warpPerspective(original, M, (dst_width, dst_height))

    return warped

image = cv2.imread(r"C:\Users\Chinar Mhatre\Documents\Mechanical Turk\CV\img\IMG_20250602_143202579_HDR.jpg")
warped = warp_green_region(image)
