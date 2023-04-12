#include "i2c_driver.h"
#include <stdio.h>
#include <avr/io.h>

//**************** Definieren der Warteschlange ***************************
struct job liste[MAXSIZE];
int front = -1;
int rear = -1;

int initalisiert = 0;


//******************* interne Funktionen der Warteschlange ***************************

/*
* Überprüft ob die Warteschlange voll
*/
uint8_t isFull() {
	if ( (front == rear +1) || (front == 0 && rear == MAXSIZE -1) )
	{
		return 1;
	}
	return 0;
}

/*
* Überprüft ob die Warteschlange leer ist
*/
uint8_t isEmpty()
{
	if (front == -1)
	{
		return 1;
	}
	return 0;
}

/*
* Methode wird vom Grundtreiber aufgerufen
* Gibt die Daten weiter an die obere Schicht und führt, wenn nicht leer, nächste Operation
* aus der Warteschlange aus
*/
void deQueue(I2C_TxRxError_t fehler)
{
	void (*callback) (I2C_TxRxError_t);
	callback = liste[front].callback_function;
	if (front == rear) // Überprüfen ob Schlange danach leer ist
	{
		front = -1;
		rear = -1;
	}
	else
	{
		front = (front + 1) % MAXSIZE;
	}
	callback(fehler); // Daten in der oberen Schicht abholen
	
	//Schauen ob noch ein Job in der Warteschlange ist
	if(!isEmpty())
	{
		//Initieren der nächsten Übertragung -> Aufrufen des Grundtreibers
		if (liste[front].read_or_write_operation == 1)
		{
			//Write Operation
			I2C_Error_t f;
			f = I2C_Transmit(liste[front].TxData_t);
			if (f == I2C_ERROR)
			{
				// Mappen des Grundtreiberfehlers auf einen Übertragungsfehler
				deQueue(I2C_TXRXERROR);
			}
		}
		else if (liste[front].read_or_write_operation == 0)
		{
			//Read Operation
			I2C_Error_t f;
			f=I2C_Receive(liste[front].TxData_t,liste[front].delay_in_ms_u8, liste[front].anzahlLeseBytes);
			if (f == I2C_ERROR)
			{								
				// Mappen des Grundtreiberfehlers auf einen Übertragungsfehler
				deQueue(I2C_TXRXERROR);
			}
		}
	}
}
//**************** Schnittstellenfunktionen zur Bedienung des Treibers *****************

/*
* Methode führt einen Read Befehl aus oder schiebt ihn in die Warteschlange, wenn gerade etwas
* gesendet/empfangen wird
*/
I2C_Error_t I2C_Read(struct I2C_TxData_t TxData_st, uint8_t delay_in_ms_u8, uint8_t number_of_read_bytes)
{
	if(isFull() || initalisiert == 0)
	{
		return I2C_ERROR;
	}
	else
	{
		struct job newJob;
		newJob.read_or_write_operation = 0; //0 für read
		newJob.callback_function =TxData_st.callback_function;
		TxData_st.callback_function = deQueue;
		if (front == -1) //Schlange ist leer, Job direkt ausführen
		{
			I2C_Error_t error;
			error = I2C_Receive(TxData_st,delay_in_ms_u8, number_of_read_bytes);			
			if (error == I2C_ERROR)
			{
				// Mappen des Grundtreiber errror auf einen Übertragungserror
				newJob.callback_function(I2C_TXRXERROR);
				return I2C_NOERROR;
				//return I2C_ERROR; //Sollte eigentlich nie passieren dürfen
			}
			
			front = 0;
			rear = (rear + 1) % MAXSIZE;
			liste[rear] = newJob;
		}
		else //Job in die Warteschlange einreihen
		{
			newJob.anzahlLeseBytes = number_of_read_bytes;
			newJob.delay_in_ms_u8 = delay_in_ms_u8;
			newJob.TxData_t = TxData_st;
			rear = (rear + 1) % MAXSIZE;
			liste[rear] = newJob;
		}
		return I2C_NOERROR;
	}
}

/*
* Methode führt eine Write Operation aus oder schiebt die Operation in die Warteschlange
* wenn gerade etwas gesendet/empfangen wird
*/
I2C_Error_t I2C_Write(struct I2C_TxData_t TxData_st)
{
	if(isFull() || initalisiert == 0)
	{
		return I2C_ERROR;
	}
	else
	{		
		struct job newJob;
		newJob.read_or_write_operation = 1; //1 für write
		newJob.callback_function =TxData_st.callback_function;
		TxData_st.callback_function = deQueue;
		if (front == -1) //Schlange ist leer
		{			
			I2C_Error_t error;
			error = I2C_Transmit(TxData_st);			
			if (error == I2C_ERROR)
			{
				// Mappen des Grundtreiber errror auf einen Übertragungserror
				newJob.callback_function(I2C_TXRXERROR);
				return I2C_NOERROR;
				//return I2C_ERROR;
			}
			front = 0;
			rear = (rear + 1) % MAXSIZE;
			liste[rear] = newJob;
		}
		else //Einreihen in die Warteschlange
		{						
			newJob.TxData_t = TxData_st;
			rear = (rear + 1) % MAXSIZE;
			liste[rear] = newJob;
		}
		return I2C_NOERROR;
	}
}

/*
* Methode ließt empfangene Daten aus dem Buffer aus
*/
I2C_Error_t I2C_Read_Data_From_Buffer(uint8_t *data, uint8_t data_size)
{
	if (initalisiert)
	{
		return I2C_Read_Data(data, data_size); // Aufrufen der Funktion des Grundtreibers
	}
	return I2C_ERROR;
}

/*
* Methode Initalisiert den I2C Treiber
*/
I2C_Error_t I2C_InitModule()
{
	I2C_Initialise();
	initalisiert = 1;
	return I2C_NOERROR;
}