import cv2
import requests
from ultralytics import YOLO

model = YOLO("yolo11n.pt")

cap = cv2.VideoCapture(0)

while True:

    ret, frame = cap.read()

    if not ret:
        break

    results = model(frame)

    person_count = 0

    for result in results:

        for box in result.boxes:

            class_id = int(box.cls[0])

            if class_id == 0:

                person_count += 1

                x1, y1, x2, y2 = map(int, box.xyxy[0])

                confidence = float(box.conf[0])

                cv2.rectangle(
                    frame,
                    (x1, y1),
                    (x2, y2),
                    (0, 255, 0),
                    2
                )

                cv2.putText(
                    frame,
                    "Person " + str(round(confidence, 2)),
                    (x1, y1 - 10),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.6,
                    (0, 255, 0),
                    2
                )

    print("People detected:", person_count)

    data = {
        "busID": "B1",
        "occupancy": person_count
    }

    try:

        response = requests.post(
            "http://127.0.0.1:5000/update-occupancy",
            json=data
        )

        print("Sent to Flask:", data)
        print("Flask response:", response.status_code)

    except requests.exceptions.RequestException:
        print("Flask server not connected")

    cv2.putText(
        frame,
        "People: " + str(person_count),
        (20, 40),
        cv2.FONT_HERSHEY_SIMPLEX,
        1,
        (0, 255, 0),
        2
    )

    cv2.imshow("Bus Occupancy", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()