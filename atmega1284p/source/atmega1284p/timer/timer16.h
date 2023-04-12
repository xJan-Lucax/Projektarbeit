/*************************************************************************
* Title		: timer.h port from ATmeg16 to ATmega1284P
* Author	: Niklas Theis (original author)
			  Dimitri Dening (ATmega1284P port)
* Created	: 26.01.2021 16:27:30 (ATmega16 version)
* Modified	: 08.11.2021 13:29:02 (ATmega1284P port)
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
    
USAGE:
	
NOTES:
               
*************************************************************************/
#ifndef TIMER16_H_
#define TIMER16_H_

#include <stdint.h>

// =================================================
//				16-bit Timer/Counter1
// =================================================

enum TIMER_ERROR_T {Timer_NoError, Timer_Error, Timer_ErrorNoCSSet, Timer_Error_NotImplemented};
typedef enum TIMER_ERROR_T TIMER_ERROR_T;

enum Timer_ClockSignal_t {CSTimerStop = 0b000, CSSystemClock = 0b001, CSSystemClockDiv8 = 0b010, CSSystemClockDiv64 = 0b011, CSSystemClockDiv256 = 0b100, CSSystemClockDiv1024 = 0b101, CSExtClockT1Fall = 0b110, CSExtClockT1Rise = 0b111};
typedef enum Timer_ClockSignal_t Timer_ClockSignal_t;

enum Timer_CTCMode_t {NOCTC= 0b00, CTCTopOCR1A = 0b01, CTCTopICR1 = 0b10};
typedef enum Timer_CTCMode_t Timer_CTCMode_t;

enum Timer_Interrupt_t {InterruptInputCapture = 5, InterruptCompareA = 4, InterruptCompareB = 3, InterruptOverflow = 2};
typedef enum Timer_Interrupt_t Timer_Interrupt_t;

struct timer_settings_t
{
	uint16_t comp_a_val;
	uint16_t comp_b_val;
	Timer_ClockSignal_t clock_signal;
	Timer_CTCMode_t CTCMode;
};
typedef struct timer_settings_t timer_settings_t;

Timer_ClockSignal_t ClockSignal;
uint8_t InitFlag;
volatile uint8_t* IRCompAFlag;
volatile uint8_t* IRCompBFlag;

TIMER_ERROR_T timer16_init(timer_settings_t timer_settings, volatile uint8_t* interrupt_flag);
TIMER_ERROR_T Timer_start();
TIMER_ERROR_T Timer_stop();

TIMER_ERROR_T timer_add_interrupt(Timer_Interrupt_t inter, volatile uint8_t* irFlag);
TIMER_ERROR_T Timer_removeInterrupt(Timer_Interrupt_t inter);

TIMER_ERROR_T Timer_calculateTimerSettings_s(uint16_t* compareAValue, Timer_ClockSignal_t* clockSignal, uint8_t seconds);
TIMER_ERROR_T timer_calculate_settings_ms(uint16_t* compareAValue, Timer_ClockSignal_t* clockSignal, uint16_t ms);

// =================================================
//				16-bit Timer/Counter3
// =================================================

typedef void (*timer_callback)(void);

void timer_start(uint8_t callback_a_seconds, timer_callback callback_a, uint8_t callback_b_seconds, timer_callback callback_b);
void timer_stop();

#endif /* TIMER16_H_ */

