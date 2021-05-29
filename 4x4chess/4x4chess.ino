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

//set up pieces here, positive is one side, negative is other side
//King = 1
//Queen = 2
//Bishop = 3
//Knight = 4
//Rook = 5
//Pawn = 6 (positive pawns go on the left (aiem), negative pawns go on the right (dhlp)
//Jester = 7 (Changes between Bishop, Knight, and Rook every turn)
int pieces[ROWS*COLS] = {
  1,0,0,-6,
  2,0,0,-5,
  3,0,0,-2,
  4,0,0,-7
};
int curPiece = 0; //holds lifted piece, allows taking pieces (since taken piece needs to be lifted too), 0 means holding nothing
unsigned long turn = 0; //increments once a lifted piece is put down

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
                    //a piece is put down
                    if (curPiece != 0){ 
                      index = kpd.key[i].kchar - 'a';
                      pieces[index] = curPiece; //piece takes new spot on board
                      
                      //turn pawn to queen if it reaches end of board
                      if (curPiece == 6 && index % COLS == 3){
                        pieces[index] = 2;
                      }
                      if (curPiece == -6 && index % COLS == 0){
                        pieces[index] = -2;
                      }
                      
                      curPiece = 0;
                      turn++;
                      //wipe board of lights
                      for (int i = 0; i < ROWS*COLS; i++){
                        Wire.beginTransmission(9);
                        Wire.write(i+SIZE);
                        Wire.endTransmission();
                      }
                      //blinks to show succesful piece move
                      Wire.beginTransmission(9);
                      Wire.write(index+SIZE); //turn OFF LED at index
                      Wire.endTransmission();
                      Wire.beginTransmission(9);
                      Wire.write(index); //turn ON LED at index
                      Wire.endTransmission();
                      Wire.beginTransmission(9);
                      Wire.write(index+SIZE); //turn OFF LED at index
                      Wire.endTransmission();
                    }
                    
                break;
                    case RELEASED:
                    msg = " RELEASED.";
                    //only do chess logic if no other piece is lifted (prevents piece being taken from overriding its taker)
                    if (curPiece == 0) {
                      index = kpd.key[i].kchar - 'a';
                      curPiece = pieces[index]; //keep track of lifted piece
                      pieces[index] = 0; //lifted piece no longer on board
                      chessMoves(index); //lights up squares the lifted piece can move to
                    }
                    
                break;
                    case IDLE:
                    msg = " IDLE.";
                }
                Serial.print("Key ");
                Serial.print(kpd.key[i].kchar);
                Serial.println(msg);
                if (curPiece != 0){
                  Serial.println("Piece being lifted");
                }
            }
        }
    }
}  // End loop

void chessMoves(int index){
  int pieceType = abs(curPiece);
  if (pieceType == 1){ //king
    kingMoves(index);
  }
  else if (pieceType == 2){ //queen
    bishopMoves(index);
    rookMoves(index);
  }
  else if (pieceType == 3){ //bishop
    bishopMoves(index);
  }
  else if (pieceType == 4){ //knight
    knightMoves(index);
  }
  else if (pieceType == 5){ //rook
    rookMoves(index);
  }
  else if (pieceType == 6){ //pawn
    pawnMoves(index);
  }
  else if (pieceType == 7){ //jester
    if (turn % 6 == 0 || turn % 6 == 1){
      bishopMoves(index);
    }
    else if (turn % 6 == 2 || turn % 6 == 3){
      knightMoves(index);
    }
    else if (turn % 6 == 4 || turn % 6 == 5){
      rookMoves(index);
    }
  }
}

void kingMoves(int index){
  int moveSet[8][2] = {
    {-1,-1},{-1,0},{-1,1},
    {0,-1},       {0,1},
    {1,-1},{1,0},{1,1}
  };
  int i = index / ROWS;
  int j = index % COLS;
  for (int x = 0; x < 8; x++){
    legalMove(i + moveSet[x][0], j + moveSet[x][1]);
  }
}

void bishopMoves(int index){
  int i = index / ROWS;
  int j = index % COLS;
  int x;
  int y;
  x = -1;
  y = -1;
  //top left diagonal
  while (legalMove(i + x, j + y)){
    x--;
    y--;
  }
  //top right diagonal
  x = -1;
  y = 1;
  while (legalMove(i + x, j + y)){
    x--;
    y++;
  }
  //bot left diagonal
  x = 1;
  y = -1;
  while (legalMove(i + x, j + y)){
    x++;
    y--;
  }
  //bot right diagonal
  x = 1;
  y = 1;
  while (legalMove(i + x, j + y)){
    x++;
    y++;
  }
}

void rookMoves(int index){
  int i = index / ROWS;
  int j = index % COLS;
  int x;
  int y;
  x = -1;
  y = 0;
  //up 
  while (legalMove(i + x, j + y)){
    x--;
  }
  //down
  x = 1;
  y = 0;
  while (legalMove(i + x, j + y)){
    x++;
  }
  //left
  x = 0;
  y = -1;
  while (legalMove(i + x, j + y)){
    y--;
  }
  //right
  x = 0;
  y = 1;
  while (legalMove(i + x, j + y)){
    y++;
  }
}

void knightMoves(int index){
  int moveSet[8][2] = {
    {2,1},{1,2},
    {-1,2},{-2,1},
    {-2,-1},{-1,-2},
    {1,-2},{2,-1}
  };
  int i = index / ROWS;
  int j = index % COLS;
  for (int x = 0; x < 8; x++){
    legalMove(i + moveSet[x][0], j + moveSet[x][1]);
  }
}

void pawnMoves(int index){
  int i = index / ROWS;
  int j = index % COLS;
  //extract 1/-1 from curPiece
  int checkSelf = curPiece;
  checkSelf = checkSelf/abs(checkSelf);
  //check one move ahead (no double move start since board is too small)
  legalMove(i,j + checkSelf);
  //if positive check right diagonals
  if (checkSelf > 0){
    if (pieces[keys[i-1][j+1] - 'a'] != 0){
      legalMove(i-1,j+1);
    }
    if (pieces[keys[i+1][j+1] - 'a'] != 0){
      legalMove(i+1,j+1);
    }
  }
  //if negative check left diagonals
  if (checkSelf < 0){
    if (pieces[keys[i-1][j-1] - 'a'] != 0){
      legalMove(i-1,j-1);
    }
    if (pieces[keys[i+1][j-1] - 'a'] != 0){
      legalMove(i+1,j-1);
    }
  }
}

//makes sure checks does not go out of bounds
//makes sure checks do not light up ally pieces
//lights up enemy pieces
//returns false for these cases to show not to check further squares
//lights up empty squares
//returns true to show that it can keep checking further squares since no obstacles
bool legalMove(int i, int j){ 
  //check out of bounds
  if (i > ROWS - 1 || i < 0){
    return false;
  }
  if (j > COLS - 1 || j < 0){
    return false;
  }
  int checkIndex = keys[i][j] - 'a';
  //square is empty
  if (pieces[checkIndex] == 0){
    Wire.beginTransmission(9);
    Wire.write(checkIndex); //turn ON LED at index
    Wire.endTransmission();
    Serial.println("LED at ");
    Serial.println(keys[i][j]);
    Serial.println(" ON");
    return true;
  }
  int checkTeam = pieces[checkIndex];
  //extract positive/negative
  checkTeam = checkTeam/abs(checkTeam);
  int checkSelf = curPiece;
  checkSelf = checkSelf/abs(checkSelf);

  //is ally
  if (checkSelf * checkTeam > 0){
    return false;
  }

  //is enemy
  if (checkSelf * checkTeam < 0){
    Wire.beginTransmission(9);
    Wire.write(checkIndex); //turn ON LED at index
    Wire.endTransmission();
    Serial.println("LED at ");
    Serial.println(keys[i][j]);
    Serial.println(" ON");
    return false;
  }
}
