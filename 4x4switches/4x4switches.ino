#include <Keypad.h> //keypad library by Mark Stanley
#include <Wire.h> //built-in arduino library for communication between 2 boards

int index;

const byte ROWS = 4;
const byte COLS = 4;
const byte SIZE = ROWS * COLS;
//subtracting char 'a' from any of these chars gives the index
//for example ('a' - 'a' = 0), ('e' - 'a' = 4)
char keys[ROWS][COLS] = {
  {'a','b','c','d'},
  {'e','f','g','h'},
  {'i','j','k','l'},
  {'m','n','o','p'},
};

//we only need ROWS + COLS total pins instead of ROWS * COLS pins as we are using a matrix
//not every switch needs a corresponding pin
//specific matrix calculations handled by <Keypad.h> library
byte rowPins[ROWS] = {13,12,11,10}; 
byte colPins[COLS] = {7,6,5,4};

//keypad library setup
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
unsigned long loopCount;
unsigned long startTime;
//this string will be modified to be printed to serial monitor purely for debugging purposes
//(the actual board has no monitor)
String msg;

void setup(){

  Serial.begin(9600);
  //Changing setHoldTime to higher values will make it take longer for
  //switch to go from PRESSED state to HOLD state
  kpd.setHoldTime(100); 
  Serial.begin(9600);
    loopCount = 0;
    startTime = millis();
    msg = "";
  //This sets the board as the Master board of the two boards used
  Wire.begin(); 
}


void loop() {
    //This all prints to serial monitor for debugging
    loopCount++;
    if ( (millis()-startTime)>5000 ) {
        Serial.print("Average loops per second = ");
        Serial.println(loopCount/5);
        startTime = millis();
        loopCount = 0;
    }

    // Fills kpd.key[ ] array with up-to 10 active keys.
    // Returns true if there are ANY active keys.
    if (kpd.getKeys())
    {
        for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
        {
            if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
            {
                switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                    case PRESSED:
                    msg = " PRESSED.";
                break;
                    //HOLD only occurs after a set amount of time since PRESSED
                    //and no RELEASED has happenen
                    case HOLD:
                    msg = " HOLD.";
                    index = kpd.key[i].kchar - 'a';
                    //
                    Wire.beginTransmission(9); // transmit to device #9
                    Wire.write(index);         // send index where switch is HOLD 
                    Wire.endTransmission();    // stop transmitting
                break;
                    case RELEASED:
                    msg = " RELEASED.";
                    index = kpd.key[i].kchar - 'a';
                    //adding board size so there are different numbers spefically for turning off LEDs
                    index += SIZE;
                    Wire.beginTransmission(9); // transmit to device #9
                    Wire.write(index);         // send modified index where switch is RELEASED
                    Wire.endTransmission();    // stop transmitting
                break;
                    case IDLE:
                    msg = " IDLE.";
                }
                Serial.print("Key ");
                Serial.print(kpd.key[i].kchar);
                Serial.println(msg);
            }
        }
    }
}  // End loop
