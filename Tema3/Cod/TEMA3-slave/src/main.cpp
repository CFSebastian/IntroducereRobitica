
#include <Arduino.h>
#include <SPI.h>

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
// RGB LEDS
#define LED_RGB1_R 5
#define LED_RGB1_G 4
#define LED_RGB1_B 3
#define LED_RGB2_R 8
#define LED_RGB2_G 7
#define LED_RGB2_B 6
// Simple Leds
//dreapta
#define LED_P1_R A1 //b
#define LED_P1_G A2 //g
#define LED_P1_B A3 //r
//stanga
#define LED_P2_R 2 //b
#define LED_P2_G A4 //g
#define LED_P2_B A5 //r
// Buttons
#define BUTTONS_PIN A0
//max values(+40 for inteferences from the leds)
#define BTN_V_P1_R 2000
#define BTN_V_P1_G 500
#define BTN_V_P1_B 390
#define BTN_V_P2_R 130
#define BTN_V_P2_G 195
#define BTN_V_P2_B 275

volatile boolean received;
volatile byte slaveReceived,slaveSend =IGNORE;
int x;
bool gameOn=0;
int correctLed = 0;
int collorGuessed = IGNORE;
int rgbLed;
int parasitVoltage = 50; //can take values betwin 0 - ~36
/*
NOTA: Arduino lpune curent pe 0 si 1 orice ar fi
*/

int buttonState;
int lastButtonState = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void delayMillis(unsigned long milliseconds)
{
  unsigned long currentTimeDelay = millis();
  unsigned long goalTimeDelay = currentTimeDelay + milliseconds;

  while (millis() <= goalTimeDelay);
}
/*
Set the RGB leds color or a group of leds color
ledGroup: 0-RGB player 1; 1-RGB player 2; 2-leds player 1; 3-leds player 2
*/
void setLedColor(int ledGroup, bool red, bool green, bool blue)
{
  /*if(ledGroup) {
    digitalWrite(LED_RGB1_R, red);
    digitalWrite(LED_RGB1_G, green);
    digitalWrite(LED_RGB1_B, blue);
  }
  else {
    digitalWrite(LED_RGB2_R, red);
    digitalWrite(LED_RGB2_G, green);
    digitalWrite(LED_RGB2_B, blue);
  }*/
  switch (ledGroup)
  {
  case 0:
    digitalWrite(LED_RGB1_R, red);
    digitalWrite(LED_RGB1_G, green);
    digitalWrite(LED_RGB1_B, blue);
    break;
  case 1:
    digitalWrite(LED_RGB2_R, red);
    digitalWrite(LED_RGB2_G, green);
    digitalWrite(LED_RGB2_B, blue);
    break;
  case 2:
    digitalWrite(LED_P1_R, red);
    digitalWrite(LED_P1_G, green);
    digitalWrite(LED_P1_B, blue);
    break;
  case 3:
    digitalWrite(LED_P2_R, red);
    digitalWrite(LED_P2_G, green);
    digitalWrite(LED_P2_B, blue);
    break;
  
  default://error
    digitalWrite(LED_RGB1_R, 1);
    digitalWrite(LED_RGB1_G, 1);
    digitalWrite(LED_RGB1_B, 1);
    digitalWrite(LED_RGB2_R, 1);
    digitalWrite(LED_RGB2_G, 1);
    digitalWrite(LED_RGB2_B, 1);
    break;
  }
}
int debounce(uint8_t  btn){ // daca e necesar
  int btnValue;
   int reading = analogRead(btn);
    if (reading != lastButtonState) {// compara tensiunea aici
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        if (reading != buttonState) // compara tensiunea aici
        {
            buttonState = reading;
            if (buttonState != 0)
            {
               btnValue=buttonState;
            }
        }
    }
    lastButtonState = reading;
    return btnValue;
}

ISR (SPI_STC_vect)
{
  slaveReceived = SPDR;   
  SPDR = slaveSend;               
  received = true;                       
}
void setup() {

  Serial.begin(BAUD_RATE);

  randomSeed(analogRead(0));

  pinMode(MISO,OUTPUT); 
  SPCR |= _BV(SPE); 
  received = false;
  SPI.attachInterrupt();    

  pinMode(LED_RGB1_R,OUTPUT);
  pinMode(LED_RGB1_G,OUTPUT);
  pinMode(LED_RGB1_B,OUTPUT);
  pinMode(LED_RGB2_R,OUTPUT);
  pinMode(LED_RGB2_G,OUTPUT);
  pinMode(LED_RGB2_B,OUTPUT);

  pinMode(LED_P1_R,OUTPUT);
  pinMode(LED_P1_G,OUTPUT);
  pinMode(LED_P1_B,OUTPUT);
  pinMode(LED_P2_R,OUTPUT);
  pinMode(LED_P2_G,OUTPUT);
  pinMode(LED_P2_B,OUTPUT);
  
   pinMode(A0, INPUT); // Butoane
  
}

void loop() {
  long int lastMillis=millis();
  int btnValue=analogRead(BUTTONS_PIN);
  if(!gameOn) {
    Serial.print("OFF --- receive:");
    Serial.print(slaveReceived);
    Serial.print(" | send:");
    Serial.println(slaveSend);
    setLedColor(2,0,0,0);
    setLedColor(0,0,0,0);
    setLedColor(3,0,0,0);
    setLedColor(1,0,0,0);
    if(btnValue > parasitVoltage) {
      gameOn = 1;
      slaveSend = GAME_START;
      delayMillis(5000);
    }
  }
  else {
    Serial.print("ON ---  receive:");
    Serial.print(slaveReceived);
    Serial.print(" | send:");
    Serial.println(slaveSend);
    if(slaveReceived >= SPI_RGB1_R && slaveReceived <= SPI_RGB1_B) {
      setLedColor(3,0,0,0);
      setLedColor(1,0,0,0);
    } else if(slaveReceived >= SPI_RGB2_R && slaveReceived <= SPI_RGB2_B) {
      setLedColor(2,0,0,0);
      setLedColor(0,0,0,0);
    }
    switch (slaveReceived)
    {
    case SPI_RGB1_R:
      setLedColor(0,1,0,0);
      correctLed = SPI_BTN1_R;
      break;
    case SPI_RGB1_G:
      setLedColor(0,0,1,0);
      correctLed = SPI_RGB1_G;
      break;
    case SPI_RGB1_B:
      setLedColor(0,0,0,1);
      correctLed = SPI_RGB1_B;
      break;
    case SPI_RGB2_R:
      setLedColor(1,1,0,0);
      correctLed = SPI_RGB2_R;
      break;
    case SPI_RGB2_G:
      setLedColor(1,0,1,0);
      correctLed = SPI_RGB2_G;
      break;
    case SPI_RGB2_B:
      setLedColor(1,0,0,1);
      correctLed = SPI_RGB2_B;
      break;
    case GAME_STOP:
      gameOn = 0;
      break;
    case IGNORE:
      break;
    default:
      Serial.print(" /_> ERROR/GARBAGE Unknown value send by master");
      setLedColor(0,1,1,1);
      setLedColor(1,1,1,1);
      break;
    }
    if(btnValue > parasitVoltage) {
      if(btnValue < BTN_V_P2_R) {
        setLedColor(3,1,0,0);
        collorGuessed = SPI_RGB2_R;
      }
      else if(btnValue < BTN_V_P2_G) {
        setLedColor(3,0,1,0);
        collorGuessed = SPI_RGB2_G;
      }
      else if(btnValue < BTN_V_P2_B) {
        setLedColor(3,0,0,1);
        collorGuessed = SPI_RGB2_B;
      }
      else if(btnValue < BTN_V_P1_B) {
        setLedColor(2,0,0,1);
        collorGuessed = SPI_RGB1_B;
      }
      else if(btnValue < BTN_V_P1_G) {
        setLedColor(2,0,1,0);
        collorGuessed = SPI_RGB1_G;
      }
      else if(btnValue < BTN_V_P1_R) {
        setLedColor(2,1,0,0);
        collorGuessed = SPI_RGB1_R;
      }
    }
    slaveSend = IGNORE;
    Serial.print("================================");
    Serial.println(collorGuessed);
    if(collorGuessed == correctLed) {
      slaveSend=collorGuessed;// se pastreaza in memorie
      collorGuessed = IGNORE;
    }
    

  }
  //delayMillis(500);

}
