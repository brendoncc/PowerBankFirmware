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
int battCharging = 1;		//used to detect if battery is charging. Based on i2c from lipo charger. Changes function of user button.
int chargerError = 1;		//read from i2c whether charger is in fault. LEDS will flash if fault occurs.
double battPercentage = 0;	//calculated from analog.
double battCurrent = 0;
volatile int ledCount = 0; 
int buttonFlag = 0;
int flashDelay = 0;

int main(void)
{
	Setup();

	while(1)
	{
		//battPercentage = (((ADCW*(5/1024))-3.5)/0.7)*100;	//ADCW holds full 10 bit result from ADCH and ADCL registers.
															//4.2V = 100%, 3.5V = 0%. Making linear approximation of V vs Charge relationship. 3.5V min due to LDO cutoffs.
		if (battPercentage < 5)
		{
			//shut down BATFET via i2c. Need to plug into power source to restore BATFET.
		}
		if(chargerError)
		{	
			TIMSK1 = 0b00000000; // stop timer
			FlashLEDs(1);
		}

		if (buttonFlag == 1)
		{
			ButtonAction();
			buttonFlag = 0;
		}

		if (battCharging)
		{
			TIMSK1 = 0b00000010;		//Generate timer interrupt on compare match of OCR1A(TCNT1)
		}
		else
		{
			TIMSK1 = 0b00000000;		//Disable timer interrupt.
		}

		//READ I2C and Update Variables Accordingly.
	}
}

void ButtonAction(void) //Determines short or long button press after interrupt and acts accordingly.
{
	TIMSK1 = 0b00000000;		//Pause timer for LED status
	LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;

	if (1) //press between 100 and 800ms is considered short.
	{
		_delay_ms(330);
		LED1_ON;
		_delay_ms(330);
		LED1_OFF;
		_delay_ms(330);
		//short button press shows voltage when not charging.
		if (battPercentage >= 85)
		{
			LED1_ON, LED2_ON, LED3_ON, LED4_ON;
		}
		else if (battPercentage >= 60)
		{
			LED1_ON, LED2_ON, LED3_ON;
		}
		else if (battPercentage >= 35)
		{
			LED1_ON, LED2_ON;
		}
		else if (battPercentage >= 10)
		{
			LED1_ON;
		}
		else
		{ 
		}
		_delay_ms(2000);
		LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;
	}
	else if (0) // press longer than 800ms is considered long.
	{
		_delay_ms(330);
		LED2_ON;
		_delay_ms(330);
		LED2_OFF;
		_delay_ms(330);
		//long button press shows battery current.
		if (battCurrent >= 3)
		{
			LED1_ON, LED2_ON, LED3_ON, LED4_ON;
		}
		else if (battCurrent >= 2)
		{
			LED1_ON, LED2_ON, LED3_ON;
		}
		else if (battCurrent >= 1)
		{
			LED1_ON, LED2_ON;
		}
		else if (battCurrent > 0)
		{
			LED1_ON;
		}
		else
		{
			// error (batt current below 0)
		}
		_delay_ms(2000);
		LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;
	}
	else //Do nothing if button press is less than 100ms or other state.
	{
	}
	ledCount = 0;	//reset timer LED so we dont start halfway through cycle
	TIMSK1 = 0b00000010;		//Resume timer after LED status

}

void FlashLEDs(int numFlashes)
{
	for (int i = 0; i < numFlashes; i++)
	{
		LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;	//all off for 250ms.
		_delay_ms(250);
		LED1_ON, LED2_ON, LED3_ON, LED4_ON;	//all on for 250ms.
		_delay_ms(250);
		LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;	//all off for 250ms.
	}
}

ISR (EXT_INT0_vect)		//Interrupt based on user button push. Used to wake uC and then set flag that button has been pushed.
{
	buttonFlag = 1;
}

ISR (TIM1_COMPA_vect)	//LED sequence to indicate battery charging.
{
	if (ledCount == 1)
	{
		LED1_ON;
	}
	else if (ledCount == 2)
	{
		LED2_ON;
	}
	else if (ledCount == 3)
	{
		LED3_ON;
	}
	else if (ledCount == 4)
	{
		LED4_ON;
	}
	if (ledCount > 4)
	{
		ledCount = 0;
		LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;
	}
	ledCount++;
}

void Setup(void)
{
	DDRA = 0b10001100;		//Set register A I/O Based on pin out in header file (1 output, 0 input).
	PORTA = 0b00000000;		//Disable pull up resistors as we have external. Set outputs to initially low. (DDR = 0, 1 enable, 0 disable, DDR = 1, 1 high, 0 low)
	DDRB = 0b00000001;		//Set register B based on pin out in header file.
	PORTB = 0b00000100;		//Disable pull up resistors. Set outputs initially low.
	
	ADMUX = 0b00000000;		//ADC ref is Vcc, No mux settings therefore ADC0 is used.
	ADCSRA = 0b11000111;	//enable (bit 7, disable before sleep) ADC and start conversion (bit 6), 128 prescaler for ADC clock.
	ADCSRB = 0b00000000;	//free running mode
	DIDR0 = 0b00000001;		//disable digital input buffer on ADC as recommended to reduce power consumption.
	
	MCUCR = 0b00000000;		//low level of INT0 triggers interrupt
	GIMSK = 0b01010000;		//enable INT0 external interrupt and allow enabling of PC interrupts 0 to 7.
	PCMSK0 = 0b00000000;	//Set bit 1 to 1 to enable PCINT1 interrupt. All others disabled. Only needed if using Lipo charging interrupt.

	TCCR1A = 0b00000000;	//16 bit timer in CTC mode
	TCCR1B = 0b00001100;	//prescaler 256 and CTC mode
	OCR1A = 15625/8;			//Max of 65535, ** T_int = (1Mhz/256)/OCR1A = 0.25s **
	TIMSK1 = 0b00000010;		//Generate interrupt on compare match of OCR1A(TCNT1)

	sei();					//global interrupts enabled

	USI_TWI_Master_Initialise();
}

