#include <Servo.h>

// === Servo setup ===
Servo motor1;  // continuous rotation
Servo motor2;  // positional

int motor1Pin = 8;
int motor2Pin = 12;
int defaultAngle1 = 93;  // neutral angle for continuous servo
unsigned long lastServoCommandTime = 0;
bool motor1WasMoved = false;

// === Stepper setup ===
#define stepPin1 2
#define dirPin1 5
#define stepPin2 3
#define dirPin2 6

bool isSpinning = false;
bool direction = true; // HIGH or LOW
int stepDelay = 700;   // μs between steps
unsigned long lastStepperCommandTime = 0;
const unsigned long commandTimeout = 500; // ms

void setup() {
  // Servos
  motor1.attach(motor1Pin);
  motor2.attach(motor2Pin);
  motor1.write(defaultAngle1);  // Neutral
  lastServoCommandTime = millis();

  // Steppers
  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  digitalWrite(dirPin1, direction ? HIGH : LOW);
  digitalWrite(dirPin2, direction ? HIGH : LOW);

  Serial.begin(9600);
  Serial.println("Ready. Commands:");
  Serial.println(" - motor1 <angle>");
  Serial.println(" - motor2 <angle>");
  Serial.println(" - front/back/left/right start");
  Serial.println(" - <direction> stop");
}

void loop() {
  handleSerialCommands();

  // === Auto-reset motor1 ===
  if (motor1WasMoved && millis() - lastServoCommandTime >= 500) {
    motor1.write(defaultAngle1);
    Serial.println("motor1 reset to neutral (93°)");
    motor1WasMoved = false;
  }

  // === Stepper stepping ===
  if (isSpinning) {
    if (millis() - lastStepperCommandTime > commandTimeout) {
      isSpinning = false;
      Serial.println("Stepper auto-stop (timeout)");
    } else {
      digitalWrite(stepPin1, HIGH);
      digitalWrite(stepPin2, HIGH);
      delayMicroseconds(stepDelay);
      digitalWrite(stepPin1, LOW);
      digitalWrite(stepPin2, LOW);
      delayMicroseconds(stepDelay);
    }
  }
}

void handleSerialCommands() {
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim();
  if (input.length() == 0) return;

  // Handle servo commands
  if (input.startsWith("motor")) {
    int spaceIndex = input.indexOf(' ');
    if (spaceIndex == -1) {
      Serial.println("Invalid servo format. Use: motor1 120");
      return;
    }

    String motor = input.substring(0, spaceIndex);
    int angle = input.substring(spaceIndex + 1).toInt();

    if (motor == "motor1") {
      motor1.write(angle);
      Serial.println("motor1 set to " + String(angle));
      lastServoCommandTime = millis();
      motor1WasMoved = true;
    } else if (motor == "motor2") {
      if (angle >= 0 && angle <= 180) {
        motor2.write(angle);
        Serial.println("motor2 set to " + String(angle));
      } else {
        Serial.println("motor2 angle must be 0 to 180");
      }
    } else {
      Serial.println("Unknown servo motor.");
    }

    return;
  }

  // Handle stepper commands
  lastStepperCommandTime = millis();

  if (input.endsWith("start")) {
    isSpinning = true;

    if (input.startsWith("front")) {
      direction = false;
      digitalWrite(dirPin1, direction ? HIGH : LOW);
      digitalWrite(dirPin2, !direction ? HIGH : LOW);
    }
    else if (input.startsWith("back")) {
      direction = true;
      digitalWrite(dirPin1, direction ? HIGH : LOW);
      digitalWrite(dirPin2, !direction ? HIGH : LOW);
    }
    else if (input.startsWith("left")) {
      direction = true;
      digitalWrite(dirPin1, direction ? HIGH : LOW);
      digitalWrite(dirPin2, direction ? HIGH : LOW);
    }
    else if (input.startsWith("right")) {
      direction = false;
      digitalWrite(dirPin1, direction ? HIGH : LOW);
      digitalWrite(dirPin2, direction ? HIGH : LOW);
    }

    Serial.print("Stepper start: ");
    Serial.println(input);
  }
  else if (input.endsWith("stop")) {
    isSpinning = false;
    Serial.print("Stepper stop: ");
    Serial.println(input);
  }
  else {
    Serial.print("Unknown command: ");
    Serial.println(input);
  }
}
