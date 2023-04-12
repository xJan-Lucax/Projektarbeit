/*************************************************************************
* Title		: C include file for LCD interface
* Author	: Nicolas Krohn
* Created	: 25.10.2021 17:56:51
* Software	: Microchip Studio V7
* Hardware	: ILI9341 2,4" TFT Touch-Display
        
DESCRIPTION:
    
USAGE:
	
NOTES:
	TODO: @Nico
	                   
*************************************************************************/
#ifndef LCD_LIB_H_
#define LCD_LIB_H_

/* General libraries */

/* User defined libraries */
#include "LCD.h"

#define LCDPORT PORTB
#define LCDDDR DDRB

static LCD_Settings_t lcd_settings = {
	.Cursor		= CursorOff,
	.Port		= &LCDPORT,
	.PortDDR	= &LCDDDR,
	.PortPIN	= &PINB
};

void lcd_init(void);

#endif /* LCD_LIB_H_ */