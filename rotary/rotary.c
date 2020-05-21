/*  Author: Steve Gunn
 * Licence: This work is licensed under the Creative Commons Attribution License.
 *           View this license at http://creativecommons.org/about/licenses/
 * 
 * Modified for FortunaHome by Alexander Miles, 2020
 */
 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "rotary.h"

volatile int8_t rotary = 0;
volatile int8_t middle = 0;
volatile int8_t switchpos = 0;

void init_rotary()
{
	/* Ensure all pins are inputs with pull-ups enabled */
	DDRE &= ~_BV(ROTA) & ~_BV(ROTB) & ~_BV(SWC);
	PORTE |= _BV(ROTA) | _BV(ROTB) | _BV(SWC);
	DDRC &= ~_BV(SWN) & ~_BV(SWE) & ~_BV(SWS) & ~_BV(SWW);
	PORTC |= _BV(SWN) | _BV(SWE) | _BV(SWS) | _BV(SWW);
	/* Configure interrupt for any edge on rotary and falling edge for button */
	EICRB |= _BV(ISC40) | _BV(ISC50) | _BV(ISC71);
}

int8_t get_rotary()
{
	static uint8_t lastAB = 0x00;
	uint8_t AB = PINE & (_BV(ROTA) | _BV(ROTB));
	if ((AB == 0x00 && lastAB == 0x20) || (AB == 0x30 && lastAB == 0x10))
		if(rotary > 5) {
			rotary = 1;
		} else {
			rotary++;
		}
	if ((AB == 0x30 && lastAB == 0x20) || (AB == 0x00 && lastAB == 0x10))
		if(rotary < 1) {
			rotary = 6;
		} else {
			rotary--;
		}
	lastAB = AB;
	return rotary;
}

uint8_t get_switch()
{
	return PINC & (_BV(SWN) | _BV(SWE) | _BV(SWS) | _BV(SWW));
}

int8_t get_middle() {
	if ((~PINE & _BV(PE7) && (switchpos == 0))) {
		switchpos = 1;
		middle = 1;
		//_delay_is(60);
		return 1;
	} else { 
		switchpos = 0;
		middle = 0;
		//_delay_ms(60);
		return 0;
	} return 0;
}



ISR(INT7_vect)
{
	get_middle();
	//_delay_ms(50);
}
