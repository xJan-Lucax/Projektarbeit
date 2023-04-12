/*************************************************************************
* Title		: timer_lib.h
* Author	: Dimitri Dening
* Created	: 13.12.2021 09:06:40
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
	Contains implementations of several timers on the ATmega1284P
	for different use cases.
    
USAGE:
	
NOTES:
	                   
*************************************************************************/
#ifndef TIMER_LIB_H_
#define TIMER_LIB_H_

// #include "timer8.h"		/* used for ... */
#include "timer16.h"		/* used for co2 sensor and rfm95 */

// ====================================
//			TIMER8 Settings
// ====================================

// None yet.

// ====================================
//			TIMER16 Settings
// ====================================
static timer_settings_t timer_settings = {
	.comp_b_val = 0,
	.CTCMode = CTCTopOCR1A
};

// extern void timer8_init(void);

extern TIMER_ERROR_T timer16_init(timer_settings_t timer_settings, volatile uint8_t* interrupt_flag);

#endif /* TIMER_LIB_H_ */