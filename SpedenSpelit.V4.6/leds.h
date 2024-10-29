#ifndef LEDS_H
#define LEDS_H
#include <arduino.h>

void initializeLeds();

void setLed(int ledNumber, bool state);

void clearAllLeds(void);

void setAllLeds(void);

void eyesOfSpede(void);

void startButtonLed(bool state);

#endif