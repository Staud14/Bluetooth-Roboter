/*
 * roboter_bluetooth.c
 *
 * Created: 21.10.2017 10:45:37
 * Author : patri
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "header/bluetooth_header/roboter_bluetooth.h"
#include "header/drive_header/roboter_drive.h"
#include "header/I2C_header/roboter_I2C.h"

unsigned char bReceive = 0;

int main(void)
{
	
	JTAG_DISABLE();
    /************************************/
    /* I2C - IO - Expander				*/
    /************************************/

    DDRC |= IO_RESET;					//PC6 ist #RST vom Portexpander
    PORTC |= IO_RESET;					//Portexpander ein

    TWBR = 12;							//TWBR=12, TWPS=0 im Reg. TWSR per default, damit f_SCL = 400 kHz
    
    I2C_init();							//I2C Initialisierung
	LCD_init();
    roboter_init();
	bluetooth_init();
	
	sei();
	
	drive(MOT_FAST_STOPP, PWM_R_STOPP, MOT_FAST_STOPP, PWM_L_STOPP);
	
	LCD_data( 'x' );
	_delay_ms(1000);
	LCD_cmd(LCD_CLEAR);
	
	
    while (1) 
    {		
<<<<<<< HEAD
		
		//bprintf('a');
		
		
			LCD_data(bReceive);	
			
			bprintf(bReceive);
			
			bReceive=0;
			
			
			
		
=======
		if(bReceive != 0)
		{
			bprintf(bReceive);
			LCD_data(bReceive);
			_delay_ms(10000);
			LCD_cmd(LCD_CLEAR);
			LCD_data(bscanf());
			_delay_ms(10000);
			LCD_cmd(LCD_CLEAR);
			bReceive=0;
		}
>>>>>>> 5cd340369b21e328ce2b491afa665b3cb6d06015
    }
}


ISR(USART1_RX_vect)
{
	bReceive = UDR1;
}
