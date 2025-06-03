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
int threshold_for_value_difference = 7;
int threshold_for_one_move = 5; //in seconds
int delay_for_parts = 200; //in ms
int current_resistance_value, move_starting_time=0,index_of_current_state,index_of_previous_state,previous_resistance_value;
int first_activated_square,second_activated_square,third_activated_square,fourth_activated_square;
//functions

int find_index_of_current_state(int current_resistance_value){
  for(int i=0; i<16; i++){
    if(abs(codes_and_values[i][4]-current_resistance_value)<= threshold_for_value_difference){
      Serial.println(" ");
      Serial.print("Current state: ");
      for (int j = 0; j < 5; j++) {
        Serial.print(codes_and_values[i][j]);
        Serial.print(" ");
      }
      return i;
    }
  }
  return -1;
}
int find_activated_square(int index_of_current_state, int index_of_previous_state){
  for(int i = 0 ; i<4;i++){
    if(codes_and_values[index_of_current_state][i]!=codes_and_values[index_of_previous_state][i]){
      return i+1;
    }
  }
  return 0;
}

bool differs_by_one_bit(int prev_index, int current_value) {
  int curr_index = find_index_of_current_state(current_value);
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
    readings[i] = analogRead(pin);
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


///////////////////////////////////////////////////////////////
void setup() {
  pinMode(A0,INPUT);// put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
  previous_resistance_value = filteredAnalogRead(A0);
  index_of_current_state = find_index_of_current_state(previous_resistance_value);
}

void loop() {
  current_resistance_value = filteredAnalogRead(A0);
  // if(!first_move_of_the_game && (millis()/1000-move_starting_time>threshold_for_one_move) && move_in_progress){
  //   move_in_progress = false;
  // }
  first_activated_square = 0;
  second_activated_square = 0;
  third_activated_square = 0;
  fourth_activated_square = 0;

  if(abs(previous_resistance_value-current_resistance_value)>threshold_for_value_difference&&
    differs_by_one_bit(index_of_current_state, current_resistance_value)){//a move is started- it is divided into four parts: beginnign, intermediate1, intermediate2, end. in each part a piece is either lifted or placed
    Serial.println("Move started");
    //first_move_of_the_game = true;
    move_starting_time = millis()/1000;
    index_of_previous_state = index_of_current_state;
    index_of_current_state = find_index_of_current_state(current_resistance_value);
    first_activated_square = find_activated_square(index_of_current_state,index_of_previous_state);
    Serial.println(" ");
    Serial.print("first_activated_square: ");
    Serial.print(first_activated_square);
    delay(delay_for_parts);
    while(millis()/1000-move_starting_time<threshold_for_one_move){//intermediate1
      previous_resistance_value = current_resistance_value;
      current_resistance_value = filteredAnalogRead(A0);
      if(abs(previous_resistance_value-current_resistance_value)>threshold_for_value_difference&&
      differs_by_one_bit(index_of_current_state, current_resistance_value)){
        index_of_previous_state = index_of_current_state;
        index_of_current_state = find_index_of_current_state(current_resistance_value);
        second_activated_square = find_activated_square(index_of_current_state,index_of_previous_state);// can add the check that if first = second then restart the move
        Serial.println(" ");
        Serial.print("second_activated_square: ");
        Serial.print(second_activated_square);
        delay(delay_for_parts);
        while(millis()/1000-move_starting_time<threshold_for_one_move){//intermediate2 or end for capture
          previous_resistance_value = current_resistance_value;
          current_resistance_value = filteredAnalogRead(A0);
          if(abs(previous_resistance_value-current_resistance_value)>threshold_for_value_difference&&
          differs_by_one_bit(index_of_current_state, current_resistance_value)){
            index_of_previous_state = index_of_current_state;
            index_of_current_state = find_index_of_current_state(current_resistance_value);
            third_activated_square = find_activated_square(index_of_current_state,index_of_previous_state);
            Serial.println(" ");
            Serial.print("third_activated_square: ");
            Serial.print(third_activated_square);
            if(second_activated_square == third_activated_square){//capture
              Serial.println(" ");
              Serial.print("captured. Move: ");
              Serial.print(first_activated_square);
              Serial.print(second_activated_square);
              Serial.println(" ");
              Serial.print("Move Over");
              first_activated_square = 0;
              second_activated_square = 0;
              third_activated_square = 0;
              fourth_activated_square =0;
              return;//because if we break it it will again go in the while loop
            }
            delay(delay_for_parts);
            while(millis()/1000-move_starting_time<threshold_for_one_move){//intermideate3 or end for enpassant or promotion
              previous_resistance_value = current_resistance_value;
              current_resistance_value = filteredAnalogRead(A0);
              if(abs(previous_resistance_value-current_resistance_value)>threshold_for_value_difference&&
              differs_by_one_bit(index_of_current_state, current_resistance_value)){
                index_of_previous_state = index_of_current_state;
                index_of_current_state = find_index_of_current_state(current_resistance_value);
                fourth_activated_square = find_activated_square(index_of_current_state,index_of_previous_state);
                Serial.println(" ");
                Serial.print("fourth_activated_square: ");
                Serial.print(fourth_activated_square);
                Serial.println(" ");
                Serial.print("castling. Move: ");
                Serial.print(first_activated_square);
                Serial.print(second_activated_square);
                Serial.print(third_activated_square);
                Serial.print(fourth_activated_square);
                Serial.println(" ");
                Serial.print("Move Over");
                first_activated_square = 0;
                second_activated_square = 0;
                third_activated_square = 0;
                fourth_activated_square =0;
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
              Serial.print("Move Over");
              first_activated_square = 0;
              second_activated_square = 0;
              third_activated_square = 0;
              fourth_activated_square =0;
              return;
            }
          }
        }
        if(!third_activated_square){//simple move
          Serial.println(" ");
          Serial.print("simple move. Move: ");
          Serial.print(first_activated_square);
          Serial.print(second_activated_square);
          first_activated_square = 0;
          second_activated_square = 0;
          third_activated_square = 0;
          fourth_activated_square =0;
          Serial.println(" ");
          Serial.print("Move Over");
          return;
        }
      }
    }
  Serial.println(" ");
  Serial.print("Move Over");
  }
  
  previous_resistance_value=current_resistance_value;
}

