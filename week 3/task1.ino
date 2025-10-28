int potPin = A0;
int ledPin = 8;
int potValue = 0;
char command;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  potValue = analogRead(potPin);
  Serial.println(potValue);

  if (Serial.available()) {
    command = Serial.read();
    if (command == '1') digitalWrite(ledPin, HIGH);
    else if (command == '0') digitalWrite(ledPin, LOW);
  }

  delay(100);
}