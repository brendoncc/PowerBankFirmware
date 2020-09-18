/*
* PowerBankFirmware.c
*
* Created: 14/08/2020 1:42:05 PM
* Author : ABJ
*/

/* Header Files */
#include "PowerBankHeader.h"
#include "USI_TWI_Master.h"

/* Global Variables */
int charging = 0;		//used to detect if battery is charging. Based on i2c from lipo charger. Changes function of user button.
volatile int LEDcount = 0;

int main(void)
{
	setup();

	while(1)
	{
		if (charging)
		{
			//enable timer to do LED sequence
			TIMSK1 = 0b00000010;		//Generate interrupt on compare match of OCR1A(TCNT1)
		}
		else
		{
			//disable interrupt
			TIMSK1 = 0b00000000;		
		}
	}
}

void ButtonAction(void) //Called on interrupt of button push after waking.
{
	int count = 0;

	while(buttonPressed && count < 200)	//kick out of loop if held longer than 2s
	{
		count++;
		_delay_ms(10);
	}

	if (count > 10 && count < 80) //press between 100 and 800ms is considered short.
	{
		buttonShort();
	}
	else if (count >= 80) // press longer than 800ms is considered long.
	{
		buttonLong();
	}
	else //Do nothing if button press is less than 100ms or other state.
	{
		return;
	}
}

void buttonShort(void)
{
	//show battery voltage
}

void buttonLong(void)
{
	if (charging)
	{
		//show charge current
	}
	else
	{
		//tbd
	}
}

ISR (EXT_INT0_vect)		//Interrupt based on user button push. Used to wake uC and then determine short or long press.
{
	ButtonAction();
}

ISR (TIM1_COMPA_vect)
{	
	if (LEDcount == 1)
	{
		LED1_ON;
	}
	else if (LEDcount == 2)
	{
		LED2_ON;
	}
	else if (LEDcount == 3)
	{
		LED3_ON;
	}
	else if (LEDcount == 4)
	{
		LED4_ON;
	}
	if (LEDcount > 4)
	{
		LEDcount = 0;
		LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;
	}
	LEDcount++;
}

void setup(void)
{
	DDRA = 0b10001100;		//Set register A I/O Based on pin out in header file (1 output, 0 input).
	PORTA = 0b00000000;		//Disable pull up resistors as we have external. Set outputs to initially low. (DDR = 0, 1 enable, 0 disable, DDR = 1, 1 high, 0 low)
	DDRB = 0b00000001;		//Set register B based on pin out in header file.
	PORTB = 0b00000100;		//Disable pull up resistors. Set outputs initially low.

	ADCSRA = 0b11000111;	//enable ADC and start conversion (bit 6), 128 prescaler for ADC clock.
	ADCSRB = 0b00000000;	//free running mode
	DIDR0 = 0b00000001;		//disable digital input buffer on ADC as recommended to reduce power consumption.
		
	MCUCR = 0b00000000;		//low level of INT0 triggers interrupt
	GIMSK = 0b01010000;		//enable INT0 external interrupt and allow enabling of PC interrupts 0 to 7.
	PCMSK0 = 0b00000000;	//Set bit 1 to 1 to enable PCINT1 interrupt. All others disabled. Only needed if using Lipo charging interrupt.

	TCCR1A = 0b00000000;	//16 bit timer in CTC mode
	TCCR1B = 0b00001100;	//prescaler 256 and CTC mode
	OCR1A = 15625;			//Max of 65535, ** T_int = (1Mhz/256)/OCR1A = 0.25s **

	sei();					//global interrupts enabled

	USI_TWI_Master_Initialise();
}

