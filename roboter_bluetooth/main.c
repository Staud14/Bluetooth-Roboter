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
//#include <stdio.h>
//#include <util/delay.h>

#include "header/bluetooth_header/roboter_bluetooth.h"
#include "header/drive_header/roboter_drive.h"
#include "header/I2C_header/roboter_I2C.h"

//States
#define STATE_MOTOR_RIGHT				1
#define STATE_MOTOR_LEFT				2
#define STATE_HORN_ON					3
#define STATE_HORN_OFF					4

//Defines for Peripherial
#define MASK_PERIPHERIAL				0xc0
#define MASK_PERIPHERIAL_HORN			0xc4

#define BLINKER_RIGHT					(1 << 0)
#define BLINKER_LEFT					(1 << 1)
#define HORN							(1 << 2)



//Defines for 16-Bit Timer1
#define PRELOAD_TIMER1 49911				//Preload 49911 and Prescaler 64 for 1s


//Global Variables
volatile unsigned char bReceive = 0;
volatile unsigned char akkuLevel, newAkkuLevel, firstCheck;
volatile unsigned char control_peripherial = 0;

//Function Prototypes
void timer_init_1(void);


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
	
	volatile unsigned char state = 0;
	volatile union ADC_frame adc;
	//int x = 0;
	
	
    /************************************/
    /* I2C - IO - Expander				*/
    /************************************/

    DDRC |= IO_RESET;					//PC6 ist #RST vom Portexpander
    PORTC |= IO_RESET;					//Portexpander ein

    TWBR = 12;							//TWBR=12, TWPS=0 im Reg. TWSR per default, damit f_SCL = 400 kHz
    //_________________________________________________________________________________________
	
	
    roboter_init();
	bluetooth_init();
	I2C_init();							//I2C Initialisierung
	LCD_init();
	timer_beeper_init();
	timer_init_1();
	
#ifdef ADC_INTERRUPT
	adc_init();
#endif

	drive(MOTR, 0);
	drive(MOTL, 0);
	
	sei();
/*	
	for(x = 0; x < 10; x++)
	{
		PORTB ^= (1 << LED_RH) | (1 << LED_RV);
		PORTB ^= (1 << LED_LH) | (1 << LED_LV) | (1 << LED_ROT) | (1 << LED_GRUEN);
		_delay_ms(500);	
	}
	PORTB &= ~(1 << LED_LH) | ~(1 << LED_LV);
	PORTB &= ~(1 << LED_RH) | ~(1 << LED_RV) | ~(1 << LED_ROT) | ~(1 << LED_GRUEN);
	
	timer_beep_tone(125);
	_delay_ms(10000);
	timer_beep_stop();	
*/
/*
	PORTB |= (1<<LED_LH) | (1<<LED_LV);
	_delay_ms(1000);
	PORTB &= ~(1 << LED_LH)  & ~(1 << LED_LV);
*/	
	
	
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
		else if((bReceive & MASK_SELECT) == MASK_PERIPHERIAL)
		{			
			if((bReceive & MASK_PERIPHERIAL_HORN) == MASK_PERIPHERIAL_HORN)
			{
				state = STATE_HORN_ON;
			}
			else if(((bReceive & MASK_SELECT) == MASK_PERIPHERIAL)		&&		((bReceive & HORN) == 0))
			{
				state = STATE_HORN_OFF;
			}
			control_peripherial = bReceive;
			bReceive = 0x00;
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
													
			case STATE_HORN_ON:						timer_beep_tone(200);
													break;
												
			case STATE_HORN_OFF:					timer_beep_stop();
													break;									
			
		}
		
			
			//adc.input = (0b00000001 << 8) + 0b00000000;					//testing lsb first or msb first
			adc.input = adc_measure(MEASURE_UB);
			bprintf(adc.OUTPUT.msb);
	}
	
#ifndef ADC_INTERRUPT
	akkuzustand();
#endif
}


//Functions
void timer_init_1(void)
{
	TCCR1A = 0x00;													//Standard Timer Mode
	TCCR1B |= (1<<CS11) | (1<<CS10);								//Prescaler
	TCNT1 = PRELOAD_TIMER1;											//Preload
	TIMSK1 = (1 << TOIE1);											//Timer Interrupt enable
}

//Interrupts Routines
ISR(USART1_RX_vect)
{
	bReceive = UDR1;
}

ISR(TIMER1_OVF_vect)												//Interrrupt sub routine timer 1 (16bit Timer)
{
	TCNT1 = PRELOAD_TIMER1;
	
	if((control_peripherial & BLINKER_RIGHT) == (BLINKER_RIGHT))
	{
		PORTB ^= (1 << LED_RH) ^ (1 << LED_RV);
		control_peripherial = 0x00;
	}
	else if((control_peripherial & BLINKER_RIGHT) == (~BLINKER_RIGHT))
	{
		PORTB &= ~(1 << LED_RH) & ~(1 << LED_RV);
		control_peripherial = 0x00;
	}
	
	
	if((control_peripherial & BLINKER_LEFT) == (BLINKER_LEFT))
	{
		PORTB ^= (1 << LED_LH) ^ (1 << LED_LV);
		control_peripherial = 0x00;
	}
	else if((control_peripherial & BLINKER_LEFT) == (~BLINKER_LEFT))
	{
		PORTB &= ~(1 << LED_LH)  & ~(1 << LED_LV);
		control_peripherial = 0x00;
	}
		
}