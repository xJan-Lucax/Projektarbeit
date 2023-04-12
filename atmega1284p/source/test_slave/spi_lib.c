/*************************************************************************
* Title		: SPI master Implementation
* Author	: Dimitri Dening
* Created	: 25.10.2021 17:56:24
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
    
USAGE:
	
NOTES:
	                   
*************************************************************************/
/* General libraries */
#include <avr/interrupt.h>

/* User defined libraries */
#include "spi_lib.h"
#include "led_lib.h"

void spi_init(data_order_t data_order, clock_polarity_t clock_polarity, clock_phase_t clock_phase){
	
	/* Set MISO output, all others input */
	DDRB = (1 << SPI_MISO);

	/* Enable SPI Interrupt Flag, SPI, Data Order, Clock Polarity, Clock Phase */	
	SPCR = (1 << SPIE) | (1 << SPE) | (data_order << DORD) | (clock_polarity << CPOL) | (clock_phase << CPHA);
	
	/* Pin Configuration for the new slave */
	PORTB |= (1 << PORTB4); // Pull High := not active
	DDRB  |= (1 << DDB4);   // @Output
	
	sei(); // global interrupt enable
	
	uint8_t dump;
	dump = SPDR;
	dump = SPDR;
}

uint8_t data[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
volatile uint8_t i = 0;

ISR(SPI_STC_vect){
	
	if (i > 7) i = 0; 
	SPDR = data[i];
	data[i] = SPDR; 
	i++;
}