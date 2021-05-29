#include <Wire.h> //built-in arduino library for communication between 2 boards

int index = -1;
const byte ROWS = 4;
const byte COLS = 4;
const byte SIZE = ROWS * COLS;
//keep track of pins
int leds[ROWS*COLS] = {
  2,3,4,5,
  6,7,8,9,
  10,11,12,13,
  14,15,16,17
};

void setup() {
  // Define LED pins as OUTPUT
  for (int i=0; i<ROWS*COLS; i++){
    pinMode (leds[i], OUTPUT);
  }
  
  // Start the I2C Bus as Slave on address 9
  Wire.begin(9); 
  // Attach a function to trigger when something is received.
  Wire.onReceive(receiveEvent);
}
void receiveEvent(int bytes) {
  index = Wire.read();    // reads an int index
}
void loop() {
  //If normal index (0,...,SIZE-1), turn on LED at index
  //if index plus board size (SIZE,...,SIZE+16), turn off LED at index 
  if (index >= SIZE){
    digitalWrite(leds[index-SIZE], LOW);
  }
  else {
    digitalWrite(leds[index], HIGH);
  }
}
