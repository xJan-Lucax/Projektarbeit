/*************************************************************************
* Title		: timer.c port from ATmeg16 to ATmega1284P
* Author	: Niklas Theis (original author)
			  Dimitri Dening (ATmega1284P port)
* Created	: 26.01.2021 16:27:08 (ATmega16 version)
* Modified	: 08.11.2021 13:29:02 (ATmega1284P port)
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
    
USAGE:
	
NOTES:
               
*************************************************************************/
/* General libraries */
#include <stdint.h>
#include <avr/io.h>
#include <avr/common.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdint.h>

/* User defined libraries */
#include "timer16.h"
#include "uart.h"
#include "io_utils.h"

enum Timer_PWMCOM1A_t {NoPWM, PWNNormal, PWMInverted};
typedef enum Timer_PWMCOM1A_t Timer_PWMCOM1A_t;

// =================================================
//				16-bit Timer/Counter1
// =================================================

ISR(TIMER1_COMPA_vect)
{
	if(IRCompAFlag != 0)
	{
		*IRCompAFlag = 1;
	}
}

ISR(TIMER1_COMPB_vect)
{
	if(IRCompBFlag != 0)
	{
		*IRCompBFlag = 1;
	}
}

TIMER_ERROR_T timer16_init(timer_settings_t timer_settings, volatile uint8_t* interrupt_flag)
{
	ClockSignal = timer_settings.clock_signal;
	
	TCCR1A = 0b00000000;
	TCCR1B = timer_settings.CTCMode << WGM12 | timer_settings.clock_signal;
	
	OCR1A = timer_settings.comp_a_val;
	OCR1B = timer_settings.comp_b_val;
	
	TCNT1 = 0;
	
	TIMSK1 |= (1 << OCIE1A); // Enable compare match interrupt
	// TIMSK1 |= (1 << OCIE1B); // Enable compare match interrupt

	InitFlag = 1;
	IRCompAFlag = interrupt_flag;
	
	sei();
	
	return Timer_NoError;
}

TIMER_ERROR_T timer_add_interrupt(Timer_Interrupt_t inter, volatile uint8_t* irFlag)
{
	TIMSK1 |= 1 << inter; // OCIE1A enable
	SREG |= 1 << SREG_I;
	IRCompAFlag = irFlag;
	TCNT1 = 0;
	return Timer_NoError;
	
}

TIMER_ERROR_T Timer_removeInterrupt(Timer_Interrupt_t inter)
{
	TIMSK1 &= ~(1 << inter);
	return Timer_NoError;
}

TIMER_ERROR_T Timer_start()
{
	if (!InitFlag || ClockSignal == CSTimerStop)
	{
		return Timer_ErrorNoCSSet;
	}
	TCCR1B |= ClockSignal;
	TCNT1 = 0;
	return Timer_NoError;
}

TIMER_ERROR_T Timer_stop()
{
	TCCR1B &= ~(CSTimerStop);
	return Timer_NoError;
}

TIMER_ERROR_T Timer_reset()
{
	TCNT1 = 0;
	return Timer_NoError;
}

TIMER_ERROR_T Timer_calculateTimerSettings_s(uint16_t* compareAValue, Timer_ClockSignal_t* clockSignal, uint8_t seconds)
{
	uint16_t prescaler[5] = {1, 8, 64, 256, 1024};
	Timer_ClockSignal_t clockSignals[5] = {CSSystemClock, CSSystemClockDiv8, CSSystemClockDiv64, CSSystemClockDiv256, CSSystemClockDiv1024};
	uint8_t i = 0;
	while ((F_CPU / prescaler[i]) * seconds >= 65536)
	{
		if(i <= 4)
		{
			i++;
		}
		else
		{
			return Timer_Error;
		}
	}
	*compareAValue = (F_CPU / prescaler[i]) * seconds;
	*clockSignal = clockSignals[i];
	
	return Timer_NoError;
}

TIMER_ERROR_T timer_calculate_settings_ms(uint16_t* compareAValue, Timer_ClockSignal_t* clockSignal, uint16_t ms)
{
	uint16_t prescaler[5] = {1, 8, 64, 256, 1024};
	Timer_ClockSignal_t clockSignals[5] = {CSSystemClock, CSSystemClockDiv8, CSSystemClockDiv64, CSSystemClockDiv256, CSSystemClockDiv1024};
	uint8_t i = 0;
	while ((F_CPU / 1000 / prescaler[i]) * ms >= 65536)
	{
		if(i <= 4)
		{
			i++;
		}
		else
		{
			return Timer_Error;
		}
	}
	
	*compareAValue = (F_CPU / 1000 / prescaler[i]) * ms;
	*clockSignal = clockSignals[i];

	return Timer_NoError;
}

// =================================================
//				16-bit Timer/Counter3
// =================================================

// TODO: @Jan implement timer here
timer_callback callback_a_temp = NULL;
timer_callback callback_b_temp = NULL;

// uses the 16 bit timer1
void timer_start(uint8_t callback_a_seconds, timer_callback callback_a, uint8_t callback_b_seconds, timer_callback callback_b) {
	TCCR3A = 0;

	TCNT3 = 0; // reset counter

	OCR3A = (F_CPU / 1024) * callback_a_seconds; // set compare values
	OCR3B = (F_CPU / 1024) * callback_b_seconds;

	SET_BIT(TIMSK3, OCIE3A); // enable output compare match interrupts
	SET_BIT(TIMSK3, OCIE3B);

	callback_a_temp = callback_a;
	callback_b_temp = callback_b;

	sei();

	TCCR3B = 0b00000101; // prescaler = 1024, activates the timer
}

void timer_stop() {
	TCCR3B = 0;
}

ISR(TIMER3_COMPA_vect) {
	if (callback_a_temp == NULL) return;
	callback_a_temp();
	callback_a_temp = NULL;
}

ISR(TIMER3_COMPB_vect) {
	if (callback_b_temp == NULL) return;
	callback_b_temp();
	callback_b_temp = NULL;
}