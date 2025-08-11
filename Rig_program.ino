//Rig_program.ino:
#include "rig_motor_functions.h"

// Odor delivery system pins and variables
const int odor_sync_pin_TTL = 13;
int activeOdorCount = 8;
int left_manifold[8]  = {0, 1, 2, 3, 4, 5, 6, 7};       // Channels 0-7
int right_manifold[8] = {23, 22, 21, 20, 19, 18, 17, 16}; // Channels 23-16

String incomingCommand = "";

void setup() {
  // Initialize motor controller
  initMotorController();
  
  // Initialize odor delivery system
  for (int i = 0; i < 8; i++) {
    pinMode(left_manifold[i], OUTPUT);
    pinMode(right_manifold[i], OUTPUT);
    digitalWrite(left_manifold[i], LOW);
    digitalWrite(right_manifold[i], LOW);
  }
  pinMode(odor_sync_pin_TTL, OUTPUT);
  digitalWrite(odor_sync_pin_TTL, LOW);
  
  // Initialize serial communication
  Serial.begin(115200);
  while(!Serial); // Wait for serial connection
  Serial.println("System Ready - Odor Delivery and Motor Control");
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      handleCommand(incomingCommand);
      incomingCommand = "";
    } else {
      incomingCommand += c;
    }
  }
}

void handleCommand(String cmd) {
  cmd.trim();
  
  // Handle odor commands
  if (cmd.startsWith("ODOR_ON")) {
    int odorNum = cmd.substring(8).toInt();
    if (odorNum >= 0 && odorNum < 8) {
      digitalWrite(left_manifold[odorNum], HIGH);
      digitalWrite(right_manifold[odorNum], HIGH);
      digitalWrite(odor_sync_pin_TTL, HIGH);
      Serial.print("Odor ON: ");
      Serial.println(odorNum);
    }
  } 
  else if (cmd.startsWith("ODOR_OFF")) {
    int odorNum = cmd.substring(9).toInt();
    if (odorNum >= 0 && odorNum < 8) {
      digitalWrite(left_manifold[odorNum], LOW);
      digitalWrite(right_manifold[odorNum], LOW);
      digitalWrite(odor_sync_pin_TTL, LOW);
      Serial.print("Odor OFF: ");
      Serial.println(odorNum);
    }
  }
  
  // Handle motor commands
  else if (cmd.startsWith("MOVE_TO ")) {
    long targetPos = cmd.substring(8).toInt();
    moveToPosition(targetPos);
    Serial.println("MOVEMENT_COMPLETE");
    Serial.println(currentPosition);
  }
  else if (cmd.startsWith("MOVE_LEFT ")) {
    long steps = cmd.substring(10).toInt();
    int moved = moveLeft(steps);
    Serial.println("MOVEMENT_COMPLETE");
    Serial.println(moved);
  }
  else if (cmd.startsWith("MOVE_RIGHT ")) {
    long steps = cmd.substring(11).toInt();
    int moved = moveRight(steps);
    Serial.println("MOVEMENT_COMPLETE");
    Serial.println(moved);
  }
  else if (cmd.equals("HOME_TO_CENTER")) {
    homeToCenter();
    Serial.println("MOVEMENT_COMPLETE");
    Serial.print("POS ");
    Serial.println(currentPosition);
  }
  else if (cmd.equals("SEEK_LEFT_LIMIT")) {
    seekLeftLimit();
    Serial.println("MOVEMENT_COMPLETE");
    Serial.print("POS ");
    Serial.println(currentPosition);
  }
  else if (cmd.equals("SEEK_RIGHT_LIMIT")) {
    seekRightLimit();
    Serial.println("MOVEMENT_COMPLETE");
    Serial.print("POS ");
    Serial.println(currentPosition);
  }
  else if (cmd.startsWith("SET_POS ")) {
    long newPos = cmd.substring(8).toInt();
    setPosition(newPos);
    Serial.print("POS ");
    Serial.println(currentPosition);
  }
  else if (cmd.equals("GET_POS")) {
    Serial.print("POS ");
    Serial.println(currentPosition);
  }
  else if (cmd.equals("EMERGENCY_STOP")) {
    emergencyStop = true;
    Serial.println("EMERGENCY_STOP_ACK");
  }
  
  // Unknown command
  else {
    Serial.print("ERROR Unknown command: ");
    Serial.println(cmd);
  }
}

