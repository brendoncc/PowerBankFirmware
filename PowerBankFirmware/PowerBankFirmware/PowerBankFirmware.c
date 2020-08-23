/*
* PowerBankFirmware.c
*
* Created: 14/08/2020 1:42:05 PM
* Author : ABJ
*/

#include "PowerBankHeader.h"

int main(void)
{
	setup();

	while (1)
	{

	}
}

ISR (TIMER0_COMPA_vect)		//timer based interrupt that triggers on compare match of OCR0A and TCNT0
{

}

ISR (TIMER1_COMPA_vect)		//timer based interrupt that triggers on compare match of OCR1A and TCNT1
{
	LED_TOGGLE;
}

void setup(void)
{
	DDRA = 0b00000000;		//Set register A to all inputs (0)
	PORTA = 0b11111111;		//enable all PORTA pull up resistors (1 enable , 0 disable, only when DDR is set to 0)
	DDRB = 0b11111111;		//Set register B to all outputs (1)
	PORTB = 0b00000000;		//all PORTB outputs initially low (1 high, 0 low)
	DDRD = 0b00000000;		//Set register D to all inputs (0)
	PORTD = 0b11111111;		//enable all PORTD pull up resistors (1 enable, 0 disable)
	TCCR0A = 0b00000010;	//8 bit timer in CTC mode
	TCCR0B = 0b00000101;	//prescaler 1024
	OCR0A = 255;			//Max of 255, ** T_int = (1/(8Mhz/Prescaler))*OCR1A =  0.03264s **
	TCCR1A = 0b00000000;	//16 bit timer in CTC mode
	TCCR1B = 0b00001100;	//prescaler 256
	OCR1A = 31250;			//Max of 65535, ** T_int = (1/(8Mhz/256))*OCR1A = 1s ** 
	TIMSK = 0b01000001;		//Compare match on OCR1A(TCNT1) and OCR0A(TCNT0)
	sei();					//global interrupts enabled
}