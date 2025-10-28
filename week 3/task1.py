import serial
import time

# Replace 'COMx' with your Arduino port (e.g., 'COM3' on Windows or '/dev/ttyUSB0' on Linux/Mac)
ser = serial.Serial('COM5', 9600)
time.sleep(2)  # wait for Arduino to reset

try:
    while True:
        # Read potentiometer value
        pot_value = ser.readline().decode().strip()
        if pot_value.isdigit():
            pot_value = int(pot_value)
            print("Potentiometer Value:", pot_value)

            # Send control signal for LED
            if pot_value > 512:
                ser.write(b'1')  # Turn ON LED
            else:
                ser.write(b'0')  # Turn OFF LED

except KeyboardInterrupt:
    ser.close()
    print("Serial connection closed.")