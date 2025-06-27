#include <Servo.h>

Servo motor1; // Lifter servo
Servo motor2; // Gripper servo

void setup() {
  Serial.begin(9600);
  motor1.attach(8); // change as needed
  motor2.attach(12); // change as needed

  motor2.write(0); // open gripper
  Serial.println("Ready. Send: <down_time_ms> <hold_time_s>");
}

void loop() {
  static String input = "";

  // Read serial input
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (input.length() > 0) {
        handleCommand(input);
        input = "";
      }
    } else {
      input += c;
    }
  }
}

void handleCommand(String cmd) {
  int sepIndex = cmd.indexOf(' ');
  if (sepIndex == -1) {
    Serial.println("Invalid input. Use: <down_time_ms> <hold_time_s>");
    return;
  }

  int downTime = cmd.substring(0, sepIndex).toInt();
  float holdTime = 2;

  Serial.print("Going down for ");
  Serial.print(downTime);
  Serial.print("ms, holding for ");
  Serial.print(holdTime);
  Serial.println("s, then going up.");

  // Move down
  motor1.write(60);
  delay(downTime);
  motor1.write(90); // stop
  Serial.println("At bottom position");

  delay(holdTime * 1000);

  // Move up
  motor1.write(120);
  delay(downTime);
  motor1.write(90); // stop
  Serial.println("Returned to top");
}
