/*
 * Wire Cutter.c
 *
 * Created: 5/2/2018 12:57:15 AM
 * Author : jawsw
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "keypad.h" //from 120b
#include "lcd.h"    //from 120b

//timing code received from 120b

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;

	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1=0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--; 
	if (_avr_timer_cntcurr == 0) { 
		TimerISR(); 
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

//end timing code received from 120b

enum states { /*L1, L2,*/ WAIT, DISPLAY, SPOOL, BLADE_DOWN, BLADE_UP } state;
/*

void test () {
	switch(state) {
		case L1:
			PORTB = 0x01;
			PORTA = 0x11;
			_delay_ms(3);
			PORTA = 0x33;
			_delay_ms(3);
			PORTA = 0x22;
			_delay_ms(3);
			PORTA = 0x66;
			_delay_ms(3);
			PORTA = 0x44;
			_delay_ms(3);
			PORTA = 0xCC;
			_delay_ms(3);
			PORTA = 0x88;
			_delay_ms(3);
			PORTA = 0x99;
			_delay_ms(3);
			counter++;
			break;
		case L2:
			PORTB = 0x02;
			PORTA = 0x99;
			_delay_ms(3);
			PORTA = 0x88;
			_delay_ms(3);
			PORTA = 0xCC;
			_delay_ms(3);
			PORTA = 0x44;
			_delay_ms(3);
			PORTA = 0x66;
			_delay_ms(3);
			PORTA = 0x22;
			_delay_ms(3);
			PORTA = 0x33;
			_delay_ms(3);
			PORTA = 0x11;
			_delay_ms(3);
			counter++;
			break;
		default: 
			PORTB = 0x03;
	}
	switch (state) {
		case L1:
			if(counter >= 50)
			{
				counter = 0;
				state = L2;
			}
			break;
		case L2:
			if(counter >= 50)
			{
				counter = 0;
				state = L2;
			}
			break;
		default:
			state = L1;
			break;
	}
}*/


unsigned char keypad_input = '0';
unsigned char input_count = 0;
const unsigned char* prompt = "Wire length:";

unsigned long spool_timer = 0;
unsigned long length_timer = 0;

unsigned char counter;

void wire_cutter()
{
	switch(state)
	{
		case WAIT:
			delay_ms(150);
			break;
		case DISPLAY:
			LCD_DisplayString(1,prompt);
			LCD_WriteData(keypad_input);
			delay_ms(150);
			break;
		case SPOOL:
			PORTA = 0x09;
			delay_ms(3);
			PORTA = 0x08;
			delay_ms(3);
			PORTA = 0x0C;
			delay_ms(3);
			PORTA = 0x04;
			delay_ms(3);
			PORTA = 0x06;
			delay_ms(3);
			PORTA = 0x02;
			delay_ms(3);
			PORTA = 0x03;
			delay_ms(3);
			PORTA = 0x01;
			delay_ms(3);
			spool_timer++;
			break;
		case BLADE_DOWN:
			PORTA = 0x10;
			delay_ms(3);
			PORTA = 0x30;
			delay_ms(3);
			PORTA = 0x20;
			delay_ms(3);
			PORTA = 0x60;
			delay_ms(3);
			PORTA = 0x40;
			delay_ms(3);
			PORTA = 0xC0;
			delay_ms(3);
			PORTA = 0x80;
			delay_ms(3);
			PORTA = 0x90;
			delay_ms(3);
			counter++;
			break;
		case BLADE_UP:
			PORTA = 0x90;
			delay_ms(3);
			PORTA = 0x80;
			delay_ms(3);
			PORTA = 0xC0;
			delay_ms(3);
			PORTA = 0x40;
			delay_ms(3);
			PORTA = 0x60;
			delay_ms(3);
			PORTA = 0x20;
			delay_ms(3);
			PORTA = 0x30;
			delay_ms(3);
			PORTA = 0x10;
			delay_ms(3);
			counter++;
			break;
	}
	switch(state)
	{
		case WAIT:
			if(GetKeypadKey() != '\0')
			{
				if(GetKeypadKey() == '*')
				{
					length_timer = (keypad_input - '0') * 100;
					state = SPOOL;
				}
				else
				{
					keypad_input = GetKeypadKey();
					state = DISPLAY;
				}
			}
			break;
		case DISPLAY:
			state = WAIT;
			break;
		case SPOOL:			
			if(spool_timer >= length_timer)
			{
				spool_timer = 0;
				state = BLADE_DOWN;
			}
			else
			{
				state = SPOOL;
			}
			break;
		case BLADE_DOWN:
			if(counter >= 50)
			{
				counter = 0;
				state = BLADE_UP;
			}
			break;
		case BLADE_UP:
			if(counter >= 50)
			{
				counter = 0;
				keypad_input = '\0';
				state = WAIT;
			}
			break;
	}
}


int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;
	TimerSet(1);
	TimerOn();
	LCD_init();
	LCD_DisplayString(1,prompt);
	while(1) {
		//test();
		wire_cutter();
		while (!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}

