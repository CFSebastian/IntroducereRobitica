#include <Arduino.h>
#include <string.h>

// define the pins
#define BTN_START 3
#define BTN_DIFFICULTY 2
#define LED_R 6
#define LED_G 4
#define LED_B 5

// debounce variables for the dificulty button
int buttonState;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// game variables
bool roundStart = false;
volatile bool btnStart = 1;
int wordInterval = 10000;
volatile unsigned long currentMillis;
int currentWord;
volatile unsigned long previousMillis;
volatile unsigned long previousWordMillis;
bool mistake;
int difficulty = 0; // 0-easy, 1-medium, 2-hard
bool difficultyChanged = true;
unsigned int currentChar = 0;
int score = 0;
bool firstWord;

const int WordNumber = 15;
const char *dictionar[WordNumber] = {
    "ember", "drift", "harbor", "willow", "cascade", "summit", "glimmer",
    "meadow", "echo", "fern", "breeze", "prism", "velvet", "quiver", "lantern"};

// set the RGB LED colors
void setLedColor(bool red, bool green, bool blue)
{
    digitalWrite(LED_R, red);
    digitalWrite(LED_G, green);
    digitalWrite(LED_B, blue);
}

// set the variables for the beginigg of the round and play the animation
void begineRound()
{
    roundStart = true;
    setLedColor(1, 1, 1);
    for (int i = 3; i > 0; i--)
    {
        Serial.println(i);
        setLedColor(0, 0, 0);
        delay(500);
        setLedColor(1, 1, 1);
        delay(500);
    }
    int currentMillis = millis();
    int previousMillis = currentMillis;
    setLedColor(0, 1, 0);
    firstWord = true;
    score = 0;
}
void round()
{
    currentMillis = millis();
    // end round if the time is over 30 seconds
    if (currentMillis - previousMillis >= 30000)
    {
        roundStart = false;
        mistake = false;
        Serial.println("Round ended");
        Serial.print("Score: ");
        Serial.println(score);
        previousWordMillis = currentMillis;
        return;
    }
    bool skipWord = false;
    // determinate if the letter that is read from the serial is the expected input
    if (Serial.available())
    {
        char letter = Serial.read();
        if (letter == 8)
        { // 8 is ASCII code for backspace
            mistake = false;
            Serial.println("Backspace");
        }
        else if (letter == dictionar[currentWord][currentChar] && !mistake)
        {
            Serial.println(letter);
            currentChar++;
            // Verify complition if the word
            if (currentChar == strlen(dictionar[currentWord]))
            {
                Serial.print("+1 for: ");
                Serial.println(dictionar[currentWord]);
                score++;
                skipWord = true;
                currentChar = 0;
                currentWord = random(WordNumber);
                previousWordMillis = currentMillis;
            }
        }
        else
        {
            Serial.print(letter);
            Serial.print(" expected: ");
            Serial.println(dictionar[currentWord][currentChar]);
            mistake = true;
        }
    }
    // update if its the start of the round or a word was completed befor the time limit
    if (firstWord)
    {
        currentWord = random(WordNumber);
        Serial.println(dictionar[currentWord]);
        previousWordMillis = currentMillis;
        firstWord = false;
    }
    else if (currentMillis - previousWordMillis >= wordInterval || skipWord)
    {
        currentWord = random(WordNumber);
        Serial.println(dictionar[currentWord]);
        previousWordMillis = currentMillis;
        currentChar = 0;
        mistake = false;
    }

    if (mistake)
    {
        setLedColor(1, 0, 0);
    }
    else
    {
        mistake = false;
        setLedColor(0, 1, 0);
    }
}

// select the dificulty, ie. the speed at which words apear
void selectDificulty()
{
    int reading = digitalRead(BTN_DIFFICULTY);
    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        if (reading != buttonState)
        {
            buttonState = reading;
            if (buttonState == LOW)
            {
                difficultyChanged = true;
                difficulty++;
                difficulty = difficulty % 3;
            }
        }
    }

    lastButtonState = reading;

    if (difficultyChanged)
    {
        delay(200);
        difficultyChanged = false;
        switch (difficulty)
        {
        case 0:
            wordInterval = 10000;
            Serial.println("Easy mode on!");
            break;
        case 1:
            wordInterval = 5000;
            Serial.println("Medium mode on!");
            break;
        case 2:
            wordInterval = 3000;
            Serial.println("Hard mode on!");
            break;
        }
    }
}

// Handle interupt
void changeStartButton()
{
    btnStart = !btnStart;
}
void setup()
{

    pinMode(LED_R, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(LED_G, OUTPUT);
    setLedColor(1, 1, 1);

    pinMode(BTN_START, INPUT_PULLUP);
    pinMode(BTN_DIFFICULTY, INPUT_PULLUP);

    Serial.begin(9800); // set baud rate

    randomSeed(analogRead(0)); // Random initialization for choosing words

    attachInterrupt(digitalPinToInterrupt(BTN_START), changeStartButton, FALLING); // interupts for BTN_START (PD3/INT2)
}

void loop()
{
    currentMillis = millis();
    if (roundStart)
    {
        // on round logic
        if (btnStart == LOW)
        {
            Serial.println("Round OFF");
            btnStart = 1;
            roundStart = false;
            setLedColor(1, 1, 1);
        }
        round();
    }
    else
    {
        // off round logic
        selectDificulty();
        if (btnStart == LOW)
        {
            Serial.println("Round On");
            btnStart = 1;
            previousMillis = currentMillis;
            previousWordMillis = currentMillis;
            begineRound();
        }
        setLedColor(1, 1, 1);
    }
}