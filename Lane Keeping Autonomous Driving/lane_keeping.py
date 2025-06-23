import cv2
import numpy as np
import math
import socket


# ROI Function
def roi(frame):
    height, width = frame.shape[:2]

    polygons = np.array([[(1 * width // 10, 1 * height // 10), (9 * width // 10, 1 * height // 10),
                          (7 * width // 10, 8 * height // 10), (3 * width // 10, 8 * height // 10)]])

    mask = np.zeros_like(frame)
    cv2.fillPoly(mask, polygons, 255)
    return cv2.bitwise_and(frame, mask)


# Sliding Window Fitting Function
def fit_lanes_with_sliding_window(edges):
    try:
        height, width = edges.shape
        nwindows = 20
        margin = 10
        minpix = 5

        # Histogram Peak Detection
        histogram = np.sum(edges[height // 2:, :], axis=0)
        midpoint = width // 2
        leftx_base = np.argmax(histogram[:midpoint])
        rightx_base = np.argmax(histogram[midpoint:]) + midpoint

        window_height = height // nwindows
        left_lane_inds = []
        right_lane_inds = []
        current_leftx = leftx_base
        current_rightx = rightx_base

        for window in range(nwindows):

            win_y_low = 8 * height // 10 - (window + 1) * window_height
            win_y_high = 8 * height // 10 - window * window_height

            # left window
            win_xleft_low = max(0, current_leftx - margin)
            win_xleft_high = min(width, current_leftx + margin)
            left_window = edges[win_y_low:win_y_high, win_xleft_low:win_xleft_high]
            left_y, left_x = left_window.nonzero()
            left_x += win_xleft_low
            left_y += win_y_low
            left_lane_inds.extend(zip(left_x, left_y))

            # right window
            win_xright_low = max(0, current_rightx - margin)
            win_xright_high = min(width, current_rightx + margin)
            right_window = edges[win_y_low:win_y_high, win_xright_low:win_xright_high]
            right_y, right_x = right_window.nonzero()
            right_x += win_xright_low
            right_y += win_y_low
            right_lane_inds.extend(zip(right_x, right_y))

            # Adjust window position
            if len(left_x) > minpix:
                current_leftx = np.int32(np.mean(left_x))
            if len(right_x) > minpix:
                current_rightx = np.int32(np.mean(right_x))

        # Check if there are enough points for fitting
        if len(left_lane_inds) < minpix or len(right_lane_inds) < minpix:
            result_img = cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)
            return result_img, None, None, None, None

        # polynomial fitting
        left_x = np.array([x for x, y in left_lane_inds])
        left_y = np.array([y for x, y in left_lane_inds])
        right_x = np.array([x for x, y in right_lane_inds])
        right_y = np.array([y for x, y in right_lane_inds])

        left_fit = np.polyfit(left_y, left_x, 2)
        right_fit = np.polyfit(right_y, right_x, 2)

        # Generate a fitting curve
        ploty = np.linspace(0, height - 1, height)
        left_fitx = left_fit[0] * ploty ** 2 + left_fit[1] * ploty + left_fit[2]
        right_fitx = right_fit[0] * ploty ** 2 + right_fit[1] * ploty + right_fit[2]

        result_img = cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)
        pts_left = np.array([np.transpose(np.vstack([left_fitx, ploty]))], dtype=np.int32)
        pts_right = np.array([np.flipud(np.transpose(np.vstack([right_fitx, ploty])))], dtype=np.int32)

        return result_img, left_fit, right_fit, pts_left, pts_right

    except Exception as e:
        print(f"Lane line detection error: {e}")
        result_img = cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)
        return result_img, None, None, None, None


# Centerline fitting and target point calculation function
def calculate_and_draw_centerline(result_img, left_fit, right_fit, ref_point, dist):
    try:
        height = result_img.shape[0]
        ploty = np.linspace(0, height - 1, height)

        left_fitx = left_fit[0] * ploty ** 2 + left_fit[1] * ploty + left_fit[2]
        right_fitx = right_fit[0] * ploty ** 2 + right_fit[1] * ploty + right_fit[2]
        mid_fitx = (left_fitx + right_fitx) / 2

        center_fit = np.polyfit(ploty, mid_fitx, 2)
        center_fitx = center_fit[0] * ploty ** 2 + center_fit[1] * ploty + center_fit[2]
        pts_center = np.array([np.transpose(np.vstack([center_fitx, ploty]))], dtype=np.int32)

        # Find the target point
        ref_x, ref_y = ref_point
        target = None
        min_diff = float('inf')
        candidate_ys = np.arange(height - 1, -1, -1)

        for y in candidate_ys:
            x = center_fit[0] * y ** 2 + center_fit[1] * y + center_fit[2]
            if x < 0 or x >= result_img.shape[1]:
                continue
            current_dist = math.hypot(x - ref_x, y - ref_y)
            diff = abs(current_dist - dist)
            if diff < min_diff:
                min_diff = diff
                target = (x, y)
            if diff < 2:
                break

        return result_img, center_fit, pts_center, target if min_diff < dist * 0.1 else None

    except Exception as e:
        print(f"Centerline calculation error: {e}")
        return result_img, None, None, None


# Initialize video stream
url = 'http://192.168.4.1'
cap = cv2.VideoCapture(url, cv2.CAP_FFMPEG)
matrix = np.load("perspective_matrix.npy")

frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
fps = cap.get(cv2.CAP_PROP_FPS) or 30
# fourcc = cv2.VideoWriter_fourcc(*'mp4v')
# out = cv2.VideoWriter('output.mp4', fourcc, fps, (frame_width, frame_height))

# steering control parameter（1 cm in reality corresponds to 4 pixels in the image）
ref_point = (frame_width // 2, frame_height)
dist = 120
tread = 60
left_fit = None
right_fit = None
last_steer_angle = 0.0
alpha = 0.5

# Initialize network connection
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('192.168.4.1', 81))

# Main loop
frame_counter = 0
while True:
    ret, frame = cap.read()
    if not ret: break

    # Frame skipping if needed
    frame_counter += 1
    if frame_counter % 2 == 0:

        # Perspective Transformation
        warped = cv2.warpPerspective(frame, matrix, (frame_width, frame_height))

        # preprocessing
        gray = cv2.cvtColor(warped, cv2.COLOR_BGR2GRAY)
        blur = cv2.GaussianBlur(gray, (5, 5), 0)
        edges = cv2.Canny(blur, 10, 50)
        edges = roi(edges)

        # lane detection
        result_img, new_left, new_right, pts_left, pts_right = fit_lanes_with_sliding_window(edges)

        # Update lane parameters
        if new_left is not None and new_right is not None:
            left_fit, right_fit = new_left, new_right

        # Centerline and target point calculation
        final_img = warped.copy()
        overlay = result_img.copy()

        if left_fit is not None and right_fit is not None:

            result_img, center_fit, pts_center, target = calculate_and_draw_centerline(
                result_img, left_fit, right_fit, ref_point, dist)

            if target is not None:
                target_x, target_y = target
                cv2.circle(overlay, (int(target_x), int(target_y)), 3, (0, 0, 255), -1)
                dx = target_x - frame_width // 2
                steer_angle = 50 * tread * dx / dist ** 2
                last_steer_angle = steer_angle
            else:
                last_steer_angle = 0.0

            cv2.putText(frame, f"Steer:{last_steer_angle:.0f}",
                        (frame_width - 80, 50),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
        else:
            last_steer_angle = 0.0

            # Send steering angle
        sock.send(f"{last_steer_angle:.0f}\n".encode())

        cv2.imshow("Lane Detection", frame)
        # out.write(frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

cap.release()
# out.release()
sock.close()
cv2.destroyAllWindows()
