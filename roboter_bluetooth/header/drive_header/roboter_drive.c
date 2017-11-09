/*
 * roboter_drive.c
 *
 * Created: 21.10.2017 
 *  Author: Patrik Staudenmayer
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include "roboter_drive.h"

#ifndef SELF_DELAY
#include <util/delay.h>
#endif


#ifdef SELF_DELAY
long int counter_timer = 0;
#endif

volatile unsigned char akkuLevel, newAkkuLevel, firstCheck;


void drive(unsigned char select, unsigned char mot_pwm)
{

	// MOTOR RECHTS
	if (select == MOTR)					//Motor rechts
	{
		OCR4B = mot_pwm;				//rechts PWM
	}
	
	
	// MOTOR LINKS
	if (select == MOTL)				//Motor links
	{
		OCR4D = mot_pwm;				//links PWM
	}
}



unsigned int adc_measure(unsigned char channel)
{
	unsigned int result=0;

	ADMUX &= ~(1<<REFS1)&~(1<<REFS0);			//ext. AREF = 5V
	ADMUX &= ~(1<<ADLAR);						//rechttsbündig

	ADCSRB &= ~(1<<MUX5);
	ADMUX &= ~(1<<MUX4)&~(1<<MUX3);
	if (channel == 0) {ADMUX &= ~(1<<MUX2)&~(1<<MUX1)&~(1<<MUX0);}			//ADC0 single ended Messung Measure UB
	if (channel == 1) {ADMUX &= ~(1<<MUX2)&~(1<<MUX1);ADMUX |= (1<<MUX0);}	//ADC1 single ended Messung LF left  (IR Empfänger links)
	if (channel == 4) {ADMUX |=  (1<<MUX2);ADMUX &= ~(1<<MUX1)&~(1<<MUX0);}	//ADC4 single ended Messung LF right (IR Empfänger rechts)

	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);	//ADC einschalten, Teiler auf 128 -> 125kHz Samplingfrequenz

	ADCSRA |= (1<<ADSC);						//start
	while(ADCSRA&(1<<ADSC));					//warten auf Wandlungsende
	result = ADCW;
	return result;
}


void akkuzustand (void)
{
	unsigned int akku;
	unsigned char i;
	
	for (i=0;i<10;i++) {akku = adc_measure(MEASURE_UB);}
	if (akku >822)										//Akku voll,	 7,2V >= Vcc > 6,8V  grün
	{PORTB|=(1<<LED_GRUEN); PORTB&=~(1<<LED_ROT);}
	if ((akku<=822)&&(akku>744))						//Akku halbvoll, 6,8V >= Vcc > 6,4V  gelb
	{PORTB|=(1<<LED_GRUEN); PORTB|=(1<<LED_ROT);}
	if (akku<=744)										//Akku leer,	 6,4V >= Vcc		 rot +  ENDE
	{PORTB&=~(1<<LED_GRUEN); PORTB|=(1<<LED_ROT);
		cli();										//alle Interrupts aus
		TCCR4B &= 0xF0;							//alle Motoren stopp, Motor PWM aus
		PORTB  &= 0x30;							//alle LEDs am Ring aus, Beeper und Motor right aus
		PORTD  =  0x08;							//Motor left und alle IR-Sender aus
		while(1)
		{
			PORTB^=(1<<LED_ROT);				//rote Duo LED blinkt in Endlsschleife = E N D E !!!!
			_delay_ms(150);
		}
	}
}


void roboter_init(void)
{	
	CLKPR = 0x80;
	CLKPR = 0x00;						//CLK_IO = f_quarz = 16 MHz, Teiler 1
	

	//LED Pins
	DDRB = DDRB |(1<<DDB0)|(1<<DDB1)|(1<<DDB2)|(1<<DDB3);	//LED Pins
	//DDRD = DDRD |(1<<DDD2)|(1<<DDD3);						//Duo LED Pins		//Disabled for Bluetooth


	//Akkuspannung
	DDRF = DDRF &~(1<<MEASURE_UB);		//ADC0	PF0

	//Motoren Pins
	DDRB = DDRB | (1<<DDB6);			//PWM-Output OC4B für MOTOR_RECHTS
	DDRD = DDRD | (1<<DDD7);			//PWM-Output OC4D für MOTOR_LINKS

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
	OCR4B = 94;							//Tastverhältnis am OC4B-Pin (PB6), PWM_rechts (retour_max = 62, vor_max = 125, stopp = 94)
	OCR4D = 94;							//Tastverhältnis am OC4D-Pin (PD7), PWM_links  (retour_max = 62, vor_max = 125, stopp = 94)


	TCCR4B = TCCR4B | (1<<CS43);
	TCCR4B = TCCR4B &~(1<<CS42);
	TCCR4B = TCCR4B &~(1<<CS41);		//f_CLK_T4 = CLK_IO/Prescaler = 16MHz/256 = 62,5kHz
	TCCR4B = TCCR4B | (1<<CS40);		//Timer4 Prescaler = 1, Start PWM
	
#ifdef SELF_DELAY	
	//Timer 0 for _delay_ms and _delay_us
	
	TCCR0A = 0x00;
	TCCR0B |= (1<<CS01);
	TCNT0 = PRELOAD_TIMER0;
	TIMSK0 |= (1 << TOIE0);
#endif

	
	akkuzustand();						//ein paar mal messen damit ADC warm läuft
	akkuzustand();
	akkuzustand();
	akkuzustand();
	akkuzustand();
}


#ifdef SELF_DELAY

void _delay_ms(long int _ms)
{
	counter_timer=0;
	while(counter_timer < ((_ms * 1000) + 1));
}

void _delay_us(long int _us)
{
	counter_timer=0;
	while(counter_timer < (_us +1));
}


ISR(TIMER0_OVF_vect)												//Interrrupt sub routine timer 0 (8bit Timer)
{
	TCNT0 = PRELOAD_TIMER0;
	
	counter_timer++;
	if (counter_timer > 4294967290)				//2^32-6			Making sure the timer doesn't overflow
	{
		counter_timer = 4294967290;
	}
	
}

#endif

ISR(ADC_vect)
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


}