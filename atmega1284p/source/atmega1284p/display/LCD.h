/*
 * LCD.h
 *
 * Created: 30.01.2021 10:38:36
 *  Author: Niklas Theis
 */ 

#pragma once

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#define setpin(port,bitnummer) port |= (1<<bitnummer)
#define clrpin(port,bitnummer) port &= ~(1<<bitnummer)

#define LCD_RS_PIN 0
#define LCD_RW_PIN 1
#define LCD_E_PIN 2
#define LCD_BUSY_PIN 7

#define MAX_CHAR_COUNT 16

enum LCD_Error_t {LCD_NoError = 0, LCD_Error = 1, LCD_Error_LineOverflow = 2, LCD_Error_OutOfList = 3};
typedef enum LCD_Error_t LCD_Error_t;
enum LCD_CursorSetting_t {CursorOff = 0, CursorOn = 1};
typedef enum LCD_CursorSetting_t LCD_CursorSetting_t;
enum LCD_Lines_t {OneLine = 0, TwoLines = 1};
typedef enum LCD_Lines_t LCD_Lines_t;

struct LCD_Settings_t
{
	volatile uint8_t* Port;
	volatile uint8_t* PortDDR;
	volatile uint8_t* PortPIN;
	LCD_CursorSetting_t Cursor;
	LCD_Lines_t Lines;
	char* LineList;
	uint8_t Linelength;
	uint8_t LineCount;
};
typedef struct LCD_Settings_t LCD_Settings_t;

LCD_Error_t LCD_Init(LCD_Settings_t* settings);
LCD_Error_t LCD_Write2Lines(char* line1, char* line2);
LCD_Error_t LCD_UpdateData();
LCD_Error_t LCD_MoveUp();
LCD_Error_t LCD_MoveDown();
LCD_Error_t LCD_Clear();

void LCD_WriteError(char* description, uint8_t errorCode);