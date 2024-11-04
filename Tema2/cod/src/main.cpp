#include <Arduino.h>
#include <avr/interrupt.h>

// pini
#define BTN_START 2
#define BTN_DIFFICULTY 3
#define LED_R 6
#define LED_G 4
#define LED_B 5

// Definește pinii și variabilele
const int redPin = 6;
const int greenPin = 4;
const int bluePin = 5;
const int startButtonPin = 2;
const int difficultyButtonPin = 3;

volatile bool isGameActive = false;
volatile int difficulty = 0; // 0: Easy, 1: Medium, 2: Hard
unsigned long lastDebounceTime = 0;
const long debounceDelay = 50;

const unsigned long roundTime = 30000; // 30 secunde
unsigned long startTime;
int correctWordsCount = 0;
String words[] = {"car", "bike", "plane", "boat", "train"};
String currentWord;
bool wordCorrect = true;

unsigned long wordInterval[] = {2000, 1500, 1000}; // Easy, Medium, Hard
unsigned long lastWordTime = 0;

void setup() {
    Serial.begin(9600);

    pinMode(LED_R,OUTPUT);
    pinMode(LED_B,OUTPUT);
    pinMode(LED_G,OUTPUT);
    setLedColor(1,1,1);
    
    pinMode(BTN_START,INPUT_PULLUP);
    pinMode(BTN_DIFFICULTY,INPUT_PULLUP);

    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    setLEDColor(255, 255, 255); // Alb pentru modul de repaus

    pinMode(startButtonPin, INPUT_PULLUP);
    pinMode(difficultyButtonPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(startButtonPin), startStopButtonPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(difficultyButtonPin), difficultyButtonPress, FALLING);

    randomSeed(analogRead(0)); // Inițializare random pentru alegerea cuvintelor
}

void loop() {
    if (isGameActive) {
        if (millis() - startTime >= roundTime) {
            endGame();
            return;
        }

        if (millis() - lastWordTime >= wordInterval[difficulty] || wordCorrect) {
            showNextWord();
            wordCorrect = false;
        }

        if (Serial.available()) {
            String inputWord = Serial.readStringUntil('\n'); // Citire input din Serial
            inputWord.trim();  // Elimină spațiile albe înainte de comparație
            if (inputWord == currentWord) {  // Compară cuvântul introdus cu cel curent
                wordCorrect = true;
                correctWordsCount++;
                setLEDColor(0, 255, 0); // Verde pentru corect
                setLedColor(0,1,0);
            } else {
                setLEDColor(255, 0, 0); // Roșu pentru greșit
                setLedColor(1,0,0);
            }
        }
    }
}


void startStopButtonPress() {
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (!isGameActive) {
            startGame();
        } else {
            endGame();
        }
        lastDebounceTime = millis();
    }
}

void difficultyButtonPress() {
    if (!isGameActive && (millis() - lastDebounceTime) > debounceDelay) {
        difficulty = difficulty % 3;
        Serial.print("Mode: ");
        switch (difficulty) {
            case 0:
              Serial.println("Easy mode on!");
              break;
            case 1: 
              Serial.println("Medium mode on!"); 
              break;
            case 2: 
              Serial.println("Hard mode on!"); 
              break;
            default:
              Serial.println("?! mode on!");
              break;
        }
        lastDebounceTime = millis();
    }
}

void startGame() {
    isGameActive = true;
    correctWordsCount = 0;
    startTime = millis();

    for (int i = 3; i > 0; i--) {
        Serial.println(i);
        setLEDColor(255, 255, 255);
        setLedColor(1,1,1);
        delay(1000);//nu merge
    }

    showNextWord();
}

void endGame() {
    isGameActive = false;
    setLEDColor(255, 255, 255); // Revină la alb în repaus
    setLedColor(1,1,1);
    Serial.print("Runda s-a incheiat. Cuvinte corecte: ");
    Serial.println(correctWordsCount);
}

void showNextWord() {
    int index = random(0, sizeof(words) / sizeof(words[0]));
    currentWord = words[index];
    Serial.print("Scrie: ");
    Serial.println(currentWord);
    int a=sizeof(words);
    int b=sizeof(words[0]);
    Serial.print("----");
    Serial.print(a);
    Serial.print("--0--");
    Serial.print(b);
    lastWordTime = millis();
    setLEDColor(0, 255, 0); // Verde pentru start cuvânt nou
    setLedColor(0,1,0);
}

void setLEDColor(int red, int green, int blue) {
    analogWrite(redPin, red);
    analogWrite(greenPin, green);
    analogWrite(bluePin, blue);
}
void setLedColor(bool red, bool green, bool blue) {
    digitalWrite(LED_R, red);
    digitalWrite(LED_G, green);
    digitalWrite(LED_B, blue);
}

