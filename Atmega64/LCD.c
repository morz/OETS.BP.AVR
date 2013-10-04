/*
 * LCD.c
 *
 * Created: 30.09.2013 23:33:45
 *  Author: dr.morz
 */ 

#include "LCD.h"
#include "propfont.h"

uint16_t ma_old_y = 0;
uint16_t mv_old_y = 0;

void lcd_cmd(unsigned char cmd)
{
	_delay_us(2);
	data_port = cmd;
	_delay_us(1);
	cmd_port |= (1<<E);
	_delay_us(12);
	cmd_port &= ~(1<<E);
	_delay_us(2);
}

void lcd_data(unsigned char data)
{
	_delay_us(2);
	cmd_port |= (1<<DI);
	_delay_us(2);
	data_port = data;
	_delay_us(2);
	cmd_port |= (1<<E);
	_delay_us(12);
	cmd_port &= ~(1<<E);
	_delay_us(2);
	cmd_port &= ~(1<<DI);
	_delay_us(2);
}

void lcd_init()
{
	data_ddr = 0xFF;
	cmd_ddr |= (1<<DI)|(1<<RW)|(1<<E)|(1<<DCS1)|(1<<DCS2)|(1<<RST);
	cmd_port |= (1<<RST);
	cmd_port &=~((1<<DCS2)|(1<<DCS1));
	cmd_port |= (1<<DCS1);
	lcd_cmd(0x3F);
	cmd_port &=~((1<<DCS2)|(1<<DCS1));
	cmd_port |= (1<<DCS2);
	lcd_cmd(0x3F);
	max_value_voltage = 0;
	max_value_ampers = 0;
	tiks = 0;
}

void lcd_clr(){
	cmd_port |= (1<<DCS2)|(1<<DCS1);
	set_x(0);
	set_y(0);
	for (int x = 0; x < 8; x++)
	{
		set_x(x);
		for (int y = 0; y < 64; y++)
		{
			set_y(y);
			lcd_data(0x00);
		}
	}
	
}

void set_x(unsigned char pos_x)
{
	data_port = pos_x|0xB8;
	_delay_us(2);
	cmd_port |= (1<<E);
	_delay_us(12);
	cmd_port &= ~(1<<E);
	_delay_us(2);
}

void set_y(unsigned char pos_y)
{
	data_port = pos_y|0x40;
	_delay_us(2);
	cmd_port |= (1<<E);
	_delay_us(12);
	cmd_port &= ~(1<<E);
	_delay_us(2);
}

void lcd_set_bar(unsigned char sector, uint16_t vvalue){
	if(sector == 2){
		cmd_port &= ~(1<<DCS1);
		cmd_port |= (1<<DCS2);
	}
	else{
		cmd_port &= ~(1<<DCS2);
		cmd_port |= (1<<DCS1);
	}
	set_x(6);
	set_y(sector == 2 ? 0 : 1);
	
	int val = (sector == 2 ? vvalue/7.8 : vvalue/47.9);
	
	int m = (sector == 2 ? max_value_ampers/7.8 : max_value_voltage/47.9);
	for (int i = 0; i < 62; i++){
		if(val >= i)
			lcd_data(0x7E);
		else
			lcd_data(0x42);
			
		if(m == i)
		{
			lcd_data(0x7E);
			if (sector == 1)
				mv_old_y = (i == 0 ? 1: i);
			else
				ma_old_y = i;
		}
	}
}

void lcd_set_max(uint16_t voltage, uint16_t ampers){
	if (max_value_voltage < voltage)
	{
		max_value_voltage = voltage;
		tiks = 0;
	}
		
	if (max_value_ampers < ampers)
	{
		tiks = 0;
		max_value_ampers = ampers;
	}
	
	if(tiks == 40){
		max_value_ampers = 0;
		max_value_voltage = 0;
		tiks = 0;
		lcd_reset_max_values();
	}
	
}

void lcd_reset_max_values(){
	cmd_port &= ~(1<<DCS1);
	cmd_port |= (1<<DCS2);
	set_x(6);
	
	set_y(ma_old_y);
	lcd_data(0x42);
	ma_old_y = 0;
	
	cmd_port &= ~(1<<DCS2);
	cmd_port |= (1<<DCS1);
	set_x(6);
	
	set_y(mv_old_y);
	lcd_data(0x42);
	mv_old_y = 0;
}

void lcd_s(unsigned char sector, unsigned char pos_x, unsigned char pos_y, unsigned char abc)
{
	if (sector == 1)
		lcd_send_block(pos_x, pos_y, 8, (uint16_t)s+abc*8);
	else
		lcd_send_block(pos_x, pos_y + 63, 8, (uint16_t)s1+abc*8);
	
	_delay_ms(2);
}

void lcd_numbers(unsigned char sector, unsigned char pos_x, unsigned char pos_y, unsigned char abc)
{
	lcd_send_block(pos_x, (sector == 2 ? pos_y += 64 : pos_y), 8, (uint16_t)numbers+abc*8);
	_delay_ms(2);
}


uint8_t lcd_send_block(uint8_t x,uint8_t y, uint8_t len, uint16_t addr)
{
	uint8_t i,col;

	if(y>63)
	{
		cmd_port &= ~(1<<DCS1);
		cmd_port |= (1<<DCS2);
		col = y-64;
	}
	else
	{
		cmd_port &= ~(1<<DCS2);
		cmd_port |= (1<<DCS1);
		col = y;
	}

	set_x(x); 
	set_y(col);
	
	for(i=0;i!=len;i++,addr++,col++)
	{
		if(64==col)	
		{
			if(cmd_port == (1<<DCS2))
				return 128;

			col=0;				
			cmd_port &= ~(1<<DCS1);
			cmd_port |= (1<<DCS2);
			set_x(x); 
			set_y(col);
		}

		lcd_data(pgm_read_byte(addr));
	}
	return y+len;
}

void lcd_big_number(unsigned char sector, uint8_t x, uint8_t y, uint16_t number)
{
	uint8_t i,col,addr;
	addr= number*24;
	if(sector == 2)
	{
		cmd_port &= ~(1<<DCS1);
		cmd_port |= (1<<DCS2);
		col = y;
	}
	else
	{
		cmd_port &= ~(1<<DCS2);
		cmd_port |= (1<<DCS1);
		col = y;
	}
	set_x(x);
	set_y(col);
	
	for(i=0;i<24;i++,addr++,col++)
	{
		if (i == 12)
		{
			col=y;
			set_x(x+1);
		}
		set_y(col);
		lcd_data(pgm_read_byte((uint16_t)BIGNUMBERS+addr));
	}	
	
}

void lcd_putc(unsigned char x,unsigned char y,char *string)
{
	uint8_t sym;

	while (*string!='\0')
	{
		if(127<y)
		{
			y=0;
			x++;
			if(x>7) break;
		}

		sym = *string-0x20;
		y = lcd_send_block(x,y+1,5,(uint16_t)symboltable+sym*5);
		string++;
	}
}
