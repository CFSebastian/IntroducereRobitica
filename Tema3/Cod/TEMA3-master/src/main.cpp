#include <Arduino.h>
#include <SPI.h>
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

#define SECOND 1000         // second in milliseconds
#define SECONDS_10 10       // seconds
#define GAME_DURATION 100   // seconds
#define ROND_DURATION 10000 // milliseconds
#define END_ROTATION 180
#define START_ROTATION 0
#define ROTATION_DISTANCE 45

struct Players
{
  unsigned int points;
  String name;
} player[2];

bool gameOn = 0;
int playerRound = 0;
int lastPlayerRound = 0;
int correctRgb = 0;
unsigned long lastRoundTime = 0;
unsigned long currentRoundTime = 0;
unsigned long gameStartTime = 0;
int randomRgb;
byte masterSend = IGNORE, masterReceive;

// delay but with millis
void delayMillis(unsigned long milliseconds)
{
  unsigned long currentTimeDelay = millis();
  unsigned long goalTimeDelay = currentTimeDelay + milliseconds;

  while (millis() <= goalTimeDelay)
    ;
}

////////////////////////////////////// DISPLAY LCD ///////////////////////////

#define RS 9
#define EN 8
#define D4 5
#define D5 4
#define D6 3
#define D7 2

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

void displayInGame(Players p1, Players p2, int stateGame)
{
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
    lcd.write(byte(1)); // Display arrow symbol
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
    lcd.write(byte(1)); // Display arrow symbol
    lcd.print(p2.name);
    lcd.print(":");
    lcd.print(p2.points);
    break;
  case 3:
    if (p1.points > p2.points)
    {
      lcd.write(byte(0)); // Trophy symbol
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
      lcd.write(byte(0)); // Trophy symbol
      lcd.print(p2.name);
      lcd.print(":");
      lcd.print(p2.points);
    }
    else if (p1.points == p2.points)
    {
      lcd.write(byte(0)); // Trophy symbol
      lcd.print(p1.name);
      lcd.print(":");
      lcd.print(p1.points);
      lcd.setCursor(0, 1);
      lcd.write(byte(0)); // Trophy symbol
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
  lcd.begin(16, 2);
  lcd.createChar(0, trophy); // Trophy symbol
  lcd.createChar(1, arrow);  // Arrow symbol
}

void displayCountdown()
{
  lcd.clear();
  for (int i = 3; i > 0; i--)
  {
    unsigned long currentTimeLcd = millis(); // Fetch current time for countdown
    unsigned long goalTime = currentTimeLcd + SECOND;
    while (millis() <= goalTime)
      ;
    lcd.setCursor(0, 0);
    lcd.print(i);
    Serial.println(i);
  }
  lcd.clear();
}

Servo myservo;
unsigned long currentServoTime = 0;
unsigned long lastServoTime = 0;
unsigned int pos = 0;

void servoSetup()
{
  myservo.attach(6); // attaches the servo on pin 6 to the servo object
}

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
  if (!gameOn)
  {
    displayInGame(player[0], player[1], 0);
    digitalWrite(SS, LOW);
    masterReceive = SPI.transfer(masterSend);
    digitalWrite(SS, HIGH);
    Serial.print("OFF --- receive:");
    Serial.print(masterReceive);
    Serial.print(" | send:");
    Serial.println(masterSend);
    servoOffGame();
    if (masterReceive == GAME_START)
    {
      displayCountdown();
      gameOn = 1; // Start the game
      servoOffGame();
      gameStartTime = millis();
      randomRgb = random() % 3;
      player[0].points = 0;
      player[1].points = 0;
      playerRound = 0;
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
    servoOnGame(GAME_DURATION);
    displayInGame(player[0], player[1], 1 + playerRound);
    if (playerRound != lastPlayerRound)
    {
      randomRgb = random() % 3;
      lastPlayerRound = playerRound;
      delayMillis(3000);
    }
    if (playerRound % 2 == 0)
    {
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
    digitalWrite(SS, LOW);
    masterSend = correctRgb;
    masterReceive = SPI.transfer(masterSend);
    digitalWrite(SS, HIGH);
    Serial.print("ON --- receive:");
    Serial.print(masterReceive);
    Serial.print(" | send:");
    Serial.println(masterSend);
    currentRoundTime = millis();

    if (millis() - lastRoundTime >= ROND_DURATION)
    {
      playerRound = (playerRound + 1) % 2;
      lastRoundTime = currentRoundTime;
    }
    else if (masterReceive == correctRgb)
    {
      int reactionTime = currentRoundTime - lastRoundTime;
      player[playerRound].points = player[playerRound].points + (ROND_DURATION - reactionTime) / 100;
      playerRound = (playerRound + 1) % 2;
      lastRoundTime = currentRoundTime;
    }
    if ((currentRoundTime - gameStartTime) / SECOND >= GAME_DURATION)
    {
      displayInGame(player[0], player[1], 3); // Show final results
      delayMillis(SECONDS_10 * SECOND);
      masterSend = GAME_STOP;
      gameOn = 0;
    }
  }
}
