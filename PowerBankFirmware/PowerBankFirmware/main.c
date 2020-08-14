/*
 * PowerBankFirmware.c
 *
 * Created: 14/08/2020 1:42:05 PM
 * Author : Brendon
 */ 

#define  F_CPU 8000000UL
#include <avr/io.h> //io header
#include <avr/interrupt.h> //interrupt header

void setup(void);	
ISR (TIMER1_COMPA_vect);
ISR (TIMER0_COMPA_vect);

int main(void)
{
	setup();

    while (1) 
    {
		//test test test - BC working copy
    }
}

void setup(void)
{
	DDRA = 0x00;		//inputs
	PORTA = 0x07;		//enable pullup resistors on bit 0 & 1 & 3
	DDRB = 0xFF;		//set portb to output
	PORTB = 0xFF;		//outputs initially high.
	DDRD = 0x60;		//bits 5 & 6 set as output.
	PORTD = 0xFF;		//bits 5 & 6 initially high, leds off. inputs pulled up
	TCCR0A = 0x02;		//8 bit timer in CTC mode
	TCCR0B = 0x04;		//prescaler 256, 256us
	OCR0A = 156.24;		//interrupt every 5ms (125us * 156.24 = 5ms)
	TCCR1A = 0x00;		//16 bit timer in CTC mode
	TCCR1B = 0x0C;		//prescaler 256, 256us.
	OCR1A = 31250;		//interrupt every 1s (125us * 31250 = 1s)
	TIMSK = 0x41;		//Compare match on OCR1A(TCNT1) and OCR0A(TCNT0)
	sei();				//global interrupts enabled
}