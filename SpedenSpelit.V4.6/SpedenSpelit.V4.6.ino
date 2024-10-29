#include "TimerOne.h"
#include "display.h"
#include "buttons.h"
#include "leds.h"
#include "SpedenSpelit.h"
#include "pitches.h"
// omia globaaleja
volatile int painetut_numerot[10]; // painallusten tallentamiseen
volatile int random_numerot[10]; // generoitujen lukujen 
volatile int laskin; // timerin muuttamisen laskuri joka nollaantuu 9:ssä ja timeri nopeutuu
volatile int randNumber;
volatile int score;
long timer = 1000000; // init timerin muuttuja jota muokataan ohjelmassa
volatile int index; // rng listan painamatonta alkiota seuraava numero/indeksi
volatile int gameState = 0;

Display display;

void setup()
{
  /*
    Initialize here all modules
  */
  Serial.begin(9600);
  initializeTimer();
  Timer1.stop();
  initializeLeds();
  initButtonsAndButtonInterrupts(&buttonPress, &startButton);
  display.initializeDisplays(2, 3, 4, 7, 8);
  startButtonLed(1);
  Serial.println("Setup valmis");
  // Välilyönnin puute asettelun vuoksi, on ok näytöllä
  display.writeToLCD("Yhden vai kahdentonnin haaste?");
  display.clearSSeg();
}

void loop()
{
  buttonsActivated();
  display.lcdInterruptCheck();
  gameStateDetect(gameState);
}

void timer1Active() {
  // Sammuttaa kaikki ledit ennen uuden sytytystä
  setLed(0,0);
  setLed(1,0);
  setLed(2,0);
  setLed(3,0);

  int rand_temp;
  randNumber = random(9,13); // muokkaa rand väli vastaamaan painettujen pinnien arvoja
  for (int i = 9; i >= 0; i--) { // listan alkioiden oikealle siirto looppi
    rand_temp = random_numerot[i-1];
    random_numerot[i] = rand_temp;
  }
  random_numerot[0] = randNumber; // uusimman numeron lisääminen indexiin 0
  setLed(randNumber - 9, 1); // ledi päälle
  index++; // rng listan painamattoman numeron seuraaja
  if (index > 9) {
    lostTheGame();
  }
}

void startButton() {
  Serial.println("Start button");
  startTheGame();
  eyesOfSpede();
}

void buttonPress(int buttonInput) {
  //napin painallus, antaa button muuttujalle painetun napin pinnin arvon
  int paino_temp;
  for (int i = 9; i >= 0; i--) {
    paino_temp = painetut_numerot[i-1];
    painetut_numerot[i] = paino_temp;
  }
  painetut_numerot[0] = buttonInput;
  checkGame(buttonInput);
}

void initializeTimer(void)
{
	Serial.println("Timerin valmistelu");
  // see requirements for the function from SpedenSpelit.
  Timer1.initialize(timer);
  Timer1.attachInterrupt(&timer1Active, timer);
}

void checkGame(int nbrOfButtonPush)
{
  // see requirements for the function from SpedenSpelit.h
  if (random_numerot[index - 1] == painetut_numerot[0]) {
    score++;
    laskin++;
    index--;
    display.writeToSSeg(score);
    setLed(nbrOfButtonPush - 9, 0);
    gameState = 4;
    // timerin nopeutus | timer globaalia muuttujaa kerrotaan 0.9 ja ajetaan timer init uudestaan keskeytysten tiheyden muuttamiseksi
    if (laskin == 10) {
      timer *= 0.9;
      laskin = 0;
      display.gameMessage(score);
      initializeTimer();
      eyesOfSpede();
      gameState = 2;
    }
  }
  else {
    lostTheGame();
  }
}

void initializeGame()
{
	Serial.println("Pelin aloitusta");
  // see requirements for the function from SpedenSpelit.h
  // ledit nollaan
  initButtonsAndButtonInterrupts(&buttonPress, &startButton);
  clearAllLeds();
  // display tyhjennys
  // listojen nollaus
  timer = 1000000;
  for (int i = 0; i < 10; i++) {
    painetut_numerot[i] = 0;
    random_numerot[i] = 0;
  }
  //muuttujien nollaus
  laskin = 0;
  randNumber = 0;;
  score = 0;
  index = 0;
  gameState = 4;
  initializeTimer();
  display.clearSSeg();
  display.gameMessage(score);
  Timer1.resume();
}

void lostTheGame() {
  Timer1.stop();
  disableButtonInterrupts();
  clearAllLeds();
  startButtonLed(1);
  Serial.println("Peli menetetty");
  gameState = 1;

  display.writeToLCD("Koitit ison etkäsaa penniäkään!");
}

void startTheGame()
{
  Serial.println("Starttia painettu");
  // see requirements for the function from SpedenSpelit.h
  initializeGame();
}

