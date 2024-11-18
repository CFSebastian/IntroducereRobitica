# Componente fizice utilizate
- 6x LED-uri (2 grupuri de câte 3 leduri, în cadrul unui grup trebuie să avem culori diferite)
- 2x LED RGB (1 pentru fiecare jucător)
- 6x butoane (3 pentru fiecare jucător)
- 1x LCD
- 1x servomotor
- 2x Breadboard
- Fire de legatura
- 2x Arduino Uno


# Codul Master
Codul master controlează un joc în care jucătorii concurează reacționând la o culoare afișată printr-un sistem RGB.
Interacțiunea are loc printr-un afișaj LCD, un servomotor și o comunicare cu un dispozitiv slave folosind SPI (Serial Peripheral Interface).
Jocul urmărește scorurile jucătorilor, gestionează rundele și actualizează stările jocului pe afișaj.


### Biblioteci Incluse

- LiquidCrystal.h: Pentru controlul afișajului LCD.
- Servo.h: Pentru controlul servomotorului.
- SPI.h: Pentru comunicarea SPI între master și dispozitivul slave.
### Pini și Definiții

- BAUD_RATE: Viteza de transmisie serială (9600 baud).
- Pini pentru RGB: Definesc pini pentru controlul componentelor RGB.
- Pini pentru Butoane: Definesc pini pentru controlul butoanelor RGB.
- Definiri pentru Controlul Jocului: GAME_START, GAME_STOP, IGNORE pentru a controla starea jocului.
- Constante de Timp: SECOND, GAME_DURATION, ROND_DURATION pentru durata jocului și a rundelor.
- Definiri pentru Servomotor: END_ROTATION, START_ROTATION, ROTATION_DISTANCE controlează poziția servomotorului.




# Codul Slave
Slave răspunde la comenzile de la master și transmite datele înapoi folosind SPI.
Comportamentul său este gestionat în principal în funcția de întrerupere ISR (Interrupt Service Routine), ISR (SPI_STC_vect).

## 1. Definirea variabilelor pentru slave


volatile boolean received;
volatile byte slaveReceived, slaveSend = IGNORE;

- received: Variabilă volatile folosită pentru a indica dacă slave a primit date de la master.
- slaveReceived: Variabilă volatile care stochează valoarea primită de slave de la master prin SPI.
- slaveSend: Variabilă care conține valoarea pe care slave o va trimite către master ca răspuns. Inițial, este setată la IGNORE (valoare definită pentru a indica lipsa unei comenzi utile).

## 2. Setarea SPI pentru slave în setup():

pinMode(MISO, OUTPUT); // Configurare pin pentru MISO (Master In Slave Out)
SPCR |= _BV(SPE); // Activarea modulului SPI ca slave
received = false; // Inițializare flag de recepție
SPI.attachInterrupt(); // Activarea întreruperii pentru recepția de date

- pinMode(MISO, OUTPUT): Configurarea pinului MISO ca ieșire. Acesta este linia prin care slave trimite date către master.
- SPCR |= _BV(SPE): Activarea modulului SPI ca slave (setarea bitului SPE - SPI Enable).
- SPI.attachInterrupt(): Activarea întreruperii atunci când slave primește date de la master.

  ## 3. Funcția de întrerupere ISR (SPI_STC_vect):

ISR (SPI_STC_vect)
{
  slaveReceived = SPDR;   // Citirea datelor primite de la master
  SPDR = slaveSend;       // Setarea datelor ce vor fi trimise către master la următoarea transmisie
  received = true;        // Setarea flag-ului pentru a indica că datele au fost primite
}


ISR (SPI_STC_vect): Această funcție este apelată automat atunci când slave primește date de la master.
- slaveReceived = SPDR;: Citirea datelor primite de la master. SPDR este registrul de date SPI și conține valoarea transmisă de master.
- SPDR = slaveSend;: Setarea valorii pe care slave o va trimite master-ului în următoarea transmisie.
- received = true;: Setează received pe true pentru a indica că slave a primit date.

## 4. Comportamentul slave în funcția loop():

 slave procesează datele primite de la master și îndeplinește diferite acțiuni în funcție de valoarea recepționată:

- Dacă gameOn este false (jocul nu este activ), slave verifică dacă a primit comanda de pornire a jocului (GAME_START) și pregătește jocul pentru start.
- Dacă gameOn este true (jocul este activ), slave:
1. Controlează LED-urile și verifică intrările de la butoane.
2. Verifică comenzile primite prin SPI (slaveReceived) pentru a stabili culoarea LED-urilor și a inițializa alte acțiuni.3.  Utilizează variabila slaveSend pentru a comunica master-ului rezultatele acțiunilor sale, de exemplu, dacă jucătorul a ghicit corect culoarea (collorGuessed == correctLed).

# Scopul slave in joc
- slave gestionează logica jocului și răspunde la comenzile primite de la master.
- Controlează starea LED-urilor și butoanele pentru fiecare jucător.
- Comunicarea bidirecțională prin SPI permite master-ului să transmită comenzi și să primească informații de la slave, permițând o colaborare eficientă în timpul jocului.



# Poze a setupului fizic
![WhatsApp Image 2024-11-18 at 19 39 23_5cd3434a](https://github.com/user-attachments/assets/bc9dec4d-d6cd-487d-9541-8930b494f71e)





# Schema electrica

![WhatsApp Image 2024-11-18 at 19 15 24_4abbd96e](https://github.com/user-attachments/assets/8106c26a-48a0-4953-8b5c-168f132674d9)
![WhatsApp Image 2024-11-18 at 19 15 24_2098d2da](https://github.com/user-attachments/assets/3a2d4eae-3b2a-44be-916d-38a9080414b5)
![WhatsApp Image 2024-11-18 at 19 15 24_f6e21fb1](https://github.com/user-attachments/assets/54ca3936-3cac-4d0a-82c1-593f9caea381)
[Link Tinkercad]([https://youtu.be/6Zd-KeOgaac](https://www.tinkercad.com/things/3RnetCqzFdo-fantastic-jaiks-bigery))

# Video cu functionalitatea montajului fizic
[Watch video here!](https://youtu.be/6Zd-KeOgaac)
