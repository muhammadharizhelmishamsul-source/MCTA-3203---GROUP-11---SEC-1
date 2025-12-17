import serial
import time
import csv

ser = serial.Serial('COM7', 9600, timeout=1)
time.sleep(2)

color_map = {0: "RED", 1: "GREEN", 2: "BLUE"}

correct = 0
total = 0

file = open("accuracy_test.csv", "w", newline="")
writer = csv.writer(file)
writer.writerow(["Actual Color", "Detected Color", "R", "G", "B", "Response Time (ms)"])

def detect_color(r, g, b):
    if r < g and r < b:
        return "RED"
    elif g < r and g < b:
        return "GREEN"
    elif b < r and b < g:
        return "BLUE"
    else:
        return "UNKNOWN"

print("Testing started... CTRL+C to stop")

try:
    while True:
        data = ser.readline().decode().strip()
        if data:
            try:
                actual_id, r, g, b, response = map(int, data.split(","))
                actual = color_map[actual_id]
                detected = detect_color(r, g, b)

                total += 1
                if actual == detected:
                    correct += 1

                accuracy = (correct / total) * 100

                print(f"Actual={actual} | Detected={detected} | "
                      f"Accuracy={accuracy:.2f}% | {response} ms")

                writer.writerow([actual, detected, r, g, b, response])

            except ValueError:
                pass

except KeyboardInterrupt:
    print("\nTest ended")
    print(f"Final Accuracy: {(correct/total)*100:.2f}%")
    file.close()
    ser.close()
