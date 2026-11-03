const int ENA = 9;
const int IN1 = 5;
const int IN2 = 6;

const int REED1 = 11; // First reed encountered going forward
const int REED2 = 12; // Second reed encountered going forward

bool movingForward = true;
int currentSpeed = 0;

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  
  pinMode(REED1, INPUT_PULLUP);
  pinMode(REED2, INPUT_PULLUP);

  // Start moving forward and speed up in the beginning
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  
  // Smoothly speed up from 0 to 160 over ~2 seconds
  for (int s = 0; s <= 150; s++) {
    analogWrite(ENA, s);
    delay(10);
  }
  currentSpeed = 150;
}

void loop() {
  if (movingForward) {
    // Reed 1: Cruise (maintain speed)
    if (digitalRead(REED1) == LOW) {
      analogWrite(ENA, 150);
      currentSpeed = 150;
      while (digitalRead(REED1) == LOW);
    }
    // Reed 2: Smoothly slow down to stop over 2 seconds, wait, reverse, and speed up
    else if (digitalRead(REED2) == LOW) {
      for (int s = currentSpeed; s >= 50; s--) {
        analogWrite(ENA, s);
        delay(20);
      }
      currentSpeed = 50;
      delay(7000); // Station wait
      
      movingForward = false;
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      
      // Speed up in the beginning of reverse trip
      for (int s = 0; s <= 150; s++) {
        analogWrite(ENA, s);
        delay(10);
      }
      currentSpeed = 150;
      while (digitalRead(REED2) == LOW);
    }
  } 
  else {
    // Reverse Trip (hits Reed 2 first, then Reed 1)
    
    // Reed 2: Cruise (maintain speed)
    if (digitalRead(REED2) == LOW) {
      analogWrite(ENA, 150);
      currentSpeed = 150;
      while (digitalRead(REED2) == LOW);
    }
    // Reed 1: Smoothly slow down to stop over 2 seconds, wait, reverse, and speed up
    else if (digitalRead(REED1) == LOW) {
      for (int s = currentSpeed; s >= 50; s--) {
        analogWrite(ENA, s);
        delay(20);
      }
      currentSpeed = 50;
      delay(7000); // Station wait
      
      movingForward = true;
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      
      // Speed up in the beginning of forward trip
      for (int s = 0; s <= 150; s++) {
        analogWrite(ENA, s);
        delay(10);
      }
      currentSpeed = 150;
      while (digitalRead(REED1) == LOW);
    }
  }
}
