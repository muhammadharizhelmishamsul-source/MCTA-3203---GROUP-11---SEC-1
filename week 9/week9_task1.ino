// ===== RGB LED PINS =====
#define RED_LED   10
#define GREEN_LED 11
#define BLUE_LED  12

// ===== TCS3200 PINS =====
#define S0 4
#define S1 5
#define S2 7
#define S3 6
#define SENSOR_OUT 8

unsigned long previousMillis = 0;
const long interval = 5000;

int colorIndex = 0; // 0=RED, 1=GREEN, 2=BLUE

void setup() {
  Serial.begin(9600);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(SENSOR_OUT, INPUT);

  // Frequency scaling = 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
}

void setColor(int r, int g, int b) {
  analogWrite(RED_LED, r);
  analogWrite(GREEN_LED, g);
  analogWrite(BLUE_LED, b);
}

int readColor(bool s2, bool s3) {
  digitalWrite(S2, s2);
  digitalWrite(S3, s3);
  delay(30);
  return pulseIn(SENSOR_OUT, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // ===== Known colour sequence =====
    if (colorIndex == 0) setColor(255, 0, 0);   // RED
    if (colorIndex == 1) setColor(0, 255, 0);   // GREEN
    if (colorIndex == 2) setColor(0, 0, 255);   // BLUE

    delay(100); // stabilisation

    unsigned long startTime = millis();

    int red   = readColor(LOW, LOW);
    int green = readColor(HIGH, HIGH);
    int blue  = readColor(LOW, HIGH);

    unsigned long responseTime = millis() - startTime;

    // Send: ActualColor,R,G,B,ResponseTime
    Serial.print(colorIndex);
    Serial.print(",");
    Serial.print(red);
    Serial.print(",");
    Serial.print(green);
    Serial.print(",");
    Serial.print(blue);
    Serial.print(",");
    Serial.println(responseTime);

    colorIndex = (colorIndex + 1) % 3;
  }
}
