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

long int counter_timer = 0;


void drive(unsigned char dir_motr, unsigned char pwm_rechts, unsigned char dir_motl, unsigned char pwm_links)
{

	// MOTOR RECHTS
	if (dir_motr == 2)				//Motor rechts Stopp
	{
		OCR4B = 0;					//rechts PWM=0
	}
	else if (dir_motr == 1)			//rechts mit PWM vorwärts
	{
		if ((pwm_rechts>=PWM_R_STOPP)&&(pwm_rechts<PWM_R_VOR_MAX+1)) {OCR4B = pwm_rechts;}
		else {OCR4B = PWM_R_VOR_MAX;}			//vollgas rechts vor
	}
	else
	{
		if ((pwm_rechts<=PWM_R_STOPP)&&(pwm_rechts>PWM_R_RET_MAX-1)) {OCR4B = pwm_rechts;}			//rechts mit PWM retour
		else {OCR4B = PWM_R_RET_MAX;}			//vollgas rechts retour
	}

	// MOTOR LINKS
	if (dir_motl == 2)				//Motor links Stopp
	{
		OCR4D = 0;					//links PWM=0
	}
	else if (dir_motl == 1)			//links mit PWM vorwärts
	{
		if ((pwm_links>PWM_L_VOR_MAX-1)&&(pwm_links<=PWM_L_STOPP)) {OCR4D = pwm_links;}
		else {OCR4D = PWM_L_VOR_MAX;}		//vollgas links  vor
	}
	else
	{
		if ((pwm_links>=PWM_L_STOPP)&&(pwm_links<PWM_L_RET_MAX+1)) {OCR4D = pwm_links;}	//links mit PWM retour
		else {OCR4D = PWM_L_RET_MAX;}		//vollgas links retour
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
	
	
	//Timer 0 for _delay_ms and _delay_us
	
	TCCR0A = 0x00;
	TCCR0B |= (1<<CS01);
	TCNT0 = PRELOAD_TIMER0;
	TIMSK0 |= (1 << TOIE0);


	
	akkuzustand();						//ein paar mal messen damit ADC warm läuft
	akkuzustand();
	akkuzustand();
	akkuzustand();
	akkuzustand();
}

void _delay_ms(long int _ms)
{
	while(counter_timer != (_ms*1000));
}

void _delay_us(long int _us)
{
	while(counter_timer != _us);
}


ISR(TIMER0_OVF_vect)												//Interrrupt sub routine timer 0 (8bit Timer)
{
	TCNT0 = PRELOAD_TIMER0;
	
	counter_timer++;
	if (counter_timer > 4294967290)
	{
		counter_timer = 4294967290;
	}
	
}