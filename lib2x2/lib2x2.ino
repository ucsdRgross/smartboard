#include <Keypad.h> //keypad library by Mark Stanley

int LED1 = 6;
int LED2 = 5; 
int LED3 = 4;
int LED4 = 3;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'a','b','c','d'},
  {'e','f','g','h'},
  {'i','j','k','l'},
  {'m','n','o','p'},
};
int leds[ROWS*COLS] = {LED1,LED2,LED3,LED4};

byte rowPins[ROWS] = {13,12,11,10}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7,6,5,4}; //connect to the column pinouts of the keypad

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
unsigned long loopCount;
unsigned long startTime;
String msg;

void setup(){
  /*
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  */
  Serial.begin(9600);
  kpd.setHoldTime(100); 
  Serial.begin(9600);
    loopCount = 0;
    startTime = millis();
    msg = "";
}


void loop() {
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
                    case HOLD:
                    msg = " HOLD.";
                    //digitalWrite(leds[(kpd.key[i].kchar - '`')], HIGH);
                break;
                    case RELEASED:
                    msg = " RELEASED.";
                    //digitalWrite(leds[(kpd.key[i].kchar - '`')], LOW);
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
