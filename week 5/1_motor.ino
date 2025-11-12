// Motor A pin definitions
const int ENA = 3; // PWM pin for speed control
const int IN1 = 12; // Direction
const int IN2 = 13; // Direction
void setup() {
pinMode(ENA, OUTPUT);
pinMode(IN1, OUTPUT);
pinMode(IN2, OUTPUT);
}
void loop() {
// Move forward
digitalWrite(IN1, HIGH);
digitalWrite(IN2, LOW);
analogWrite(ENA, 200); // Speed: 0-255
delay(2000);
// Move backward
digitalWrite(IN1, LOW);
digitalWrite(IN2, HIGH);
analogWrite(ENA, 200);
delay(2000);
// Stop
analogWrite(ENA, 0);


delay(1000);
}
