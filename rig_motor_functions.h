// rig_motor_functions.h - Final Silent Smooth Fast Edition
#ifndef MOTOR_FUNCTIONS_H
#define MOTOR_FUNCTIONS_H

#include <Arduino.h>

#define DEBUG 1  // Enable serial debugging

volatile bool emergencyStop = false;

// Pin definitions
const int stepPin = 8;
const int dirPin = 9;
const int ledPin = 13;
const int leftLimitPin = 26;   // Physical left limit switch
const int rightLimitPin = 27;  // Physical right limit switch

// Optimized movement parameters
const int minDelay = 160;      // Optimal balance between speed and silence
const int maxDelay = 450;      // For smooth starts
const int accelSteps = 400;    // Smooth but not sluggish
const int endBuffer = 150;     // Steps to stay away from each limit

// Homing parameters (faster but still reliable)
const int homingSpeed = 350;   // Faster than before but still safe
const int homingBackoffSpeed = 500; // Slightly faster backoff

// Direction control
const bool LEFT_DIR = HIGH;
const bool RIGHT_DIR = LOW;

// Measured values
long totalSteps = 184245;
long usableSteps = totalSteps - (2*endBuffer);
long adjusted_left_limit = endBuffer;
long adjusted_right_limit = totalSteps - endBuffer;
long middle = usableSteps/2;
long currentPosition = 0;

// Function declarations
void stepMotor(int delayTime, bool isHoming = false);
bool safeToMove();
void moveToPosition(long targetPos);
void moveToAdjustedLeftLimit();
void moveToAdjustedRightLimit();
long measure_rail();

// ===== Optimized stepMotor() =====
void stepMotor(int delayTime, bool isHoming) {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(isHoming ? 6 : 8); // Shorter pulse for homing
  digitalWrite(stepPin, LOW);
  if(delayTime > (isHoming ? 6 : 8)) {
    delayMicroseconds(delayTime - (isHoming ? 6 : 8));
  }
}

// Safety check
bool safeToMove() {
  if ((digitalRead(leftLimitPin) == LOW && digitalRead(dirPin) == LEFT_DIR)) {
    digitalWrite(ledPin, HIGH);
    return false;
  }
  if ((digitalRead(rightLimitPin) == LOW && digitalRead(dirPin) == RIGHT_DIR)) {
    digitalWrite(ledPin, HIGH);
    return false;
  }
  digitalWrite(ledPin, LOW);
  return true;
}

void setPosition(long newPosition) {
  newPosition = constrain(newPosition, adjusted_left_limit, adjusted_right_limit);
  currentPosition = newPosition;
  
  #if DEBUG
  Serial.print("Position manually set to: ");
  Serial.println(currentPosition);
  #endif
}

// ===== Silent moveToPosition() =====
void moveToPosition(long targetPos) {
  emergencyStop = false;
  targetPos = constrain(targetPos, adjusted_left_limit, adjusted_right_limit);
  
  bool moveDirection = (targetPos > currentPosition) ? RIGHT_DIR : LEFT_DIR;
  digitalWrite(dirPin, moveDirection);
  
  #if DEBUG
  Serial.print("Moving from ");
  Serial.print(currentPosition);
  Serial.print(" to ");
  Serial.print(targetPos);
  Serial.print(" (Direction: ");
  Serial.print(moveDirection == RIGHT_DIR ? "RIGHT" : "LEFT");
  Serial.println(")");
  #endif

  long stepsToMove = abs(targetPos - currentPosition);
  int dynamicAccel = min(accelSteps, stepsToMove/2);
  
  // Smoother cubic acceleration
  for(int i = 0; i < dynamicAccel && safeToMove(); i++) {
    float t = (float)i / dynamicAccel;
    float ease = t*t*(3 - 2*t);  // Cubic easing
    int delayTime = maxDelay - (maxDelay - minDelay) * ease;
    stepMotor(delayTime);
    currentPosition += (moveDirection == RIGHT_DIR) ? 1 : -1;
  }

  // Cruise phase with resonance management
  while(abs(targetPos - currentPosition) > dynamicAccel && safeToMove()) {
    stepMotor(minDelay);
    currentPosition += (moveDirection == RIGHT_DIR) ? 1 : -1;
    
    // Add tiny irregular pauses to break up resonance
    static byte counter = 0;
    if(++counter % 37 == 0) {  // Prime number for irregular pattern
      delayMicroseconds(15);
    }
  }

  // Smooth deceleration
  while(currentPosition != targetPos && safeToMove()) {
    long remaining = abs(targetPos - currentPosition);
    float t = (float)remaining / dynamicAccel;
    t = constrain(t, 0.0, 1.0);
    float ease = t*t*(3 - 2*t);  // Cubic easing
    int delayTime = minDelay + (maxDelay - minDelay) * (1.0 - ease);
    stepMotor(delayTime);
    currentPosition += (moveDirection == RIGHT_DIR) ? 1 : -1;
  }

  if(emergencyStop) {
    digitalWrite(stepPin, LOW);
    Serial.println("MOVEMENT_STOPPED");
    emergencyStop = false;
  }
}

void seekRightLimit() {
  #if DEBUG
  Serial.println("Seeking right limit...");
  #endif
  
  digitalWrite(dirPin, RIGHT_DIR);
  while(digitalRead(rightLimitPin)) {
    if(!safeToMove()) return;
    stepMotor(homingSpeed, true);  // Faster homing speed
  }
  
  digitalWrite(dirPin, LEFT_DIR);
  for(int i = 0; i < endBuffer; i++) {
    if(!safeToMove()) return;
    stepMotor(homingBackoffSpeed, true);  // Faster back-off
  }
  
  currentPosition = adjusted_right_limit;
  
  #if DEBUG
  Serial.print("Found right limit, position set to: ");
  Serial.println(currentPosition);
  #endif
}

void seekLeftLimit() {
  #if DEBUG
  Serial.println("Seeking left limit...");
  #endif
  
  digitalWrite(dirPin, LEFT_DIR);
  while(digitalRead(leftLimitPin)) {
    if(!safeToMove()) return;
    stepMotor(homingSpeed, true);  // Faster homing speed
  }
  
  digitalWrite(dirPin, RIGHT_DIR);
  for(int i = 0; i < endBuffer; i++) {
    if(!safeToMove()) return;
    stepMotor(homingBackoffSpeed, true);  // Faster back-off
  }
  
  currentPosition = adjusted_left_limit;
  
  #if DEBUG
  Serial.print("Found left limit, position set to: ");
  Serial.println(currentPosition);
  #endif
}

void homeToCenter() {
  emergencyStop = false;
  digitalWrite(ledPin, LOW);
  
  seekLeftLimit();
  
  // Move to center with smooth profile
  long centerPos = adjusted_left_limit + (usableSteps / 2);
  moveToPosition(centerPos);
  
  #if DEBUG
  Serial.print("Moved to center position: ");
  Serial.println(currentPosition);
  #endif
}

// ===== Optimized moveRight() =====
int moveRight(long steps) {
  emergencyStop = false;
  if (steps <= 0) return 0;
  
  #if DEBUG
  Serial.print("Moving right ");
  Serial.print(steps);
  Serial.println(" steps");
  #endif

  long stepsMoved = 0;
  digitalWrite(dirPin, RIGHT_DIR);
  
  long maxPossible = adjusted_right_limit - currentPosition;
  steps = min(steps, maxPossible);
  
  int actualAccelSteps = min(accelSteps, steps/2);
  int actualDecelSteps = min(accelSteps, steps - actualAccelSteps);
  
  // Smoother acceleration
  for (int i = 0; i < actualAccelSteps && safeToMove(); i++) {
    float t = (float)i / actualAccelSteps;
    float ease = t*t*(3 - 2*t);  // Cubic easing
    int delayTime = maxDelay - (maxDelay - minDelay) * ease;
    stepMotor(delayTime);
    currentPosition++;
    stepsMoved++;
  }

  // Cruise phase with resonance management
  long cruiseSteps = steps - (actualAccelSteps + actualDecelSteps);
  for (long i = 0; i < cruiseSteps && safeToMove(); i++) {
    stepMotor(minDelay);
    currentPosition++;
    stepsMoved++;
    
    // Irregular pattern to break resonance
    if(i % 41 == 0) {  // Prime number pattern
      delayMicroseconds(10);
    }
  }
  
  // Smoother deceleration
  for (int i = actualDecelSteps; i > 0 && safeToMove(); i--) {
    float t = (float)i / actualDecelSteps;
    float ease = t*t*(3 - 2*t);  // Cubic easing
    int delayTime = minDelay + (maxDelay - minDelay) * (1.0 - ease);
    stepMotor(delayTime);
    currentPosition++;
    stepsMoved++;
  }

  if(emergencyStop) {
    digitalWrite(stepPin, LOW);
    Serial.println("MOVEMENT_STOPPED");
    emergencyStop = false;
  }
  
  #if DEBUG
  Serial.print("Moved right ");
  Serial.print(stepsMoved);
  Serial.println(" steps");
  #endif
  
  return stepsMoved;
}

// ===== Optimized moveLeft() =====
int moveLeft(long steps) {
  emergencyStop = false;
  if (steps <= 0) return 0;
  
  #if DEBUG
  Serial.print("Moving left ");
  Serial.print(steps);
  Serial.println(" steps");
  #endif

  long stepsMoved = 0;
  digitalWrite(dirPin, LEFT_DIR);
  
  long maxPossible = currentPosition - adjusted_left_limit;
  steps = min(steps, maxPossible);
  
  int actualAccelSteps = min(accelSteps, steps/2);
  int actualDecelSteps = min(accelSteps, steps - actualAccelSteps);
  
  // Smoother acceleration
  for (int i = 0; i < actualAccelSteps && safeToMove(); i++) {
    float t = (float)i / actualAccelSteps;
    float ease = t*t*(3 - 2*t);  // Cubic easing
    int delayTime = maxDelay - (maxDelay - minDelay) * ease;
    stepMotor(delayTime);
    currentPosition--;
    stepsMoved++;
  }
  
  // Cruise phase with resonance management
  long cruiseSteps = steps - (actualAccelSteps + actualDecelSteps);
  for (long i = 0; i < cruiseSteps && safeToMove(); i++) {
    stepMotor(minDelay);
    currentPosition--;
    stepsMoved++;
    
    // Irregular pattern to break resonance
    if(i % 43 == 0) {  // Different prime number
      delayMicroseconds(10);
    }
  }
  
  // Smoother deceleration
  for (int i = actualDecelSteps; i > 0 && safeToMove(); i--) {
    float t = (float)i / actualDecelSteps;
    float ease = t*t*(3 - 2*t);  // Cubic easing
    int delayTime = minDelay + (maxDelay - minDelay) * (1.0 - ease);
    stepMotor(delayTime);
    currentPosition--;
    stepsMoved++;
  }

  if(emergencyStop) {
    digitalWrite(stepPin, LOW);
    Serial.println("MOVEMENT_STOPPED");
    emergencyStop = false;
  }
  
  #if DEBUG
  Serial.print("Moved left ");
  Serial.print(stepsMoved);
  Serial.println(" steps");
  #endif
  
  return stepsMoved;
}

void initMotorController() {
  #if DEBUG
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Initializing motor controller...");
  #endif

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(leftLimitPin, INPUT_PULLUP);
  pinMode(rightLimitPin, INPUT_PULLUP);
  
  digitalWrite(stepPin, LOW);
  digitalWrite(ledPin, LOW);
  
  currentPosition = 0; 
  
  #if DEBUG
  Serial.println("Motor controller ready");
  #endif
}

#endif
