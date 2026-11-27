#include <Wire.h>
#include <EEPROM.h>
#include "DFRobot_LedDisplayModule.h"

// Motor & Driver Pins
const int ENA = 9;
const int IN1 = 6;
const int IN2 = 5;

// Sensors
const int REED1 = 12;  // Home / Stop sensor on return
const int REED2 = 3;   // Far station sensor
const int LDR_PIN = A2;

const int DARK_THRESHOLD = 750; // readings for darkness

DFRobot_LedDisplayModule LED(&Wire, 0x48);

bool movingForward = true;
bool trainRunning = false; // Tracks whether a round trip is currently active
bool armed = true;         // Tracks whether the system has seen light and is ready to trigger again
int currentSpeed = 0;
int count = 0;             // Total round trip counter

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  
  pinMode(REED1, INPUT_PULLUP);
  pinMode(REED2, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  
  Wire.begin();
  while (LED.begin(LED.e4Bit) != 0);  // screen connect
  LED.setBrightness(4);

  // Load saved count from memory
  EEPROM.get(0, count);
  if (count < 0 || count > 9999) count = 0;
  showNumber(count);

  Serial.begin(9600);
  Serial.println("Train System Initialized with LED Counter. Waiting for darkness to start...");
}

void loop() {
  // --- IDLE STATE: Wait for darkness if train is not running ---
  if (!trainRunning) {
    int ldrValue = analogRead(LDR_PIN);
    Serial.print("LDR Value: ");
    Serial.print(ldrValue);
    Serial.print(" | Armed: ");
    Serial.println(armed ? "YES" : "NO (Waiting for light)");

    // 1. Check if it's bright again to re-arm the system
    if (ldrValue <= (DARK_THRESHOLD - 100)) { 
      if (!armed) {
        armed = true;
        Serial.println("Light detected! System re-armed.");
      }
    }

    // 2. Check if it gets dark AND the system is armed
    if (ldrValue > DARK_THRESHOLD && armed) { 
      Serial.println("Darkness detected! Starting 1 round trip...");
      trainRunning = true;
      movingForward = true;
      armed = false; // Disarm so it cannot trigger again until it sees light
      
      // Start moving forward from home station
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      
      // Smoothly speed up from 0 to 140
      for (int s = 0; s <= 140; s++) {
        analogWrite(ENA, s);
        currentSpeed = s;
        delay(20);
      }
      Serial.print("Current Speed: ");
      Serial.println(currentSpeed);
    }
    
    delay(1000); // Check the light level once every second while waiting
    return;      // Skip the rest of loop until triggered
  }

  // --- ACTIVE ROUND TRIP ---
  if (movingForward) {
    // --- FORWARD TRIP ---
    // Reed 1: Cruise (maintain speed)
    if (digitalRead(REED1) == LOW) {
      analogWrite(ENA, 140);
      currentSpeed = 140;
      Serial.print("Reed 1 Hit - Current Speed: ");
      Serial.println(currentSpeed);
      while (digitalRead(REED1) == LOW);
    }
    // Reed 2: Far station reached - slow down, wait, reverse direction
    else if (digitalRead(REED2) == LOW) {
      Serial.println("Reed 2 Hit - Slowing down at far station...");
      for (int s = currentSpeed; s >= 60; s--) {
        analogWrite(ENA, s);
        currentSpeed = s;
        delay(33);
      }
      currentSpeed = 60;
      
      delay(5000); // Station wait
      
      movingForward = false;
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      
      Serial.println("Reversing direction back to home...");
      for (int s = 60; s <= 140; s++) {
        analogWrite(ENA, s);
        currentSpeed = s;
        delay(25);
      }
      currentSpeed = 140;
      Serial.print("Current Speed: ");
      Serial.println(currentSpeed);
      
      while (digitalRead(REED2) == LOW);
    }
  } 
  else {
    // --- REVERSE TRIP (Heading back to home station) ---
    // Reed 2 (Reverse): Cruise
    if (digitalRead(REED2) == LOW) {
      analogWrite(ENA, 140);
      currentSpeed = 140;
      Serial.print("Reed 2 (Reverse) Hit - Current Speed: ");
      Serial.println(currentSpeed);
      while (digitalRead(REED2) == LOW);
    }
    // Reed 1 (Reverse): Back at home station - stop completely & increment counter
    else if (digitalRead(REED1) == LOW) {
      Serial.println("Reed 1 (Reverse) Hit - Arrived home. Stopping train...");
      
      // Smoothly stop the train
      for (int s = currentSpeed; s >= 0; s--) {
        analogWrite(ENA, s);
        currentSpeed = s;
        delay(31);
      }
      currentSpeed = 0;
      analogWrite(ENA, 0);
      
      // Turn off motor driver outputs
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);

      // --- INCREMENT COUNTER & SAVE ---
      count++;
      if (count > 9999) count = 0;
      EEPROM.put(0, count);

      // Party easter egg every 50 trips
      if (count % 50 == 0) {
        unsigned long start = millis();
        LED.setBrightness(4);

        while (millis() - start < 2000) {
          for (int d = 1; d <= 4; d++) { LED.setDisplayArea(d); LED.print("-"); delay(35); }
          for (int d = 3; d >= 2; d--) { LED.setDisplayArea(d); LED.print("-"); delay(35); }
        }

        while (millis() - start < 3500) {
          LED.setDisplayArea(1, 2, 3, 4);
          int pick = random(3);
          if (pick == 1) LED.print("----");
          else           LED.print(random(1000, 9999));
          delay(45);
        }

        showNumber(count);
        while (millis() - start < 5000) {
          LED.setBrightness(0);
          delay(150);
          LED.setBrightness(4);
          delay(150);
        }

        LED.setBrightness(5); // Restore normal brightness
      }

      showNumber(count); // Update display with new count
      
      Serial.println("Round trip complete. Waiting for light before next trigger cycle.");
      
      // Reset state so it waits for light -> then darkness
      trainRunning = false;
      
      while (digitalRead(REED1) == LOW);
    }
  }
}

// Helper function to display numbers without leading zeros
void showNumber(int n) {
  if (n < 10)       LED.setDisplayArea(4);
  else if (n < 100)  LED.setDisplayArea(3, 4);
  else if (n < 1000) LED.setDisplayArea(2, 3, 4);
  else               LED.setDisplayArea(1, 2, 3, 4);
  LED.print(n);
}
//Project by Noor Zatara, Samuel Kuperhouse and Bayezid Sikder
