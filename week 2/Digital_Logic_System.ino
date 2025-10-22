// Define the pins for each segment (D0 to D6)
const int segmentA = 7; // D0
const int segmentB = 6; // D1
const int segmentC = 4; // D2
const int segmentD = 2; // D3
const int segmentE = 1; // D4
const int segmentF = 9; // D5
const int segmentG = 10; // D6

// Define push button pins
const int buttonInc = 11; // increment button
const int buttonReset = 12; // reset button

int number = 0; // start from 0

void setup() {
  // Initialize the digital pins as OUTPUTs
  pinMode(segmentA, OUTPUT);
  pinMode(segmentB, OUTPUT);
  pinMode(segmentC, OUTPUT);
  pinMode(segmentD, OUTPUT);
  pinMode(segmentE, OUTPUT);
  pinMode(segmentF, OUTPUT);
  pinMode(segmentG, OUTPUT);

  // Initialize buttons as INPUT_PULLUP (press = LOW)
  pinMode(buttonInc, INPUT_PULLUP);
  pinMode(buttonReset, INPUT_PULLUP);

  // Start with 0 displayed
  showNumber(number);
}

// Function to turn OFF all segments
void clearDisplay() {
  digitalWrite(segmentA, LOW);
  digitalWrite(segmentB, LOW);
  digitalWrite(segmentC, LOW);
  digitalWrite(segmentD, LOW);
  digitalWrite(segmentE, LOW);
  digitalWrite(segmentF, LOW);
  digitalWrite(segmentG, LOW);
}

// Function to display number (0–9)
void showNumber(int n) {
  clearDisplay();

  switch (n) {
    case 0:
      digitalWrite(segmentA, HIGH);
      digitalWrite(segmentB, HIGH);
      digitalWrite(segmentC, HIGH);
      digitalWrite(segmentD, HIGH);
      digitalWrite(segmentE, HIGH);
      digitalWrite(segmentF, HIGH);
      break;
    case 1:
      digitalWrite(segmentB, HIGH);
      digitalWrite(segmentC, HIGH);
      break;
    case 2:
      digitalWrite(segmentA, HIGH);
      digitalWrite(segmentB, HIGH);
      digitalWrite(segmentG, HIGH);
      digitalWrite(segmentE, HIGH);
      digitalWrite(segmentD, HIGH);
      break;
    case 3:
      digitalWrite(segmentA, HIGH);
      digitalWrite(segmentB, HIGH);
      digitalWrite(segmentC, HIGH);
      digitalWrite(segmentD, HIGH);
      digitalWrite(segmentG, HIGH);
      break;
    case 4:
      digitalWrite(segmentF, HIGH);
      digitalWrite(segmentG, HIGH);
      digitalWrite(segmentB, HIGH);
      digitalWrite(segmentC, HIGH);
      break;
    case 5:
      digitalWrite(segmentA, HIGH);
      digitalWrite(segmentF, HIGH);
      digitalWrite(segmentG, HIGH);
      digitalWrite(segmentC, HIGH);
      digitalWrite(segmentD, HIGH);
      break;
    case 6:
      digitalWrite(segmentA, HIGH);
      digitalWrite(segmentF, HIGH);
      digitalWrite(segmentE, HIGH);
      digitalWrite(segmentD, HIGH);
      digitalWrite(segmentC, HIGH);
      digitalWrite(segmentG, HIGH);
      break;
    case 7:
      digitalWrite(segmentA, HIGH);
      digitalWrite(segmentB, HIGH);
      digitalWrite(segmentC, HIGH);
      break;
    case 8:
      digitalWrite(segmentA, HIGH);
      digitalWrite(segmentB, HIGH);
      digitalWrite(segmentC, HIGH);
      digitalWrite(segmentD, HIGH);
      digitalWrite(segmentE, HIGH);
      digitalWrite(segmentF, HIGH);
      digitalWrite(segmentG, HIGH);
      break;
    case 9:
      digitalWrite(segmentA, HIGH);
      digitalWrite(segmentB, HIGH);
      digitalWrite(segmentC, HIGH);
      digitalWrite(segmentD, HIGH);
      digitalWrite(segmentF, HIGH);
      digitalWrite(segmentG, HIGH);
      break;
  }
}

void loop() {
  // Read button states (LOW when pressed)
  bool incPressed = digitalRead(buttonInc) == LOW;
  bool resetPressed = digitalRead(buttonReset) == LOW;

  // Increment button
  if (incPressed) {
    number++;
    if (number > 9) number = 0; // loop back to 0 after 9
    showNumber(number);
    delay(300); // debounce
  }

  // Reset button
  if (resetPressed) {
    number = 0;
    showNumber(number);
    delay(300); // debounce
  }
}
