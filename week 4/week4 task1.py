import serial
import matplotlib
matplotlib.use('TkAgg')  # force external window
import matplotlib.pyplot as plt
import time

# --- Serial setup ---
ser = serial.Serial('COM7', 9600, timeout=1)
time.sleep(2)
ser.flushInput()

# --- Calibration step ---
print("Calibrating... please keep the sensor still.")
time.sleep(2)  # wait for Arduino to start sending data
baseline_samples = []
start_time = time.time()

# Collect ~1 second worth of data for stable averaging
while time.time() - start_time < 1.0:
    line = ser.readline().decode('utf-8', errors='ignore').strip()
    if line:
        parts = line.split(',')
        if len(parts) == 3:
            try:
                ax0 = int(parts[0])
                ay0 = int(parts[1])
                baseline_samples.append((ax0, ay0))
            except ValueError:
                pass

# Compute average baseline if we got enough samples
if baseline_samples:
    avg_ax = sum(p[0] for p in baseline_samples) / len(baseline_samples)
    avg_ay = sum(p[1] for p in baseline_samples) / len(baseline_samples)
    baseline = (avg_ax, avg_ay)
    print(f"Baseline set to: ax0={avg_ax:.2f}, ay0={avg_ay:.2f}")
else:
    print("⚠️ No valid data during calibration, using (0,0) as baseline.")
    baseline = (0, 0)

print("Calibration done! Starting plot...\n")


# --- Plot setup ---
plt.ion()
fig, ax = plt.subplots()
ax.set_xlim(-20000, 20000)
ax.set_ylim(-20000, 20000)
ax.set_xlabel("AX")
ax.set_ylabel("AY")
ax.set_title("Real-Time Accelerometer Plot (MPU6050)")
# --- Graph paper style grid ---
ax.grid(True, which='both', color='gray', linestyle='--', linewidth=0.5)

# mark origin for reference
ax.plot(0, 0, 'k+', markersize=10)

dot, = ax.plot([], [], 'ko')  # moving dot
trail, = ax.plot([], [], 'k-', alpha=0.3)  # faint trail

x_vals, y_vals = [], []

print("Listening to serial data... (press Ctrl+C to stop)")

# --- Main loop ---
while True:
    try:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if line:
            parts = line.split(',')
            # Expecting 3 values: ax, ay, az
            if len(parts) == 3:
                try:
                    ax_val = int(parts[0]) - baseline[0]
                    ay_val = int(parts[1]) - baseline[1]

                    x_vals.append(ax_val)
                    y_vals.append(ay_val)

                    # Keep only the latest 100 points to avoid overflow
                   # x_vals = x_vals[-100:]
                   # y_vals = y_vals[-100:]

                    dot.set_data([x_vals[-1]], [y_vals[-1]])  # moving dot
                    trail.set_data(x_vals, y_vals)  # trail of motion

                    plt.pause(0.05)
                except ValueError:
                    pass  # skip bad lines
    except KeyboardInterrupt:
        print("\nStopped by user.")
        break
