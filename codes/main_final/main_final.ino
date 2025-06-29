#include <EEPROM.h>
#include <Servo.h>

Servo motor1; // Lifter
Servo motor2; // Gripper
#define stepPin1 2
#define dirPin1 5
#define stepPin2 3
#define dirPin2 6
#define feedback A0

#define endEffectorPin 9  // PWM pin for motor1 (for lowering/lifting)

const int stepsPerCm = 50;
const int cmPerSquare = 5;
const int stepsPerSquare = stepsPerCm * cmPerSquare;
const int originOffsetX = -5.5 * stepsPerCm;
const int originOffsetY = -13 * stepsPerCm;
const int gripperTime = 2000;

int currentX = 0;
int currentY = 0;
const int holdTime = 2000; // milliseconds
const int endEffectorSpeed = 60; // analogWrite duty

void setup() {
  Serial.begin(9600);
  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(feedback, INPUT);
  motor1.attach(8);
  motor2.attach(7);

  motor2.write(0); // Open gripper
  loadLastPosition();
}

void loop() {
  if (Serial.available()) {
    String fullCommand = Serial.readStringUntil('\n');
    fullCommand.trim();

    int i = 0;
    while (i < fullCommand.length()) {
      // Parse move like "pe2e4" or "Bf1c4"
      char piece = fullCommand.charAt(i++);
      String move = "";
      move+=fullCommand.charAt(i++);
      move+=fullCommand.charAt(i++);
      move+=fullCommand.charAt(i++);
      move+=fullCommand.charAt(i++);
      handleMove(piece, move);
    }
    Serial.println("Move over");
  }
}

void handleMove(char piece, String move) {
  String fromSquare, toSquare;
  fromSquare = move.substring(0, 2);
  toSquare = move.substring(2);

  moveToSquare(fromSquare);
  openEffector();
  delay(gripperTime);
  lowerEffector(piece);
  //delay(holdTime);
  closeEffector(piece);
  delay(gripperTime);
  delay(1000);
  raiseEffector(piece);

  moveToSquare(toSquare);
  lowerEffector(piece);
  openEffector();
  delay(gripperTime);
  raiseEffector(piece);
}

void lowerEffector(char piece) {
  int downTime = getDownTime(piece);
  Serial.println("Lowering...");
  motor1.write(60);
  delay(downTime);
  motor1.write(93); // stop
}

void raiseEffector(char piece) {
  int downTime = getDownTime(piece);
  Serial.println("Raising...");
  motor1.write(120);
  unsigned long startTime = millis();
  while(millis()-startTime<3000 && !feedbackRead()){

  }
  motor1.write(93); // stop
}
void openEffector(){
  //int gripAngle = getGripAnle(char piece);
  Serial.println("Opening...");
  motor2.write(0);
}
void closeEffector(char piece){
  int gripAngle = getGripAngle(piece);
  Serial.println("Closing...");
  motor2.write(gripAngle);
}

int getDownTime(char piece){
  switch (toupper(piece)) {
    case 'Q': return 1550;
    case 'K': return 1450;
    case 'N': return 1600;
    case 'B': return 1700;
    case 'R': return 1700;
    case 'P': return 1800;
    default:  return 1550;
  }
}
int getGripAngle(char piece) {
  switch (toupper(piece)) {
    case 'Q': return 120;
    case 'K': return 130;
    case 'N': return 140;
    case 'B': return 120;
    case 'R': return 140;
    case 'P': return 140;
    default:  return 120;
  }
}

void moveToSquare(String square) {
  if (square.length() != 2) return;
  Serial.println("Moving to "+square);
  char colChar = square.charAt(0);
  int row = square.substring(1).toInt();
  int squareX = (toupper(colChar) - 'A') * cmPerSquare + cmPerSquare / 2.0;
  int squareY = (row - 1) * cmPerSquare + cmPerSquare / 2.0;
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
  if (dx != 0) {
    bool dir = dx < 0;
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

  if (dy != 0) {
    bool dir = dy > 0;
    digitalWrite(dirPin1, dir ? HIGH : LOW);
    digitalWrite(dirPin2, dir ? LOW : HIGH);
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

void saveCurrentPosition() {
  EEPROM.put(0, currentX);
  EEPROM.put(sizeof(int), currentY);
}

void loadLastPosition() {
  EEPROM.get(0, currentX);
  EEPROM.get(sizeof(int), currentY);
}
bool feedbackRead(){
  int ones = 0,zeroes = 0;
  for(int i = 0 ; i < 10;i++)
  {
    int an = analogRead(feedback);
    if(an>800) ones++;
    
    else zeroes++;
    Serial.println(an);
    delay(5);
  }
  if(ones>zeroes) return true;
  return false;
  
}
