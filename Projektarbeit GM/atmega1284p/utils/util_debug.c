/*************************************************************************
* Title		: Debugging Utility File
* Author	: Dimitri Dening
* Created	: 25.10.2021 17:21:52
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P

DESCRIPTION:
	                   
*************************************************************************/
#include "util_debug.h"

#define  DEBUG_SIZE 100

int debug_putchar(char c, FILE *stream)
{
	(void) stream; //unused
	volatile static char debug[DEBUG_SIZE];
	static uint8_t ptr = 0;
	debug[ptr++] = c;
	if (c < 32 || ptr == DEBUG_SIZE) {
		ptr = 0;
		(void) debug;
	}
	return 0;
}