/*************************************************************************
* Title		: OLED Implementation
* Author	: Nicolas Krohn
* Created	: 25.10.2021 17:56:51
* Software	: Microchip Studio V7
* Hardware	: W204-XLG 2,4" OLED-Display
        
DESCRIPTION:
    
USAGE:
	
NOTES:
	TODO: https://....pdf
	                   
*************************************************************************/
/* General libraries */

#include <avr/interrupt.h>
#include <util/delay.h>

/* User defined libraries */
#include "lcd_lib.h"
#include "spi.h"
#include "uart.h"


static slave_info slave_device;

void callback_function (void){}

//-----------------------------------------------------
//Attention, you need to change solder bridges!
//-----------------------------------------------------

static void WriteIns		(uint8_t ins);
static void WriteData		(uint8_t data);
unsigned char CheckBusy		(void);

static void WriteIns(uint8_t ins)
{
	spi_error_t err;
	
	// CheckBusy();
	//_delay_ms(100);

	uint8_t tx_data[] = { 0x00, 0x00, 0x00, 0x00, ins };
	err = spi_write(&slave_device, tx_data, 5, NORMAL, DEFAULT, NULL);
}

static void WriteData(uint8_t ins)
{
	spi_error_t err;
	
	// CheckBusy();
	//_delay_ms(100);

	uint8_t tx_data[] = { 0x00, 0x00, 0x00, 0x02, ins };
	err = spi_write(&slave_device, tx_data, 5, NORMAL, DEFAULT, NULL);
}

void display_on(void){
		
	WriteIns(0x02);	//return home
	_delay_ms(100);
	WriteIns(0x0E);	//turn cursor on
}

void display_off(void){
	
	WriteIns(0x01);	//clear display
}

void display_on_off(void){
	
	uint8_t oled_on[]  = { 0x00, 0x00, 0x00, 0x00, 0x0C };
	uint8_t oled_off[] = { 0x00, 0x00, 0x00, 0x00, 0x08 };	
		
	for (uint8_t i = 0; i < 20; i++){
		WriteIns(0x0E);	//turn cursor on
		_delay_ms(300);
		spi_write(&slave_device, oled_on, 5, NORMAL, DEFAULT, NULL);
		_delay_ms(300);
		spi_write(&slave_device, oled_off, 5, NORMAL, DEFAULT, NULL);
		_delay_ms(300);
	}
}

static void setupDisplay(void)
{
	WriteIns(0x38);	//function set european chararacter set
	_delay_ms(100);
	_delay_ms(100);
	_delay_ms(100);
	WriteIns(0x08);	//display off
	_delay_ms(100);
	WriteIns(0x06);	//entry mode set increment cursor by 1 not shifting display
	_delay_ms(100);
	WriteIns(0x17);	//Character mode and internel power on (have to turn on internel power to get the best brightness)
	_delay_ms(100);
	WriteIns(0x01);	//clear display
	_delay_ms(100);
	_delay_ms(100);
	WriteIns(0x0C);	//display on
	_delay_ms(100);
	WriteIns(0x0E);	//turn cursor on
	_delay_ms(100);
	WriteIns(0x02);	//return home
	tracser();
	// DisplayOnOff(DISPLAY_ON | CURSOR_ON | BLINK_ON);
}

static void helloWorld(void){
	
		spi_error_t err;

		// Display On/Off Control (: 0x00, 0x00, 0x00, 0x00, 0x0E
        // hello world example:  0x00, 0x00, 0x00, 0x02, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64
		
		
		uint8_t oled_clr[]  = { 0x00, 0x00, 0x00, 0x00, 0x0E};
		uint8_t oled_home[] = { 0x00, 0x00, 0x00, 0x00, 0x02};
		//err = spi_write(&slave_device, oled_clr, 5, NORMAL, DEFAULT, NULL);
		
		//_delay_ms(100);	
		
		uint8_t display_on[]={ 0x00, 0x00, 0x00, 0x02, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64, 0x21};
		err = spi_write(&slave_device, display_on, 16, NORMAL, DEFAULT, NULL);
	
		uint8_t display_off[] = { 0x00, 0x00, 0x00, 0x00, 0x02 };
		// 1 2 3 4 5 6 7 8 9 10	
		//   ========00001000			
		// uint8_t display_on[]	= { 0x03, 0x00 };
		// uint8_t display_off[]	= { 0x02, 0x00 };

		for (uint8_t i = 0; i < 1; i++){
			err = spi_write(&slave_device, display_on, 6, NORMAL, DEFAULT, NULL);
			_delay_ms(300);
			//err = spi_write(&slave_device, display_off, 5, NORMAL, DEFAULT, NULL);
			_delay_ms(200);
		}	
}

static loop_character(void) {
	for (uint8_t i = 0; i < 30; i++){
		for (uint8_t i = 0; i < 2; i++){ // 256
			_delay_ms(50);
			WriteData(0b00000000+i);
		}
	}
}

	
void lcd_init(uint8_t cs){

	slave_device = spi_create_slave(cs, cs, cs);
	//
	setupDisplay();
	_delay_ms(300);
}

void tracser(){
	uart_put("TEST!! OLED TEST!! OLED TEST!! OLED TEST!! OLED TEST!! OLED TEST!! OLED TEST!!");
}

