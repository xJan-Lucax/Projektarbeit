/*************************************************************************
* Title		: uart.c
* Author	: Dimitri Dening
* Created	: 10.12.2021 17:48:20
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
	Simple uart implementation for the ATmega1284P to support debugging sessions.        
*************************************************************************/
/* General libraries */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* User defined libraries */
#include "uart.h"

#define BAUDRATE 38400UL
#define UART_OUTPUT_DEBUG 1

void uart_init (void){
	uint16_t ubrr = (uint16_t) ((uint32_t) (F_CPU/(16*BAUDRATE)) - 1);
	
	/* Set baud rate */
	UBRR0H = (uint8_t) (ubrr >> 8);
	UBRR0L = (uint8_t) (ubrr);
	/* Enable receiver and transmitter */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	/* Set frame format: 8 bit data, 2 stop bit */
	UCSR0C = (1 << USBS0) | (3 << UCSZ00);
	
	volatile dump;
}

void uart_putc (char c){
	#if UART_OUTPUT_DEBUG
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1 << UDRE0)));

	/* Put data into buffer, sends the data */
	UDR0 = c;
	#endif
}

void uart_puts(char *s){
	#if UART_OUTPUT_DEBUG
	while (*s){
		uart_putc(*s++);
	}
	#endif
}

void uart_putn(uint8_t num){
	#if UART_OUTPUT_DEBUG
	uint8_t temp = num;
	char string[8];
	
	itoa(temp, string, 10);
	uart_puts(string);
	#endif
}

void uart_put(const char* format, ...){
	#if UART_OUTPUT_DEBUG
	char buffer[256];

	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, 256, format, ap);
	va_end(ap);

	uart_puts(buffer);
	uart_puts("\r\n");
	#endif
}