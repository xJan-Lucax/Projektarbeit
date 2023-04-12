/*************************************************************************
* Title		: ATmega1284P Test Slave
* Author	: Dimitri Dening
* Created	: 28.11.2021 14:25:17
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
    
USAGE:
	
NOTES:
	                   
*************************************************************************/
/* General libraries */
#include <avr/interrupt.h>
#include <util/delay.h>

/* Peripheral libraries */
#include "spi_lib.h"

/* User defined libraries */
#include "led_lib.h"

int main(void){
	
	cli(); // disable all interrupts first, workaround for garbage spi packages 
	
	led_init();
	
	/* INIT */
	spi_init(SPI_MSB, SPI_SCK_LOW, SPI_SCK_LEADING);
	
    while (1) {
	}
}

