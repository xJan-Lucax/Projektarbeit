/*************************************************************************
* Title		: led_lib.h
* Author	: Dimitri Dening
* Created	: 25.11.2021 21:42:49
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
* License	: MIT License
* Usage		: see Doxygen manual
*
*		Copyright (C) 2021 Dimitri Dening
*
*		Permission is hereby granted, free of charge, to any person obtaining a copy
*		of this software and associated documentation files (the "Software"), to deal
*		in the Software without restriction, including without limitation the rights
*		to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*		copies of the Software, and to permit persons to whom the Software is
*		furnished to do so, subject to the following conditions:
*
*		The above copyright notice and this permission notice shall be included in all
*		copies or substantial portions of the Software.
*
*		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*		IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*		FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*		AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*		LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*		OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*		SOFTWARE.
*
*************************************************************************/

/**
@file led_lib.h    
@author Dimitri Dening
@date 25.11.2021
@copyright (C) 2021 Dimitri Dening, GNU General Public License Version 3
@brief Simple led library to control leds on the STK600.
@bug No known bugs.       
*/
#ifndef LED_LIB_H_
#define LED_LIB_H_

#define LED_PIN  PINA
#define LED_DDR  DDRA
#define LED_PORT PORTA
#define LED0 0x00 /* reserved for co2 sensor */
#define LED1 0x01 /* reserved for co2 sensor */
#define LED2 0x02 /* reserved for co2 sensor */
#define LED3 0x03
#define LED4 0x04 
#define LED5 0x05 
#define LED6 0x06 
#define LED7 0x07 /* reserved for spi error code */
#define LED_ERROR LED7

/**
 * @brief Initialize led pins to indicate ppm status when using the co2 sensor and an error code for the <spi.h>.
 * @return	None
 */
void led_init(void);

void led_activate(uint8_t led);

void led_deactivate(uint8_t led);

void led_toggle(uint8_t led);

void led_counter(uint8_t byte);

#endif /* LED_LIB_H_ */