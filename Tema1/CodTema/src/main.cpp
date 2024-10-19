#include <Arduino.h>

//Definuri pinilor de la leduri si butoane
#define LED1 10
#define LED2 9
#define LED3 8
#define LED4 7
#define LEDR 6
#define LEDB 5
#define LEDG 4
#define BTN1 3
#define BTN2 2


unsigned int liber=0; // variabila ce reperzint starea statiei(daca este libiera sau nu)
unsigned int ledDelay=400;// variabila cu cate ms sa fie diley-urile din cod
unsigned long startTime=0;// retine cand incepe incarcarea
unsigned long sdBTN2=0;// retine inceputul apasat prelungite pentru btn de off
int lsBTN2=HIGH;// retine ultima stare a butonului pentru debouncing
int stBTN2;// retine starea actuala a butonului

/*
  Functie pentru a reda animatia de end atunci cand se termina incarcarea sau este intrerupta
*/
void animatieEnd() {
  for(int i=0;i<3;i++) {
    digitalWrite(LED1,HIGH);
    digitalWrite(LED2,HIGH);
    digitalWrite(LED3,HIGH);
    digitalWrite(LED4,HIGH);
    _delay_ms(ledDelay);
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,LOW);
    digitalWrite(LED4,LOW);
    _delay_ms(ledDelay);
  }
}

void setup() {
  //setare leduri
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  pinMode(LED4,OUTPUT);
  pinMode(LEDR,OUTPUT);
  pinMode(LEDB,OUTPUT);
  pinMode(LEDG,OUTPUT);
  //setaare butoane
  pinMode(BTN1,INPUT_PULLUP);
  pinMode(BTN2,INPUT_PULLUP);
}

void loop() {
  if(liber) {// cerificare daca statie este folosita
    digitalWrite(LEDG,LOW);
    digitalWrite(LEDR,HIGH);
    digitalWrite(LEDB,LOW);
    unsigned int curentTime = millis();// citirea timpului curetn
    unsigned int timePast=curentTime-startTime;// calcularea duratei de timp de la inceperea incarcarii

    // if si else pentru a verifica cat la suta e incarcata bateria (25%, 50%, 75%, 100%)
    if(12000 < timePast) {
      animatieEnd();
      liber=0;
    } 
    else if(9000 < timePast) {
      digitalWrite(LED1,HIGH);
      digitalWrite(LED2,HIGH);
      digitalWrite(LED3,HIGH);
      digitalWrite(LED4,HIGH);
      _delay_ms(ledDelay);
      digitalWrite(LED4,LOW);
      _delay_ms(ledDelay);
    } 
    else if(6000 < timePast) {
      digitalWrite(LED1,HIGH);
      digitalWrite(LED2,HIGH);
      digitalWrite(LED3,HIGH);
      _delay_ms(ledDelay);
      digitalWrite(LED3,LOW);
      _delay_ms(ledDelay);
    } 
    else if(3000 < timePast) {
      digitalWrite(LED1,HIGH);
      digitalWrite(LED2,HIGH);
      _delay_ms(ledDelay);
      digitalWrite(LED2,LOW);
      _delay_ms(ledDelay);
    } 
    else {
      digitalWrite(LED1,HIGH);
      _delay_ms(ledDelay);
      digitalWrite(LED1,LOW);
      _delay_ms(ledDelay);
    }
    /*Cod de debouncing pentru buonul de off folosita in loop*/
    int cBTN2 = digitalRead(BTN2);// memoreaza starea curenta a butonului de off
    if(cBTN2 != lsBTN2){
      sdBTN2=millis();
    }
    if((millis()-sdBTN2) >=1000){
      if(cBTN2 != stBTN2) {
        stBTN2 = cBTN2;
        if(!stBTN2) {// se activeaza in butonul inca este apasat
          animatieEnd();
          liber=0;
        }
      }    
    }
    lsBTN2=cBTN2;// retine ultima stare pentru a fi folosita din nou in loop
    
  }
  else {
    digitalWrite(LEDG,HIGH);
    digitalWrite(LEDR,LOW);
    digitalWrite(LEDB,LOW);
    //verificam in caz ca se apasa pentru a incepe incarcarea memoreaza cand incepe si seteaza var. de stare
    if(!digitalRead(BTN1)) {
      startTime = millis();
      liber=1;
    }
  }
}

