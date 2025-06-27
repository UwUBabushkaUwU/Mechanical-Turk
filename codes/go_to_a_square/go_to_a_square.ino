#include <EEPROM.h>
#define stepPin1 2
#define dirPin1 5
#define stepPin2 3
#define dirPin2 6
const int stepsPerCm = 50;
const int cmPerSquare = 5;
const int stepsPerSquare = stepsPerCm * cmPerSquare;
// New offsets in steps
const int originOffsetX = -5.5 * stepsPerCm;
const int originOffsetY = -13 * stepsPerCm;
int currentX = 0;
int currentY = 0;
void setup() {
  Serial.begin(9600);
  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  loadLastPosition();
}
void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command == "zero") {
      moveToOrigin();
    } else {
      moveToSquare(command);
    }
  }
}
void moveToSquare(String square) {
  if (square.length() < 2) return;
  char colChar = square.charAt(0);
  int row = square.substring(1).toInt();
  // Convert to zero-based index
  int squareX = (toupper(colChar) - 'A') * cmPerSquare + (cmPerSquare / 2.0);
  int squareY = (row - 1) * cmPerSquare + (cmPerSquare / 2.0);
  // Convert to steps and apply offset
  int targetX = squareX * stepsPerCm - originOffsetX;
  int targetY = squareY * stepsPerCm - originOffsetY;
  int deltaX = targetX - currentX;
  int deltaY = targetY - currentY;
  moveXY(deltaX, deltaY);
  currentX = targetX;
  currentY = targetY;

  saveCurrentPosition();
}
void moveXY(int dx, int dy) {
  // Direction logic for H-Bot
  // For H-bot: depending on dx, dy you might need to vary both motors
  // Move X first
  if (dx != 0) {
    bool dir = dx < 0;  // INVERTED: dx > 0 was right, now left
    digitalWrite(dirPin1, dir ? HIGH : LOW);
    digitalWrite(dirPin2, dir ? HIGH : LOW);
  for (int i = 0; i < abs(dx); i++) {
    digitalWrite(stepPin1, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(700);
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(700);
  }
}
  // Then move Y
  if (dy != 0) {
    bool dir = dy > 0;
    digitalWrite(dirPin1, dir ? HIGH : LOW);
    digitalWrite(dirPin2, dir ? LOW : HIGH);  // Opposite directions
    for (int i = 0; i < abs(dy); i++) {
      digitalWrite(stepPin1, HIGH);
      digitalWrite(stepPin2, HIGH);
      delayMicroseconds(700);
      digitalWrite(stepPin1, LOW);
      digitalWrite(stepPin2, LOW);
      delayMicroseconds(700);
    }
  }
}
void moveToOrigin() {
  int deltaX = -currentX;
  int deltaY = -currentY;
  moveXY(deltaX, deltaY);
  currentX = 0;
  currentY = 0;
  saveCurrentPosition();
}
void saveCurrentPosition() {
  EEPROM.put(0, currentX);
  EEPROM.put(sizeof(int), currentY);
}
void loadLastPosition() {
  EEPROM.get(0, currentX);
  EEPROM.get(sizeof(int), currentY);
}