// RGB LED pins
int redPin = 10;
int greenPin = 11;
int bluePin = 12;

// Button pin
int buttonPin = 2;

int buttonState = HIGH;
int lastButtonState = HIGH;
int colorState = 0;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP); // internal pull-up
}

void loop() {
  buttonState = digitalRead(buttonPin);

  // Detect button press (falling edge)
  if (lastButtonState == HIGH && buttonState == LOW) {
    colorState++;
    if (colorState > 3) {
      colorState = 0;
    }
    delay(200); // debounce
  }

  lastButtonState = buttonState;

  // Set color based on state
  switch (colorState) {
    case 0: // OFF
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, LOW);
      digitalWrite(bluePin, LOW);
      break;

    case 1: // RED
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, LOW);
      digitalWrite(bluePin, LOW);
      break;

    case 2: // GREEN
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      digitalWrite(bluePin, LOW);
      break;

    case 3: // BLUE
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, LOW);
      digitalWrite(bluePin, HIGH);
      break;
  }
}
