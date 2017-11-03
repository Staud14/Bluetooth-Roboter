/*
 * roboter_bluetooth.c
 *
 * Created: 21.10.2017 10:37:24
 *  Author: patri
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "roboter_bluetooth.h"


void bluetooth_init(void)
{
	//Activate the send and receive pins
	UCSR1B |= (1 << TXEN1) | (1 << RXEN1);
	
	//Interrupt enablen
	UCSR1B |= (1 << RXCIE1);
	
	//Serial infterface configuration
	UCSR1C = 0b00000110;		//0b 00						00			011					0 
								//   Asynchronos Usart		Parity		8Bit receive		clock parity
								
	UBRR1H = (uint8_t)(UART_BAUD_RATE_CALC(UART_BAUD_RATE,F_CPU) >> 8);
	UBRR1L = (uint8_t)UART_BAUD_RATE_CALC(UART_BAUD_RATE,F_CPU);
	
	//Handshake should be programmed
	
}

unsigned int bprintf(char trans)
{
	while( !(UCSR1A & (1<<UDRE1)));		//warte bis UDR bereit		UDRE in UCSR1A = 0 wenn sende puffer leer
	UDR1 = trans;
	return TRANSMITTED;
}

unsigned char bscanf(void)
{
	while ( !(UCSR1A & ( 1 << RXC1)));				//auf zeichen warten
	return UDR1;
}

