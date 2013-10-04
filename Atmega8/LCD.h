/*
 * LCD.h
 *
 * Created: 30.09.2013 23:32:24
 *  Author: dr.morz
 */ 


#ifndef LCD_H_
#define LCD_H_
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define data_port PORTD
#define data_ddr DDRD
#define cmd_port PORTB
#define cmd_ddr DDRB
#define DCS1 0
#define DCS2 1
#define DI 2
#define RW 3
#define E 4
#define RST 5

uint16_t max_value_voltage;
uint16_t max_value_ampers;
uint16_t tiks;


void lcd_cmd(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_init();
void set_x(unsigned char pos_x);
void set_y(unsigned char pos_y);
void lcd_s(unsigned char sector, unsigned char pos_x, unsigned char pos_y, unsigned char abc);
void lcd_numbers(unsigned char sector, unsigned char pos_x, unsigned char pos_y, unsigned char abc);
uint8_t lcd_send_block(uint8_t x,uint8_t y, uint8_t len, uint16_t addr);
void lcd_putc(unsigned char x,unsigned char y,char *string);
void lcd_big_number(unsigned char sector, uint8_t x,uint8_t y, uint16_t number);
void lcd_set_bar(unsigned char sector, uint16_t vvalue);
void lcd_set_max(uint16_t voltage, uint16_t ampers);
void lcd_reset_max_values();

#endif /* LCD_H_ */