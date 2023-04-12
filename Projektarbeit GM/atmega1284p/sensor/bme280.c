/*
 * BME280.c
 *
 * Created: 06.05.2021 20:55:15
 *  Author: Jonas Bruns
 */ 

#include "pressure_sens.h"

#define BME280_LEN_W 2
#define BME280_LEN_R 1
#define BME280_ADDRESS 0xF7
#define BME280_DELAY_TIME 0

PressureSensorData_t *daten;

uint16_t dig_T1; 
int16_t dig_T2;
int16_t dig_T3;

uint16_t dig_P1 = 0;
int16_t dig_P2 = 0;
int16_t dig_P3 = 0;
int16_t dig_P4 = 0;
int16_t dig_P5 = 0;
int16_t dig_P6 = 0;
int16_t dig_P7 = 0;
int16_t dig_P8 = 0;
int16_t dig_P9 = 0;

uint16_t dig_H1 = 0;
int16_t dig_H2 = 0;
uint16_t dig_H3 = 0;
int16_t dig_H4 = 0;
int16_t dig_H5 = 0;
int16_t dig_H6 = 0;

uint32_t t_fine;

// interne Funktionen

void BME280_CB_trimmP(I2C_TxRxError_t fehler)
{
	if (fehler == I2C_TXRXERROR){return;}
	
	uint8_t gelesen[20];
	I2C_Read_Data_From_Buffer(gelesen,20);
	
	dig_P1 = gelesen[0];
	dig_P1 += gelesen[1]<<8;
	dig_P1 = (uint16_t)dig_P1;

	dig_P2 = gelesen[2];
	dig_P2 += gelesen[3]<<8;
	dig_P2 = (int)dig_P2;
	if (dig_P2 > 32767)
	{
		dig_P2 = dig_P2 - 65536;
	}

	dig_P3 = gelesen[4];
	dig_P3 += gelesen[5]<<8;
	dig_P3 = (int)dig_P3;
	if (dig_P3 > 32767)
	{
		dig_P3 = dig_P3 - 65536;
	}
	
	dig_P4 = gelesen[6];
	dig_P4 += gelesen[7]<<8;
	dig_P4 = (int)dig_P4;
	if (dig_P4 > 32767)
	{
		dig_P4 = dig_P4 - 65536;
	}
	
	dig_P5 = gelesen[8];
	dig_P5 += gelesen[9]<<8;
	dig_P5 = (int)dig_P5;
	if (dig_P5 > 32767)
	{
		dig_P5 = dig_P5 - 65536;
	}
	
	dig_P6 = gelesen[10];
	dig_P6 += gelesen[11]<<8;
	dig_P6 = (int)dig_P6;
	if (dig_P6 > 32767)
	{
		dig_P6 = dig_P6 - 65536;
	}
	
	dig_P7 = gelesen[12];
	dig_P7 += gelesen[13]<<8;
	dig_P7 = (int)dig_P7;
	if (dig_P7 > 32767)
	{
		dig_P7 = dig_P7 - 65536;
	}

	dig_P8 = gelesen[14];
	dig_P8 += gelesen[15]<<8;
	dig_P8 = (int)dig_P8;
	if (dig_P8 > 32767)
	{
		dig_P8 = dig_P8 - 65536;
	}
	
	dig_P9 = gelesen[16];
	dig_P9 += gelesen[17]<<8;
	dig_P9 = (int)dig_P9;
	if (dig_P9 > 32767)
	{
		dig_P9 = dig_P9 - 65536;
	}
	
	dig_H1 = gelesen[19];
	dig_H1 = (int)dig_H1;

	return;
}

void BME280_CB_trimmH(I2C_TxRxError_t fehler)
{
	if (fehler == I2C_TXRXERROR){return;}
	uint8_t gelesen[7];
	I2C_Read_Data_From_Buffer(gelesen,7);
	
	dig_H2 = gelesen[0];
	dig_H2 += gelesen[1]<<8;
	dig_H2 = (int)dig_H2;
	if (dig_H2 > 32767)
	{
		dig_H2 = dig_H2 - 65536;
	}

	dig_H3 = gelesen[2];
	dig_H3 = (int)dig_H3;

	dig_H4 = (gelesen[4]&0b00001111);
	dig_H4 += (gelesen[3])<<4;
	dig_H4 = (int)dig_H4;
	if (dig_H4 > 2047)
	{
		dig_H4 = dig_H4 - 4096;
	}
	
	dig_H5 = ((gelesen[4]&0b11110000)<<4);
	dig_H5 += gelesen[5];
	dig_H5 = (int)dig_H5;
	if (dig_H5 > 2047)
	{
		dig_H5 = dig_H5 - 4096;
	}
	
	dig_H6 = gelesen[6];
	dig_H6 = (int)dig_H6;
	
	return;
}

void BME280_CB_trimmT(I2C_TxRxError_t fehler)
{
	if (fehler == I2C_TXRXERROR){return;}
	uint8_t gelesen[6];
	I2C_Read_Data_From_Buffer(gelesen,6);

	dig_T1 = gelesen[0];
	dig_T1 += gelesen[1]<<8;
	dig_T1 = (int)dig_T1;

	dig_T2 = gelesen[2];
	dig_T2 += gelesen[3]<<8;
	dig_T2 = (int)dig_T2;
	if (dig_T2 > 32767)
	{
		dig_T2 = dig_T2 - 65536;
	}
	
	dig_T3 = gelesen[4];
	dig_T3 += gelesen[5]<<8;
	dig_T3 = (int)dig_T3;
	if (dig_T3 > 32767)
	{
		dig_T3 = dig_T3 - 65536;
	}
	
	return;
}

double BME280_berechneP(double adc_P)
{	
	double var1, var2, p;
	var1 = ((double)t_fine/2.0) - 64000.0;
	var2 = var1 * var1  * ((double)dig_P6) / 32768.0;
	var2 = var2 + var1 * ((double)dig_P5) *2.0;
	var2 = (var2/4.0)+(((double)dig_P4) * 65536.0);
	var1 = (((double)dig_P3) * var1 * var1  / 524288 + ((double)dig_P2) * var1) / 524288.0;
	var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);
	if (var1 == 0.0)
	{
		return 0;
	}
	p =1048576.0 - (double)adc_P;
	p = (p - (var2 / 4096.0)) * 6250.0 / var1;
	var1 = ((double)dig_P9) * p * p / 2147483648.0;
	var2 = p * ((double)dig_P8) / 32768.0;
	p = p + (var1 + var2 + ((double)dig_P7)) / 16.;
	p = p/100;
	
	return p ;
}

double BME280_berechneH(double adc_H)
{
	double var_H;
	
	var_H = (((double)t_fine) - 76800.0);
	var_H = (adc_H - (((double)dig_H4) * 64.0 +((double)dig_H5) / 16384.0 * var_H)) * (((double)dig_H2) / 65536.0 * (1.0 + ((double)dig_H6) / 67108864.0 * var_H * (1.0 + ((double)dig_H3) / 67108864.0 *var_H)));
	var_H = var_H * (1.0 - ((double) dig_H1) * var_H / 524288.0);

	if (var_H > 100.0)
	{
		var_H = 100.0;
	}
	else if (var_H < 0.0)
	{
		var_H = 0.0;
	}

	return var_H;
}

double BME280_berechneT(float adc_T)
{
	float var1, var2, t;
	
	var1 = (((float)adc_T)/16384.0 - ((float) dig_T1)/1024.0) * ((float)dig_T2);
	var2 = ((((float)adc_T)/131072.0 - ((float)dig_T1)/8192.0) * (((float)adc_T)/131072.0 - ((float)dig_T1)/8192.0)) * ((float)dig_T3) ;
	t_fine = (var1 + var2);
	t = (var1 + var2)/5210.0;
	
	return t;
}

void BME280_CB_GetMeasurmentData(I2C_TxRxError_t fehler)
{
	if (fehler == I2C_TXRXERROR){return;}
	uint8_t messwerte[20];
	int32_t temperatur;
	int32_t pressure;
	int32_t humidty;

	I2C_Read_Data_From_Buffer(messwerte,8);
	
	pressure = (uint32_t)messwerte[0]<<12;
	pressure += messwerte[1]<<4;
	pressure += (messwerte[2] & 0b11110000)>>4;
	
	temperatur = (uint32_t)messwerte[3]<<12;
	temperatur += messwerte[4]<<4;
	temperatur += (messwerte[5] & 0b11110000)>>4;
	
	humidty = (uint32_t)messwerte[6]<<8;
	humidty += messwerte[7];
	
	if (humidty == 32767)
	{
		Pres_StartMeasurement();
	}
	
	daten->Temperature_value = BME280_berechneT(temperatur);
	daten->Pressure_value = BME280_berechneP(pressure);
	daten->Humidty_value = BME280_berechneH(humidty);
	return;
}

void BME280_CB(I2C_TxRxError_t fehler)
{
	return;
}

// Funktionen für Schnittstelle

Pres_Error_t Pres_InitSensor(PressureSensorData_t *PressureSensorData)
{
	if (PressureSensorData == 0) return Pres_ERROR;

	daten = PressureSensorData;
	
	uint8_t bytes[1] = {0x88};
	I2C_TxData_t SendData;
	SendData.Address = BME280_ADDRESS;
	SendData.NumberOfBytes = BME280_LEN_R;
	memcpy(SendData.TxBytes,bytes,BME280_LEN_R);
	SendData.callback_function = &BME280_CB_trimmT;
	I2C_Error_t Error_T = I2C_Read(SendData,BME280_DELAY_TIME,6);

	uint8_t bytesP[BME280_LEN_R] = {0x8E};
	I2C_TxData_t SendDataP;
	SendDataP.Address = BME280_ADDRESS;
	SendDataP.NumberOfBytes = BME280_LEN_R;
	memcpy(SendDataP.TxBytes,bytesP,BME280_LEN_R);
	SendDataP.callback_function = &BME280_CB_trimmP;
	I2C_Error_t Error_P = I2C_Read(SendDataP,BME280_DELAY_TIME,20);
	
	uint8_t bytesH[BME280_LEN_R] = {0xE1};
	I2C_TxData_t SendDataH;
	SendDataH.Address = BME280_ADDRESS;
	SendDataH.NumberOfBytes = BME280_LEN_R;
	memcpy(SendDataH.TxBytes,bytesH,BME280_LEN_R);
	SendDataH.callback_function = &BME280_CB_trimmH;
	I2C_Error_t Error_H = I2C_Read(SendDataH,BME280_DELAY_TIME,7);
	
	if (Error_T || Error_P ||Error_H) {return Pres_ERROR;}
	return Pres_NOERROR;
}

Pres_Error_t Pres_StartMeasurement(void)
{
	uint8_t bytes[BME280_LEN_W] = {0xF4, 0b00100111};
	I2C_TxData_t SendData;
	SendData.Address = BME280_ADDRESS;
	SendData.NumberOfBytes = BME280_LEN_W;
	memcpy(SendData.TxBytes,bytes,BME280_LEN_W);
	SendData.callback_function = &BME280_CB;
	I2C_Error_t Error_A = I2C_Write(SendData);
	
	uint8_t dat[BME280_LEN_W] = {0xF2, 0b001};
	I2C_TxData_t humidtyon;
	humidtyon.Address = 0x77;
	humidtyon.NumberOfBytes = BME280_LEN_W;
	memcpy(humidtyon.TxBytes,dat,BME280_LEN_W);
	humidtyon.callback_function = &BME280_CB;
	I2C_Error_t Error_B = I2C_Write(humidtyon);
	
	uint8_t data[BME280_LEN_W] = {0xF5, 0x00};
	I2C_TxData_t filter;
	filter.Address = 0x77;
	filter.NumberOfBytes = BME280_LEN_W;
	memcpy(filter.TxBytes,data,BME280_LEN_W);
	filter.callback_function = &BME280_CB;
	I2C_Error_t Error_C = I2C_Write(filter);
	
	if (Error_A || Error_B || Error_C ) {return Pres_ERROR;}
	return Pres_NOERROR;
}

Pres_Error_t Pres_GetSensorData(void)
{
	uint8_t bytesGet[BME280_LEN_R] = {0xF7};
	I2C_TxData_t SendData;
	SendData.Address = BME280_ADDRESS;
	SendData.NumberOfBytes = 1;
	memcpy(SendData.TxBytes,bytesGet,BME280_LEN_R);
	SendData.callback_function = &BME280_CB_GetMeasurmentData;
	I2C_Error_t Error = I2C_Read(SendData, BME280_DELAY_TIME, 9);
	
	if (Error) {return Pres_ERROR;}
	return Pres_NOERROR;
}

Pres_Error_t Pres_Reset(void)
{
	uint8_t dat[BME280_LEN_W] = {0xE0, 0xB6};
	I2C_TxData_t data;
	data.Address = 0x77;
	data.NumberOfBytes = BME280_LEN_W;
	memcpy(data.TxBytes,dat,BME280_LEN_W);
	data.callback_function = &BME280_CB;
	I2C_Error_t Error = I2C_Write(data);
		
	if (Error) {return Pres_ERROR;}
	return Pres_NOERROR;
}
