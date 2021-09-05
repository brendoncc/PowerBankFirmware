/*
* PowerBankHeader.h
*
* Description: Contains all definitions and header files required for Power Bank Firmware
* Created: 14/08/2020 1:42:05 PM
* Author : 
*/

/* Header Includes and Clock Speed Defines */
#define  F_CPU 1000000UL	//1 MHz clock for compatibility with I2C library
#include <avr/io.h> //io header
#include <avr/interrupt.h> //interrupt header
#include <avr/sleep.h>
#include <util/delay.h> //delay header

/* 
* Bit masking of IO
*
* |=	Bitwise OR (Used to turn a bit ON)
* ^=	Bitwise XOR (Used to toggle a bit)
* &=	Bitwise AND (Used read the current value of a bit)
* ~		Bitwise Complement (Use with Bitwise AND to turn a bit off)
*
* X = Register Name, Y = Location of Bit in Register
*
* TOGGLE (PORTX ^= (1<<Y))		 // 1 XOR	1 = 0,	0 XOR	1 = 1. Used for toggling a bit.
* ON (PORTX |= (1<<Y))			 // 1 OR	1 = 1,	0 OR	1 = 1. Used for changing a bit to 1.
* OFF (PORTX &= ~(1<<Y))		 // 1 AND	0 = 0,	0 AND	0 = 0. Used for changing a bit to 0. The 1<<Y is complemented therefore = 0
* STATE (PINX &= (1<<Y))		 // 1 AND	1 = 1,	0 AND	1 = 0. Used for reading the current state of a bit.
*
*/

/* Pinout for AT Tiny 44A
* PA0/ADC0 = Vbat Analog
* PA1/PCINT1 = Interrupt from Lipo Charger
* PA2 = LED1
* PA3 = LED2
* PA4 = SCK/SCL (ISP / i2c Lipo Charger)
* PA5 = MISO (ISP)
* PA6 = MOSI/SDA (ISP / i2c Lipo Charger)
* PA7 = LED3
* PB0 = LED4
* PB1 = Not Used
* PB2/INT0 = Button
* PB3 = Reset (ISP)
*/

/* Defines for IO - I2C Pins are defined in USI_TWI_Master.h */ 
#define CHARGER_INT PCINT1
#define BUTTON_PRESSED !(PINB &= (1<<2))	//Read active low button press


#define LED4_ON (PORTA |= (1<<2))		//Set PA2 High
#define LED3_ON (PORTA |= (1<<3))		//Set PA3 High
#define LED2_ON (PORTA |= (1<<7))		//Set PA7 High
#define LED1_ON (PORTB |= (1<<0))		//Set PB0 High

#define LED4_OFF (PORTA &= ~(1<<2))		//Set PA2 Low
#define LED3_OFF (PORTA &= ~(1<<3))		//Set PA3 Low
#define LED2_OFF (PORTA &= ~(1<<7))		//Set PA7 Low
#define LED1_OFF (PORTB &= ~(1<<0))		//Set PB0 Low

#define LED4_TOGGLE (PORTA ^= (1<<2))
#define LED3_TOGGLE (PORTA ^= (1<<3))		//Set PA3 Low
#define LED2_TOGGLE (PORTA ^= (1<<7))		//Set PA7 Low
#define LED1_TOGGLE (PORTB ^= (1<<0))


/* Function Prototypes */
void Setup(void);
void ButtonAction(void);
void ButtonShort(void);
void ButtonLong(void);
void FlashLEDs(int numFlashes);
ISR (TIMER1_COMPA_vect);
ISR (TIMER0_COMPA_vect);