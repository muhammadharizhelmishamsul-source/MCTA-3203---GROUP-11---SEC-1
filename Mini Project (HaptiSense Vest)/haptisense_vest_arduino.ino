#include <SoftwareSerial.h>

// Software Serial for ESP32 communication
#define ARDUINO_RX 0
#define ARDUINO_TX 1

SoftwareSerial espSerial(ARDUINO_RX, ARDUINO_TX);

// Motor A Pins
#define ENA 10
#define IN1 7
#define IN2 8

// Motor B Pins
#define ENB 11
#define IN3 5
#define IN4 3

// Security Features
#define BUTTON_PIN 2
#define BUZZER_PIN 4

int distance = 0;
int previousDistance = -1;
bool systemLocked = true;  // Start locked

// Button handling
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Speed levels
#define SPEED_STRONG  255
#define SPEED_MEDIUM  170
#define SPEED_LOW     85

// Distance thresholds
#define RANGE_MIN     50
#define RANGE_MAX     300
#define THRESHOLD_CLOSE  100
#define THRESHOLD_FAR    200

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200);
  
  // Motor pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Button and Buzzer
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  stopMotors();
  
  Serial.println("========================================");
  Serial.println("   DUAL MOTOR + SECURITY SYSTEM");
  Serial.println("========================================");
  Serial.println("🔘 Button Pin 2 - Press to LOCK");
  Serial.println("🔊 Buzzer Pin 4 - Wrong face alert");
  Serial.println("🔒 Status: LOCKED (waiting for Face ID 1)");
  Serial.println("========================================\n");
  
  // Startup beep
  beep(100);
}

void loop() {
  // Check button for lock
  checkButton();
  
  // Read commands from ESP32
  if (espSerial.available()) {
    String input = espSerial.readStringUntil('\n');
    input.trim();
    
    if (input.startsWith("D:")) {
      // Distance data
      distance = input.substring(2).toInt();
      
      if (distance != previousDistance) {
        Serial.print("📥 Distance: ");
        Serial.print(distance);
        Serial.print(" cm → ");
        
        controlMotors(distance);
        previousDistance = distance;
      }
      
    } else if (input == "ALERT") {
      // ⚠️ WRONG FACE DETECTED
      Serial.println("\n🚨🚨🚨 INTRUDER ALERT! 🚨🚨🚨");
      Serial.println("Wrong face detected by HuskyLens!");
      Serial.println("System remains LOCKED\n");
      intruderAlert();
      
    } else if (input == "UNLOCK") {
      // ✅ CORRECT FACE - System unlocked
      systemLocked = false;
      Serial.println("\n✅✅✅ SYSTEM UNLOCKED ✅✅✅");
      Serial.println("Face ID 1 verified - Syazmie detected");
      Serial.println("Motors now active\n");
      successBeep();
    }
  }
}

void checkButton() {
  bool buttonState = digitalRead(BUTTON_PIN);
  
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW) {
      Serial.println("\n🔘🔘🔘 LOCK BUTTON PRESSED! 🔘🔘🔘");
      Serial.println("Sending LOCK command to ESP32...");
      Serial.println("Show Face ID 1 to unlock again\n");
      
      // Send lock command to ESP32
      espSerial.println("LOCK");
      
      // Update local status
      systemLocked = true;
      
      // Stop motors immediately
      stopMotors();
      
      // Lock confirmation beeps
      beep(150);
      delay(100);
      beep(150);
      
      // Wait for button release
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }
  
  lastButtonState = buttonState;
}

void intruderAlert() {
  // LOUD ALARM: 4 fast beeps
  for (int i = 0; i < 4; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(150);
  }
}

void successBeep() {
  // Success: 2 short beeps
  beep(100);
  delay(100);
  beep(100);
}

void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

void controlMotors(int d) {
  if (d >= RANGE_MIN && d <= RANGE_MAX) {
    int speed;
    
    if (d <= THRESHOLD_CLOSE) {
      speed = SPEED_STRONG;
      Serial.print("STRONG ⚡⚡⚡ (");
    } 
    else if (d <= THRESHOLD_FAR) {
      speed = SPEED_MEDIUM;
      Serial.print("MEDIUM ⚡⚡ (");
    } 
    else {
      speed = SPEED_LOW;
      Serial.print("LOW ⚡ (");
    }
    
    // Motor A
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, speed);
    
    // Motor B
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, speed);
    
    Serial.print(speed);
    Serial.println(")");
    
  } else {
    stopMotors();
    if (d == 0) {
      Serial.println("STOPPED 🔒 (System Locked)");
    } else {
      Serial.println("STOPPED 🛑 (Out of range)");
    }
  }
}

void stopMotors() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  
  analogWrite(ENB, 0);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  
  delayMicroseconds(100);
}