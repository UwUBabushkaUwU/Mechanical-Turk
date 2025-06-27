#include <ESP32Servo.h>

Servo myServo;
int servoPin = 13;
int angle = 90; // default starting angle

void setup() {
  Serial.begin(115200);
  myServo.attach(servoPin);
  myServo.write(angle);
  Serial.println("Enter angle between 0 and 180:");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();  // remove whitespace/newlines

    if (input.length() == 0) return;

    int inputAngle = input.toInt();
    if (inputAngle >= 0 && inputAngle <= 180) {
      myServo.write(inputAngle);
      Serial.print("Moved to angle: ");
      Serial.println(inputAngle);
    } else {
      Serial.println("Invalid angle. Enter a number from 0 to 180.");
    }
  }
}
