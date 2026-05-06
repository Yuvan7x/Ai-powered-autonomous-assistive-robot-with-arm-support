import cv2
import numpy as np

# Start Webcam
cap = cv2.VideoCapture(0)

while True:

    # Read Camera Frame
    ret, frame = cap.read()

    if not ret:
        break

    # Resize Frame
    frame = cv2.resize(frame, (640, 480))

    # Convert BGR to HSV
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Red Matchbox Color Range
    lower_red1 = np.array([0, 120, 70])
    upper_red1 = np.array([10, 255, 255])

    lower_red2 = np.array([170, 120, 70])
    upper_red2 = np.array([180, 255, 255])

    # Create Mask
    mask1 = cv2.inRange(hsv, lower_red1, upper_red1)
    mask2 = cv2.inRange(hsv, lower_red2, upper_red2)

    mask = mask1 + mask2

    # Remove Noise
    kernel = np.ones((5,5), np.uint8)

    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)

    # Find Object Contours
    contours, _ = cv2.findContours(
        mask,
        cv2.RETR_EXTERNAL,
        cv2.CHAIN_APPROX_SIMPLE
    )

    object_detected = False

    for cnt in contours:

        area = cv2.contourArea(cnt)

        # Minimum Area
        if area > 3000:

            x, y, w, h = cv2.boundingRect(cnt)

            # Draw Bounding Box
            cv2.rectangle(
                frame,
                (x, y),
                (x+w, y+h),
                (0,255,0),
                3
            )

            # Object Label
            cv2.putText(
                frame,
                "MATCHBOX DETECTED",
                (x, y-10),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.8,
                (0,255,0),
                2
            )

            print("OBJECT DETECTED")

            object_detected = True

    # If No Object Found
    if object_detected == False:

        cv2.putText(
            frame,
            "SEARCHING THE OBJECT",
            (150, 40),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.8,
            (0,0,255),
            2
        )

        print("SEARCHING THE OBJECT")

    # Show Output
    cv2.imshow("AI Object Detection", frame)

    # Press Q to Exit
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Close Everything
cap.release()
cv2.destroyAllWindows()