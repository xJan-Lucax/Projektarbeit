/*
 * LCD.c
 *
 * Created: 30.01.2021 10:38:29
 *  Author: Niklas Theis
 */ 
#include "LCD.h"

uint8_t ErrorActive;

uint8_t CurrentLine;
uint8_t LineCount;
uint8_t LineLength;

char* LineList;

volatile uint8_t* LcdPort;
volatile uint8_t* LcdDDR;
volatile uint8_t* LcdPin;

uint8_t LCD_WhileBusy()
{
	uint8_t status = 0;
	*LcdDDR = 0x0f;
	*LcdPort = 0b00000010;
	
	clrpin(*LcdPort, LCD_RS_PIN);
	setpin(*LcdPort, LCD_RW_PIN);
	_delay_us(1);
	do {
		setpin(*LcdPort,LCD_E_PIN);
		_delay_us(1);
		status = *LcdPin & 0b11110000;
		clrpin(*LcdPort,LCD_E_PIN);
		setpin(*LcdPort, LCD_E_PIN);
		_delay_us(1);
		status |= ((*LcdPin>>4) & 0b00001111);
		clrpin(*LcdPort,LCD_E_PIN);
	}
	while (status & (1<<LCD_BUSY_PIN));
	*LcdDDR = 0xff;
	return 0;
}

LCD_Error_t LCD_8Bit(char x)
{
	*LcdPort = x;
	setpin(*LcdPort,LCD_E_PIN);
	_delay_us (1);
	clrpin(*LcdPort,LCD_E_PIN);
	
	return LCD_NoError;
}

LCD_Error_t LCD_4Bit(char x, char rs)
{
	LCD_8Bit((x & 0xf0) | (rs<<LCD_RS_PIN));
	LCD_8Bit(((x & 0x0f)<<4) | (rs<<LCD_RS_PIN));
	LCD_WhileBusy();
	
	return LCD_NoError;
}

LCD_Error_t LCD_Init(LCD_Settings_t* settings)
{
	LcdPort = settings->Port;
	LcdDDR = settings->PortDDR;
	LcdPin = settings->PortPIN;
	
	LineList = settings->LineList;
	LineLength = settings->Linelength;
	LineCount = settings->LineCount;
	
	CurrentLine = 0;
	ErrorActive = 0;
	
	*LcdDDR = 0xff;

	_delay_ms (15);
	LCD_8Bit(0x30);
	_delay_ms (4.1);
	LCD_8Bit(0x30);
	_delay_ms (0.1);
	LCD_8Bit(0x30);
	_delay_ms (1);
	LCD_8Bit(0x20);

	LCD_4Bit(0x28,0);
	LCD_4Bit((settings->Cursor << 1 | 0b11 << 2),0);
	LCD_4Bit(0x01,0);
	LCD_4Bit(0x02,0);
	_delay_ms (5);
	
	return LCD_NoError;
}

LCD_Error_t LCD_Clear()
{
	LCD_4Bit(0x01, 0);
	_delay_ms(2);
	
	return LCD_NoError;
}

void LCD_WriteError(char* description, uint8_t errorCode)
{
	if (ErrorActive != 0 || description == 0)
	{
		return;
	}
	
	char line2[8];
	sprintf(line2, "Code: %d", errorCode);
	
	LCD_Clear();
	LCD_Write2Lines(description, line2);
	ErrorActive = 1;
	return;
}

LCD_Error_t LCD_Write2Lines(char* line1, char* line2)
{
	int i = 0;
	LCD_Error_t status = LCD_NoError;
	while (*line1)
	{
		LCD_4Bit(*line1, 1);
		line1++;
		i++;
		if(i == 40)
		{
			status = LCD_Error_LineOverflow;
			break;
		}
	}
	LCD_4Bit(0xc0, 0);
	i = 0;
	while (*line2)
	{
		LCD_4Bit(*line2, 1);
		line2++;
		i++;
		if(i == 40)
		{
			status = LCD_Error_LineOverflow;
			break;
		}
	}
	
	return status;
}


LCD_Error_t LCD_UpdateData()
{
	if (ErrorActive)
	{
		return LCD_NoError;
	}
	
	if (CurrentLine >= LineCount) return LCD_Error_OutOfList;
	char* line1 = LineList + CurrentLine * LineLength;
	char* line2 = LineList + (CurrentLine + 1) * LineLength;
	
	LCD_Clear();
	LCD_Write2Lines(line1, line2);
	
	return LCD_NoError;
}

LCD_Error_t LCD_MoveUp()
{
	if (ErrorActive)
	{
		ErrorActive = 0;
		LCD_UpdateData();
		
		return LCD_NoError;
	}
	
	if (CurrentLine != 0) CurrentLine--;
	LCD_UpdateData();
	
	return LCD_NoError;
}

LCD_Error_t LCD_MoveDown()
{
	if (ErrorActive)
	{
		ErrorActive = 0;
		LCD_UpdateData();
		
		return LCD_NoError;
	}
	
	if (CurrentLine < LineCount - 2) CurrentLine++;
	LCD_UpdateData();
	
	return LCD_NoError;
}