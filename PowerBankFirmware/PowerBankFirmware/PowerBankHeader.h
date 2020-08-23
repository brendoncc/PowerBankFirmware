/*
* PowerBankHeader.h
*
* Description: Contains all definitions and header files required for Power Bank Firmware
* Created: 14/08/2020 1:42:05 PM
* Author : ABJ
*/

/* Header Includes and Clock Speed Defines */

#define  F_CPU 8000000UL
#include <avr/io.h> //io header
#include <avr/interrupt.h> //interrupt header
#include <util/delay.h> //delay header


/* Bit masking of IO
* |=	Bitwise OR
* ^=	Bitwise XOR
* &=	Bitwise AND
* ~		Bitwise Complement
*/

#define LED_TOGGLE (PORTB ^= (1<<0))	 // 1 XOR	1 = 0,	0 XOR	1 = 1. Used for toggling a bit.
#define LED_ON (PORTB |= (1<<0))		 // 1 OR	1 = 1,	0 OR	1 = 1. Used for changing a bit to 1.
#define LED_OFF (PORTB &= ~(1<<0))		 // 1 AND	0 = 0,	0 AND	0 = 0. Used for changing a bit to 0. The 1<<0 is complemented therefore = 0
#define LED_STATE (PINB &= (1<<0))		 // 1 AND	1 = 1,	0 AND	1 = 0. Used for reading the current state of a bit.

/* Pinout for AT Tiny 44A (Used in lipo charger) - Testing with AT Tiny 2313A
* PA0 = Vbat Analog
* PA1 = Interrupt from Lipo Charger
* PA2 = LED1
* PA3 = LED2
* PA4 = SCK/SCL (ISP / i2c Lipo Charger)
* PA5 = MISO (ISP)
* PA6 = MOSI/SDA (ISP / i2c Lipo Charger)
* PA7 = LED3
* PB0 = LED4
* PB1 = Not Used
* PB2 = Button
* PB3 = Reset (ISP)
*/

/* Function Prototypes */

void setup(void);
ISR (TIMER1_COMPA_vect);
ISR (TIMER0_COMPA_vect);