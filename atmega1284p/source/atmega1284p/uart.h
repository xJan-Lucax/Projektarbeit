/*************************************************************************
* Title		: uart.h
* Author	: Dimitri Dening
* Created	: 10.12.2021 17:48:20
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
	Simple uart implementation for the ATmega1284P to support debugging sessions.                
*************************************************************************/
#ifndef UART_H
#define UART_H

#include <avr/io.h>

void uart_init (void);

void uart_puts (char *s);

void uart_putc(char c);

void uart_putn(uint8_t num);

void uart_put(const char* format, ...);

#endif /* UART_H  */