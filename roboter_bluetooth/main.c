/*
 * roboter_bluetooth.c
 *
 * Created: 21.10.2017 10:45:37
 */ 

#define F_CPU 16000000UL




#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>

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
volatile unsigned char control_peripherial = 0;

//Function Prototypes
void timer_init_1(void);
void sendADC(void);


int main(void)
{	
	JTAG_DISABLE();
	
	volatile unsigned char state = 0;
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
	
	drive(MOTR, 0);
	drive(MOTL, 0);
	
	LCD_cmd(LCD_INIT);
	LCD_cmd(LCD_CLEAR);
	
	
	sei();
/*
//Kalibrierungs Hilfe	
	unsigned char akku_in;
	char akku_let[8];
	
	while(1)
	{
		akku_in = (adc_measure(MEASURE_UB) >> 2 );
		sprintf(akku_let, "%d",akku_in);
		LCD_string(akku_let);
		_delay_ms(2000);
		LCD_cmd(LCD_CLEAR);
		akkuzustand();
	}
*/
	
    while (1) 
    {	
		//Reads the wished state from the received Data		
		if((char)(bReceive & MASK_SELECT) == MOTR)
		{
			state = STATE_MOTOR_RIGHT;
		}
		else if((char)(bReceive & MASK_SELECT) == MOTL)
		{
			state = STATE_MOTOR_LEFT;
		}
		else if((char)(bReceive & MASK_SELECT) == MASK_PERIPHERIAL)
		{			
			if((char)(bReceive & MASK_PERIPHERIAL_HORN) == MASK_PERIPHERIAL_HORN)
			{
				state = STATE_HORN_ON;
			}
			else if( (char)(bReceive & HORN) == (char)0 )
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
		sendADC();
		akkuzustand();

	}
}


void sendADC(void)
{
	int adc_value = adc_measure(MEASURE_UB);	//Measuring the Battery Voltage
	bprintf((char) (adc_value>>2));				//Upper byte
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
	
	if((char)(control_peripherial & (char)BLINKER_RIGHT) == (char)(BLINKER_RIGHT))
	{
		PORTB ^= (1 << LED_RH) ^ (1 << LED_RV);
		//control_peripherial = 0x00;
	}
	else//if((char)(control_peripherial & (char)BLINKER_RIGHT) == (char)(~BLINKER_RIGHT))
	{
		PORTB &= ~(1 << LED_RH) & ~(1 << LED_RV);
		//control_peripherial = 0x00;
	}
	
	
	if((char)((char)control_peripherial & (char)BLINKER_LEFT) == (char)(BLINKER_LEFT))
	{
		PORTB ^= (1 << LED_LH) ^ (1 << LED_LV);
		//control_peripherial = 0x00;
	}
	else//if((char)((char)control_peripherial & (char)BLINKER_LEFT) == (char)(~BLINKER_LEFT))
	{
		PORTB &= ~(1 << LED_LH)  & ~(1 << LED_LV);
		//control_peripherial = 0x00;
	}
		
}