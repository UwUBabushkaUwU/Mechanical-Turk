#include <Servo.h>

Servo motor1; // Lifter
Servo motor2; // Gripper

const float holdTimeSeconds = 2.0;

void setup() {
  Serial.begin(9600);
  motor1.attach(8);
  motor2.attach(12);

  motor2.write(0); // Open gripper
  Serial.println("Ready. Format: <piece> <angle> (e.g., pawn 100)");
}

void loop() {
  static String input = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (input.length() > 0) {
        processCommand(input);
        input = "";
        Serial.println("Ready for next command:");
      }
    } else {
      input += c;
    }
  }
}

int getDropTime(String piece) {
  piece.toLowerCase();
  if (piece == "queen") return 1600;
  if (piece == "king") return 1500;
  if (piece == "knight") return 1650;
  if (piece == "bishop") return 1750;
  if (piece == "rook") return 1600;
  if (piece == "pawn") return 1750;
  return -1; // Unknown
}

void processCommand(String input) {
  input.trim();
  int spaceIndex = input.indexOf(' ');
  if (spaceIndex == -1) {
    Serial.println("Invalid format. Use: <piece> <angle>");
    return;
  }

  String piece = input.substring(0, spaceIndex);
  int angle = input.substring(spaceIndex + 1).toInt();
  int downTime = getDropTime(piece);

  if (downTime == -1) {
    Serial.println("Unknown piece: " + piece);
    return;
  }

  Serial.print("Testing ");
  Serial.print(piece);
  Serial.print(" at ");
  Serial.print(angle);
  Serial.println(" degrees");

  runCycle(downTime, angle);
}

void runCycle(int downTime, int angle) {
  Serial.println("Opening gripper");
  motor2.write(0);
  delay(300);

  Serial.println("Lowering...");
  motor1.write(60);
  delay(downTime);
  motor1.write(93); // stop

  Serial.println("Closing gripper");
  motor2.write(angle);
  delay(holdTimeSeconds * 1000);

  Serial.println("Lifting...");
  motor1.write(120);
  delay(downTime+50);
  motor1.write(93);

  Serial.println("Done.");
}
