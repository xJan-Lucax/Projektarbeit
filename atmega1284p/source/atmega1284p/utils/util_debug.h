/*************************************************************************
* Title		: Include Debugging Utility File
* Author	: Dimitri Dening
* Created	: 25.10.2021 17:21:52
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P

DESCRIPTION:
	This file allows you to use the printf function and monitor the output console.
    
USAGE:
	Include this <util_debug.h> header file into your project.
	Paste following code snippet into your function, which you want to debug.
	
		static FILE debug_out = FDEV_SETUP_STREAM(debug_putchar, NULL, _FDEV_SETUP_WRITE);
		stdout = &debug_out;
		stderr = &debug_out;
		
	Follow this guide to setup the util_debug.c> file.
	https://www.mikrocontroller.net/topic/445097
	
NOTES:
	                   
*************************************************************************/
#ifndef UTIL_DEBUG_H_
#define UTIL_DEBUG_H_

#include <stdio.h>

int debug_putchar(char c, FILE *stream);

#endif /* UTIL_DEBUG_H_ */