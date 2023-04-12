/*************************************************************************
* Title		: spi_io.h
* Author	: Dimitri Dening
* Created	: 05.11.2021 19:39:26
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
    
USAGE:
	
NOTES:
	This file should only be included from <spi_lib.h>, never directly.
	                   
*************************************************************************/
#ifndef SPI_IO_H_
#define SPI_IO_H_

#include <avr/io.h>

/* SPI Port Declaration */
#if defined(__AVR_ATmega1284P__)
#	define SPI_SCK		PORTB7
#	define SPI_MOSI		PORTB5
#	define SPI_MISO		PORTB6
#	define SPI_SS		PORTB4
#else
#  if !defined(__COMPILING_AVR_LIBC__)
#    warning "ATmega1284P not found"
#  endif
#endif

#endif /* SPI_IO_H_ */