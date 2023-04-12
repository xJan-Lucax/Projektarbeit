/*************************************************************************
* Title		: spi_lib.h include file for SPI slave interface
* Author	: Dimitri Dening
* Created	: 25.10.2021 17:56:35
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
    
USAGE:
	
NOTES:
	                   
*************************************************************************/

#ifndef SPI_LIB_H_
#define SPI_LIB_H_

/* General libraries */

/* User defined libraries */
#include "spi_io.h"

typedef enum {
	SPI_MSB = 0x00,
	SPI_LSB = 0x01
} data_order_t;

typedef enum {
	SPI_SCK_LOW = 0x00,
	SPI_SCK_HIGH = 0x01
} clock_polarity_t;

typedef enum {
	SPI_SCK_LEADING = 0x00,
	SPI_SCK_TRAILING = 0x01
} clock_phase_t;

typedef enum {
	SPI_CLOCK_DIV4 = 0x00,
	SPI_CLOCK_DIV16 = 0x01,
	SPI_CLOCK_DIV64 = 0x02,
	SPI_CLOCK_DIV128 = 0x03,
	// The following clock settings will set the SPI2X bit in the SPSR.
	// The SCK frequency will be doubled when the SPI is in master mode.
	SPI_CLOCK_DIV2 = 0x04,
	SPI_CLOCK_DIV8 = 0x05,
	SPI_CLOCK_DIV32 = 0x06,
	SPI_CLOCK_DIV64X = 0x07
} clock_rate_t;

/* 
DESCRIPTION:
	Initializes the SPI as slave.
	
ARGS:
	Check the <spi_io.h> file for the entire valid argument list.

RETURN:
	None

EXAMPLE:
	spi_init(SPI_LSB, SPI_SCK_HIGH, SPI_SCK_TRAILING);
*/
void spi_init(data_order_t data_order, clock_polarity_t clock_polarity, clock_phase_t clock_phase);

#endif /* SPI_LIB_H_ */