/*
 * roboter_bluetooth.c
 *
 * Created: 21.10.2017 10:45:37
 * Author : patrik
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>

#include "header/bluetooth_header/roboter_bluetooth.h"
#include "header/drive_header/roboter_drive.h"
#include "header/I2C_header/roboter_I2C.h"

unsigned char bReceive = 0;

struct adctobyte
{
	unsigned int L : 8;
	unsigned int U : 2;
}
union ADC
{
	unsigned int in;
	struct adctobyte out 10;
}




int main(void)
{	
	JTAG_DISABLE();
    /************************************/
    /* I2C - IO - Expander				*/
    /************************************/

    DDRC |= IO_RESET;					//PC6 ist #RST vom Portexpander
    PORTC |= IO_RESET;					//Portexpander ein

    TWBR = 12;							//TWBR=12, TWPS=0 im Reg. TWSR per default, damit f_SCL = 400 kHz
    
    roboter_init();
	bluetooth_init();
	I2C_init();							//I2C Initialisierung
	LCD_init();
	
	
	sei();
	
	
	
	drive(MOTR, 0);
	drive(MOTL, 0);	
	
    while (1) 
    {	
		
		bprintf(bReceive);
					
		if((bReceive & MASK_SELECT) == MOTR)
		{
			drive(MOTR, ((bReceive & MASK_PWM) + 62));
			bReceive = 0;
		}
		else if((bReceive & MASK_SELECT) == MOTL)
		{
			drive(MOTL, ((bReceive & MASK_PWM) + 62));
			bReceive = 0;
		}
		{
			union ADC adc;
			adc.in = (0b00000001 << 8) + 0b00000000;
			//adc.in = adc_measure(MEASURE_UB);
			bprintf(adc.out.L);
		}
	}
}


ISR(USART1_RX_vect)
{
	bReceive = UDR1;
}
