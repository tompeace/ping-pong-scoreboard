#include "HT1632.h"
#include "pitches.h"

//declare variables for matrix display
#define DATA 3
#define WR   2
#define CS   4
#define CS2  5
#define CS3  6
#define CS4  7

//declare variables for game buttons and mode selector
#define button1 8
#define button2 9
#define musicPin 12
#define gameModeSelecta A0

//declare matrix variable
HT1632LEDMatrix matrix = HT1632LEDMatrix(DATA, WR, CS, CS2, CS3, CS4);



//Button States
int b1State = 0; //current value for button state
int b2State = 0;
int prev1State = 0; //previous value for button pushstate
int prev2State = 0;

//variables for long press
long holdLimit = 700;
long holdTime = 0;
boolean allow = false;
int press1State = 0;
int press2State = 0;

//variable for storing individual player score
int player1Score = 0;
int player2Score = 0;
int prev1Score = 0;
int prev2Score = 0;

//variables to tell whos serve it is
int totalScore = 0; // keep track of total score to deduce serve turn
boolean serve = 0; //player1 = 0, player2 = 1
int serveLimit = 0; // # of serves per player for different pointLimits 
int changeServe = false; // 

//game mode variables
int gameModeVal = 0; // value read on gameModeSelecta pin
int pointLimit = 0; //point limits for different games


int x = 0; //x coordinate for scrolling text horizontally 



//gameState values for navigation through different screens
int gameState = 0;

//gameState functions labelled as follows
/* 
 gameSelect = 0; 
 game = 1;
 gameEnd = 2;
 playAgain = 3
 serveSelect = 4;
 */

String winner = "";

void setup () {
  //initialise serial connection for debugging  
  Serial.begin(9600);
  //initialse buttons, knob and lcd
  pinMode(gameModeSelecta, INPUT);
  pinMode(button1, INPUT); 
  pinMode(button2, INPUT);
  matrix.begin(HT1632_COMMON_16NMOS);
}

void loop() {
  //more initialising
  b1State = digitalRead(button1);
  b2State = digitalRead(button2);
  matrix.setTextSize(1);
  matrix.setTextColor(1);

  //run the game modes on a loop indefinetley
  if (gameState == 0) {
    gameSelect();
  } 
  else if (gameState == 1) {
    game();
  } 
  else if (gameState == 2) {
    gameEnd();
  } 
  else if (gameState == 3) {
    playAgain();
  } 
  else if (gameState == 4) {
    serveSelect();
  }

} 

// GAMESTATE = 0
void gameSelect() {
  //find out pointLimit from position of the potentiometer
  gameModeVal = analogRead(gameModeSelecta);


  //detect game mode by dividing the reading of gameModeSelecta 
  //by the number of desired pointLimits and returning the result through
  //switch function, assigning variables for point limit and serve turn 
  //which are apssed to the game function
  switch(gameModeVal/(1024/3)) {
  case 0:
    pointLimit = 5;
    serveLimit = 1;
    break;
  case 1:
    pointLimit = 11;
    serveLimit = 3;
    break;
  case 2:
    pointLimit = 21;
    serveLimit = 5;
    break;
  default: 
    pointLimit = 11;
    serveLimit = 3;
  }

  //print current game mode
  matrix.fillRect(0, 0, 96, 16, 0);
  matrix.setCursor(1, 4);
  matrix.print("Select a game-");
  matrix.print(pointLimit);
  matrix.writeScreen();  
  if (b1State == HIGH || b2State == HIGH) {
    delay(250); //add a delay so that the confirmation button press doesnt increment to the score
    gameState = 4;
    matrix.fillRect(0, 0, 96, 16, 0);    
  }
}




//GAMESTATE = 1
void game() {
  matrix.setTextSize(2); //make the score fill the display
  x = 0; //reset x everytime function is entered

  
  if (totalScore > 0 && totalScore % serveLimit == 0 && changeServe == true) {
    serve = !serve;
    changeServe = false;     
  }
  
  if ((b1State == HIGH || b2State == HIGH) && (press1State == LOW || press2State == LOW)) {
    holdTime = millis();
    allow = true;
  }

  if (allow == true && (b1State == HIGH || b2State == HIGH) && (press1State == LOW || press2State == HIGH)) {
    if ((millis() - holdTime) >= holdLimit) {
      player1Score = 0;
      player2Score = 0;
      totalScore = 0;
      matrix.fillRect(0, 0, 96, 16, 0);
    }
  }
  press1State = b1State;
  press2State = b2State;


  //Increment the score for player 1 
  //if his button is pressed

  if (gameState == 1) { //set the game mode
    if(b1State != prev1State) {
      if(b1State == HIGH) { //everytime the button is pressed...
        totalScore++;
        player1Score ++; //incremement the score by one.
        tone(musicPin, 220, 75);
        prev1Score = player1Score; //save previous score for player 1
        changeServe = true;        
      }
    }
    prev1State = b1State;

    //same as above but for player 2
    if(b2State != prev2State) { 
      if(b2State == HIGH) {
        totalScore++;
        player2Score ++;
        tone(musicPin, 196, 75);
        prev2Score = player2Score;
        changeServe = true;
      }
    }
  }
  prev2State = b2State; //save previous score for player 2

  //store total score to find out serve turn


  //sets up a mechanism in the event of a tie-breaker
  if(player1Score >= pointLimit) {
    if (player1Score > player2Score && player1Score - player2Score <= 1) {
      serveLimit = 1;
      deuce();
    } 
    //delcares a winner and pass on "winner" to the gameEnd(); function
    else {
      delay(500);
      winner = "Player 1"; //passes on the value to gameEnd() to be printed to screen
      gameState = 2; //go to gameEnd function 
      allow = false; // long presses no longer allowed     
      matrix.fillRect(0, 0, 96, 16, 0); 
    }
  } 
  else if (player2Score >= pointLimit) {
    if (player2Score > player1Score && player2Score - player1Score <= 1) {
      serveLimit = 1;
      deuce();
    } 
    else {
      delay(500);
      winner = "Player 2";
      gameState = 2;
      allow = false;
      matrix.clearScreen();
    } 
  } 
  else {

    //print out the part of the message 
    //which will always be displayed   
    matrix.fillRect(0, 0, 96, 16, 0); 

    switch (serve) {
    case 0:
      matrix.setCursor(0, 1);
      matrix.print("<");
      break;
    case 1:
      matrix.setCursor(86, 1);
      matrix.print(">");
      break;
    default:
      break;
    }

    if (player1Score > 9) {
      matrix.setCursor(19, 1);
    } 
    else {
      matrix.setCursor(31, 1);
    }
    matrix.print(player1Score);
    matrix.setCursor(43, 1);
    matrix.print("-");
    matrix.setCursor(55, 1);
    matrix.print(player2Score);
    matrix.writeScreen();
  }
}

//tie-breaker function
void deuce() {
  pointLimit++;
  game();  
}

//GAMESTATE = 2
//print out the score and declare 
//a winner if pointLimit is reached
void gameEnd () {
 
  matrix.fillRect(0, 0, 96, 16, 0);

  //Display final score
  matrix.setTextSize(2);
  if (player1Score > 9) {
    matrix.setCursor((x+19), 1);
  } 
  else {
    matrix.setCursor((x+31), 1);
  }   
  matrix.print(player1Score);
  matrix.setCursor((x+43), 1);
  matrix.print("-");
  matrix.setCursor((x+55), 1);
  matrix.print(player2Score);
  matrix.writeScreen();
  delay(500);


  //Scroll winner

  matrix.fillRect(0, 0, 96, 16, 0);
  matrix.setTextSize(1);

  matrix.setCursor(x+8, 4);
  //matrix.fillRect(x, 4, 84, 9, 0);

  matrix.print(winner);
  matrix.print(" wins!");

  matrix.writeScreen();
  delay(500);

  if(x >= 96) {
    x = -82;
  }
  
  //if button is pressed take the game into the next game state 
  if (b1State == HIGH || b2State == HIGH) {
    gameState = 3;
    matrix.fillRect(0, 0, 96, 16, 0);
    delay(500);
  }   
} 

//GAMESTATE = 3
//clear the player scores and total score for the next game
void playAgain() {

  player1Score = 0;
  player2Score = 0;
  totalScore = 0;
  matrix.setCursor(19, 4);
  matrix.print("Play again?");
  matrix.writeScreen();
  //if button is pressed take the game back to the beginning! 
  if (b1State == HIGH || b2State == HIGH) {
    delay(500);
    gameState = 0;
    matrix.fillRect(0, 0, 96, 16, 0);
  }
}

//GAMESTATE = 4
//Pick a player to start the serve. whoever wins the rally presses
//their button and the serve round begins with them.
void serveSelect() {

  matrix.fillRect(0, 0, 96, 16, 0);
  matrix.setTextSize(2);
  matrix.setCursor(0, 1);
  matrix.print("<");
  matrix.setCursor(19, 1);
  matrix.print("Rally");
  matrix.setCursor(86, 1);
  matrix.print(">");
  matrix.writeScreen();

  //pass on the value of serve into the game to initialise the serving rounds
  if (b1State == HIGH) {
    serve = 0;
    delay(250);
    gameState = 1;
    matrix.fillRect(0, 0, 96, 16, 0);

  } 
  else if (b2State == HIGH) {
    serve = 1;
    delay(250);
    gameState = 1;
    matrix.fillRect(0, 0, 96, 16, 0);
  }    
}
