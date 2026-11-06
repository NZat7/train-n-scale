const int ENA = 9;
const int IN1 = 5;
const int IN2 = 6;

const int REED1 = 11; // First reed encountered going forward
const int REED2 = 12; // Second reed encountered going forward (Station / Stop)

bool movingForward = true;
int currentSpeed = 0;

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  
  pinMode(REED1, INPUT_PULLUP);
  pinMode(REED2, INPUT_PULLUP);

  // Initialize Serial Monitor
  Serial.begin(9600);
  Serial.println("Train System Initialized.");

  // Start moving forward and speed up in the beginning
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  
  // Smoothly speed up from 0 to 160 over ~2 seconds
  for (int s = 0; s <= 160; s++) {
    analogWrite(ENA, s);
    currentSpeed = s;
    delay(26);
  }
  Serial.print("Current Speed: ");
  Serial.println(currentSpeed);
}

void loop() {
  if (movingForward) {
    // --- FORWARD TRIP ---
    // Reed 1: Cruise (maintain speed)
    if (digitalRead(REED1) == LOW) {
      analogWrite(ENA, 160);
      currentSpeed = 160;
      Serial.print("Reed 1 Hit - Current Speed: ");
      Serial.println(currentSpeed);
      while (digitalRead(REED1) == LOW);
    }
    // Reed 2: Smoothly slow down to 60 over 2 seconds, wait, reverse, and speed up
    else if (digitalRead(REED2) == LOW) {
      Serial.println("Reed 2 Hit - Slowing down to 60...");
      for (int s = currentSpeed; s >= 60; s--) {
        analogWrite(ENA, s);
        currentSpeed = s;
        delay(15);
      }
      currentSpeed = 60;
      Serial.print("Current Speed: ");
      Serial.println(currentSpeed);
      
      delay(7000); // Station wait
      
      movingForward = false;
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      
      Serial.println("Reversing direction - Speeding up...");
      for (int s = 60; s <= 160; s++) {
        analogWrite(ENA, s);
        currentSpeed = s;
        delay(20);
      }
      currentSpeed = 160;
      Serial.print("Current Speed: ");
      Serial.println(currentSpeed);
      
      while (digitalRead(REED2) == LOW);
    }
  } 
  else {
    // --- REVERSE TRIP (Hits Reed 1 first, then Reed 2) ---
    // Reed 1 (Reverse): Cruise (maintain speed)
    if (digitalRead(REED1) == LOW) {
      analogWrite(ENA, 160);
      currentSpeed = 160;
      Serial.print("Reed 1 (Reverse) Hit - Current Speed: ");
      Serial.println(currentSpeed);
      while (digitalRead(REED1) == LOW);
    }
    // Reed 2 (Reverse): Smoothly slow down to 60 over 2 seconds, wait, reverse, and speed up
    else if (digitalRead(REED2) == LOW) {
      Serial.println("Reed 2 (Reverse) Hit - Slowing down to 60...");
      for (int s = currentSpeed; s >= 60; s--) {
        analogWrite(ENA, s);
        currentSpeed = s;
        delay(15);
      }
      currentSpeed = 60;
      Serial.print("Current Speed: ");
      Serial.println(currentSpeed);
      
      delay(7000); // Station wait
      
      movingForward = true;
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      
      Serial.println("Resuming forward direction - Speeding up...");
      for (int s = 60; s <= 160; s++) {
        analogWrite(ENA, s);
        currentSpeed = s;
        delay(26);
      }
      currentSpeed = 160;
      Serial.print("Current Speed: ");
      Serial.println(currentSpeed);
      
      while (digitalRead(REED2) == LOW);
    }
  }
}
