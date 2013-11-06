/*
 * Main.c
 *
 * Created: 30.09.2013 23:32:24
 *  Author: dr.morz
 */ 

#include <avr/interrupt.h>
#include "LCD.h"

volatile unsigned long value;
volatile unsigned int adc_counter;
volatile unsigned int segment=1;
volatile int display = 0;

ISR(ADC_vect)
{
	// 2.56/1023 = 0,002502 V
	if(segment == 1){
		value = value + (ADC*1000)/341; // 3000 => 30.00 V
	}else{
		value = value + (ADC*1000)/2046; // 500 => 5.00 A
	}
	adc_counter++; // увеличиваем значение до 127.
	ADCSRA |= 1<<ADSC;
	
}


void avr_init()
{
	adc_counter=0;
	DDRC = 0x00;
	ADCSRA = 0x8F;
	ADMUX|=(1<<REFS0)|(1<<REFS1); 
	sei();
	ADCSRA |= 1<<ADSC;
}

char SYMBOLS[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int main(void)
{
	avr_init();
	lcd_init();
	lcd_cmd(0xC0);
	
	_delay_ms(500);	
	
	int s = 0;
	for (int i=0;i<8;++i)
	{
		for (int b=0;b<8;++b)
		{
			lcd_s(1, i, b*8, s++);
		}
	}
	s = 0;
	for (int i=0;i<8;++i)
	{
		for (int b=0;b<8;++b)
		{
			lcd_s(2, i, b*8, s++);
		}
	}
	lcd_putc(1, 8, "VOLTAGE:");
	lcd_putc(1, 77, "AMPERS:");
	lcd_putc(4, 37+64, ".");
	lcd_putc(4, 37, ".");
	
	while(1)
	{
		if (adc_counter > 100)
		{
			display= value/adc_counter;
			tiks++;
			
			lcd_big_number(segment, 3, 8, SYMBOLS[display % 10000 / 1000]);
			lcd_big_number(segment, 3, 20+4, SYMBOLS[display % 1000 / 100]);
			lcd_numbers(segment, 4, 34+8, SYMBOLS[display % 100 / 10]);
			lcd_numbers(segment, 4, 34+16, SYMBOLS[display % 10 / 1]);
			lcd_set_bar(segment, display);
			if (segment == 2)
				lcd_set_max(0, display);
			else
				lcd_set_max(display, 0);
			adc_counter = 0;
			value = 0;
			if (segment==1){
				segment = 2;
				ADMUX&=~0X0F; 
				char ch = 1;
				ch&=0x07;
				ADMUX|=ch;
				ADCSRA|=(1<<ADSC); 
			}else if(segment == 2){
				segment = 1;
				char ch = 0;
				ADMUX&=~0X0F; 
				ch&=0x07;
				ADMUX|=ch;
				ADCSRA|=(1<<ADSC); 
			}
			_delay_ms(100);
		}
	};
}