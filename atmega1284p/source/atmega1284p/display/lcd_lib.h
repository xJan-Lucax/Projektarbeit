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
#include <stdio.h>
#include <stdlib.h>

//-----------------------------------------------------
//File: lcd_spi.h
//Auth: ELECTRONIC ASSEMBLY JM
//DATE: 04-17-2014
//-----------------------------------------------------

//--- Standard definitions for LCD ---
#define LCD_HOME_L1	0x80
#define LINE1	0

//HD44780
#define LINE2	LINE1+0x40
#define LINE3	LINE1+0x14
#define	LINE4 	LINE2+0x14

//Character table
#define CG0 0x00  //240 char from CGRom, 8 CGRam (8 free definable chars)
#define CG1 0x01  //248 char from CGRom, 8 CGRam (8 free definable chars)
#define CG2 0x02  //250 char from CGRom, 8 CGRam (6 free definable chars)
#define CG3 0x03  //256 char from CGRom, 8 CGRam (0 free definable chars) ->interesting for ROMC

#define DISPLAY_ON 	0x04
#define DISPLAY_OFF 0xFB
#define CURSOR_ON	0x02
#define CURSOR_OFF	0xFD
#define BLINK_ON	0x01
#define BLINK_OFF	0xFE

#define LCDPORT PORTA
#define LCDDDR DDRA

static LCD_Settings_t lcd_settings = {
	.Cursor		= CursorOff,
	.Port		= &LCDPORT,
	.PortDDR	= &LCDDDR,
	.PortPIN	= &PINA
};


void lcd_init(uint8_t cs);
void initDispl(void);
void WriteChar		(char character);
void WriteString	(char * string);
void SetPostion		(char pos);
void DisplayOnOff	(char data);
void DefineCharacter(unsigned char postion, unsigned char *data);
void ClrDisplay		(void);
void SetROM			(unsigned char rom);
void display_on_off(void);
void display_on(void);
void display_off(void);

#endif /* LCD_LIB_H_ */

