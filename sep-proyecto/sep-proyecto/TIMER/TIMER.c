#include <avr/io.h>

#include "TIMER.h"

void TIMER_Init(void)
{
	OCR1A = 62500; //1000HZ62500
	TCCR1B |= (1 << WGM12);
	// Mode 4, CTC on OCR1A
	TIMSK1 |= (1 << OCIE1A);
	//Set interrupt on compare match
	TCCR1B |= (1 << CS12);
	// set prescaler to 1024 and start the timer

	
	OCR2A = 156; // 156
	TCCR2A |= (1 << WGM21);
	// Set to CTC Mode
	TIMSK2 |= (1 << OCIE2A);
	//Set interrupt on compare match
	TCCR2B |= (1 << CS12) | (1 << CS10);
	// set prescaler to 64 and starts PWM
	
	// Set the Timer Mode to CTC
	TCCR0A |= (1 << WGM01);
	// Set the value that you want to count to
	OCR0A = 62;
	TIMSK0 |= (1 << OCIE0A);    //Set the ISR COMPA vect
	TCCR0B |= (1 << CS02);
	// set prescaler to 256 and start the timer
}
