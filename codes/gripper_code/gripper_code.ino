#include <Servo.h>

Servo motor1;  // continuous rotation
Servo motor2;  // positional

int motor1Pin = 8;
int motor2Pin = 12;

int defaultAngle1 = 93;  // neutral angle for continuous servo
unsigned long lastCommandTime = 0;
bool motor1WasMoved = false;

void setup() {
  Serial.begin(9600);  // Start serial over USB

  motor1.attach(motor1Pin);
  motor2.attach(motor2Pin);

  motor1.write(defaultAngle1);  // set motor1 to neutral

  Serial.println("Send commands like 'motor1 120' or 'motor2 45':");
}

void loop() {
  String input = "";

  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.length() == 0) return;

    int spaceIndex = input.indexOf(' ');
    if (spaceIndex == -1) {
      Serial.println("Invalid format. Use: motor1 120");
      return;
    }

    String motor = input.substring(0, spaceIndex);
    int angle = input.substring(spaceIndex + 1).toInt();

    if (motor == "motor1") {
      motor1.write(angle);
      Serial.println("motor1 set to " + String(angle));
      lastCommandTime = millis();
      motor1WasMoved = true;
    } else if (motor == "motor2") {
      if (angle >= 0 && angle <= 180) {
        motor2.write(angle);
        Serial.println("motor2 set to " + String(angle));
      } else {
        Serial.println("Angle must be 0 to 180");
      }
    } else {
      Serial.println("Unknown motor. Use motor1 or motor2.");
    }
  }

  // Reset motor1 to neutral after 0.5 seconds
  if (motor1WasMoved && millis() - lastCommandTime >= 500) {
    motor1.write(defaultAngle1);
    Serial.println("motor1 reset to neutral (93°)");
    motor1WasMoved = false;
  }
}
