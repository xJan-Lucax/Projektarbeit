#pragma once
#include <avr/io.h>

//*********************** Definitionen ***************************
#define I2C_BUFFER_SIZE 21 //Auf die größte zu Übertragende Nachricht setzen; CO2 Sensor braucht 18 Byte

//I2C Allgemeine Status Codes
#define I2C_START		0x08
#define I2C_REP_START	0x10 // Nicht implementiert, da SCD30 REP_START nicht unterstützt
#define I2C_ARB_LOST	0x38 // Nicht implementiert

// Master Transmitter Status codes
#define I2C_MTX_ADR_ACK            0x18  // SLA+W übertragen und ACK empfangen
#define I2C_MTX_ADR_NACK           0x20  // SLA+W übertragen und NACK empfangen
#define I2C_MTX_DATA_ACK           0x28  // Datenbyte übertragen und ACK empfangen
#define I2C_MTX_DATA_NACK          0x30  // Datenbyte übertragen und NACK empfangen

// Master Receiver Status codes
#define I2C_MRX_ADR_ACK            0x40  // SLA+R übertragen und ACK empfangen
#define I2C_MRX_ADR_NACK           0x48  // SLA+R übertragen und NACK empfangen
#define I2C_MRX_DATA_ACK           0x50  // Datenbyte übertragen und ACK empfangen
#define I2C_MRX_DATA_NACK          0x58  // Datenbyte übertragen und NACK empfangen


// Allgemeine Fehler codes
#define I2C_NO_STATE               0xF8  // Kein Status
#define I2C_BUS_ERROR              0x00  // Bus Error durch Illegale START oder STOP Kondition

//********************** Makros für die Übertragung ****************************
#define I2C_TWCR_Initialise() (TWCR = (1<<TWEN)|(0<<TWIE)|(0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC))
#define I2C_SendStart()	(TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)|(1<<TWIE))
#define I2C_SendStop()	(TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN)|(0<<TWIE))

#define I2C_SendACK()	(TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA))
#define I2C_SendNACK()	(TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE))
#define I2C_SendTransmit() (TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE))


//************************ eigene Typdefinitionen ****************
typedef enum
{
	I2C_NOERROR = 0,
	I2C_ERROR = 1,
}I2C_Error_t;

typedef enum
{
	I2C_NOTXRXERROR = 0,
	I2C_TXRXERROR = 1,
}I2C_TxRxError_t;

struct I2C_TxData_t
{
	uint8_t Address;		// Adresse ist 7 Byte lang
	uint8_t NumberOfBytes;  // Anzahl bytes ohne Adresse
	uint8_t TxBytes[10];		// Datenarray
	void (*callback_function)(I2C_TxRxError_t);

};
typedef struct I2C_TxData_t I2C_TxData_t;
//************** Funktionsdeklarationen ***********************
void I2C_Initialise(void);

I2C_Error_t I2C_Transmit(struct I2C_TxData_t TxData_st);

I2C_Error_t I2C_Receive(struct I2C_TxData_t TxData_st, uint8_t delay_in_ms_u8, uint8_t anzahlLeseBytes);

I2C_Error_t I2C_Read_Data(uint8_t *data, uint8_t dataSize);