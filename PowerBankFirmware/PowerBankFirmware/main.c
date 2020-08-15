/*
 * PowerBankFirmware.c
 *
 * Created: 14/08/2020 1:42:05 PM
<<<<<<< Updated upstream
 * Author : Brendon is an absolute monkey
=======
 * Author : Brendon is an absolute monkey - Thats rude be nice to Matt's son and Alex's father
>>>>>>> Stashed changes
 */ 

#define  F_CPU 8000000UL
#include <avr/io.h> //io header
#include <avr/interrupt.h> //interrupt header

void setup(void);	
ISR (TIMER1_COMPA_vect);
ISR (TIMER0_COMPA_vect);

//bit masking of IO
#define LED_TOGGLE (PORTB ^= (1<<0))

int main(void)
{
	setup();

    while (1) 
    {
		//test test test - BC working copy
    }
}

ISR (TIMER0_COMPA_vect)	//timer based interrupt that triggers on compare match of OCR0A and TCNT0
{

}

ISR (TIMER1_COMPA_vect) //timer based interrupt that triggers on compare match of OCR1A and TCNT1
{
	LED_TOGGLE;
}

void setup(void)
{
	DDRA = 0b00000000;		//Set register A to all inputs (0)
	PORTA = 0b11111111;		//enable all PORTA pull up resistors (1 enable , 0 disable)
	DDRB = 0b11111111;		//Set register B to all outputs (1)
	PORTB = 0b00000000;		//all PORTB outputs initially low (1 high, 0 low)
	DDRD = 0b00000000;		//Set register D to all inputs (0)
	PORTD = 0b11111111;		//enable all PORTD pull up resistors (1 enable, 0 disable)
	TCCR0A = 0x02;			//8 bit timer in CTC mode
	TCCR0B = 0x04;			//prescaler 256, 256us
	OCR0A = (31250 / 1000);			//interrupt every 1ms
	TCCR1A = 0x00;			//16 bit timer in CTC mode
	TCCR1B = 0x0C;			//prescaler 256, 256us.
	OCR1A = (31250);			//interrupt every 1s (8000000/256/31250 = 1)
	TIMSK = 0x41;			//Compare match on OCR1A(TCNT1) and OCR0A(TCNT0)
	sei();					//global interrupts enabled
}