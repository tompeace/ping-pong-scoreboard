#include <LiquidCrystal.h>

//declare pins for lcd display
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//setup pins for input buttons
const int button1 = 8;
const int button2 = 9;
const int gameModeSelecta = A2;

//value for each buttons pushstate
int b1State = 0;
int b2State = 0;

//previous value for button pushstate
int prev1State = 0;
int prev2State = 0;

//variable for storing individual player score
int player1Score = 0;
int player2Score = 0;
int prev1Score = 0;
int prev2Score = 0;

//game mode variables
int gameModeVal = 0;
int pointLimit = 0;
int prevPointLimit = 0;

int pointLimitSet = false;
//int inGame = false;
//int gameEnd = false;
//int playAgain = false;

int gameState = 0;
  /* 
    gameSelect = 0;
    game = 1;
    gameEnd = 2;
    playAgain = 3
  */

String winner = "";

void setup () {
  //initialise serial connection for debugging  
  Serial.begin(9600);
  //initialse buttons, knob and lcd
  //pinMode(resetPin, OUTPUT);
  pinMode(gameModeSelecta, INPUT);
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  lcd.begin(16, 2); 
}


void loop() {
  b1State = digitalRead(button1);
  b2State = digitalRead(button2);

  if (gameState == 0) {
    gameSelect();
  } else if (gameState == 1) {
    game();
  } else if (gameState == 2) {
    gameEnd();
  } else if (gameState == 3) {
    playAgain();
  }
} 

// GAMESTATE = 0
//A menu for choosing how many points to play to
void gameSelect() {
  
  //find out pointLimit from position of the potentiometer
  gameModeVal = analogRead(gameModeSelecta);
  
  
  //detect game mode
  switch(gameModeVal/(1024/3)) {
  case 0:
    pointLimit = 11;
    break;
  case 1:
    pointLimit = 5;
    break;
  case 2:
    pointLimit = 21;
    break;
  default: 
    pointLimit = 11;
  }

  //display the current game mode
  lcd.setCursor(2, 0);
  lcd.print("Select a game");  
  lcd.setCursor(8,1);
  lcd.print(pointLimit);
  lcd.print(" ");     
  if (b1State == HIGH || b2State == HIGH) {
    delay(500); //add a delay so that the confirmation button press doesnt increment to the score
    gameState = 1;
    //refresh the screen
    lcd.clear();        
  }
}


//GAMESTATE = 3
//A menu to ask the player if they want to play again, 
//but also to reset all the values from the previous match back to zero
void playAgain() {
  
  player1Score = 0;
  player2Score = 0;
  lcd.setCursor(0, 0);
  lcd.print("Play again?");
  //if button is pressed, go to next menu
  if (b1State == HIGH || b2State == HIGH) {
    delay(500);
    gameState = 0;
    lcd.clear();
  }
}


//GAMESTATE = 2 
//A screen in which a winner is declared
void gameEnd () {

  //print out the score and declare 
  //a winner if pointLimit is reached 
  
  lcd.setCursor(1, 0);
  lcd.print(winner);
  lcd.print(" wins!");
  Serial.println(b1State);
  //if a button is pressed, go to next menu
  if (b1State == HIGH || b2State == HIGH) {
    delay(500);
    gameState = 3;
    lcd.clear();
  }   
} 

//a function that creates a tiebreaker scenario
void deuce() {
  pointLimit++;
  game();  
}


//GAMESTATE = 1
void game () {

  //Increment the score of either player 
  //if the buttons are pressed
  if (gameState == 1) { //setting winner to determine which game mode we are in
    if(b1State != prev1State) {
      if(b1State == HIGH) {
        player1Score ++;  
      }
    }
    prev1State = b1State;
    if(b2State != prev2State) {
      if(b2State == HIGH) {
        player2Score ++;
      }
    }
  }
  //score the current score until the next point is scored
  prev2State = b2State;
  
  //while the player are under the pointLimit
  if(player1Score >= pointLimit) {
    //check to see if the point difference is of 2, and if it is NOT,
    //increment the point limit until the required difference is reached.
    if (player1Score > player2Score && player1Score - player2Score <= 1) {
      deuce();
    } 
    else { 
      //if the socre difference is larger than 2 upon reaching pointLimit,
      //declare a winner
      winner = "Player 1";
      delay(500);
      //go to gameEnd();
      gameState = 2;
      lcd.clear();
    }

  }
  //same as for player 1
  else if (player2Score >= pointLimit) {
    if (player2Score > player1Score && player2Score - player1Score <= 1) {
      deuce();
    } 
    else {
      winner = "Player 2";
      delay(500);
      gameState = 2;
      lcd.clear();
    } 
  } 
  else {  
    //if the game in progress print out the current score
    lcd.setCursor(0, 0);
    lcd.print("the score is: ");
    lcd.setCursor(0, 1);
    lcd.print(player1Score);
    lcd.print(" - ");
    lcd.print(player2Score);
  }
}

