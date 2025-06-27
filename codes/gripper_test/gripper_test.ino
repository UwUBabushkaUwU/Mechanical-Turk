#include <Servo.h>

Servo motor;
int motorPin = 3;

void setup() {
  Serial.begin(9600);
  motor.attach(motorPin);
  Serial.println("Enter a value (0 to 180):");
  Serial.println("  - ~90 = stop");
  Serial.println("  - >90 = forward");
  Serial.println("  - <90 = backward");
}

void loop() {
  if (Serial.available()) {
    int speed = Serial.parseInt();  // Read number
    if (speed >= 0 && speed <= 180) {
      motor.write(speed);
      Serial.println("Set to: " + String(speed));
    } else {
      Serial.println("Invalid. Enter 0–180.");
    }
    while (Serial.available()) Serial.read();  // clear input buffer
  }
}
