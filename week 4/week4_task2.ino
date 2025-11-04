#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Servo.h>

Adafruit_MPU6050 mpu;
Servo gateServo;

const int redLED = 7;
const int greenLED = 6;
const int servoPin = 3;

void setup() {
  Serial.begin(9600);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  gateServo.attach(servoPin);
  gateServo.write(0);

  if (!mpu.begin()) {
    Serial.println("[Arduino]: ERROR: MPU6050 not detected!");
    while (1);
  }

  Serial.println("[Arduino]: Arduino ready...");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    if (cmd == 'A') {
      Serial.println("[Arduino]: Authorized: Opening gate...");
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, LOW);
      gateServo.write(90);
      delay(2000);
      gateServo.write(0);
      digitalWrite(greenLED, LOW);
    }

    else if (cmd == 'D') {
      Serial.println("[Arduino]: Denied: Access blocked!");
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, LOW);
      gateServo.write(0);
    }

    else if (cmd == 'M') {
      Serial.println("[Arduino]: Starting motion detection...");
      bool motionDetected = detectCircularMotion();
      if (motionDetected) {
        Serial.println("MOTION_OK");    // <— clean and single word
      } else {
        Serial.println("MOTION_FAIL");  // <— clean and single word
      }
    }
  }
}

bool detectCircularMotion() {
  sensors_event_t a, g, temp;
  unsigned long start = millis();
  bool moved = false;
  float threshold = 0.3;

  while (millis() - start < 5000) {  // 5 seconds window
    mpu.getEvent(&a, &g, &temp);

    float gx = g.gyro.x;
    float gy = g.gyro.y;
    float gz = g.gyro.z;

    if (abs(gx) > threshold || abs(gy) > threshold || abs(gz) > threshold) {
      moved = true;
    }

    delay(250); // fewer serial events = less corruption
  }

  if (moved) {
    Serial.println("[Arduino]: Motion detected!");
  } else {
    Serial.println("[Arduino]: No motion detected.");
  }

  return moved;
}
