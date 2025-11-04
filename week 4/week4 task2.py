import serial
import time
import re
import threading
import keyboard  # pip install keyboard

SERIAL_PORT = 'COM7'
BAUD_RATE = 9600

# List of authorized RFID UIDs
authorized_cards = ["0013078065", "04AABBCCDD", "1234567890"]

# Thread-safe UID
uid_lock = threading.Lock()
pending_uid = None

def send_to_arduino(ser, ch):
    """Send single-char command to Arduino"""
    try:
        ser.write(ch.encode())
        ser.flush()
    except Exception as e:
        print("⚠️ Error sending to Arduino:", e)

def hid_listener():
    """Capture HID-style RFID input"""
    global pending_uid
    buf = ""
    while True:
        event = keyboard.read_event(suppress=False)
        if event.event_type != keyboard.KEY_DOWN:
            continue
        name = event.name
        if name == "enter":
            uid = buf.strip()
            buf = ""
            if uid:
                with uid_lock:
                    pending_uid = uid.upper()
            continue
        if len(name) == 1:
            buf += name

def handle_uid(uid, ser):
    """Process a scanned UID"""
    uid_clean = uid.strip().upper()
    print(f"\n💳 Card detected → UID: {uid_clean}")

    if uid_clean in authorized_cards:
        print("✅ Authorized card detected.")
        print("🌀 Requesting motion verification...\n")
        send_to_arduino(ser, 'M')

        start = time.time()
        motion_ok = False
        while time.time() - start < 8:
            try:
                line = ser.readline().decode(errors='ignore').strip()
            except Exception:
                continue
            if not line:
                continue
            # Only check for motion result
            if re.search(r"MOTION[_\s]?OK", line, re.IGNORECASE):
                motion_ok = True
                break
            if re.search(r"MOTION[_\s]?FAIL", line, re.IGNORECASE):
                motion_ok = False
                break

        if motion_ok:
            print("✅ Motion verified successfully.")
            send_to_arduino(ser, 'A')
            print("🟢 Access granted.\n")
        else:
            print("🔴 Motion invalid or timeout.")
            send_to_arduino(ser, 'D')
            print("⛔ Access denied.\n")
    else:
        print("❌ Unauthorized card detected.")
        send_to_arduino(ser, 'D')
        print("⛔ Access denied.\n")

    # Display ready message after every card scan
    print("Ready to scan RFID cards...\n")

def main():
    global pending_uid
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.5)
        time.sleep(2)
        print(f"✅ Connected to Arduino on {SERIAL_PORT}")
        print("Ready to scan RFID cards...\n")

        threading.Thread(target=hid_listener, daemon=True).start()

        while True:
            # Check for HID card input
            with uid_lock:
                if pending_uid:
                    uid_to_process = pending_uid
                    pending_uid = None
                else:
                    uid_to_process = None

            if uid_to_process:
                handle_uid(uid_to_process, ser)

            # Simulation keys (optional; remove if not needed)
            if keyboard.is_pressed('1'):
                handle_uid(authorized_cards[0], ser)
                time.sleep(0.4)
            elif keyboard.is_pressed('2'):
                handle_uid("SIM_UNAUTH_123", ser)
                time.sleep(0.4)

            time.sleep(0.05)

    except serial.SerialException as e:
        print("⚠️ Serial error:", e)
    except KeyboardInterrupt:
        print("\n🛑 Program terminated by user.")
    finally:
        try:
            ser.close()
        except:
            pass
        print("🔌 Serial connection closed.")

if __name__ == "__main__":
    main()
