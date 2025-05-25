// --- Constants and Thresholds ---
const int NUM_CODES = 16;
const int VALUE_TOLERANCE = 7;
const int READ_TOLERANCE = 15;
const int NUM_SAMPLES = 7;
const int MOVE_TIMEOUT = 5; // seconds
const int DELAY_BETWEEN_PARTS = 200; // ms

// --- Mapping: 4-bit code + expected resistance value ---
const int codes_and_values[NUM_CODES][5] = {
  {0, 0, 0, 0, 1023}, {0, 0, 0, 1, 703}, {0, 0, 1, 0, 843}, {0, 0, 1, 1, 613},
  {0, 1, 0, 0, 930}, {0, 1, 0, 1, 658}, {0, 1, 1, 0, 779}, {0, 1, 1, 1, 579},
  {1, 0, 0, 0, 979}, {1, 0, 0, 1, 682}, {1, 0, 1, 0, 812}, {1, 0, 1, 1, 597},
  {1, 1, 0, 0, 893}, {1, 1, 0, 1, 639}, {1, 1, 1, 0, 753}, {1, 1, 1, 1, 564}
};

// --- Globals ---
int prev_value, curr_value;
int prev_index, curr_index;
int move_start_time;

int activated_squares[4] = {0, 0, 0, 0};

// --- Setup ---
void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  delay(1000);

  prev_value = stable_read();
  prev_index = find_code_index(prev_value);

  Serial.println("Program started.");
}

// --- Loop ---
void loop() {
  curr_value = stable_read();
  curr_index = find_code_index(curr_value);

  if (curr_index == -1 || prev_index == -1) {
    prev_value = curr_value;
    prev_index = curr_index;
    return; // ignore noisy or unknown readings
  }

  if (abs(curr_value - prev_value) > VALUE_TOLERANCE && bit_difference(prev_index, curr_index) == 1) {
    move_start_time = millis();
    handle_move(); // go into move capture logic
  }

  prev_value = curr_value;
  prev_index = curr_index;
}

// --- Move Handling ---
void handle_move() {
  Serial.println("\nMove started");

  for (int part = 0; part < 4; part++) {
    delay(DELAY_BETWEEN_PARTS);
    if ((millis() - move_start_time) / 1000 > MOVE_TIMEOUT) break;

    int next_value = stable_read();
    int next_index = find_code_index(next_value);

    if (next_index == -1 || bit_difference(curr_index, next_index) != 1) continue;

    int changed_square = get_changed_square(curr_index, next_index);
    activated_squares[part] = changed_square;

    Serial.print("Step ");
    Serial.print(part + 1);
    Serial.print(" — Activated square: ");
    Serial.println(changed_square);
    // Print new 4-bit state after change
    Serial.print("  New State: ");
    for (int j = 0; j < 4; j++) {
      Serial.print(codes_and_values[curr_index][j]);
    }
    Serial.println();
    curr_index = next_index;
  }

  print_move_type();
  reset_move_state();
}

// --- Utilities ---

// Smooth reading with median + outlier rejection
int stable_read() {
  int values[NUM_SAMPLES];
  for (int i = 0; i < NUM_SAMPLES; i++) {
    values[i] = analogRead(A0);
    delay(2);
  }

  // Sort
  for (int i = 0; i < NUM_SAMPLES - 1; i++) {
    for (int j = i + 1; j < NUM_SAMPLES; j++) {
      if (values[i] > values[j]) {
        int t = values[i]; values[i] = values[j]; values[j] = t;
      }
    }
  }

  int median = values[NUM_SAMPLES / 2];
  long sum = 0;
  int count = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    if (abs(values[i] - median) <= READ_TOLERANCE) {
      sum += values[i];
      count++;
    }
  }

  return count > 0 ? sum / count : median;
}

// Find matching code index based on resistance
int find_code_index(int val) {
  for (int i = 0; i < NUM_CODES; i++) {
    if (abs(codes_and_values[i][4] - val) <= VALUE_TOLERANCE) {
      return i;
    }
  }
  return -1;
}

// Count how many bits differ between two code entries
int bit_difference(int idx1, int idx2) {
  int diff = 0;
  for (int i = 0; i < 4; i++) {
    if (codes_and_values[idx1][i] != codes_and_values[idx2][i]) diff++;
  }
  return diff;
}

// Which bit (square) changed
int get_changed_square(int idx1, int idx2) {
  for (int i = 0; i < 4; i++) {
    if (codes_and_values[idx1][i] != codes_and_values[idx2][i]) return i + 1;
  }
  return 0;
}

// Interpret move based on steps
void print_move_type() {
  Serial.print("\nMove: ");
  for (int i = 0; i < 4; i++) {
    if (activated_squares[i] > 0) {
      Serial.print(activated_squares[i]);
      Serial.print(" ");
    }
  }

  if (activated_squares[0] && activated_squares[1] && activated_squares[1] == activated_squares[2]) {
    Serial.println("→ Capture");
  } else if (activated_squares[3]) {
    Serial.println("→ Castling or Promotion");
  } else if (activated_squares[2]) {
    Serial.println("→ En Passant or Promotion");
  } else if (activated_squares[1]) {
    Serial.println("→ Normal Move");
  } else {
    Serial.println("→ Unknown/Invalid");
  }
}

// Clear state after a move
void reset_move_state() {
  for (int i = 0; i < 4; i++) activated_squares[i] = 0;
}
