/*
Controls a mixture of 7-segment displays and an LCD screen. Uses Serial-to-Parallel ports to multiply the number of
available outputs.
7-segment displays are of the common cathode variety, LCD screen is a 1602A, controlled by an Arduino Uno. Compatibility 
with other devices not ensured.

Might be a bit of a mess.

v5 - 18.12.2023 - Jenna Salmela
*/

/*
HOW TO USE:
  1. Check the 7-segment display and LCD display numbers/amounts from the start of the "protected" section below, adjust numbers
    if you have more or fewer displays
  2. Set the DEBUGFLAG to 1 to get Serial.prints from each activating function, or to 0 to not
  3. Create a "Display [object name]" object 
  4. Call [object name].initializeDisplays();, give function five pins connected to an StP port
  5. Place [object name].lcdInterruptCheck(); to the loop function (or some other constantly repeating section of code) - this writes to the
    LCD screen and instruction at a time. Can be skipped if no LCDs are attached.
  6. Use [object name].writeToSSeg(), [object name].writeToLCD() and other public functions to control the attached displays.
*/

#ifndef DISPLAY_H
#define DISPLAY_H
#include <arduino.h>

#define DEBUGFLAG 0

class Display {
  public:
    /*
    Display setup - make sure to call during setup
    Parameters: the board locations/pin numbers of the five pins that are used for StP control.
      1. Serial (data line)
      2. Serial clock (output that moves a bit from serial input to serial registers)
      3. Register clock (output that moves data from the serial registers to output registers)
      4. Serial clear (clears the serial registers' contents)
      5. Output enable (enables or disables the output of output registers' contents)
    */
    int initializeDisplays(uint8_t ser, uint8_t serClock, uint8_t regClock, uint8_t serClear, uint8_t opEnable);

    /*
    Writes the number 0 to the 7-segment display
    */
    int clearSSeg();

    /*
    Takes a given score, converts it to individual digits, updates/saves digits in 7-segment-display-specific registers, then updates 
    all screens with the current contents of all registers.
    Numbers larger than what the 7-segs can display (as defined by the number of 7-segment displays, which is a const int written at 
    the top of this file) will be reduced down to a displayable size before the breakdown & display processes start (for example with
    two 7-segment displays, attempting to display the number 123 would lead to the number being cut down to 23 and then drawn as such).
    Parameters:
      score = 16 bit integer value to be printed
    */
    int writeToSSeg(uint16_t score);

    /*
    Not much to say here
    Inputs the screen clearing instruction to the queue manager
    */
    int clearLCD();

    /*
    Writes a message related to the game's progress onto the LCD
    A relatively high score scrambles the sent message in a cursed manner (intended behaviour)
    */
    int gameMessage(int score);

    /*
    Writes a message to the LCD display. Sent message can be terminated with a null zero ('\0'), although non-terminated messages won't throw up issues.
    The message is written to the LCD in two rows of 16, so longer messages will be cut off & you can use spaces to format the message (e.g. moving the start of a word to the next line)
    */
    int writeToLCD(char message[]);

    /*
    Function placed in the .ino's loop to call the queue execution function once per frame
    */
    void lcdInterruptCheck();
    bool lcdInterruptActive;

  protected:
    // The numbers of: 7-segment displays attached, lcd screens attached, Serial-to-Parallel ports required to feed data
    // to all attached displays
    static const uint8_t segmentDisplayAmount = 3, lcdDisplayAmount = 1, stpTotal = (segmentDisplayAmount + (2 * lcdDisplayAmount));

    // The numbers/locations of the pins that are used to control StP ports
    uint8_t serial, serialClock, registerClock, serialClear, outputEnable;

    /*
    The registers that hold data about the desired states of all outputs in the display section
    Structure of a 7-seg register: [A][B][C][D] [E][F][G][x] - letters correspond to outputs leading to individual segments 
    on each 7-seg display, LSB is used to pass data to the next StP and its state is irrelevant
    Structure of an LCD register: 
      registers[n] = [x][x][x][rSelect] [rWrite][enable][p7][x] 
      registers[n + 1] = [p6][p5][p4][p3] [p2][p1][p0][x]
        rSelect: Register Select, tells the device whether it's receiving an instruction or data to display
        rWrite: Read/Write, tells the device whether it gets or gives data
        Enable: essentially a clock pulse
        pins 7 through 0: Used to input both instruction details and character data
        x's are not used as outputs and their states don't matter
    */
    volatile uint8_t registers[stpTotal];

    // An LCD display has limits on its input intake speed, so instructions are queued up and called through
    // timer interrupts one at a time. This is the max length of the queue.
    static const uint8_t lcdQueueSize = 20;
    
    // The the different commands available for the LCD (here as variables because it's eaasier to remember a word than number)
    static const uint8_t pause = 0, clear = 1, moveSet = 2, displaySet = 3, dataSet = 4, write = 5, clrCsr = 6;

    // Number of characters in the message currently being displayed
    uint8_t messageLength;
    // The maximum amount of characters allowed in a message sent to an LCD display (not related to the size of the display)
    static const uint8_t maxMessageLength = 50;
    // A tracker of which character of the message should be printed next
    volatile uint8_t messageProgress;

    // An LCD display has limits on its input intake speed, so instructions are queued up and called through
    // timer interrupts one at a time.
    volatile uint8_t lcdInstructionQueue[lcdQueueSize];
    // The message that is being written to the LCD display (or was written once writing is done)
    // Max message length is set as 100 here, could be changed if desired
    volatile uint8_t currentMessage[maxMessageLength];
    
    /*
    Enters the contents of all display-related system registers into the serial-to-parallel chain, then outputs said data onto displays
    */
    int updateDisplays();

    /*
    Converts a set of digits into the bits required to display that number on a 7-segment display, plus saves said bits into
    the system's 7-segment-displays' registers
    */
    int scoreToDigits(uint8_t digits[]);
    
    /*
    Initializes screen with basic settings
    */
    int initializeLCD();    
    
    /*
    Enters given instruction to the end of the existing LCD input queue. 
    Available instructions (both number and word are valid inputs):
      1. clear
      2. movSet
      3. dispSet
      4. dataSet
      5. write
      6. clrCsr
    */
    int lcdQueueManager(int instructionNo, int holdBackInterrupt = 0);
    
    /*
    Executes instructions from the LCD's instruction queue. Triggered through instruction-staggering timer interrupts, or
    if the queue is empty, though a direct call from the queue manager function
    */
    int lcdQueueInterrupt();

    /*
    Writes data from registers[] to the LCD itself
    Pulses the Enable input to low, high and again low. Uses StP between each step to send the signal to the LCD
    */
    int pulseLCDEnable();
};

#endif
