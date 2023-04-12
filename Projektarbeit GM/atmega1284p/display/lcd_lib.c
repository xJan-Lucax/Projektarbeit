/*************************************************************************
* Title		: LCD Implementation
* Author	: Nicolas Krohn
* Created	: 25.10.2021 17:56:51
* Software	: Microchip Studio V7
* Hardware	: ILI9341 2,4" TFT Touch-Display
        
DESCRIPTION:
    
USAGE:
	
NOTES:
	TODO: https://....pdf
	                   
*************************************************************************/
/* General libraries */

/* User defined libraries */
#include "lcd_lib.h"

void lcd_init(void){
	LCD_Init(&lcd_settings);
}
