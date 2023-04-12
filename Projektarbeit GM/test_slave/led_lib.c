/*************************************************************************
* Title		: led_lib.c
* Author	: Dimitri Dening
* Created	: 25.11.2021 21:42:39
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
    
USAGE:
	
NOTES:
	                   
*************************************************************************/
/* General libraries */
#include <avr/io.h>
#include <stdint.h>

/* User defined libraries */
#include "led_lib.h"

void led_init(void){
	
	/* Set all led pins @output*/
	LED_DDR = 0xff;
	
	/* All led pins are driven high (not active) */
	LED_PORT = 0xff;
}

void led_activate(uint8_t led){
	LED_PORT &= ~(1 << led);
}

void led_deactivate(uint8_t led){
	LED_PORT |= (1 << led);
}

void led_toggle(uint8_t led){
	LED_PORT ^= (1UL << led);
}

void led_counter(uint8_t byte){
	led_deactivate(LED0);
	led_deactivate(LED1);
	led_deactivate(LED2);
	
	if (byte & (1 << LED0)) led_activate(LED0);
	if (byte & (1 << LED1)) led_activate(LED1);
	if (byte & (1 << LED2)) led_activate(LED2);
}
