#include <HardwareSerial.h>
#include "HUSKYLENS.h"

// UART to Arduino
HardwareSerial mySerial(1);
#define ARDUINO_TX 17
#define ARDUINO_RX 16

// TF-Luna UART
HardwareSerial TFSerial(2);
#define TFLUNA_RX 27
#define TFLUNA_TX 26

// HuskyLens I2C
HUSKYLENS huskylens;

uint16_t distance = 0;
bool systemUnlocked = false;
bool faceDetectedBefore = false;

// Timing control
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 300;
unsigned long lastHuskyCheck = 0;
const unsigned long huskyCheckInterval = 500;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 100;
unsigned long lastWrongFaceTime = 0;
const unsigned long wrongFaceCooldown = 3000;

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, ARDUINO_RX, ARDUINO_TX);
  TFSerial.begin(115200, SERIAL_8N1, TFLUNA_RX, TFLUNA_TX);
  
  // Initialize HuskyLens
  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println("HuskyLens connection failed! Check wiring.");
    delay(1000);
  }
  
  huskylens.writeAlgorithm(ALGORITHM_FACE_RECOGNITION);
  
  Serial.println("========================================");
  Serial.println("  TF-Luna LiDAR + HuskyLens System");
  Serial.println("========================================");
  Serial.println("🔒 SYSTEM LOCKED - Face ID 1 required");
  Serial.println("🔘 Button can LOCK anytime");
  Serial.println("✅ Face ID 1 can UNLOCK anytime");
  Serial.println("========================================\n");
  
  delay(1000);
}

void loop() {
  // Check for lock command from Arduino
  if (mySerial.available()) {
    String cmd = mySerial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd == "LOCK") {
      // Lock the system (can be unlocked again with face)
      systemUnlocked = false;
      faceDetectedBefore = false;
      
      Serial.println("\n========================================");
      Serial.println("🔒 SYSTEM LOCKED!");
      Serial.println("Button pressed on Arduino");
      Serial.println("Show Face ID 1 to unlock");
      Serial.println("========================================\n");
    }
  }
  
  // Always check HuskyLens (even when locked)
  if (millis() - lastHuskyCheck >= huskyCheckInterval) {
    lastHuskyCheck = millis();
    checkHuskyLens();
  }
  
  // Always read distance
  readTFLunaDistance();
  
  // Send data to Arduino
  if (millis() - lastSendTime >= sendInterval) {
    lastSendTime = millis();
    
    if (systemUnlocked) {
      mySerial.print("D:");
      mySerial.println(distance);
    } else {
      mySerial.println("D:0");  // Locked - motors off
    }
  }
  
  // Print status
  if (millis() - lastPrintTime >= printInterval) {
    lastPrintTime = millis();
    
    if (systemUnlocked) {
      Serial.print("🔓 UNLOCKED | 📏 Distance: ");
      if (distance > 0) {
        Serial.print(distance);
        Serial.print(" cm");
        
        if (distance >= 50 && distance <= 300) {
          Serial.print(" | Motor: ");
          if (distance <= 100) {
            Serial.println("STRONG ⚡⚡⚡");
          } else if (distance <= 200) {
            Serial.println("MEDIUM ⚡⚡");
          } else {
            Serial.println("LOW ⚡");
          }
        } else {
          Serial.println(" | Motor: OFF");
        }
      } else {
        Serial.println("---");
      }
    } else {
      Serial.println("🔒 LOCKED - Show Face ID 1 to unlock");
    }
  }
  
  delay(50);
}

void checkHuskyLens() {
  if (!huskylens.request()) {
    return;
  }
  
  if (huskylens.isLearned()) {
    if (huskylens.available()) {
      HUSKYLENSResult result = huskylens.read();
      
      if (result.command == COMMAND_RETURN_BLOCK) {
        int faceID = result.ID;
        
        if (faceID == 1) {
          // ✅ CORRECT FACE - Always unlocks
          if (!systemUnlocked) {
            systemUnlocked = true;
            faceDetectedBefore = true;
            
            Serial.println("\n========================================");
            Serial.println("✅ FACE ID 1 DETECTED - Syazmie");
            Serial.println("🔓 SYSTEM UNLOCKED!");
            Serial.println("========================================\n");
            
            mySerial.println("UNLOCK");
          }
          
        } else {
          // ❌ WRONG FACE - Only alert when locked
          if (!systemUnlocked && (millis() - lastWrongFaceTime >= wrongFaceCooldown)) {
            lastWrongFaceTime = millis();
            
            Serial.println("\n========================================");
            Serial.print("⚠️ INTRUDER ALERT! Face ID: ");
            Serial.println(faceID);
            Serial.println("🚨 Access Denied - Only ID 1 allowed");
            Serial.println("========================================\n");
            
            mySerial.println("ALERT");
          }
        }
      }
    }
  }
}

void readTFLunaDistance() {
  static uint8_t buffer[9];
  static int bufferIndex = 0;
  
  while (TFSerial.available()) {
    uint8_t byte = TFSerial.read();
    
    if (bufferIndex == 0 && byte != 0x59) {
      continue;
    }
    
    buffer[bufferIndex++] = byte;
    
    if (bufferIndex == 9) {
      if (buffer[0] == 0x59 && buffer[1] == 0x59) {
        uint8_t checksum = 0;
        for (int i = 0; i < 8; i++) {
          checksum += buffer[i];
        }
        
        if (checksum == buffer[8]) {
          distance = buffer[2] + (buffer[3] << 8);
          
          if (distance < 20 || distance > 800) {
            distance = 0;
          }
        } else {
          distance = 0;
        }
      }
      bufferIndex = 0;
    }
  }
}