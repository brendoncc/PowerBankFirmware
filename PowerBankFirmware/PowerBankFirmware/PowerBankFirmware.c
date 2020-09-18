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

int main(void)
{
	setup();

	while(1)
	{
		if (charging)
		{
			//enable timer to do LED sequence
		}
		else
		{
			//disable timer
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

	if (count > 10 && count < 60) //press between 100 and 600ms is considered short.
	{
		buttonShort;
	}
	else if (count >= 60) // press longer than 600ms is considered long.
	{
		buttonLong;
	}
	else //Do nothing if button press is less than 100ms or other state.
	{
		return;
	}
}

void buttonShort(void);
{
	//show battery voltage
}

void buttonLong(void);
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

ISR (INT0_vect)		//Interrupt based on user button push. Used to wake uC and then determine short or long press.
{
	ButtonAction();
}

void setup(void)
{
	DDRA = 0b10001100;		//Set register A I/O Based on pin out in header file (1 output, 0 input).
	PORTA = 0b00000000;		//Disable pull up resistors as we have external. Set outputs to initially low. (DDR = 0, 1 enable, 0 disable, DDR = 1, 1 high, 0 low)
	DDRB = 0b00000001;		//Set register B based on pin out in header file.
	PORTB = 0b00000000;		//Disable pull up resistors. Set outputs initially low.
	//EICRB
	//EIMSK
	 
	sei();					//global interrupts enabled
	USI_TWI_Master_Initialise();
}

