const int ENA = 9;
const int IN1 = 6;
const int IN2 = 5;

const int REED1 = 12;  // Home / Stop sensor on return
const int REED2 = 3; // Far station sensor

const int LDR_PIN = A2;
const int DARK_THRESHOLD = 800; // Adjust based on your Serial Monitor readings for darkness

bool movingForward = true;
bool trainRunning = false; // Tracks whether a round trip is currently active
int currentSpeed = 0;

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  
  pinMode(REED1, INPUT_PULLUP);
  pinMode(REED2, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  // Initialize Serial Monitor
  Serial.begin(9600);
  Serial.println("Train System Initialized. Waiting for darkness to start...");
}

void loop() {
  // --- IDLE STATE: Wait for darkness if train is not running ---
  if (!trainRunning) {
    int ldrValue = analogRead(LDR_PIN);
    Serial.print("LDR Value: ");
    Serial.println(ldrValue);

    // Check if it gets dark (higher values = darker)
    if (ldrValue > DARK_THRESHOLD) { 
      Serial.println("Darkness detected! Starting 1 round trip...");
      trainRunning = true;
      movingForward = true;
      
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
        delay(10);
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
    // Reed 1 (Reverse): Back at home station - stop completely
    else if (digitalRead(REED1) == LOW) {
      Serial.println("Reed 1 (Reverse) Hit - Arrived home. Stopping train...");
      
      // Smoothly stop the train
      for (int s = currentSpeed; s >= 0; s--) {
        analogWrite(ENA, s);
        currentSpeed = s;
        delay(25);
      }
      currentSpeed = 0;
      analogWrite(ENA, 0);
      
      // Turn off motor driver outputs
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      
      Serial.println("Round trip complete. Waiting for next dark trigger.");
      
      // Reset state so it waits for darkness again
      trainRunning = false;
      
      while (digitalRead(REED1) == LOW);
    }
  }
}
