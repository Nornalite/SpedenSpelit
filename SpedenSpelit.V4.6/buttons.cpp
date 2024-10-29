#include "buttons.h"

volatile bool ISRactivated = 0;

const byte firstPin = 9; // first PinChangeInterrupt on D-bus
const byte lastPin =  12; // last PinChangeInterrupt on D-bus
const byte startButton = 13; //start button seperately for clarity

void (*interruptFunction)(int); //pointer for pins (int is for pin number)
void (*startButtonInterruptFunction)(); //pointer for pin 7 (start button)

void initButtonsAndButtonInterrupts(void (*function)(int), void (*startFunction)()) {
  //fuction address for "function" which calls game button interrupts and "startFunction" for calling the start button interrupt
    for (byte pin = firstPin; pin <= lastPin; ++pin) { //D-bus defines buttons as...buttons (INPUT_PULLUP)
        pinMode(pin, INPUT_PULLUP);
    }
    pinMode(startButton, INPUT_PULLUP); //pinmode for start button

    interruptFunction = function;
    startButtonInterruptFunction = startFunction;
    
    //interrupts for pins
    PCICR |= (1 << PCIE0);
    PCMSK0 |= (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3) | (1 << PCINT4) | (1 << PCINT5);

    ISRactivated = false;

    sei();
}

void disableButtonInterrupts(void) { //function for disabling button interrupts when called
    //disables interrupts for pins (game buttons)
    PCMSK0 &= ~((1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3) | (1 << PCINT4));
    //optionally, you can disable the entire PCIE2 if needed if you want to also disable pin (start button)
    //PCICR &= ~(1 << PCIE2);
}

void buttonsActivated(void) {
  if (ISRactivated == false) {
    return;
  }
  
  delay(5);
  
  //debounce prevents false button presses
  static unsigned long lastInterruptTime = 0;
  unsigned long currentTime = millis(); //stores time in milliseconds
  unsigned long debounceInterval = 100; //any additional transitions duting this time are ignored to prevent false button presses
  //adjust if needed

  /*Serial.print(lastInterruptTime);
  Serial.print(", ");
  Serial.println(currentTime);*/
  
    if (currentTime - lastInterruptTime > debounceInterval) { //checks if enough time has passed since last button press
        for (byte pin = firstPin; pin <= lastPin; pin++) { //D-bus for detecting low state in pins
            if (digitalRead(pin) == LOW) { //this part reads which button has been pressed when grounded
                interruptFunction(pin); //calling function
            }
        }
        if (digitalRead(startButton) == LOW) {
            startButtonInterruptFunction(); //calling function
        }

        lastInterruptTime = currentTime;
    }
  
  ISRactivated = false;
}

ISR(PCINT0_vect) {
    ISRactivated = true;
}