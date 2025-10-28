#include <Servo.h>

Servo myservo;
int potPin = A0;    // Potentiometer connected to A0
int servoPin = 9;   // Servo control pin
int ledPin = 6;     // LED pin (PWM capable)

void setup() {
  Serial.begin(9600);
  myservo.attach(servoPin);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  int potValue = analogRead(potPin);            // Read potentiometer (0–1023)
  int angle = map(potValue, 0, 1023, 0, 180);   // Map to servo angle
  int brightness = map(potValue, 0, 1023, 0, 255); // Map to LED brightness

  myservo.write(angle);         // Move servo
  analogWrite(ledPin, brightness); // Adjust LED brightness

  Serial.println(potValue);     // Send pot value to Python for plotting
  delay(100);                   // Short delay for smooth movement
}
