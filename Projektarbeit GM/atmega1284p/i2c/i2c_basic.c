

//Einbinden der Grundtreiber Header Datei
#include "i2c_basic.h"
// Allgemein benötigte Includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//**************** Variablen für die Übertagung *******************

uint8_t I2C_buf[I2C_BUFFER_SIZE];		//Interner Buffer der zu übertragenden Daten
volatile uint8_t I2C_bufPtr;			// Zeiger auf das aktuelle Datenbyte
uint8_t I2C_msgSize;					// Länge der aktuellen Übertragungsnachricht
static volatile uint8_t Slave_Address;	//Adresse des Kommunikationspartners
uint8_t delay_time;						// Wartezeit zwischen einem Read und Write Befehl
void (*callback_R) (I2C_TxRxError_t);	// Rückruffunktion
uint8_t numberOfReadBytes;				// Anzahl der zu lesenden Bytes in einer Übertragung
static uint8_t volatile transmit_in_progress; //1 = ja; 0 = nein

//******************************************************************


//*********************** interne Funktionen ***********************

/*
* Funktion gibt zurück ob die Logik gerade arbeitet, oder der nächste
* Übertragungschritt gemacht werden darf
*/
uint8_t I2C_Busy()
{
	return (TWCR & (1 << TWIE)); //Wenn das Interrupt Status Bit gesetzt ist, arbeitet die Logik
}

/*
* Starten des Read Vorgangs nach dem vorrangegangenen Write Vorgangs mit 
* entsprechender Wartezeit zwischen den Befehlen
*/
void Start_Reading()
{
	I2C_msgSize = numberOfReadBytes;
	Slave_Address = Slave_Address | 0b00000001; // Ändern der Adresse in eine Lese Adresse
	for (uint8_t c= 0; c < delay_time; c++)
	{
		_delay_ms(1);
	}
	I2C_SendStart();
}

//********************* Ende interne Funktionen **************************

//********************* Funktionen für Schnittstelle **********************

/*
* Methode Initalisiert den I2C Treiber und setzt die Taktrate der 
* Clock Leitung
*/
void I2C_Initialise(void)
{
	TWBR = 0x5C; // Hier wird die Taktrate des SCL Leitung festgelegt
	TWDR = 0xFF; // Default Wert des Datenregisters
	I2C_TWCR_Initialise();
	transmit_in_progress = 0; // Initalisieren der Übertragungsaktivitätsvariable auf Null
} 

/*
* Methode startet einen Sendevorgang über den I2C Bus
*/
I2C_Error_t I2C_Transmit(struct I2C_TxData_t TxData_st)
{
	if (TxData_st.NumberOfBytes <= I2C_BUFFER_SIZE && transmit_in_progress == 0)
	{
		if (I2C_Busy())
		{			
			return I2C_ERROR; //Logik arbeitet, daher Error. 
		}
		transmit_in_progress = 1;
		numberOfReadBytes = 0; // Da dies nur ein Schreib Vorgang ist
		I2C_msgSize = TxData_st.NumberOfBytes; // Anzahl der Bytes in den Speicher einlesen
		Slave_Address = (TxData_st.Address << 1);
		callback_R = TxData_st.callback_function;
		for(uint8_t temp = 0; temp <I2C_msgSize; temp++)
		{
			I2C_buf[temp] = TxData_st.TxBytes[temp];
		}	
		I2C_SendStart();
		return I2C_NOERROR;
	}
	else
	{
		return I2C_ERROR; //Daten sind zu lang für den Buffer oder es wird gerade etwas übertragen
	}
}

/*
* Funktion startet einen Write mit anschließendem Read Vorgang
*/
I2C_Error_t I2C_Receive(struct I2C_TxData_t TxData_st, uint8_t delay_in_ms_u8, uint8_t anzahlLeseBytes)
{
	if (I2C_Busy() || transmit_in_progress == 1 || TxData_st.NumberOfBytes > I2C_BUFFER_SIZE)
	{
		return I2C_ERROR;
	}
	//Kommunikationsdaten zum Empfangen speichern
	transmit_in_progress = 1;
	numberOfReadBytes = anzahlLeseBytes;
	delay_time = delay_in_ms_u8;
	//Kommunikationsdaten für den Sendevorgang speichern
	Slave_Address = (TxData_st.Address << 1);
	I2C_msgSize = TxData_st.NumberOfBytes;
	callback_R = TxData_st.callback_function;
	for (unsigned char temp = 0; temp < I2C_msgSize; temp++)
	{
		I2C_buf[temp] = TxData_st.TxBytes[temp];
	}
	I2C_SendStart();
	return I2C_NOERROR;
}

/*
* Funktion ließt empfangene Übertragungsdaten aus dem interen Buffer aus
*/
I2C_Error_t I2C_Read_Data(uint8_t *data, uint8_t dataSize)
{
	if (dataSize < I2C_BUFFER_SIZE && !I2C_Busy() && transmit_in_progress == 0)
	{
		for (uint8_t i=0; i<dataSize;i++)
		{
			data[i] = I2C_buf[i];
		}
		return I2C_NOERROR;
	}
	else
	{
		return I2C_ERROR; // Fehler beim Auslesen der Daten
	}
}

//*************** Ende Funktionen für Schnittstelle *************************

//*********************** Logik in ISR **************************************
ISR(TWI_vect)
{
	switch (TWSR)
	{
		case I2C_START:
		case I2C_REP_START:
			I2C_bufPtr = 0;
			TWDR = Slave_Address; 
			I2C_SendTransmit();
			break;
		//**************** Statuscodes Master Transmitter Mode ***********************
		case I2C_MTX_ADR_ACK:
		case I2C_MTX_DATA_ACK:
			if (I2C_bufPtr < I2C_msgSize)
			{
				TWDR = I2C_buf[I2C_bufPtr++];
				I2C_SendTransmit();
			}
			else // Übertragung erfolgreich beendet entweder beenden oder empfangen einleiten
			{
				if (numberOfReadBytes > 1) // Hier beginnen des Empfangen
				{
					I2C_SendStop();
					Start_Reading();
				}
				else
				{					
					I2C_SendStop();
					// Fertig mit der Übertragung					
					transmit_in_progress = 0;
					callback_R(I2C_NOTXRXERROR);
				}
			}
			break;
		//*************** Statuscodes Master Slave Mode *****************
		case I2C_MRX_DATA_ACK:
			I2C_buf[I2C_bufPtr++] = TWDR;
		case I2C_MRX_ADR_ACK:
			if (I2C_bufPtr < (I2C_msgSize-1)) //Letztes Byte Nacken -> signalisiert Übertragung beendet
			{
				I2C_SendACK();
			}
			else
			{
				I2C_SendNACK();
			}
			break;
		case I2C_MRX_DATA_NACK:
			I2C_buf[I2C_bufPtr] = TWDR;
			I2C_SendStop();
			//Fertig mit dem empfangen von Daten
			transmit_in_progress = 0;
			callback_R(I2C_NOTXRXERROR); // Oberer Schicht mitteilen dass Daten abholbereit sind
			break;
		//**************** Fehlercases ******************************************
		case I2C_MTX_ADR_NACK:
		case I2C_MRX_ADR_NACK:
		case I2C_MTX_DATA_NACK:
		case I2C_BUS_ERROR:
			I2C_SendStop();
			transmit_in_progress = 0;
			callback_R(I2C_TXRXERROR);
			break;
		default:
			I2C_SendStop();
			transmit_in_progress = 0;
			callback_R(I2C_TXRXERROR);
	}
}
