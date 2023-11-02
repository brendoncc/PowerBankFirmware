/*
* PowerBankFirmware.c
*
* Created: 14/08/2020 1:42:05 PM
* Author : BC
*/

/* Header Files */
#include "PowerBankHeader.h"
#include "USI_TWI_Master.h"

/* Global Variables */
int battCharging = 0;		//used to detect if battery is charging. Based on i2c from lipo charger.
volatile int ledCount = 0;
//float battVoltage = 0.0;	//calculated from analog.
float battVoltageBQ = 0.0;	//Read from I2C, BQ25895 has internal ADC
float battCurrentBQ = 0.0;	//Read from I2C, BQ25895 has internal ADC
unsigned char messageBuf[4];	//buffer for I2C comms
unsigned char slaveAdd = 0x6a;	//BQ25895 I2C slave address
float uvlo_voltage = 3.0;

/* Overview

Button push will do 1 of 3 things
- short press: show battery voltage
- short press while battery charging: show battery charging current
- long press (5s): turn off BatFET, requires long press (2s) to turn back on

If battery is charging, LED will show charging pattern. Pattern length is based on the battery voltage 

If battery is not charging, uC sleeps automatically and wakes every 8s to poll BQ25895 for 3 variables: Battery charging status, battery voltage, and battery charge current. 

If battery voltage is detected to drop below 3.0V, the unit will automatically disconnect the battery from the system.

User can preserve battery life by shutting down the unit with a 5s button press. This will disconnect the battery from the system. To reconnect, button needs to be held for 2s. 

*/

int main(void)
{
	Setup();
	_delay_ms(50); //time for BQ25895 to fully start up before sampling ADCs and I2C. 
	BQRead(); //initial status, gets read again before any action so that registers can update

	while(1)
	{
		//Check if BQ25895 will automatically update input current when unrecognized supply. 
		//If not, check if charging supply is unrecognized, if so, every 30s, re run the detection algorithm 
		//ReadADC();
		BQRead();

		if (battVoltageBQ < uvlo_voltage) //UVLO threshold
		{
			BQShutdown();
		}

		//add if statement for fault here - blink lights rapidly for fault. To read the current fault status, the host has to read REG0C two times consecutively.
 
		if (battCharging)
		{
			if(battCharging == 0x03)
			{
				TIMSK1 = 0b00000000;	//Disable timer compare match that drives LED timing
				LED1_ON,LED2_ON, LED3_ON,LED4_ON; //all LEDs on to show battery fully charged
			}
			else
			{
				TIMSK1 = 0b00000010;	//Enable timer compare match that drives LED timing
				//add code here to check if unknown adapter type - if unknown adapter, re run the current optimizer every minute or so to try get MPP out of solar input.
			}

		}
		else
		{
			TIMSK1 = 0b00000000;	//Disable timer compare match that drives LED timing
			ledCount = 0;
			LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;
			sleep_cpu(); //sleep CPU, only way to wake is INT0 or watchdog. 

		}
	}
}

// void ReadADC(void) //read battery voltage from ADC
// {
// 	battVoltage = (ADCH *5.07)/256;
// }

void BQShutdown(void) //quick flash of LEDs to show shutdown, then tell BQ to disconnect BATFET
{
	_delay_ms(100);
	LED1_ON,LED2_ON, LED3_ON,LED4_ON;	//flash all LEDs twice quickly to show shutdown
	_delay_ms(100);
	LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;
	_delay_ms(100);
	LED1_ON,LED2_ON, LED3_ON,LED4_ON;
	_delay_ms(100);
	LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;

	messageBuf[0] = (slaveAdd<<TWI_ADR_BITS) | (0<<TWI_READ_BIT); //first byte: first bit is R/W, upper 7 are address. Set to write
	messageBuf[1] = 0x09;	//second byte: register address to read or write to
	messageBuf[2] = 0b01100100; //third byte: Data to write: Force BATFET off
	USI_TWI_Start_Transceiver_With_Data(messageBuf,3); //transmit to slave
}

void BQRead(void)
{
	_delay_ms(10);

	//read charging status
	messageBuf[0] = (slaveAdd<<TWI_ADR_BITS) | (0<<TWI_READ_BIT); //first byte: first bit is R/W, upper 7 are address. Set to write since we are telling slave what register we want
	messageBuf[1] = 0x0B;	//second byte: register address to read or write to
	USI_TWI_Start_Random_Read(messageBuf,3);	//start I2C read, 3 bytes are 2 bytes to send + 1 byte to read		
	battCharging = ((messageBuf[1]&(3<<3))>>3); 
	
	_delay_ms(10);
	
	//start ADC conversion to read voltage and current
	messageBuf[0] = (slaveAdd<<TWI_ADR_BITS) | (0<<TWI_READ_BIT); //first byte: first bit is R/W, upper 7 are address. Set to write
	messageBuf[1] = 0x02;	//second byte: register address to read or write to
	messageBuf[2] = 0b10111101; //third byte: Data to write: Start ADC single shot conversion (rest is default)
	USI_TWI_Start_Transceiver_With_Data(messageBuf,3); //transmit to slave
 	
	_delay_ms(10);

	//read battery voltage after ADC conversion
	messageBuf[0] = (slaveAdd<<TWI_ADR_BITS) | (0<<TWI_READ_BIT); //first byte: first bit is R/W, upper 7 are address. Set to write since we are telling slave what register we want
	messageBuf[1] = 0x0E;	//second byte: register address to read or write to
	USI_TWI_Start_Random_Read(messageBuf,3); //start I2C read, 3 bytes are 2 bytes to send + 1 byte to read
	battVoltageBQ = ((((messageBuf[1]))*0.02)+2.304); //multiply by 0.02V (V per decimal), offset by 2.304V (register starts at 2.304V)
	
	_delay_ms(10);

	//read battery current after ADC conversion
	messageBuf[0] = (slaveAdd<<TWI_ADR_BITS) | (0<<TWI_READ_BIT); //first byte: first bit is R/W, upper 7 are address. Set to write since we are telling slave what register we want
	messageBuf[1] = 0x12;	//second byte: register address to read or write to
	USI_TWI_Start_Random_Read(messageBuf,3);	//start I2C read, 3 bytes are 2 bytes to send + 1 byte to read
	battCurrentBQ = (((messageBuf[1]))*0.05); // multiply by 0.05A (A per decimal)
}

void ButtonActionShort(void) //Short button press shows battery voltage if not charging, shows battery charge current if charging
{
	LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;

	if (battCharging == 0x00 || battCharging == 0x03)
	{
		_delay_ms(200);
		LED1_ON;
		_delay_ms(200);
		LED1_OFF;
		_delay_ms(200);

		if (battVoltageBQ >= 4.00)
		{
			LED1_ON, LED2_ON, LED3_ON, LED4_ON; //4 LEDS on above 4V
		}
		else if (battVoltageBQ >= 3.80) 
		{
			LED1_ON, LED2_ON, LED3_ON; //3 LEDS on between 3.8V and 4V
		}
		else if (battVoltageBQ >= 3.40)
		{
			LED1_ON, LED2_ON; //2 LEDS on between 3.4V and 3.8V
		}
		else if (battVoltageBQ >= uvlo_voltage)
		{
			LED1_ON; //1 LED on between 3.0V and 3.4V (shuts down below 3.0V)
		}
		else
		{
		}
	}
	else if (battCharging == 0x01 || battCharging == 0x02)
	{
		_delay_ms(200);
		LED1_ON,LED2_ON;
		_delay_ms(200);
		LED1_OFF,LED2_OFF;
		_delay_ms(200);

		if (battCurrentBQ >= 1.5)
		{
			LED1_ON, LED2_ON, LED3_ON, LED4_ON; //4 LEDS on above 1.5A.
		}
		else if (battCurrentBQ >= 1)
		{
			LED1_ON, LED2_ON, LED3_ON; //3 LEDS on between 1A and 1.5A
		}
		else if (battCurrentBQ >= 0.5)
		{
			LED1_ON, LED2_ON; //2 LEDS on between 0.5A and 1.0A
		}
		else if (battCurrentBQ >= 0.05)
		{
			LED1_ON; //1 LED on between 0.05 and 0.5A
		}
		else
		{
		}
	}
	_delay_ms(1000);
	LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;
	_delay_ms(500);
}


ISR (EXT_INT0_vect)		//Interrupt based on user button push. Used to wake uC and call function
{
	int buttoncount = 0;
	ledCount = 0; //reset LED count for charging pattern timer so it doesnt start halfway when button is pushed during charging

	while(BUTTON_PRESSED && buttoncount <= 500)
	{
		buttoncount++;
		_delay_ms(10);
	}
	
	BQRead();	

	if (buttoncount > 500)
	{
		BQShutdown();
	}
	else
	{
		ButtonActionShort();
	}
}

ISR (WATCHDOG_vect)		//watchdog interrupt wakes uC every 8s to monitor
{
	WDTCSR = 0b01101001;	//set watchdog back to interrupt instead of reset. If this doesnt happen, second watchdog time out results in reset. 
}

ISR (TIM1_COMPA_vect)	//LED sequence to indicate battery charging and dynamically show voltage increasing.
{
		//Need to characterize battery pack and correspond accurate voltages to percentages
		if (ledCount == 1 && battVoltageBQ >= uvlo_voltage) //0 - 25% capacity
		{
			LED1_ON;
		}
		else if (ledCount == 2 && battVoltageBQ >= 3.40) //25 - 50% capacity
		{
			LED2_ON;
		}
		else if (ledCount == 3 && battVoltageBQ >= 3.80) //50 - 75% capacity
		{
			LED3_ON;
		}
		else if (ledCount == 4 && battVoltageBQ >= 4.00) // 75 - 100% capacity 
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
	
	PRR = 0b00000001;		//make sure //ADCs// and timers are powered up. Write 0b00000001 to shut down ADC before sleep
	//ADMUX = 0b00000000;		//ADC ref is Vcc, No mux settings therefore ADC0 is used.
	//ADCSRA = 0b11100000;	//enable (bit 7, disable before sleep) ADC, start conversion (bit 6), enable auto trigger, no prescaler for ADC clock.
	//ADCSRB = 0b00010000;	//free running mode, left adjust result to read only ADCH 8 bit 
	//DIDR0 = 0b00000001;		//disable digital input buffer on ADC as recommended to reduce power consumption.
	
	MCUCR = 0b00110000;		//enable sleep mode, sleep; command set to power down. Can only wake from INT0 low level or watchdog.
	GIMSK = 0b01010000;		//enable INT0 external interrupt and allow enabling of PC interrupts 0 to 7.
	PCMSK0 = 0b00000000;	//Set bit 1 to 1 to enable PCINT1 interrupt. All others disabled. Only needed if using Lipo charging interrupt.

	TCCR1A = 0b00000000;	//16 bit timer in CTC mode
	TCCR1B = 0b00001100;	//prescaler 256 and CTC mode
	OCR1A = 15625/8;		//Max of 65535, ** T_int = (1Mhz/256)/OCR1A = 0.5s **
	TIMSK1 = 0b00000000;	//Set bit 1 to generate interrupt on compare match of OCR1A(TCNT1)

	WDTCSR = 0b01101001;	//watchdog enabled, set to interrupt not reset. 8s watchdog timing. If watchdog not reset after interrupt, next watchdog timeout will cause reset. 

	_delay_ms(200);
	LED1_ON,LED2_ON, LED3_ON,LED4_ON;	//flash all LEDs twice to show power on (slower than shutdown LED flash so user can tell the difference)
	_delay_ms(200);
	LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;
	_delay_ms(200);
	LED1_ON,LED2_ON, LED3_ON,LED4_ON;
	_delay_ms(200);
	LED1_OFF, LED2_OFF, LED3_OFF, LED4_OFF;

	sei();	//global interrupts enabled

	USI_TWI_Master_Initialise();	//initalize I2C interface
}

