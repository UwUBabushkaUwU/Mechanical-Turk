import cv2

url = "http://172.23.150.121:8080/video"
cap = cv2.VideoCapture(url)

while True:
    ret, raw_frame = cap.read()
    if not ret:
        break
    width = raw_frame.shape[1]
    cropped = raw_frame[:, :int(width * 0.7)]
    frame = cv2.rotate(cropped, cv2.ROTATE_90_COUNTERCLOCKWISE)


    cv2.imshow("Camera Feed", cv2.resize(frame, (640, int(frame.shape[0]*640/frame.shape[1]))))
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    if cv2.waitKey(1) & 0xFF == ord('s'):
        cv2.imwrite("captured_image.jpg", frame)
        print("Image saved as captured_image.jpg")

cap.release()
cv2.destroyAllWindows()
