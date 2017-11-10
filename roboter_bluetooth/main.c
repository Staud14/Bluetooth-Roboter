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

#define TRUE            1
#define FALSE           0

#include "header/bluetooth_header/roboter_bluetooth.h"
#include "header/drive_header/roboter_drive.h"
#include "header/I2C_header/roboter_I2C.h"

unsigned char bReceive = 0;
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
			union ADC_frame adc;
			adc.input = (0b00000001 << 8) + 0b00000000;
			//adc.in = adc_measure(MEASURE_UB);
			bprintf(adc.OUTPUT.lsb);
		}
	}
}


ISR(USART1_RX_vect)
{
	bReceive = UDR1;
}

/*ISR(ADC_vect)
{
	unsigned int wait;
	unsigned char x, y;

	newAkkuLevel = ADCH;
	if (akkuLevel > newAkkuLevel)
	akkuLevel = newAkkuLevel;

	if (firstCheck)
	{
		firstCheck = FALSE;

		PORTB |= (1<<LED_LV)|(1<<LED_LH);
		PORTB |= (1<<LED_RV)|(1<<LED_RH);


		if (akkuLevel > 210)
		{
			x = (akkuLevel - 200) / 5;

			PORTB &= ~(1 << LED_GRUEN);
			for (y = 0; y < x; y++)
			{
				for(wait = 0; wait < 60000; wait++)  PORTB |=  (1 << LED_GRUEN);
				for(wait = 0; wait < 60000; wait++)  PORTB &= ~(1 << LED_GRUEN);
			}
		}
		else if (akkuLevel > 205)
		{

			for(wait = 0; wait < 60000; wait++)
			{
				PORTB |= (1 << LED_ROT);
				PORTB |= (1 << LED_GRUEN);
			}
		}
		else
		{
			for(wait = 0; wait < 60000; wait++)  PORTB |= (1 << LED_ROT);
		}
		PORTB &= ~(1 << LED_GRUEN);
		PORTB &= ~(1 << LED_ROT);
	}

	if (akkuLevel < 200)
	{

		TCCR4B &= 0xF0;		//alle Motoren stopp, Motor PWM aus
		PORTB  &= 0x30;		//alle LEDs am Ring aus, Beeper und Motor right aus
		PORTD  =  0x08;		//Motor left und alle IR-Sender aus

		cli();
		while(1)
		{
			for(wait = 0; wait < 60000; wait++)  PORTB |= (1 << LED_ROT);
			for(wait = 0; wait < 40000; wait++)  PORTB &= ~(1 << LED_ROT);
		}
		PORTB &= ~(1<<LED_LV) & ~(1<<LED_LH);
		PORTB &= ~(1<<LED_RV) & ~(1<<LED_RH);

	}


}*/