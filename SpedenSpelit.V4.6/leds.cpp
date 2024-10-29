#include "leds.h"

int ledNumber0 = A2;
int ledNumber1 = A3;
int ledNumber2 = A4;
int ledNumber3 = A5;
int eyeLeds = 6;
int startLed = A0;
uint8_t brightness = 0;

/*
  initializeLeds() subroutine intializes analog pins A2,A3,A4,A5
  to be used as outputs. Speden Spelit leds are connected to those
  pins.  
*/

void initializeLeds(){//sets analog pins as outputs

pinMode(ledNumber0, OUTPUT);
pinMode(ledNumber1, OUTPUT);
pinMode(ledNumber2, OUTPUT);
pinMode(ledNumber3, OUTPUT);
pinMode(eyeLeds, OUTPUT); 
pinMode(startLed, OUTPUT);
}

void startButtonLed(bool state ){
  if(state == 1){
    digitalWrite(startLed, HIGH);
  }
  else if(state == 0){
    digitalWrite(startLed, LOW);
  }
}
/*
  setLed(int) sets correct led number given as 0,1,2 or 3
  led number 0 corresponds to led connected at Arduino pin A2
  led number 1 => Arduino pin A3
  led number 2 => Arduino pin A4
  led number 3 => Arduino pin A5
  
  parameters:
  int ledNumber is 0,1,2 or 3
*/
void setLed(int ledNumber, bool state){
  if(ledNumber == 0){
      if (state == 1){
      digitalWrite(ledNumber0, HIGH);
      }
      else if(state == 0){
      digitalWrite(ledNumber0, LOW);
      }
  }
  if(ledNumber == 1){
      if (state == 1){
      digitalWrite(ledNumber1, HIGH);
    }
      else if(state == 0){
      digitalWrite(ledNumber1, LOW);
      }
  }
  if(ledNumber == 2){
      if (state == 1){
      digitalWrite(ledNumber2, HIGH);
  }
      else if(state == 0){
      digitalWrite(ledNumber2, LOW);
      }
  }
  if(ledNumber == 3){
      if (state == 1){
      digitalWrite(ledNumber3, HIGH); 
  }
      else if(state == 0){
      digitalWrite(ledNumber3, LOW);
      }
}
}



/*
  clearAllLeds(void) subroutine clears all leds
*/
void clearAllLeds(void){

  digitalWrite(ledNumber0, LOW);
  digitalWrite(ledNumber1, LOW);
  digitalWrite(ledNumber2, LOW);
  digitalWrite(ledNumber3, LOW);
  digitalWrite(startLed, LOW);
  digitalWrite(eyeLeds, LOW);

}

void setAllLeds(void){

  digitalWrite(ledNumber0, HIGH);
  digitalWrite(ledNumber1, HIGH);
  digitalWrite(ledNumber2, HIGH);
  digitalWrite(ledNumber3, HIGH);
  digitalWrite(startLed, HIGH);
  digitalWrite(eyeLeds, HIGH);

}


void eyesOfSpede(){ 

  brightness = brightness * 2 + 1; //adds brightness

  if (brightness > 255) {
    brightness = 255; // Returns brightness to zero when it rises over 255
  }

  analogWrite(eyeLeds, brightness); // Sends PWM-signal to the leds

}