/*
 * roboter_drive.c
 *
 * Created: 21.10.2017 
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include "roboter_drive.h"


#include <util/delay.h>



void drive(unsigned char select, unsigned char mot_pwm)
{

	// MOTOR RECHTS
	if (select == MOTR)					//Motor rechts
	{
		if(mot_pwm == 94)
		{
			mot_pwm = 0;
		}
		
		OCR4B = mot_pwm;				//rechts PWM
	}
	
	
	// MOTOR LINKS
	if (select == MOTL)				//Motor links
	{
		if(mot_pwm == 94)
		{
			mot_pwm = 0;
		}
		
		OCR4D = mot_pwm;				//links PWM
	}
}




unsigned int adc_measure(unsigned char channel)
{
	unsigned int result=0;

	ADMUX = 0;			//ext. AREF = 5V  rechttsb�ndig
							

	ADCSRB &= ~(1<<MUX5);
	
	ADMUX &= ~(1<<MUX4)&~(1<<MUX3);
	ADMUX |= channel;							//ADC0 single ended Messung Measure UB
												//ADC1 single ended Messung LF left  (IR Empf�nger links)
												//ADC4 single ended Messung LF right (IR Empf�nger rechts)

	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);	//ADC einschalten, Teiler auf 128 -> 125kHz Samplingfrequenz

	ADCSRA |= (1<<ADSC);						//start
	
	while(ADCSRA&(1<<ADSC));					//warten auf Wandlungsende
	result = ADCW;
	return result;
}

void akkuzustand (void)
{
	unsigned int akku_in;
	unsigned char i;
	unsigned char akku;
	
	for (i=0;i<10;i++) {akku_in = adc_measure(MEASURE_UB);}
	akku = (akku_in >> 2);
	
	if (akku > LEVEL1)												//Akku voll,	 8,4 >= Vcc > 7,6V  gr�n
	{
		PORTB |= (1<<LED_GRUEN); 
		PORTB &= ~(1<<LED_ROT);
	}
		
	else if ( (akku <= LEVEL1) && (akku > LEVEL2) )					//Akku halbvoll, 7,6 >= Vcc > 6,6V  rot
	{
		PORTB &= ~(1<<LED_GRUEN); 
		PORTB|=(1<<LED_ROT);
	}
		
	else if (akku <= LEVEL2)										//Akku leer,	 6,6V >= Vcc		 rot blinkend alle 4 sek +  ENDE
	{	PORTB &= ~(1<<LED_GRUEN); 
		PORTB|=(1<<LED_ROT);
		cli();														//alle Interrupts aus
		TCCR4B &= 0xF0;												//alle Motoren stopp, Motor PWM aus
		PORTB  &= 0x30;												//alle LEDs am Ring aus, Beeper und Motor right aus
		PORTD  =  0x08;												//Motor left und alle IR-Sender aus
		while(1)
		{
			PORTB |= (1<<LED_ROT);									//rote Duo LED blinkt in Endlsschleife = E N D E !!!!
			_delay_ms(400);
			PORTB &= ~(1<<LED_ROT);
			_delay_ms(3600);
		}
	}
}


void roboter_init(void)
{	
	CLKPR = 0x80;
	CLKPR = 0x00;						//CLK_IO = f_quarz = 16 MHz, Teiler 1
	

	//LED Pins
	DDRB = DDRB |(1<<DDB0)|(1<<DDB1)|(1<<DDB2)|(1<<DDB3);	//LED Pins
	
	//DDRD = DDRD |(1<<DDD2)|(1<<DDD3);						//Disabled for Bluetooth
	DDRB = 0xff;
	PORTB = 0;



	//Motoren Pins
	DDRB = DDRB | (1<<DDB6);			//PWM-Output OC4B f�r MOTOR_RECHTS
	DDRD = DDRD | (1<<DDD7);			//PWM-Output OC4D f�r MOTOR_LINKS

	//Timer 4 im Fast PWM Mode konfigurieren
	TCCR4A = TCCR4A | (1<<PWM4B);
	TCCR4C = TCCR4C | (1<<PWM4D);
	TCCR4D = TCCR4D &~(1<<WGM41);
	TCCR4D = TCCR4D &~(1<<WGM40);		//Fast PWM am OC4B und OC4D

	TCCR4A = TCCR4A &~(1<<COM4B0);
	TCCR4A = TCCR4A | (1<<COM4B1);		//COM4B1:0=2

	TCCR4C = TCCR4C &~(1<<COM4D0);
	TCCR4C = TCCR4C | (1<<COM4D1);		//COM4D1:0=2

	TC4H = 0x03;
	OCR4C = 0xE8;						//f_PWM = f_CLK_T4/(1+OCR4C) = 62,5kHz/1000 = 62,5 Hz
	TC4H = 0x00;
	OCR4B = 94;							//Tastverh�ltnis am OC4B-Pin (PB6), PWM_rechts (retour_max = 62, vor_max = 125, stopp = 94)
	OCR4D = 94;							//Tastverh�ltnis am OC4D-Pin (PD7), PWM_links  (retour_max = 62, vor_max = 125, stopp = 94)


	TCCR4B = TCCR4B | (1<<CS43);
	TCCR4B = TCCR4B &~(1<<CS42);
	TCCR4B = TCCR4B &~(1<<CS41);		//f_CLK_T4 = CLK_IO/Prescaler = 16MHz/256 = 62,5kHz
	TCCR4B = TCCR4B | (1<<CS40);		//Timer4 Prescaler = 1, Start PWM
	

	akkuzustand();						//nicht ben�tigt falls interrupt vorhanden
	akkuzustand();						//ein paar mal messen damit ADC warm l�uft
	akkuzustand();
	akkuzustand();
	akkuzustand();

}


void timer_beeper_init(void)
{
	//Timer0
	TCNT0 = 0;
	OCR0A = 0;
	
	TCCR0A |= (1<<COM0A0) | (1<<WGM01) | (1<<WGM00);
	TCCR0B |= (1<<WGM02);
}

void timer_beep_tone(unsigned int frequenz)
{
	OCR0A = frequenz;
	TCCR0B |= (1<<CS02);
}

void timer_beep_stop(void)
{
	TCCR0B &= ~(1<<CS02);
	TCNT0 = 0;
	OCR0A = 0;
}