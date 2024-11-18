#include <Arduino.h>
#include <SPI.h>

// Include bibliotecile necesare:
#include <LiquidCrystal.h>
#include <Servo.h>

#define BAUD_RATE 9600 // Rata de baud pentru comunicare serială

// Definirea pinilor pentru LED-uri RGB controlate prin SPI
#define SPI_RGB1_R 1
#define SPI_RGB1_G 2
#define SPI_RGB1_B 3
#define SPI_RGB2_R 4
#define SPI_RGB2_G 5
#define SPI_RGB2_B 6

// Definirea pinilor pentru butoane controlate prin SPI
#define SPI_BTN1_R 11
#define SPI_BTN1_G 12
#define SPI_BTN1_B 13
#define SPI_BTN2_R 14
#define SPI_BTN2_G 15
#define SPI_BTN2_B 16

// Comenzi pentru joc
#define GAME_START 21
#define GAME_STOP 20
#define IGNORE 23

// Definirea duratelor de timp în milisecunde
#define SECOND 1000 // O secundă în milisecunde
#define GAME_DURATION 80 // Durata jocului în secunde
#define ROND_DURATION 5000 // Durata unei runde în milisecunde
#define END_ROTATION 180
#define START_ROTATION 0
#define ROTATION_DISTANCE 45

// Structură pentru jucători care conține numele și punctele
struct Players {
  unsigned int points; // Puncte jucător
  String name; // Nume jucător
} player[2]; // Declarație pentru doi jucători

bool gameOn = 0; // Starea jocului (pornit/oprit)
int playerRouond = 0; // Jucătorul curent în rundă
int lastPlayerRound = 0; // Ultimul jucător din rundă
int correctRgb = 0; // Valoarea RGB corectă pentru rundă
unsigned long lastRoundTime = 0; // Timpul de start al ultimei runde
unsigned long currentRoundTime = 0; // Timpul curent
unsigned long gameStartTime = 0; // Timpul de start al jocului
int randomRgb; // Culoare RGB aleasă aleatoriu pentru rundă
byte masterSend = IGNORE, masteReceive; // Variabile pentru comunicare SPI

// Funcție pentru o întârziere bazată pe millis()
void delayMillis(unsigned long milliseconds) {
  unsigned long currentTimeDelay = millis();
  unsigned long goalTimeDelay = currentTimeDelay + milliseconds;

  while (millis() <= goalTimeDelay);
}

////////////////////////////////////// AFIȘAJ LCD ///////////////////////////

// Pini pentru LCD
#define RS 9
#define EN 8
#define D4 5
#define D5 4
#define D6 3
#define D7 2

// Definire caractere personalizate pentru afișajul LCD (trofeu și săgeată)
byte trophy[8] = {
  0b00100,
  0b11111,
  0b10101,
  0b11111,
  0b01110,
  0b00100,
  0b00100,
  0b01110
};
byte arrow[8] = {
  0b00000,
  0b00100,
  0b00110,
  0b11111,
  0b00110,
  0b00100,
  0b00000,
  0b00000
};

// Inițializare LCD
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

// Variabilă pentru timpul de actualizare a afișajului
unsigned long lastTime = 0;

// Funcție pentru afișarea informațiilor jocului pe LCD
void displayInGame(Players p1, Players p2, int stateGame) {
  Serial.print(p1.name);
  Serial.print("----");
  Serial.print(p1.points);
  Serial.print("----");
  Serial.print(p2.name);
  Serial.print("----");
  Serial.println(p2.points);
  lcd.clear();
  lcd.setCursor(0, 0);
  switch (stateGame) {
  case 0:
    lcd.print("Joc de Reflex");
    break;
  case 1:
    lcd.write(byte(1)); // Afișează săgeata
    lcd.print(p1.name);
    lcd.print(":");
    lcd.print(p1.points);
    lcd.setCursor(0, 1);
    lcd.print(p2.name);
    lcd.print(": ");
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
    if (p1.points > p2.points) {
      lcd.write(byte(0)); // Afișează trofeul
      lcd.print(p1.name);
      lcd.print(":");
      lcd.print(p1.points);
      lcd.setCursor(0, 1);
      lcd.print(p2.name);
      lcd.print(":");
      lcd.print(p2.points);
    } else if (p1.points < p2.points) {
      lcd.print(p1.name);
      lcd.print(p1.points);
      lcd.print(":");
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
      lcd.print(p2.name);
      lcd.print(":");
      lcd.print(p2.points);
    } else if (p1.points == p2.points) {
      lcd.write(byte(0));
      lcd.print(p1.name);
      lcd.print(":");
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

// Funcție pentru inițializarea LCD-ului
void initiateLCD() {
  lcd.begin(16, 2);
  lcd.createChar(0, trophy);
  lcd.createChar(1, arrow);
}

// Funcție pentru afișarea unei numărători inverse
void displayCountdown() {
  lcd.clear();
  for (int i = 3; i > 0; i--) {
    unsigned long currentTimeLcd = millis();
    unsigned long goalTime = currentTimeLcd + SECOND;
    while (millis() <= goalTime);
    lcd.setCursor(0, 0);
    lcd.print(i);
    Serial.println(i);
  }
  lcd.clear();
}

//////////////////////////////// Servomotor ////////////////////////////////
Servo myservo; // Obiect servo pentru controlul unui servomotor
unsigned long currentServoTime = 0;
unsigned long lastServoTime = 0;
unsigned int pos = 0; // Variabilă pentru stocarea poziției servomotorului

// Funcție pentru inițializarea servomotorului
void servoSetup() {
  myservo.attach(6);
}

// Funcție pentru controlul servomotorului în timpul jocului
void servoOnGame(int gameDurationServo) {
  unsigned long goalTime = currentServoTime + gameDurationServo * SECOND * ROTATION_DISTANCE / END_ROTATION;
  if (millis() >= goalTime) {
    currentServoTime = millis();
    pos += ROTATION_DISTANCE;
    if (pos > END_ROTATION) {
      pos = START_ROTATION;
    }
    myservo.write(pos);
    Serial.println(pos);
  }
}

// Funcție pentru resetarea servomotorului la poziția inițială
void servoOffGame() {
  pos = 0;
  myservo.write(0);
}

void setup() {
  Serial.begin(BAUD_RATE);
  randomSeed(analogRead(0)); // Inițializare pentru generarea numerelor aleatorii
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.setDataMode(SPI_MODE0);
  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);
  myservo.attach(6);
  lcd.begin(16, 2);
  lcd.createChar(0, trophy);
  lcd.createChar(1, arrow);

  player[0].name = "P1"; // Inițializare nume jucători
  player[0].points = 0;
  player[1].name = "P2";
  player[1].points = 0;

  randomRgb = random() % 3; // Generare culoare aleatorie
  myservo.write(0); // Setare poziție de start pentru servomotor
  delay(1000);
  gameStartTime = millis(); // Înregistrare timp de start al jocului
}

void loop() {
  if (!gameOn) {
    displayInGame(player[0], player[1], 0); // Afișare stare inițială pe LCD
    digitalWrite(SS, LOW);
    masterSend = IGNORE;
    masteReceive = SPI.transfer(masterSend);
    digitalWrite(SS, HIGH);
    if (masteReceive == GAME_START) {
      player[0].points = 0;
      player[1].points = 0;
      gameStartTime = millis();
      gameOn = 1;
    }
  } else {
    currentRoundTime = millis();
    if (currentRoundTime - lastRoundTime >= ROND_DURATION) {
      lastPlayerRound = playerRouond;
      playerRouond = (playerRouond + 1) % 2;
      randomRgb = random() % 3;
      lastRoundTime = millis();
    }
    servoOnGame(GAME_DURATION);
    currentRoundTime = millis();
    if (currentRoundTime - gameStartTime >= GAME_DURATION * SECOND) {
      gameOn = 0;
      servoOffGame();
    }
  }
}
