const int codes_and_values[31][6] = {
{0, 0, 0, 0, 1, 614},
{0, 0, 0, 1, 0, 767},
{0, 0, 0, 1, 1, 512},
{0, 0, 1, 0, 0, 884},
{0, 0, 1, 0, 1, 561},
{0, 0, 1, 1, 0, 687},
{0, 0, 1, 1, 1, 474},
{0, 1, 0, 0, 0, 947},
{0, 1, 0, 0, 1, 586},
{0, 1, 0, 1, 0, 724},
{0, 1, 0, 1, 1, 492},
{0, 1, 1, 0, 0, 827},
{0, 1, 1, 0, 1, 537},
{0, 1, 1, 1, 0, 652},
{0, 1, 1, 1, 1, 457},
{1, 0, 0, 0, 0, 984},
{1, 0, 0, 0, 1, 599},
{1, 0, 0, 1, 0, 745},
{1, 0, 0, 1, 1, 501},
{1, 0, 1, 0, 0, 855},
{1, 0, 1, 0, 1, 549},
{1, 0, 1, 1, 0, 669},
{1, 0, 1, 1, 1, 466},
{1, 1, 0, 0, 0, 913},
{1, 1, 0, 0, 1, 573},
{1, 1, 0, 1, 0, 704},
{1, 1, 0, 1, 1, 483},
{1, 1, 1, 0, 0, 801},
{1, 1, 1, 0, 1, 526},
{1, 1, 1, 1, 0, 635},
{1, 1, 1, 1, 1, 449}
};
int threshold_for_value_difference = 5;
int threshold_for_one_move = 2; //in seconds
int current_resistance_value, move_starting_time=0,index_of_current_state,index_of_previous_state,previous_resistance_value;
int first_activated_square,second_activated_square,third_activated_square,fourth_activated_square;
bool first_move_of_the_game = true;
bool move_in_progress = false;
//functions

int find_index_of_current_state(int current_resistance_value){
  for(int i=0; i<31; i++){
    if(abs(codes_and_values[i][5]-current_resistance_value)<= threshold_for_value_difference){
      Serial.println("Current state: ");
      for (int j = 0; j < 6; j++) {
        Serial.print(codes_and_values[i][j]);
        Serial.print(" ");
      }
      return i;
    }
  }
  return -1;
}
int find_activated_square(int index_of_current_state, int index_of_previous_state){
  for(int i = 0 ; i<5;i++){
    if(codes_and_values[index_of_current_state][i]!=codes_and_values[index_of_previous_state][i]){
      return i+1;
    }
  }
  return 0;
}
///////////////////////////////////////////////////////////////
void setup() {
  pinMode(A0,INPUT);// put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
  previous_resistance_value = analogRead(A0);
  index_of_current_state = find_index_of_current_state(previous_resistance_value);
}

void loop() {
  current_resistance_value = analogRead(A0);
  // if(!first_move_of_the_game && (millis()/1000-move_starting_time>threshold_for_one_move) && move_in_progress){
  //   move_in_progress = false;
  // }
  first_activated_square = 0;
  second_activated_square = 0;
  third_activated_square = 0;
  fourth_activated_square = 0;

  if(abs(previous_resistance_value-current_resistance_value)>threshold_for_value_difference){//a move is started- it is divided into four parts: beginnign, intermediate1, intermediate2, end. in each part a piece is either lifted or placed
    first_move_of_the_game = true;
    move_starting_time = millis()/1000;
    index_of_previous_state = index_of_current_state;
    index_of_current_state = find_index_of_current_state(current_resistance_value);
    first_activated_square = find_activated_square(index_of_current_state,index_of_previous_state);
    Serial.println("first_activated_square: ");
    Serial.print(first_activated_square);
    while(millis()/1000-move_starting_time<threshold_for_one_move){//intermediate1
      if(abs(previous_resistance_value-current_resistance_value)>threshold_for_value_difference){
        index_of_previous_state = index_of_current_state;
        index_of_current_state = find_index_of_current_state(current_resistance_value);
        second_activated_square = find_activated_square(index_of_current_state,index_of_previous_state);// can add the check that if first = second then restart the move
        Serial.println("second_activated_square: ");
        Serial.print(second_activated_square);
        while(millis()/1000-move_starting_time<threshold_for_one_move){//intermediate2 or end for capture
          if(abs(previous_resistance_value-current_resistance_value)>threshold_for_value_difference){
            index_of_previous_state = index_of_current_state;
            index_of_current_state = find_index_of_current_state(current_resistance_value);
            third_activated_square = find_activated_square(index_of_current_state,index_of_previous_state);// can add the check that if first = second then restart the move
            Serial.println("third_activated_square: ");
            Serial.print(third_activated_square);
            if(second_activated_square == third_activated_square){//capture
              Serial.println("captured. Move: ");
              Serial.print(first_activated_square);
              Serial.print(second_activated_square);
              break;
            }
            while(millis()/1000-move_starting_time<threshold_for_one_move){//intermideate3 or end for enpassant or promotion
              if(abs(previous_resistance_value-current_resistance_value)>threshold_for_value_difference){
                index_of_previous_state = index_of_current_state;
                index_of_current_state = find_index_of_current_state(current_resistance_value);
                fourth_activated_square = find_activated_square(index_of_current_state,index_of_previous_state);// can add the check that if first = second then restart the move
                Serial.println("fourth_activated_square: ");
                Serial.print(fourth_activated_square);
                Serial.println("castling. Move: ");
                Serial.print(first_activated_square);
                Serial.print(second_activated_square);
                Serial.print(third_activated_square);
                Serial.print(fourth_activated_square);
              }
            }
            if(!fourth_activated_square){//en passant or promotion
              Serial.println("en passant or promotion. Move: ");
              Serial.print(first_activated_square);
              Serial.print(second_activated_square);
              Serial.print(third_activated_square);
            }
          }
        }
        if(!third_activated_square){//simple move
          Serial.println("simple move. Move: ");
          Serial.print(first_activated_square);
          Serial.print(second_activated_square);
        }
      }
      break;
    }
  }
  previous_resistance_value=current_resistance_value;
}

