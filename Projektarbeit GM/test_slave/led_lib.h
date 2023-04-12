/*************************************************************************
* Title		: led_lib.h
* Author	: Dimitri Dening
* Created	: 25.11.2021 21:42:49
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
    
USAGE:
	
NOTES:
	                   
*************************************************************************/
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
#define LED7 0x07 /* reserved for error code */
#define LED_ERROR LED7

/* 
DESCRIPTION:
	Initialize led pins to indicate ppm status 
	when using the co2 sensor and an error code
	for the <spi_lib.h>.
ARGS:
	None
	
RETURN:
	None
*/
void led_init(void);

void led_activate(uint8_t led);

void led_deactivate(uint8_t led);

void led_toggle(uint8_t led);

void led_counter(uint8_t byte);

#endif /* LED_LIB_H_ */