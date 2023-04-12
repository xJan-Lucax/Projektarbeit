#pragma once

#include "i2c_basic.h"
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

//**************** lokale Definitionen **********************
#define MAXSIZE 4 // Länge der Warteschlange

//**************** eigene lokale Typdeklarationen *************
struct job
{
	struct I2C_TxData_t TxData_t;
	uint8_t anzahlLeseBytes;
	uint8_t delay_in_ms_u8;
	uint8_t read_or_write_operation;
	void (*callback_function) (I2C_TxRxError_t);
};


//******************** Funktionsdeklarationen des kompletten Interfaces *****************
I2C_Error_t I2C_InitModule(void);

I2C_Error_t I2C_Write(struct I2C_TxData_t TxData_st);

I2C_Error_t I2C_Read(struct I2C_TxData_t TxData_st, uint8_t delay_in_ms_u8, uint8_t number_of_read_bytes);

I2C_Error_t I2C_Read_Data_From_Buffer(uint8_t *data, uint8_t data_size);