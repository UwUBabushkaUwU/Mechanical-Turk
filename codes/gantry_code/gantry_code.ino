#define stepPin1 2
#define dirPin1 5
#define stepPin2 3
#define dirPin2 6

bool isSpinning = false;
bool direction = true; // true = HIGH, false = LOW
int stepDelay = 700;   // Microseconds between steps

unsigned long lastCommandTime = 0;
const unsigned long commandTimeout = 500; // Auto-stop timeout (ms)

void setup() {
  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  
  Serial.begin(9600);
  digitalWrite(dirPin1, direction ? HIGH : LOW);
  digitalWrite(dirPin2, direction ? HIGH : LOW);

  Serial.println("Stepper Ready. Commands: front/back/left/right start/stop");
}

void loop() {
  handleSerialCommands();

  // Auto-stop if no command received recently
  if (isSpinning && (millis() - lastCommandTime > commandTimeout)) {
    isSpinning = false;
    Serial.println("Auto-stop triggered due to serial timeout.");
  }

  if (isSpinning) {
    digitalWrite(stepPin1, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(stepDelay);
  }
}

void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    lastCommandTime = millis();

    if (command.endsWith("start")) {
      isSpinning = true;

      if (command.startsWith("front")) {
        direction = false;
        digitalWrite(dirPin1, direction ? HIGH : LOW);
        digitalWrite(dirPin2, !direction ? HIGH : LOW);
      }
      else if (command.startsWith("back")) {
        direction = true;
        digitalWrite(dirPin1, direction ? HIGH : LOW);
        digitalWrite(dirPin2, !direction ? HIGH : LOW);
      }
      else if (command.startsWith("left")) {
        direction = true;
        digitalWrite(dirPin1, direction ? HIGH : LOW);
        digitalWrite(dirPin2, direction ? HIGH : LOW);
      }
      else if (command.startsWith("right")) {
        direction = false;
        digitalWrite(dirPin1, direction ? HIGH : LOW);
        digitalWrite(dirPin2, direction ? HIGH : LOW);
      }

      Serial.print("Start command received: ");
      Serial.println(command);
    }

    else if (command.endsWith("stop")) {
      isSpinning = false;
      Serial.print("Stop command received: ");
      Serial.println(command);
    }

    else {
      Serial.print("Unknown command: ");
      Serial.println(command);
    }
  }
}
