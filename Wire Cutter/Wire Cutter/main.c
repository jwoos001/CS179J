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

int input_counter = 0;
unsigned char* input[2];
unsigned char keypad_input = '0';
const unsigned char* prompt = "Wire length:";

unsigned long spool_timer = 0;
unsigned long length_timer = 0;

unsigned char counter;

void wire_cutter()
{
	switch(state)
	{
		case WAIT: //polling for input
			delay_ms(150);
			break;
		case DISPLAY: //update the LCD
			LCD_DisplayString(1,prompt);
			for(int i = 0; i < input_counter; i++)
			{
				LCD_WriteData(input[i]);
			}
			delay_ms(150);
			break;
		case SPOOL: //roll the spool to the input
			PORTA = 0x3D;
			delay_ms(5);
			PORTA = 0x34;
			delay_ms(5);
			spool_timer++;
			break;
		case BLADE_DOWN:
			PORTB = 0x01;
			delay_ms(10);
			PORTB = 0x03;
			delay_ms(10);
			PORTB = 0x02;
			delay_ms(10);
			PORTB = 0x06;
			delay_ms(10);
			PORTB = 0x04;
			delay_ms(10);
			PORTB = 0x0C;
			delay_ms(10);
			PORTB = 0x08;
			delay_ms(10);
			PORTB = 0x09;
			delay_ms(10);
			counter++;
			break;
		case BLADE_UP:
			PORTB = 0x09;
			delay_ms(10);
			PORTB = 0x08;
			delay_ms(10);
			PORTB = 0x0C;
			delay_ms(10);
			PORTB = 0x04;
			delay_ms(10);
			PORTB = 0x06;
			delay_ms(10);
			PORTB = 0x02;
			delay_ms(10);
			PORTB = 0x03;
			delay_ms(10);
			PORTB = 0x01;
			delay_ms(10);
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
					unsigned char tens = input[0] - '0';
					unsigned char ones = input[1] - '0';
					length_timer = ((tens*10) + ones) * 35;
					if(input_counter < 2)
					{
						length_timer = 0;
					}
					input[0] = '0';
					input[1] = '0';
					input_counter = 0;
					state = SPOOL;
				}
				else if(GetKeypadKey() == '#')
				{
					input[0] = '0';
					input[1] = '0';
					input_counter = 0;
				}
				else
				{
					keypad_input = GetKeypadKey();
					if(input_counter < 2)
					{
						input[input_counter] = keypad_input;
						input_counter++;
					}
					state = DISPLAY;
				}
			}
			else
			{
				state = WAIT;
			}
			/*if(PINB&0x10 == 0x10)
			{
				LCD_DisplayString(1, "Coin input received");
			}*/
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
			if(counter >= 40)		{
				counter = 0;
				state = BLADE_UP;
			}
			break;
		case BLADE_UP:
			if(counter >= 30)
			{
				counter = 0;
				keypad_input = '\0';
				state = DISPLAY;
			}
			break;
	}
}


int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xEF; PORTB = 0x10;
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
