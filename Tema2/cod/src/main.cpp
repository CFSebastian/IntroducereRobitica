#include <Arduino.h>
#include <string.h>

#define RED_LED 6
#define GREEN_LED 5
#define BLUE_LED 4
#define START_BUTTON 3
#define DIFF_BUTTON 2
#define DICTIONARY_SIZE 15
// put function declarations here:
bool gameStarted = false;
bool btnStart=1;
bool btnStop=1;
int interval_cuvinte=10000;
bool roundStarted = false;
int currentMillis = millis();
int currentWord=0;
int previousMillis = currentMillis;
int previousWordMillis = currentMillis;
bool mistake;
int difficulty = 1; //1-easy, 2-medium, 3-hard
bool diffChanged = false;
unsigned int literaCurenta=0;
int score = 0;
bool selectDifficulty = false;
const char* dictionar[DICTIONARY_SIZE] = {
        "abandon", "abilitate", "abstract", "accesibil", "activitate",
        "adaptabil", "ambiguu", "analogie", "antagonist", "anxietate",
        "baza", "beneficiu", "biblioteca", "binefacere", "blocaj",
    };
void changeStartButton() {
  btnStart = !btnStart;
}
void changeDifficulty(){
  diffChanged = true;
  difficulty++;
  if(difficulty>3)
    difficulty = 1;
}
void startGame(){
  if(!gameStarted){
    gameStarted = true;
    int currentMillis = millis();
    int previousMillis = currentMillis;
    int previousMillis2 = currentMillis;
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);
    char nr='2';
    Serial.println("3\n");
    while(currentMillis-previousMillis < 3000){
      currentMillis = millis();
      if(currentMillis - previousMillis2 >= 1000 && nr!='0'){
        Serial.print(nr);
        Serial.println("\n");
        nr--;
        previousMillis2 = currentMillis; 
        
      }
     
        digitalWrite(RED_LED, !digitalRead(RED_LED));
        digitalWrite(GREEN_LED, !digitalRead(GREEN_LED));
        digitalWrite(BLUE_LED, !digitalRead(BLUE_LED));
        delay(500);
    
  }

  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  randomSeed(millis());
  // Amestecarea array-ului folosind o metodă simplă
  for (int i = DICTIONARY_SIZE - 1; i > 0; i--) {
        // Generăm un index aleator
        int j = random(i + 1); // Folosim random() pentru a obține un număr aleator
        // Schimbăm elementele
        const char* temp = dictionar[i];
        dictionar[i] = dictionar[j];
        dictionar[j] = temp;
    }

    currentWord = 0;
    score=0;
    
  }
}
void round(){
  if(roundStarted){
    if(currentMillis-previousMillis < 30000){
      currentMillis = millis();
      bool skipWord = false;
       if (Serial.available() > 0) {
                char litera = Serial.read(); // Read the next character from serial
                // Compare the letter with the current word
                if (litera == 8) { // ASCII code for backspace
                    mistake = false;
                    Serial.println("Backspace pressed");
                }
                else
                  if (litera == dictionar[currentWord-1][literaCurenta] && !mistake) {
                      Serial.println("Correct letter: " + String(litera));
                      literaCurenta++; // Move to the next letter

                      // Check if the entire word has been guessed
                      if (literaCurenta == strlen(dictionar[currentWord-1])) {
                          Serial.println("Word complete: " + String(dictionar[currentWord-1]));
                          score++;
                          skipWord = true;
                          literaCurenta = 0; // Reset letter index
                          currentWord++; // Move to the next word
                          if (currentWord >= DICTIONARY_SIZE) {
                              currentWord = 0; // Loop back to the first word
                          }
                          // Update previous word millis for the next word
                          previousWordMillis = currentMillis; 
                      }
                  } 
                  else {
                      Serial.println("Wrong letter: " + String(litera) + " Expected: " + String(dictionar[currentWord-1][literaCurenta]));
                      mistake = true;
                    }
                  }
      if(currentWord==0) {
        Serial.println(dictionar[currentWord]);
        currentWord++;
        previousWordMillis = currentMillis;
        }
      else if(currentMillis-previousWordMillis >= interval_cuvinte || skipWord) {
        Serial.println(dictionar[currentWord]);
        previousWordMillis = currentMillis;
        currentWord++;
        literaCurenta = 0;
        mistake = false;
    }
    }
    else {
      roundStarted = false;
      mistake = false;
      Serial.println("Round ended");
      Serial.println("Score: " + String(score));
      previousWordMillis = currentMillis;
    }
  }
}
void setup() {
  // put your setup code here, to run once:
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(DIFF_BUTTON, INPUT_PULLUP);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(START_BUTTON),changeStartButton,FALLING);
  attachInterrupt(digitalPinToInterrupt(DIFF_BUTTON),changeDifficulty,FALLING);
}

void loop() {
  currentMillis = millis();
  if(gameStarted){
    if(btnStart == LOW){
      Serial.println("Stop button pressed");
      btnStart = 1;
      gameStarted = false;
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(BLUE_LED, HIGH);
      delay(500);//de rezolvat mai tarziu
     
    }
   
    if(roundStarted)
      round();
    if(mistake) { 
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(BLUE_LED, LOW);
      }
      else {
        mistake = false;
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, LOW);
      }
  }
  else {
    if(!selectDifficulty) {
     Serial.println("Select difficulty: ");
      selectDifficulty = true;
    }
    
    
    if(diffChanged) {
      delay(200);
      diffChanged = false;
      switch (difficulty) {
        case 1:
          interval_cuvinte = 10000;
          Serial.println("Easy");
          break;
        case 2:
          interval_cuvinte = 5000;
          Serial.println("Medium");
          break;
        case 3:
          interval_cuvinte = 3000;
          Serial.println("Hard");
          break;
      }
    }
    if(btnStart == LOW&& !gameStarted) {
      Serial.println("Start button pressed");
      btnStart = 1;
      previousMillis = currentMillis;
      previousWordMillis = currentMillis;
      roundStarted = true;
      startGame();
      delay(500);
    }
    
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);
  }
}