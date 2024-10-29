#ifndef BUTTONS_H
#define BUTTONS_H
#include <arduino.h>

void buttonsActivated(void);

void initButtonsAndButtonInterrupts(void (*function)(int), void (*startFunction)());
/*function is for the game buttons. It takes one parameter which
 is a pointer to a function
 startFunction is a pointer only for the start button
*/
void disableButtonInterrupts(void);
/*this function disables button interrupts from pins 2-5 (game buttons)
 */


#endif
