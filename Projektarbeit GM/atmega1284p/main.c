/********************************************************************
* Title		: Projektarbeit
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
* Author	: Dimitri Dening
*			  Nicolas Krohn
*			  Jan-Luca Nettingsmeier
*			  Philipp Husemann
*********************************************************************/

/* General libraries */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <string.h>
#include <util/delay.h>

/* Peripheral libraries */
#include "lcd_lib.h"
#include "led_lib.h"
#include "uart.h"

/* User defined libraries */
#include "timer_lib.h"
#include "sensors.h"
#include "outputs.h"
#include "lorawan.h"

/* Define CPU frequency in Hz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 10000000UL
#endif

#define LCD_REFRESH_RATE 20

typedef enum {
	lcd_xyz,
	rfm95
} device_t;

typedef struct {
	device_t device;
	uint8_t address;
} addr_t;

addr_t addr_tbl[] = {
	//	  DEVICE	  ADDRESS
	//		|			 |
	//		|			 |
	//		|			 |
	//-----------------------------------
	{	  lcd_xyz	,	0x03	},
	{	  rfm95		,	0x04	}
};

uint8_t timer_interrupt_flag = 0;

lorawan_state *state = NULL;

static void delay(int t) {
	while(t--) _delay_ms(1000);
}

static void downlink_received() {
	uint8_t *payload;
	uint8_t payload_length = lorawan_read_received_data(state, &payload);
	
	uart_puts("downlink received!\r\n");
	
	if (payload_length == 0) return;

	char *tmp = malloc(4*sizeof(uint8_t));
	if (payload_length >= 1){
		/** do something with the payload */
		for(uint8_t i = 0; i < payload_length; i++){
			itoa(payload[i],tmp,16);
			uart_puts(tmp);
		}
	}
	
	free(tmp);
	free(payload);
}

int main(void) {   
	
	cli(); // disable all interrupts

	wdt_reset();
	MCUSR &= ~(1 << WDRF);
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	WDTCSR = 0x00;
	
	uart_init();
	uart_puts("Uart init");
	eeprom_init();
	uart_puts("eeprom init");
	led_init();

		
	state = lorawan_init(addr_tbl[rfm95].address);

	// lcd_xyz_init(addr_tbl[lcd_xyz].address);	// TODO: @Nico
	
	// lcd_init();
	
	// timer_calculate_settings_ms(&(timer_settings.comp_a_val), &(timer_settings.clock_signal), 500);
	// timer16_init(timer_settings, &timer_interrupt_flag);
	
	/*
	sensor_data_t sensor_data;
	sensor_init(&sensor_data);  _delay_ms(1000);
	sensor_start_measurement(); _delay_ms(2);
	
	output_init(&sensor_data, &lcd_settings, no_align);
	*/

	uint8_t refresh_rate = LCD_REFRESH_RATE;

    while(true) {
		//uart_puts("While schleife");

		while (refresh_rate--){

			//if(timer_interrupt_flag){
			//sensor_get_data();
			//output_update_data();
			//timer_interrupt_flag = 0;
			//}
			//_delay_us(1);
			//output_activate_interrupt();

			_delay_ms(500);
		}

		uint8_t *payload = (uint8_t*) "Test payload";
		lorawan_send_data(state, payload, sizeof(uint8_t) * strlen(payload), &downlink_received);
		uart_put("sending payload...");

		refresh_rate = LCD_REFRESH_RATE;
    }
}