#include <Arduino.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <SPI.h>

// include the library code:
#include <LiquidCrystal.h>
#include <Servo.h>

#define BAUD_RATE 9600

#define SPI_RGB1_R 1
#define SPI_RGB1_G 2
#define SPI_RGB1_B 3
#define SPI_RGB2_R 4
#define SPI_RGB2_G 5
#define SPI_RGB2_B 6

#define SPI_BTN1_R 11
#define SPI_BTN1_G 12
#define SPI_BTN1_B 13
#define SPI_BTN2_R 14
#define SPI_BTN2_G 15
#define SPI_BTN2_B 16

#define GAME_START 21
#define GAME_STOP 20
#define IGNORE 23

#define SECOND 1000         // second in milis
#define GAME_DURATION 100   // secunde
#define ROND_DURATION 10000 // milisecond
#define END_ROTATION 180
#define START_ROTATION 0
#define ROTATION_DISTANCE 45

struct Players
{
  unsigned int points;
  String name;
} player[2];

bool gameOn = 0;
int playerRouond = 0;
int lastPlayerRound = 0;
int correctRgb = 0;
unsigned long lastRoundTime = 0;
unsigned long currentRoundTime = 0;
unsigned long gameStartTime = 0;
int randomRgb;
byte masterSend = IGNORE, masteReceive;

// delay but with millis
void delayMillis(unsigned long milliseconds)
{
  unsigned long currentTimeDelay = millis();
  unsigned long goalTimeDelay = currentTimeDelay + milliseconds;

  while (millis() <= goalTimeDelay)
    ;
}

////////////////////////////////////// DISPLAY LCD ///////////////////////////

// Display PINS
#define RS 9
#define EN 8
#define D4 5
#define D5 4
#define D6 3
#define D7 2
// special characters for LCD
byte trophy[8] = {
    0b00100,
    0b11111,
    0b10101,
    0b11111,
    0b01110,
    0b00100,
    0b00100,
    0b01110};
byte arrow[8] = {
    0b00000,
    0b00100,
    0b00110,
    0b11111,
    0b00110,
    0b00100,
    0b00000,
    0b00000};

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

unsigned long lastTime = 0;
/*
  Display one of the states of the game
  0 - Game OFF; 1- player 1 round; 2 - player 2 round; 3 - show the winner
  default - eror "ceva?!"
*/
void displayInGame(Players p1, Players p2, int stateGame)
{
  // for debug:
  Serial.print(p1.name);
  Serial.print("----");
  Serial.print(p1.points);
  Serial.print("----");
  Serial.print(p2.name);
  Serial.print("----");
  Serial.println(p2.points);
  lcd.clear();
  lcd.setCursor(0, 0);
  switch (stateGame)
  {
  case 0:
    lcd.print("Joc de Reflex");
    break;
  case 1:
    lcd.write(byte(1));
    lcd.print(p1.name);
    lcd.print(":");
    lcd.print(p1.points);
    lcd.setCursor(0, 1);
    lcd.print(p2.name);
    lcd.print(":");
    lcd.print(p2.points);
    break;
  case 2:
    lcd.print(p1.name);
    lcd.print(":");
    lcd.print(p1.points);
    lcd.setCursor(0, 1);
    lcd.write(byte(1));
    lcd.print(p2.name);
    lcd.print(":");
    lcd.print(p2.points);
    break;
  case 3:
    if (p1.points > p2.points)
    {
      lcd.write(byte(0));
      lcd.print(p1.name);
      lcd.print(":");
      lcd.print(p1.points);
      lcd.setCursor(0, 1);
      lcd.print(p2.name);
      lcd.print(":");
      lcd.print(p2.points);
    }
    else if (p1.points < p2.points)
    {
      lcd.print(p1.name);
      lcd.print(":");
      lcd.print(p1.points);
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
      lcd.print(p2.name);
      lcd.print(":");
      lcd.print(p2.points);
    }
    else if (p1.points == p2.points)
    {
      lcd.write(byte(0));
      lcd.print(":");
      lcd.print(p1.name);
      lcd.print(p1.points);
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
      lcd.print(p2.name);
      lcd.print(":");
      lcd.print(p2.points);
    }
    break;
  default:
    lcd.print("ceva?!");
    break;
  }
}
void initiateLCD()
{
  // set up the LCD's number of columns and rows
  lcd.begin(16, 2);
  lcd.createChar(0, trophy);
  lcd.createChar(1, arrow);
}

void displayCountdown()
{
  lcd.clear();
  for (int i = 3; i > 0; i--)
  {
    unsigned long currentTimeLcd = millis();
    unsigned long goalTime = currentTimeLcd + SECOND;
    while (millis() <= goalTime)
      ;
    lcd.setCursor(0, 0);
    lcd.print(i);
    Serial.println(i);
  }
  lcd.clear();
}

////////////////////////////////servomotor////////////////////////////////////////////
Servo myservo; // create servo object to control a servo
unsigned long currentServoTime = 0;
unsigned long lastServoTime = 0;
unsigned int pos = 0; // variable to store the servo position

void servoSetup()
{
  myservo.attach(6); // attaches the servo on pin 6 to the servo objec
}
/*
Servo logic when the game is on
the global servo will move evry 1/4 of the round 45 degrees
*/
void servoOnGame(int gameDurationServo)
{
  unsigned long goalTime = currentServoTime + gameDurationServo * SECOND * ROTATION_DISTANCE / END_ROTATION;
  if (millis() >= goalTime)
  {
    currentServoTime = millis();

    pos += ROTATION_DISTANCE;
    if (pos > END_ROTATION)
    {
      pos = START_ROTATION;
    }
    myservo.write(pos);
    Serial.println(pos);
  }
}

void servoOffGame()
{
  pos = 0;
  myservo.write(0);
}

void setup()
{

  Serial.begin(BAUD_RATE);

  randomSeed(analogRead(0));

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.setDataMode(SPI_MODE0);
  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);
  initiateLCD();
  servoSetup();
  player[0].name = "P1";
  player[0].points = 0;
  player[1].name = "P2";
  player[1].points = 0;

  randomRgb = random() % 3;
  myservo.write(0);
  gameStartTime = millis();
}

void loop()
{
  // for debug:
  // servoOnGame(GAME_DURATION);
  if (!gameOn)
  {
    // Game OFF logic
    displayInGame(player[0], player[1], 0);
    digitalWrite(SS, LOW);                   // Starts communication with Slave connected to master
    masteReceive = SPI.transfer(masterSend); // Send the mastersend value to slave also receives value from slave
    digitalWrite(SS, HIGH);
    // debug
    Serial.print("OFF --- receive:");
    Serial.print(masteReceive);
    Serial.print(" | send:");
    Serial.println(masterSend);
    servoOffGame();
    if (masteReceive == GAME_START)
    { // if ani button on slave activated
      displayCountdown();
      gameOn = 1;
      servoOffGame();
      gameStartTime = millis();
      randomRgb = random() % 3;
      player[0].points = 0;
      player[1].points = 0;
      playerRouond = 0;
      lastPlayerRound = 0;
      correctRgb = 0;
    }
    else
    {
      masterSend = GAME_STOP;
    }
  }
  else
  {
    // Game ON logic
    servoOnGame(GAME_DURATION);
    displayInGame(player[0], player[1], 1 + playerRouond);
    if (playerRouond != lastPlayerRound)
    { // set the acctive player
      randomRgb = random() % 3;
      lastPlayerRound = playerRouond;
      delayMillis(3000);
    }
    if (playerRouond % 2 == 0)
    { // chooses a random RGB color for the player to react
      switch (randomRgb)
      {
      case 0:
        correctRgb = SPI_RGB1_R;
        break;
      case 1:
        correctRgb = SPI_RGB1_G;
        break;
      case 2:
        correctRgb = SPI_RGB1_B;
        break;
      default:
        break;
      }
    }
    else
    {
      switch (randomRgb)
      {
      case 0:
        correctRgb = SPI_RGB2_R;
        break;
      case 1:
        correctRgb = SPI_RGB2_G;
        break;
      case 2:
        correctRgb = SPI_RGB2_B;
        break;
      default:
        break;
      }
    }
    ////////////up : create round///////////////
    digitalWrite(SS, LOW); // Starts communication with Slave connected to master
    masterSend = correctRgb;
    masteReceive = SPI.transfer(masterSend); // Send the mastersend value to slave also receives value from slave
    digitalWrite(SS, HIGH);
    ////////// down verify the results that the slave provided///////////
    // for debug:
    Serial.print("ON --- receive:");
    Serial.print(masteReceive);
    Serial.print(" | send:");
    Serial.println(masterSend);
    currentRoundTime = millis();

    if (millis() - lastRoundTime >= ROND_DURATION)
    { // if player didn`t answer intime
      playerRouond = (playerRouond + 1) % 2;
      lastRoundTime = currentRoundTime;
    }
    else if (masteReceive == correctRgb)
    { // if player didn`t answer coorect
      int reactionTime = currentRoundTime - lastRoundTime;
      player[playerRouond].points = player[playerRouond].points + (ROND_DURATION - reactionTime) / 100;
      playerRouond = (playerRouond + 1) % 2;
      lastRoundTime = currentRoundTime;
    }
    if ((currentRoundTime - gameStartTime) / SECOND >= GAME_DURATION)
    { // if game went on long enough
      displayInGame(player[0], player[1], 3);
      delayMillis(10 * SECOND);
      masterSend = GAME_STOP;
      gameOn = 0;
    }
  }
  // for debug:
  // displayCountdown();
  // servoOnGame(ROND_DURATION);
  /* Serial.print(millis());
   delay(500);
   Serial.print(" | ");
   Serial.println(millis());
   delay(500);*/
  /*byte masterSend,masteReceive;
   */
  // delayMillis(100);
}