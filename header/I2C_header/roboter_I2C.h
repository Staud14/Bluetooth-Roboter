/*
 * roboter_I2C.h
 *
 * Created: 21.10.2017 11:06:05
 *  Author: patri
 */ 


#ifndef ROBOTER_I2C_H_
#define ROBOTER_I2C_H_

/////////////////////////////////////////////////////////////////////////////
//
// 32U4 + Portexpander MCP23008	+ LCD       	f_CPU = 16 MHz
//												f_SCL = 400 kHz
//
// ------------------------- I2C -------------------------------------------
// Beschaltung:
// LCD	MCP233008
// DB7	GP3
// DB6	GP2
// DB5	GP1
// DB4	GP0
// E    GP5
// RW	GND
// RS	GP4
//
//               f_CPU
// f_SCL = ---------------------  => TWBR=12 mit TWPS=0
//         16 + 2*TWBR * 4^TWPS
//
// Bem.: f_CPU muss min. 16*f_SCL sein
//       TWBR muss Wert > 10 haben
//
// MCP23008 Adr.: 0100 000  (A2=A1=A0=0)
//          BANK=1, Byte Mode
//
//
// ---------------------------- LCD ----------------------------------------
// Befehle: LCD_init(), LCD_cmd(char data), LCD_send(char data)
//          LCD_string(char *data)
//
// LCD_init();               initialisiert Port D
//                           und LCD im 4-Bit Mode, 2 Zeilen, 5x7 Dots
//                           Bsp.: LCD_init();
//
// LCD_cmd(char data);       schickt Befehl ans LCD
//                           Bsp.: LCD_cmd(0xC5);  //gehe zu 2. Zeile, 6. Position
//
// LCD_data(char data);      schickt Daten ans LCD
//                           Bsp.: LCD_data(0xEF); //sendet ein ö
//
// LCD_string(char *data);   schickt eine Zeichenkette ans LCD
//                           Bsp.: LCD_string("Hallo");    //sendet Hallo
//
////////////////////////////////////////////////////////////////////////////

#define I2C_ADDR 0b01000000								//MCP23008 Portexpander
#define E	5
#define RS	4
#define DB4	0
#define DB5	1
#define DB6	2
#define DB7	3

#define LCD_INIT	0x0C
#define LCD_CLEAR	0x01

#define IO_RESET		(1<<6)


void I2C_init(void);
void LCD_init(void);
void write_i2c(char addr, char data);
void LCD_data(char data);
void LCD_cmd(char cmd);
void LCD_string(char *data);


#endif /* ROBOTER_I2C_H_ */