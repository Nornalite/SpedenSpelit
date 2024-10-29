/*
Controls a mixture of 7-segment displays and an LCD screen. Uses Serial-to-Parallel ports to multiply the number of
available outputs.
7-segment displays are of the common cathode variety, LCD screen is a 1602A, controlled by an Arduino Uno. Compatibility 
with other devices not ensured.

Might be a bit of a mess.

v5 - 18.12.2023 - Jenna Salmela
*/

#include "display.h"

/*
Display setup - make sure to call during setup
Parameters: 
  The board locations/pin numbers of the five pins that are used for StP control.
    1. Serial (data line)
    2. Serial clock (output that moves a bit from serial input to serial registers)
    3. Register clock (output that moves data from the serial registers to output registers)
    4. Serial clear (clears the serial registers' contents)
    5. Output enable (enables or disables the output of output registers' contents)
*/
int Display::initializeDisplays(uint8_t ser, uint8_t serClock, uint8_t regClock, uint8_t serClear, uint8_t opEnable) {
  
  #if DEBUGFLAG == 1
  Serial.println("Initializing displays");
  #endif
  
  // Initializes all registers as 0
  for (int i = 0; i < stpTotal; i++) {
    registers[i] = 0;
  }
  for (int j = 0; j < lcdQueueSize; j++) {
    lcdInstructionQueue[j] = 0;
  }
  for (int k = 0; k < maxMessageLength; k++) {
    currentMessage[k] = 0;
  }
  
  // Pin prep
  // Take given pin number data, assign to be used by the program
  serial = ser;
  serialClock = serClock;
  registerClock = regClock;
  serialClear = serClear;
  outputEnable = opEnable;
  
  // Prep StP control pins
  pinMode(serial, OUTPUT);  
  pinMode(serialClock, OUTPUT);
  pinMode(registerClock, OUTPUT);
  pinMode(serialClear, OUTPUT);
  pinMode(outputEnable, OUTPUT);

  // Prep pin states
  digitalWrite(serial, LOW);
  digitalWrite(serialClock, LOW);
  digitalWrite(registerClock, LOW);
  digitalWrite(serialClear, HIGH);
  digitalWrite(outputEnable, LOW);

  // 7-segment prep
  if (segmentDisplayAmount > 0) {
    // Wipes 7-segment display to 0
    clearSSeg();
  }

  // LCD prep
  if (lcdDisplayAmount > 0) {
    #if DEBUGFLAG == 1
    Serial.println("Preparing to initialize LCD");
    #endif
    
    // Initializes the screen's display and input settings
    initializeLCD();
  }

  return 0;
}

/*
Writes a zero to the 7-segment display
*/
int Display::clearSSeg() {
  #if DEBUGFLAG == 1
  Serial.println("Clearing 7-segment");
  #endif
  writeToSSeg(0);
}

/*
Takes a given score, converts it to individual digits, updates/saves digits in 7-segment-display-specific registers, then updates 
all screens with the current contents of all registers.
Numbers larger than what the 7-segs can display (as defined by the number of 7-segment displays, which is a const int written at 
the top of this file) will be reduced down to a displayable size before the breakdown & display processes start (for example with
two 7-segment displays, attempting to display the number 123 would lead to the number being cut down to 23 and then drawn as such).
Parameters:
  score = 16 bit integer value to be printed
*/
int Display::writeToSSeg(uint16_t score) {
  #if DEBUGFLAG == 1
  Serial.println("Writing to 7-segment");
  #endif

  // Cut score down if necessary
  while (score > pow(10, segmentDisplayAmount) - 1) {
    score -= pow(10, segmentDisplayAmount);
  }

  uint8_t digits[segmentDisplayAmount];
  // The score is processed a magnitude at a time (starting from the top). reductionTotal is the total sum of the previous
  // stages and used to cut the score down to the right size for processing
  // e.g. 327/100=3 => 327-300=27 -> 27/10=2 => 327-320=7 -> 7/1=7, digits are 3, 2 and 7
  uint16_t reductionTotal = 0;
  for (int i = 0; i < segmentDisplayAmount; i++) {
    digits[i] = (score - reductionTotal) / (int)(pow(10, (segmentDisplayAmount - 1 - i)) + 0.5);
    reductionTotal += digits[i] * (int)(pow(10, (segmentDisplayAmount - 1 - i)) + 0.5);
  }

  scoreToDigits(digits);
  updateDisplays();

  return 0;
}

/*
Writes a message related to the game's progress onto the LCD
A relatively high score scrambles the sent message in a cursed manner (intended behaviour)
*/
int Display::gameMessage(int score) {
  #if DEBUGFLAG == 1
  Serial.println("Writing game message to LCD");
  #endif  

  bool highScore = false;

  if (score >= 80) {
    score -= 80;
    highScore = true;
  }

  switch (score) {
    case 0: {
      char message[maxMessageLength] = {"Saatko yli kaksitoista?"};
      writeToLCD(message);
      break;
    }
    case 10: {
      char message[60] = {"...ja yhdeksän."};
      writeToLCD(message);
      break;
    }
    case 20: {
      char message[60] = {"No silleen silleen ja tälleen."};
      writeToLCD(message);
      break;
    }
    case 30: {
      char message[60] = {"No voi rähmä!"};
      writeToLCD(message);
      break;
    }
    case 40: {
      char message[60] = {"No voi mikä     huono tuuri!"};
      writeToLCD(message);
      break;
    }
    case 50: {
      char message[60] = {"Tää peli oli    harjoituksena!"};
      writeToLCD(message);
      break;
    }
    case 60: {
      char message[60] = {"Älä hyvä mies   viimeistä saa!"};
      writeToLCD(message);
      break;
    }
    case 70: {
      char message[60] = {"Naama umpeen    siellä sivulla!"};
      writeToLCD(message);
      break;
    }
    default: {
      char message[60] = {"Voi rähmä!"};
      writeToLCD(message);
      break;
    }
  }

  if (highScore == true) {
    for (int i = 0; i < maxMessageLength / 2; i += 2) {
      currentMessage[i] |= (1 << 7);
    }
  }
}

/*
Writes a message to the LCD display. Sent message can be terminated with a null zero ('\0'), although non-terminated messages won't throw up issues.
The message is written to the LCD in two rows of 16, so longer messages will be cut off & you can use spaces to format the message (e.g. moving the start of a word to the next line)
*/
int Display::writeToLCD(char message[]) {
  #if DEBUGFLAG == 1
  Serial.println("Writing to LCD");
  #endif
  
  // The display's in-built character database luckily corresponds quite closely to standard ASCII, so for most
  // letters you can just take the char and paste it onto the message array to be passed to the display
  // There are a few inaccuracies with non-alphabet characters (like \ to yen), but I can't be arsed to look into those right now
  int i = 0;
  int j = 0;
  messageLength = 0;
  while (message[j] != '\0' && (i < maxMessageLength)) {
    if (15 < i && i < 40) {
      currentMessage[i] = 32;
      messageLength++;
      i++;
      continue;
    }
    if (31 < message[j] && message[j] < 128) {
      currentMessage[i] = message[j];
    }
    // Here you could add support for some of the extra characters available on the display
    else {
      switch (message[j]) {
        // ä (only lower case will ever be printed)
        case -61:
          currentMessage[i] = 0b11100001;
          j++;
          break;
        // maybe ö, not tested
        case 148:
          currentMessage[i] = 0b11101111;
          j++;
          break;
        default:
          // space
          currentMessage[i] = 32;
          break;
      }
    }
    messageLength++;
    i++;
    j++;
  }
  // Fill the rest of the array with zeroes
  for (; i < maxMessageLength; i++) {
    currentMessage[i] = 0;
  }

  // Let the queue manager know that it should write characters starting from 0
  messageProgress = 0;
  // First move the cursor to the corner of the screen
  lcdQueueManager(clear, 1);
  lcdQueueManager(clrCsr);
  // Then start writing the message
  lcdQueueManager(write, 2);
}

/*
Not much to say here
Inputs the screen clearing instruction to the queue manager
*/
int Display::clearLCD() {
  #if DEBUGFLAG == 1
  Serial.println("Clearing LCD");
  #endif
  lcdQueueManager(clear);
}

/*
Calls the LCD queue execution function if there's anything left to do
*/
void Display::lcdInterruptCheck() {
  if (lcdDisplayAmount < 1) {
    return;
  }

  if (lcdInterruptActive) {
    #if DEBUGFLAG == 1
    Serial.println("Redirecting to LCD instruction queue manager");
    #endif
    lcdQueueInterrupt();
  } 
}


/********************************************************/
// Below code intended for internal use

/*
Enters the contents of all display-related system registers into the serial-to-parallel chain, then outputs said data onto displays
*/
int Display::updateDisplays() {
  #if DEBUGFLAG == 1
  Serial.println("Writing data to StPs");
  #endif

  // Allow serial registers to take in/retain data
  digitalWrite(serialClear, HIGH);
  
  // Variable to hold the next 1 or 0 to be passed to the registers
  uint8_t bit;
  // Registers are pushed in "butt-first"
  for (int j = stpTotal; j > 0; j--) {
    for (int i = 0; i < 8; i++) {
      // Starting from LSB, take each bit of the register
      bit = ((registers[j - 1] & (1 << i)) >> i);
      // Pass the serial output
      if (bit == 1) {
        digitalWrite(serial, HIGH);
      }
      else {
        digitalWrite(serial, LOW);
      }
      //Serial.print(bit);
      // And activate the serial clock pulse to make the serial register(s) take that bit in
      digitalWrite(serialClock, HIGH);
      digitalWrite(serialClock, LOW);
    }
  }

  // Give the signal to pass serial data to display registers
  digitalWrite(registerClock, HIGH);
  digitalWrite(registerClock, LOW);

  return 0;
}

/*
Converts a set of digits into the bits required to display that number on a 7-segment display, plus saves said bits into
the system's 7-segment-displays' registers
*/
int Display::scoreToDigits(uint8_t digits[]) {
  #if DEBUGFLAG == 1
  Serial.println("Converting score to digits");
  #endif  
  
  // Leading zeroes are not displayed
  int leadingZeroes = 0;
  // Checks all the digits from most significant number(?) to second-to-last, ticking up the leading zero counter for as long as 
  // zeroes come up
  for (int j = 0; j < segmentDisplayAmount - 1; j++) {
    if (digits[j] == 0) {
      leadingZeroes++;
    }
    else break;
  }

  // The different bit combinations that produce a matching number on a 7-segment display
  for (int i = 0; i < segmentDisplayAmount; i++) {
    switch (digits[i]) {
    case 0:
      // Once you're further down the number than leading zeroes reach, print a normal zero
      if (i > leadingZeroes - 1) {
        registers[i] = 0b11111100;
      }
      // Otherwise make the display completely empty
      else {
        registers[i] = 0b00000000;
      } 
      break;
    case 1:
      registers[i] = 0b01100000;
      break;
    case 2:
      registers[i] = 0b11011010;
      break;
    case 3:
      registers[i] = 0b11110010;
      break;
    case 4:
      registers[i] = 0b01100110;
      break;
    case 5:
      registers[i] = 0b10110110;
      break;
    case 6:
      registers[i] = 0b10111110;
      break;
    case 7:
      registers[i] = 0b11100000;
      break;
    case 8:
      registers[i] = 0b11111110;
      break;
    case 9:
      registers[i] = 0b11110110;
      break;
    default:
      return 1;
      break;
    }
  }
  
  return 0;
}

/*
Initializes screen with basic settings
*/
int Display::initializeLCD() {
  #if DEBUGFLAG == 1
  Serial.println("Initalizing LCD");
  #endif

  // The 4 steps of setting up the display's settings:
  // Clearing the screen
  lcdQueueManager(clear, 1);
  // Setting the cursor movement direction to left & screen movement off
  lcdQueueManager(moveSet);
  // Display on, no visible cursor, no cursor blink
  lcdQueueManager(displaySet);
  // Data bus to 8-bit, 2 lines on the display, text font 5x8 pixels
  lcdQueueManager(dataSet, 2);
}

/*
Executes instructions from the LCD's instruction queue one at a time. Rerouted to from the game's loop function
*/
int Display::lcdQueueInterrupt() {  
  cli();
  #if DEBUGFLAG == 1
  Serial.print("In the LCD instruction queue: ");
  for (int i = 0; i < 10; i++) {
    Serial.print(lcdInstructionQueue[i]);
    Serial.print(" ");
  }
  Serial.println();
  #endif

  switch (lcdInstructionQueue[0]) {
  // End of queue, pause interrupts and take note
  case pause:
    #if DEBUGFLAG == 1
    Serial.println("LCD instruction queue empty");
    #endif
    lcdInterruptActive = false;
    break;
  // Clear display
  case clear:
    #if DEBUGFLAG == 1
    Serial.println("LCD clear activates");
    #endif
    // Enter the instruction to wipe the screen
    registers[segmentDisplayAmount] = 0;
    registers[segmentDisplayAmount + 1] = (1 << 1);
    pulseLCDEnable();
    break;
  // Display movement settings
  case moveSet:
    #if DEBUGFLAG == 1
    Serial.println("LCD movement settings activates");
    #endif
    // Cursor moves right, screen says put between inputs
    registers[segmentDisplayAmount] = 0;
    registers[segmentDisplayAmount + 1] = (1 << 2) | (1 << 3);
    pulseLCDEnable();
    break;
  // Adjusts display [display rather than movement] settings
  case displaySet:
    #if DEBUGFLAG == 1
    Serial.println("LCD display settings activates");
    #endif
    // Screen on, no cursor, no cursor blink
    registers[segmentDisplayAmount] = 0;
    registers[segmentDisplayAmount + 1] = (1 << 3) | (1 << 4);
    pulseLCDEnable();
    break;
  // Define data bus as 8-bit, display with two rows and font as 5x8
  case dataSet:
    #if DEBUGFLAG == 1
    Serial.println("LCD data settings activates");
    #endif
    registers[segmentDisplayAmount] = 0;
    registers[segmentDisplayAmount + 1] = (1 << 4) | (1 << 5) | (1 << 6);
    pulseLCDEnable();
    break;
  // Write out a character of a message on screen
  case write:
    #if DEBUGFLAG == 1
    Serial.println("LCD write activates");
    #endif
    // Set data writing bit to 1 and push character data onto the correct ouput slots
    registers[segmentDisplayAmount] = ((1 << 4) | ((currentMessage[messageProgress] & (1 << 7)) >> 7) << 1);
    registers[segmentDisplayAmount + 1] = (currentMessage[messageProgress] << 1);
    messageProgress++;
    pulseLCDEnable();
    break; 
  // This command moves both the cursor and data address back to 0
  case clrCsr:
    #if DEBUGFLAG == 1
    Serial.println("LCD cursor clear activated");
    #endif
    registers[segmentDisplayAmount] = 0;
    registers[segmentDisplayAmount + 1] = (1 << 1);
    break;
  }

  // If the process of printing a message is still ongoing, don't advance the queue
  if (lcdInstructionQueue[0] == 5 && messageProgress < messageLength) {
    return 0;
  }
  // Shift the instruction queue ahead by one step
  for (int i = 0; i < (lcdQueueSize - 1); i++) {
    lcdInstructionQueue[i] = lcdInstructionQueue[i + 1];
  }
  lcdInstructionQueue[lcdQueueSize - 1] = 0;
  sei();
}

/*
Enters given instruction to the end of the existing LCD input queue. 
Available instructions (both number and word are valid inputs):
  1. clear
  2. movSet
  3. dispSet
  4. dataSet
  5. write
  6. clrCsr
holdBackInterrupt (optional argument): typically the queue manager will call queueInterruptManager if there's nothing else in the
queue, but this can cause problems [that will not be elaborated because I've forgotten them by now]
*/
int Display::lcdQueueManager(int instructionNo, int holdBackInterrupt = 0) {
  
  #if DEBUGFLAG == 1
  Serial.println("Writing to LCD instruction queue");
  #endif

  // Run down the queue until you find the first 0 (empty instruction slot)
  int queueNo;
  //Serial.print("In the instruction queue, line no. ");
  #if DEBUGFLAG == 1
    Serial.print("LCD instruction queue: ");
    #endif
  for (int i = 0; i < lcdQueueSize; i++) {
    queueNo = i;
    
    // Here could be inserted some sort of check that removes existing write operations if a clear command comes in
    //if (instructionNo == 1 && lcdInstructionQueue[i] == 5)
    
    #if DEBUGFLAG == 1
    Serial.print(lcdInstructionQueue[i]);
    Serial.print(", ");
    #endif

    if (lcdInstructionQueue[i] == 0) {
      break;
    }
  } 

  #if DEBUGFLAG == 1
  Serial.println();
  #endif

  // Operations are differentiated by instruction numbers in the LCD interrupt manager
  lcdInstructionQueue[queueNo] = instructionNo;

  // If there is nothing else awaiting execution, call the interrupt manager directly (otherwise let the
  // interrupt program run on timer)
  if (holdBackInterrupt == 1) {
    return;
  }
  else if (queueNo == 0 || holdBackInterrupt == 2) {
    lcdInterruptActive = true;
  }
}

/*
Writes data from registers[] to the LCD itself
Pulses the Enable input (which gets the LCD to take in a new command) to low, high and again low. Uses StP between each step to send the signal to the LCD
*/
int Display::pulseLCDEnable() {
  #if DEBUGFLAG == 1
  Serial.println("Writing data to LCD");
  #endif

  // The enable bit is the third-to-last one here
  registers[segmentDisplayAmount] &= 0b11111011;
  updateDisplays();
  registers[segmentDisplayAmount] |= 0b00000100;
  updateDisplays();
  registers[segmentDisplayAmount] &= 0b11111011;
  updateDisplays();
}
