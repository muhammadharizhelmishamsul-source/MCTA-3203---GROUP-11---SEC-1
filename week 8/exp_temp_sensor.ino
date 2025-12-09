#include "BluetoothSerial.h"
#include "DHT.h"

// --- Bluetooth ---
BluetoothSerial SerialBT;
const char* deviceName = "ESP32-DHT22-Monitor";

// --- DHT22 Sensor ---
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- Output LED/Fan ---
const int FAN_PIN = 2;

void setup() {
  Serial.begin(115200);

  // Start Bluetooth
  SerialBT.begin(deviceName);
  Serial.println("Bluetooth Started. Pair now.");

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  dht.begin();
}

void loop() {

  // ---- Read DHT22 ----
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    SerialBT.println("DHT22 Error: No sensor detected");

    // Serial Plotter — send dummy value (optional)
    Serial.println(0);
  } 
  else {

    // ---- Bluetooth output ----
    SerialBT.print("Temperature: ");
    SerialBT.print(temp);
    SerialBT.print(" °C | Humidity: ");
    SerialBT.print(hum);
    SerialBT.println(" %");

    // ---- Serial Plotter graph output ----
    // DO NOT add text, only numbers
    Serial.println(temp);
  }

  // ---- Bluetooth Commands ----
  if (SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();

    if (cmd.equalsIgnoreCase("FAN ON")) {
      digitalWrite(FAN_PIN, HIGH);
      SerialBT.println("LED turned ON");
    }
    else if (cmd.equalsIgnoreCase("FAN OFF")) {
      digitalWrite(FAN_PIN, LOW);
      SerialBT.println("LED turned OFF");
    }
    else {
      SerialBT.println("Unknown command");
    }
  }

  delay(1000); // Update every second
}
