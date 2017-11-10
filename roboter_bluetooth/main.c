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
#include <stdio.h>

#include "header/bluetooth_header/roboter_bluetooth.h"
#include "header/drive_header/roboter_drive.h"
#include "header/I2C_header/roboter_I2C.h"

#define STATE_MOTOR_RIGHT			1
#define STATE_MOTOR_LEFT			2

#define STATE_BLINKER_RIGHT_ON		3
#define STATE_BLINKER_LEFT_ON		4

#define STATE_BLINKER_RIGHT_OFF		5
#define STATE_BLINKER_LEFT_OFF		6

#define STATE_EMERGGENCY_FLASHER	7
#define STATE_HORN					8  



volatile unsigned char bReceive = 0;
volatile unsigned char akkuLevel, newAkkuLevel, firstCheck;


union ADC_frame
{
	unsigned int input;
	
	struct output
	{
		unsigned char lsb;
		unsigned char msb;
	}OUTPUT;
	
};




int main(void)
{	
	volatile unsigned char state = 0;
	
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
		
		//bprintf(bReceive);
		
		
		//Reads the wished state from the received Data		
		if((bReceive & MASK_SELECT) == MOTR)
		{
			state = STATE_MOTOR_RIGHT;
		}
		else if((bReceive & MASK_SELECT) == MOTL)
		{
			state = STATE_MOTOR_LEFT;
		}
		
		
		//State Machine for the different controls
		switch(state)
		{
			case STATE_MOTOR_RIGHT:					drive(MOTR, ((bReceive & MASK_PWM) + 62));
													bReceive = 0;
													state = 0;
													break;
										
			case STATE_MOTOR_LEFT:					drive(MOTL, ((bReceive & MASK_PWM) + 62));
													bReceive = 0;
													state = 0;
													break;
										
			case STATE_BLINKER_RIGHT_ON:				
													break;
										
			case STATE_BLINKER_LEFT_ON:	
													break;
													
			case STATE_BLINKER_RIGHT_OFF:
													break;
													
			case STATE_BLINKER_LEFT_OFF:
													break;
										
			case STATE_EMERGGENCY_FLASHER:			
													break;
													
			case STATE_HORN:						
													break;
		}
		
		
		
		{
			volatile union ADC_frame adc;
			//adc.input = (0b00000001 << 8) + 0b00000000;					//testing lsb first or msb first
			adc.input = adc_measure(MEASURE_UB);
			bprintf(adc.OUTPUT.msb);
		}
	}
}


ISR(USART1_RX_vect)
{
	bReceive = UDR1;
}

