// Motor 1 Pins (Rotation)
const int enable1 = 2;
const int stepPin1 = 4; 
const int dirPin1 = 5;
// Motor 2 Pins (Height/Lead Screw)
const int enable2 = 3;
 const int stepPin2 = 6;
  const int dirPin2 = 7;
// Microstepping Pins
const int ms1 = 8; 
const int ms2 = 9;
 const int ms3 = 10;

// Global tracking variables
float currentPosition = 0.0;
float currentHeight = 0.0;

// Motor Specs
const float STEPS_PER_REV = 200.0;
const int MICROSTEPPING = 16;
const float LEAD = 1.25; 

void setup() {
  pinMode(enable1, OUTPUT); pinMode(enable2, OUTPUT);
  pinMode(stepPin1, OUTPUT); pinMode(dirPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT); pinMode(dirPin2, OUTPUT);
  pinMode(ms1, OUTPUT); pinMode(ms2, OUTPUT); pinMode(ms3, OUTPUT);

  digitalWrite(enable1, LOW); digitalWrite(enable2, LOW);

  // Set Microstepping to 1/16 (DRV8825: L, L, H)
  digitalWrite(ms1, LOW); digitalWrite(ms2, LOW); digitalWrite(ms3, HIGH);

  Serial.begin(9600);
  while (!Serial) { delay(1); }
  Serial.println(F("System Ready - Enter Commands (e.g., A 90 or H 5)"));
}

void loop() {
  if (Serial.available() > 0) {
    char motorType = Serial.read(); 
    float targetvalue = Serial.parseFloat();

    if (motorType == 'A' || motorType == 'a') {
      Serial.print(F("Moving Angle to: ")); Serial.println(targetvalue);
      float error = rotation(targetvalue, currentPosition);
      printResults("Rotation", targetvalue, currentPosition, error, "deg");
    } 
    else if (motorType == 'H' || motorType == 'h') {
      Serial.print(F("Moving Height to: ")); Serial.println(targetvalue);
      float error = height(targetvalue, currentHeight); 
      printResults("Height", targetvalue, currentHeight, error, "mm");
    }
    
    while(Serial.available() > 0) { Serial.read(); }
  }
}

void printResults(String label, float target, float actual, float error, String unit) {
  Serial.println("--- " + label + " Report ---");
  Serial.print(F("Target: ")); Serial.print(target, 4); Serial.println(" " + unit);
  Serial.print(F("Actual: ")); Serial.print(actual, 4); Serial.println(" " + unit);
  Serial.print(F("Error : ")); Serial.print(error, 4);  Serial.println(" " + unit);
  Serial.println(F("-----------------------"));
}

float rotation(float angleDeg, float &currPosRef) {
  digitalWrite(enable1, LOW); 

  float stepsPerRev = STEPS_PER_REV * (float)MICROSTEPPING; 
  float angleToMove = angleDeg - currPosRef;
  long steps = abs((angleToMove / 360.0) * stepsPerRev);
  float actualDegreesMoved = (steps * 360.0) / stepsPerRev;

  digitalWrite(dirPin1, (angleToMove > 0) ? HIGH : LOW);

  // Ramp Logic: Ramp up/down for 20% of steps, but max 150 steps
  long rampSteps = min(steps / 5, 150L); 
  int delayTime = 1000; // Starting speed (slow)
  int minDelay = 400;   // Top speed (fast)

  for(long x = 0; x < steps; x++) {
    digitalWrite(stepPin1, HIGH); 
    delayMicroseconds(delayTime);
    digitalWrite(stepPin1, LOW); 
    delayMicroseconds(delayTime);

    // Acceleration
    if (x < rampSteps && delayTime > minDelay) {
      delayTime -= 2;
    }
    // Deceleration
    else if (x > (steps - rampSteps) && delayTime < 1000) {
      delayTime += 2;
    }
  }

  if (angleToMove > 0) currPosRef += actualDegreesMoved;
  else currPosRef -= actualDegreesMoved;

  delay(200);
  return abs(angleDeg - currPosRef);
}

float height(float targetH, float &currHeightRef) {
  digitalWrite(enable2, LOW); 

  float stepsPerRev = STEPS_PER_REV * (float)MICROSTEPPING; 
  float stepsPerMm = stepsPerRev / LEAD; 
  float heightToMove = targetH - currHeightRef;
  long steps = abs(heightToMove * stepsPerMm);
  float actualMmMoved = (float)steps / stepsPerMm;

  digitalWrite(dirPin2, (heightToMove > 0) ? HIGH : LOW);

  long rampSteps = min(steps / 5, 150L);
  int delayTime = 1000;
  int minDelay = 400;

  for(long x = 0; x < steps; x++) {
    digitalWrite(stepPin2, HIGH); 
    delayMicroseconds(delayTime);
    digitalWrite(stepPin2,+ LOW); 
    delayMicroseconds(delayTime);
  
    if (x < rampSteps && delayTime > minDelay) {
      delayTime -= 3; // Lead screws can accelerate faster
    }
    else if (x > (steps - rampSteps) && delayTime < 1000) {
      delayTime += 3;
    }
  }

  if (heightToMove > 0) currHeightRef += actualMmMoved;
  else currHeightRef -= actualMmMoved;

  return abs(targetH - currHeightRef);
}