const int codes_and_values[16][5] = {
{0, 0, 0, 0, 1023},
{0, 0, 0, 1, 703},
{0, 0, 1, 0, 843},
{0, 0, 1, 1, 613},
{0, 1, 0, 0, 930},
{0, 1, 0, 1, 658},
{0, 1, 1, 0, 779},
{0, 1, 1, 1, 579},
{1, 0, 0, 0, 979},
{1, 0, 0, 1, 682},
{1, 0, 1, 0, 812},
{1, 0, 1, 1, 597},
{1, 1, 0, 0, 893},
{1, 1, 0, 1, 639},
{1, 1, 1, 0, 753},
{1, 1, 1, 1, 564}
};

const char* squares_and_units[][5]{
  {"A8", "B8", "C8", "D8", "A0"}, {"H8", "G8", "F8","E8", "A1"},
  {"A7", "B7", "C7", "D7", "A2"}, {"H7", "G7", "F7","E7", "A3"},
  {"A6", "B6", "C6", "D6", "A4"}, {"H6", "G6", "F6","E6", "A5"},
  {"A5", "B5", "C5", "D5", "A6"}, {"H5", "G5", "F5","E5", "A7"},
  {"A4", "B4", "C4", "D4", "A8"}, {"H4", "G4", "F4","E4", "A9"},
  {"A3", "B3", "C3", "D3", "A10"}, {"H3", "G3", "F3","E3", "A11"},
  {"A2", "B2", "C2", "D2", "A12"}, {"H2", "G2", "F2","E2", "A13"},
  {"A1", "B1", "C1", "D1", "A14"}, {"H1", "G1", "F1","E1", "A15"},
};
int threshold_for_value_difference = 7;
int threshold_for_one_move = 5; //in seconds
int delay_for_parts = 300; //in ms
int current_resistance_value[16], move_starting_time=0,index_of_current_substate[16],index_of_previous_substate[16],previous_resistance_value[16];
char *first_activated_square,*second_activated_square,*third_activated_square,*fourth_activated_square;
bool first_move_of_the_game = true;
bool move_in_progress = false;
int voltage_reading_offset = 0;
//functions

int find_index_of_current_substate(int current_resistance_value){
  int closest_index = -1;
  int min_diff = INT8_MAX;

  for (int i = 0; i < 16; i++) {
      int diff = abs(codes_and_values[i][4] - current_resistance_value);
      if (diff < min_diff) {
          min_diff = diff;
          closest_index = i;
      }
  }

  return closest_index;
}
char* find_activated_square(int index_of_current_substate[16], int index_of_previous_substate[16]){
  for(int i = 0 ; i<16;i++){
    for(int j = 0 ; j < 4 ; j++){
      if(codes_and_values[index_of_current_substate[i]][j]!=codes_and_values[index_of_previous_substate[i]][j]){
        char* square = squares_and_units[i][j];
        return square;
      }
    }
  }
  return NULL;
}

bool differs_by_one_bit(int prev_index, int current_value) {
  int curr_index = find_index_of_current_substate(current_value);
  if (curr_index == -1 || prev_index == -1) return false;
  int diff = 0;
  for (int i = 0; i < 4; i++) {
    if (codes_and_values[prev_index][i] != codes_and_values[curr_index][i]) diff++;
  }
  return diff == 1;
}

int filteredAnalogRead(int pin, int n = 15, int m = 15) {
  int readings[n];
  for (int i = 0; i < n; i++) {
    readings[i] = analogRead(pin)+voltage_reading_offset;
    delay(5); 
  }
  for (int i = 0; i < n - 1; i++) {
    for (int j = i + 1; j < n; j++) {
      if (readings[i] > readings[j]) {
        int temp = readings[i];
        readings[i] = readings[j];
        readings[j] = temp;
      }
    }
  }
  int median = readings[n / 2];
  long sum = 0;
  int count = 0;
  for (int i = 0; i < n; i++) {
    if (abs(readings[i] - median) <= m) {
      sum += readings[i];
      count++;
    }
  }
  if (count > 0) {
    return sum / count;
  } else {
    return median;
  }
}

bool condition(){
  for(int i = 0; i < 16; i++){
    if(abs(previous_resistance_value[i]-current_resistance_value[i])>threshold_for_value_difference&&
    differs_by_one_bit(index_of_previous_substate[i], current_resistance_value[i])){
      return true;
    }
  }
  return false;
}

void update_current_and_previous_resistance_values(bool update_previous=true, bool update_current = true,int SAMPLES=100, int median_threshold = 10x  ){
  if(update_previous){
    for(int i = 0; i < 16; i++){
      previous_resistance_value[i]=current_resistance_value[i];
    }
  }
  if(!update_current) return;
  
  int readings[16][SAMPLES];

  // Step 1: Gather readings
  for (int sample = 0; sample < SAMPLES; sample++) {
    for (int pin = 0; pin < 16; pin++) {
      readings[pin][sample] = analogRead(pin) + voltage_reading_offset;
    }
    delay(5); // One delay per sample, shared for all pins
  }

  // Step 2: Process each pin's readings
  for (int pin = 0; pin < 16; pin++) {
    // Sort readings to find median
    for (int i = 0; i < SAMPLES - 1; i++) {
      for (int j = i + 1; j < SAMPLES; j++) { 
        if (readings[pin][i] > readings[pin][j]) {
          int temp = readings[pin][i];
          readings[pin][i] = readings[pin][j];
          readings[pin][j] = temp;
        }
      }
    }

    int median = readings[pin][SAMPLES / 2];
    long sum = 0;
    int count = 0;

    // Average values close to median
    for (int i = 0; i < SAMPLES; i++) {
      if (abs(readings[pin][i] - median) <= median_threshold) {
        sum += readings[pin][i];
        count++;
      }
    }

    current_resistance_value[pin] = (count > 0) ? sum / count : median;
  }
  return;
}
void update_index_of_current_substate_and_previous_substate(){
  for(int i = 0; i < 16; i++){
    index_of_previous_substate[i] = index_of_current_substate[i];
    index_of_current_substate[i] = find_index_of_current_substate(current_resistance_value[i]);
  }
}

void move_over() {
  Serial.println(" ");
  Serial.print("Move Over");
  for (int i = 0; i < 16; i++) {
    if (i % 2 == 0) {
      for (int j = 0; j < 4; j++) {
        Serial.print(codes_and_values[find_index_of_current_substate(current_resistance_value[i])][j]);
        Serial.print(" ");
      }
    } else {
      for (int j = 3; j >= 0; j--) {
        Serial.print(codes_and_values[find_index_of_current_substate(current_resistance_value[i])][j]);
        Serial.print(" ");
      }
    }
    if (i % 2) Serial.println(" ");
    else Serial.print(" ");
  }
  return;
}


///////////////////////////////////////////////////////////////
void setup() {
  pinMode(A0,INPUT);// put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
  Serial.println("Hello");
  update_current_and_previous_resistance_values(false,true);
  update_current_and_previous_resistance_values();
  Serial.println("current and previous resistance  values have been set");
  for(int i = 0; i < 16; i++){
    //index_of_previous_substate[i] = index_of_current_substate[i];
    index_of_current_substate[i] = find_index_of_current_substate(current_resistance_value[i]);
  }
  for(int i = 0; i < 16;i++){
    Serial.print(current_resistance_value[i]);
    Serial.print(" ");
    if(i%2) Serial.println(" ");
    else Serial.print(" ");
  }
    
  
  Serial.println(" ");
  for(int i = 0; i < 16;i++){
    for(int j = 0 ;j<4 ; j++){
      Serial.print(codes_and_values[find_index_of_current_substate(current_resistance_value[i])][j]);
      Serial.print(" ");
    }
    if(i%2) Serial.println(" ");
    else Serial.print(" ");
    
  }
  
}

void loop() {
  update_current_and_previous_resistance_values();
  update_index_of_current_substate_and_previous_substate();
  

  
  
  first_activated_square = NULL;
  second_activated_square = NULL;
  third_activated_square = NULL;
  fourth_activated_square = NULL;

  if(condition()){//a move is started- it is divided into four parts: beginnign, intermediate1, intermediate2, end. in each part a piece is either lifted or placed
    Serial.println("Move started");
    
    move_starting_time = millis()/1000;
    first_activated_square = find_activated_square(index_of_current_substate,index_of_previous_substate);
    Serial.println(" ");
    Serial.print("first_activated_square: ");
    Serial.print(first_activated_square);
    delay(delay_for_parts);
    while(millis()/1000-move_starting_time<threshold_for_one_move){//intermediate1
      update_current_and_previous_resistance_values();
      update_index_of_current_substate_and_previous_substate();
      if(condition()){
        second_activated_square = find_activated_square(index_of_current_substate,index_of_previous_substate);// can add the check that if first = second then restart the move
        Serial.println(" ");
        Serial.print("second_activated_square: ");
        Serial.print(second_activated_square);
        delay(delay_for_parts);
        while(millis()/1000-move_starting_time<threshold_for_one_move){//intermediate2 or end for capture
          update_current_and_previous_resistance_values();
          update_index_of_current_substate_and_previous_substate();
          if(condition()){
            third_activated_square = find_activated_square(index_of_current_substate,index_of_previous_substate);
            Serial.println(" ");
            Serial.print("third_activated_square: ");
            Serial.print(third_activated_square);
            if(second_activated_square == third_activated_square){//capture
              Serial.println(" ");
              Serial.print("captured. Move: ");
              Serial.print(first_activated_square);
              Serial.print(second_activated_square);
              move_over();
              return;//because if we break it it will again go in the while loop
            }
            delay(delay_for_parts);
            while(millis()/1000-move_starting_time<threshold_for_one_move){//intermideate3 or end for enpassant or promotion
              update_current_and_previous_resistance_values();
              update_index_of_current_substate_and_previous_substate();
              if(condition()){
                fourth_activated_square = find_activated_square(index_of_current_substate,index_of_previous_substate);
                Serial.println(" ");
                Serial.print("fourth_activated_square: ");
                Serial.print(fourth_activated_square);
                Serial.println(" ");
                Serial.print("castling. Move: ");
                Serial.print(first_activated_square);
                Serial.print(second_activated_square);
                Serial.print(third_activated_square);
                Serial.print(fourth_activated_square);
                move_over();
                return;
              }
            }
            if(!fourth_activated_square){//en passant or promotion
              Serial.println(" ");
              Serial.print("en passant or promotion. Move: ");
              Serial.print(first_activated_square);
              Serial.print(second_activated_square);
              Serial.print(third_activated_square);
              Serial.println(" ");
              move_over();
              return;
            }
          }
        }
        if(!third_activated_square){//simple move
          Serial.println(" ");
          Serial.print("simple move. Move: ");
          Serial.print(first_activated_square);
          Serial.print(second_activated_square);
          move_over();
          return;
        }
      }
    }
  move_over();
  }
}

