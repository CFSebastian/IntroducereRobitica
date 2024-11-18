#include <Arduino.h>
#include <SPI.h> // Include librăria SPI pentru comunicare SPI

#define BAUD_RATE 9600 // Setează viteza de comunicare serială

// Definire pinii pentru LED-urile RGB (valori SPI de trimis/recepționat)
#define SPI_RGB1_R 1
#define SPI_RGB1_G 2
#define SPI_RGB1_B 3
#define SPI_RGB2_R 4
#define SPI_RGB2_G 5
#define SPI_RGB2_B 6

// Definire pinii pentru butoanele RGB (valori SPI)
#define SPI_BTN1_R 11
#define SPI_BTN1_G 12
#define SPI_BTN1_B 13
#define SPI_BTN2_R 14
#define SPI_BTN2_G 15
#define SPI_BTN2_B 16

#define GAME_START 21 // Comandă pentru pornirea jocului
#define GAME_STOP 20  // Comandă pentru oprirea jocului
#define IGNORE 23     // Valoare pentru ignorare

// Definire pinii pentru controlul LED-urilor RGB
#define LED_RGB1_R 5
#define LED_RGB1_G 4
#define LED_RGB1_B 3
#define LED_RGB2_R 8
#define LED_RGB2_G 7
#define LED_RGB2_B 6

// Definire pinii pentru LED-uri simple (dreapta și stânga)
#define LED_P1_R A1 // LED dreapta - roșu
#define LED_P1_G A2 // LED dreapta - verde
#define LED_P1_B A3 // LED dreapta - albastru
#define LED_P2_R 2  // LED stânga - roșu
#define LED_P2_G A4 // LED stânga - verde
#define LED_P2_B A5 // LED stânga - albastru

// Pin pentru citirea butoanelor analogice
#define BUTTONS_PIN A0

// Valorile maxime de tensiune (cu marja de +40 pentru interferențe LED-uri)
#define BTN_V_P1_R 2000
#define BTN_V_P1_G 500
#define BTN_V_P1_B 390
#define BTN_V_P2_R 130
#define BTN_V_P2_G 195
#define BTN_V_P2_B 275

volatile boolean received;       // Variabilă pentru a indica dacă un mesaj SPI a fost primit
volatile byte slaveReceived;     // Variabilă pentru valoarea primită de la master
volatile byte slaveSend = IGNORE; // Valoarea de trimis către master
int x;
bool gameOn = 0;                // Starea jocului (0 = oprit, 1 = pornit)
int correctLed = 0;             // LED-ul corect care trebuie ghicit
int collorGuessed = IGNORE;     // Culoarea ghicită
int rgbLed;
int parasitVoltage = 50;        // Valoare pentru filtrarea tensiunii (0 - ~36)

// Variabile pentru debouncing-ul butoanelor
int buttonState;
int lastButtonState = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Funcție pentru delay
void delayMillis(unsigned long milliseconds)
{
  unsigned long currentTimeDelay = millis();
  unsigned long goalTimeDelay = currentTimeDelay + milliseconds;
  while (millis() <= goalTimeDelay);
}

// Funcție pentru setarea culorii LED-urilor
void setLedColor(int ledGroup, bool red, bool green, bool blue)
{
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
  default: // Dacă apare o eroare
    digitalWrite(LED_RGB1_R, 1);
    digitalWrite(LED_RGB1_G, 1);
    digitalWrite(LED_RGB1_B, 1);
    digitalWrite(LED_RGB2_R, 1);
    digitalWrite(LED_RGB2_G, 1);
    digitalWrite(LED_RGB2_B, 1);
    break;
  }
}

// Funcție pentru debouncing-ul unui buton
int debounce(uint8_t btn)
{
  int btnValue;
  int reading = analogRead(btn);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    if (reading != buttonState)
    {
      buttonState = reading;
      if (buttonState != 0)
      {
        btnValue = buttonState;
      }
    }
  }
  lastButtonState = reading;
  return btnValue;
}

// ISR (Interrupt Service Routine) pentru recepționarea mesajelor prin SPI
ISR(SPI_STC_vect)
{
  slaveReceived = SPDR;   // Citește valoarea primită de la master
  SPDR = slaveSend;       // Trimite valoarea către master
  received = true;        // Setează flag-ul de recepție
}

// Funcția de setup
void setup() {
  Serial.begin(BAUD_RATE); // Inițializează comunicarea serială
  randomSeed(analogRead(0)); // Inițializează un seed aleator pentru random()

  pinMode(MISO, OUTPUT); // Pinul MISO trebuie să fie setat pe OUTPUT
  SPCR |= _BV(SPE);      // Activează modul SPI slave
  received = false;
  SPI.attachInterrupt(); // Activează întreruperea SPI

  // Setează pinii pentru LED-uri ca OUTPUT
  pinMode(LED_RGB1_R, OUTPUT);
  pinMode(LED_RGB1_G, OUTPUT);
  pinMode(LED_RGB1_B, OUTPUT);
  pinMode(LED_RGB2_R, OUTPUT);
  pinMode(LED_RGB2_G, OUTPUT);
  pinMode(LED_RGB2_B, OUTPUT);
  pinMode(LED_P1_R, OUTPUT);
  pinMode(LED_P1_G, OUTPUT);
  pinMode(LED_P1_B, OUTPUT);
  pinMode(LED_P2_R, OUTPUT);
  pinMode(LED_P2_G, OUTPUT);
  pinMode(LED_P2_B, OUTPUT);
  pinMode(A0, INPUT); // Setează pinul pentru butoane ca INPUT
}

// Funcția principală loop
void loop() {
  long int lastMillis = millis();
  int btnValue = analogRead(BUTTONS_PIN); // Citește valoarea de la buton

  if (!gameOn) { // Dacă jocul este oprit
    Serial.print("OFF --- receive:");
    Serial.print(slaveReceived);
    Serial.print(" | send:");
    Serial.println(slaveSend);
    setLedColor(2, 0, 0, 0);
    setLedColor(0, 0, 0, 0);
    setLedColor(3, 0, 0, 0);
    setLedColor(1, 0, 0, 0);
    if (btnValue > parasitVoltage) {
      gameOn = 1; // Pornește jocul
      slaveSend = GAME_START;
      delayMillis(5000);
    }
  } else { // Dacă jocul este pornit
    Serial.print("ON ---  receive:");
    Serial.print(slaveReceived);
    Serial.print(" | send:");
    Serial.println(slaveSend);

    // Control LED-uri pe baza valorii recepționate prin SPI
    if (slaveReceived >= SPI_RGB1_R && slaveReceived <= SPI_RGB1_B) {
      setLedColor(3, 0, 0, 0);
      setLedColor(1, 0, 0, 0);
    } else if (slaveReceived >= SPI_RGB2_R && slaveReceived <= SPI_RGB2_B) {
      setLedColor(2, 0, 0, 0);
      setLedColor(0, 0, 0, 0);
    }

    switch (slaveReceived) {
      case SPI_RGB1_R:
        setLedColor(0, 1, 0, 0);
        correctLed = SPI_BTN1_R;
        break;
      case SPI_RGB1_G:
        setLedColor(0, 0, 1, 0);
        correctLed = SPI_RGB1_G;
        break;
      case SPI_RGB1_B:
        setLedColor(0, 0, 0, 1);
        correctLed = SPI_RGB1_B;
        break;
      case SPI_RGB2_R:
        setLedColor(1, 1, 0, 0);
        correctLed = SPI_RGB2_R;
        break;
      case SPI_RGB2_G:
        setLedColor(1, 0, 1, 0);
        correctLed = SPI_RGB2_G;
        break;
      case SPI_RGB2_B:
        setLedColor(1, 0, 0, 1);
        correctLed = SPI_RGB2_B;
        break;
      case GAME_STOP:
        gameOn = 0;
        break;
      case IGNORE:
        break;
      default:
        Serial.print(" /_> ERROR/GARBAGE Unknown value send by master");
        setLedColor(0, 1, 1, 1);
        setLedColor(1, 1, 1, 1);
        break;
    }

    if (btnValue > parasitVoltage) {
      if (btnValue < BTN_V_P2_R) {
        setLedColor(3, 1, 0, 0);
        collorGuessed = SPI_RGB2_R;
      } else if (btnValue < BTN_V_P2_G) {
        setLedColor(3, 0, 1, 0);
        collorGuessed = SPI_RGB2_G;
      } else if (btnValue < BTN_V_P2_B) {
        setLedColor(3, 0, 0, 1);
        collorGuessed = SPI_RGB2_B;
      } else if (btnValue < BTN_V_P1_B) {
        setLedColor(2, 0, 0, 1);
        collorGuessed = SPI_RGB1_B;
      } else if (btnValue < BTN_V_P1_G) {
        setLedColor(2, 0, 1, 0);
        collorGuessed = SPI_RGB1_G;
      } else if (btnValue < BTN_V_P1_R) {
        setLedColor(2, 1, 0, 0);
        collorGuessed = SPI_RGB1_R;
      }
    }

    Serial.print("================================");
    Serial.println(collorGuessed);
    if (collorGuessed == correctLed) {
      slaveSend = collorGuessed; // Salvează valoarea ghicită
      collorGuessed = IGNORE;
    }
  }

  delayMillis(500); // Delay pentru controlul ciclului
}
