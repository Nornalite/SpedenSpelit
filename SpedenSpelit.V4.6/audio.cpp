#include "pitches.h"

const int buzzerPin = 5;//pin will change in final version

void gameStateDetect(int gameState) {
  //int gameState = 0;
  static bool effectStart = true;
  static unsigned long effectStartTime = 0;
  if (gameState != 1 && gameState != 2)
  {
    effectStart = true;
  }
  if (gameState == 1) {
    if (effectStart == true) {
      effectStart = false;
      effectStartTime = millis();
    }
    if (millis() - effectStartTime < 750) {
      failEffect();
    }
    else if (millis() - effectStartTime < 2000) {
      //
    }
    else {
      gameState = 0;
    }
  }
  if (gameState == 0) {
    startMusic();
  }
  else if (gameState == 2) {
    if (effectStart == true) {
      effectStart = false;
      effectStartTime = millis();
    }
    if (millis() - effectStartTime < 1500) {
      winEffect();
    }
  }
}

void playMelody(const int melody[], const int noteDurations[], int melodyLength) {
  //Serial.println("playMelody");
  static int thisNote = 0;
  static unsigned long noteStartTime = 0;
  int noteDuration = 1000 / noteDurations[thisNote];

  if (millis() - noteStartTime >= noteDuration) {
    tone(buzzerPin, melody[thisNote], noteDuration);
    noteStartTime = millis();
    thisNote++;
  }
  else if (thisNote > melodyLength - 1){
    thisNote = 0;
  }
}

void winEffect() {
  int melody[] = {
    //melody has the pitches we want the function to play
    NOTE_C4, NOTE_E4, NOTE_C5
  };

  int noteDurations[] = {
    //note durations for each pitch presented in melody
    8, 8, 8
  };

  playMelody(melody, noteDurations, 3);
  //call to the function playMelody
  //3 is the number of pitches used in this melody
}

void failEffect() {
  //Serial.print("failEf");
  int melody[] = {
    NOTE_C3, NOTE_G2, NOTE_C3
  };

  int noteDurations[] = {
    8, 8, 2
  };

  playMelody(melody, noteDurations, 3);
}

void startMusic() {
  int melody[] = {
    NOTE_B5, NOTE_B5, NOTE_C6, NOTE_B5, NOTE_B5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_FS5, NOTE_G5, NOTE_B5, NOTE_A5, NOTE_B5, NOTE_A5, NOTE_G5, NOTE_G5, NOTE_FS5, NOTE_G5, NOTE_A5, NOTE_C6, NOTE_B5, NOTE_A5, NOTE_G5, NOTE_G5};
  int noteDurations[] = {8, 16, 8, 16, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 1};
  playMelody(melody, noteDurations, 24);
}

void backgroundMusic() {
  int melody[] = {
    NOTE_A3, NOTE_E3, NOTE_A3, NOTE_D4,
    NOTE_A3, NOTE_E3, NOTE_A3, NOTE_D4,
    NOTE_A3, NOTE_E3, NOTE_A3, NOTE_D4,
    NOTE_A3, NOTE_E3, NOTE_A3, NOTE_D4,
    
    NOTE_G3, NOTE_D3, NOTE_G3, NOTE_C4,
    NOTE_G3, NOTE_D3, NOTE_G3, NOTE_C4,
    NOTE_G3, NOTE_D3, NOTE_G3, NOTE_C4,
    NOTE_G3, NOTE_D3, NOTE_G3, NOTE_C4,

    NOTE_A3, NOTE_E3, NOTE_A3, NOTE_D4,
    NOTE_A3, NOTE_E3, NOTE_A3, NOTE_D4,
    NOTE_A3, NOTE_E3, NOTE_A3, NOTE_D4,
    NOTE_A3, NOTE_E3, NOTE_A3, NOTE_D4,
    
    NOTE_F3, NOTE_C3, NOTE_F3, NOTE_A3,
    NOTE_F3, NOTE_C3, NOTE_F3, NOTE_A3,
    NOTE_F3, NOTE_C3, NOTE_F3, NOTE_A3,
    NOTE_F3, NOTE_C3, NOTE_F3, NOTE_A3,

    NOTE_F3, NOTE_C2, NOTE_F2
  };

  int noteDurations[] = {
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,

    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,

    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,

    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,
    8, 8, 8, 8,

    2, 2, 1,
  };

  playMelody(melody, noteDurations, 67);
}

void buttonSound() {
  int melody[] = {
    //melody has the pitches we want the function to play
    NOTE_C5
  };

  int noteDurations[] = {
    //note durations for each pitch presented in melody
    8
  };

  playMelody(melody, noteDurations, 1);
}
