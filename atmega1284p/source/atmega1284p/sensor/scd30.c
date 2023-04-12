/*
 * SCD30.c
 *
 * Created: 22.01.2021
 *  Author: Marvin Thümmler
 */ 
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include "co2_sensor.h"

void Callback(I2C_TxRxError_t error)
{
	return;
}

CO2_Error_t CO2_StartMeasurement(uint16_t Ambient_Pressure_in_mBar)
{
	if (Ambient_Pressure_in_mBar != 0) //0 is default 1013.25 mBar
	{
		if (Ambient_Pressure_in_mBar < 700 || Ambient_Pressure_in_mBar > 1400) //Check for valid range
		{
			return CO2_ERROR;
		}
	}
	
	uint8_t txbytes[CO2_LEN_W_ARG] = {0x00, 0x10,  Ambient_Pressure_in_mBar >> 8, Ambient_Pressure_in_mBar & 0xFF, calc_CRC_8(Ambient_Pressure_in_mBar)};
	
	I2C_TxData_t SendData_t;
	SendData_t.Address = CO2_ADDRESS;
	SendData_t.NumberOfBytes = CO2_LEN_W_ARG;
	memcpy(SendData_t.TxBytes,txbytes,CO2_LEN_W_ARG);
	//SendData_t.TxBytes = txbytes;
	SendData_t.callback_function = &Callback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	if (Error == I2C_NOERROR) SensorDataCO2->MeasState_en = CO2_MEAS_RUNNING;
	return Error;
}

CO2_Error_t CO2_StopMeasurement(void)
{
	uint8_t txbytes[CO2_LEN_W_NOARG] = {0x01, 0x04};
		
	I2C_TxData_t SendData_t;
	SendData_t.Address = CO2_ADDRESS;
	SendData_t.NumberOfBytes = CO2_LEN_W_NOARG;
	memcpy(SendData_t.TxBytes,txbytes,CO2_LEN_W_NOARG);
	//SendData_t.TxBytes = txbytes;
	SendData_t.callback_function = &Callback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	if (Error == I2C_NOERROR) SensorDataCO2->MeasState_en = CO2_MEAS_STOPPED;
	return Error;
}
CO2_Error_t CO2_SetMeasurementInterval(uint16_t Interval_in_s)
{
	if (Interval_in_s < 2|| Interval_in_s > 1800) //Check for valid range
	{
		return CO2_ERROR;
	}

	uint8_t txbytes[CO2_LEN_W_ARG] = {0x46, 0x00, Interval_in_s >> 8, Interval_in_s & 0xFF, calc_CRC_8(Interval_in_s)};

	I2C_TxData_t SendData_t;
	SendData_t.Address = CO2_ADDRESS;
	SendData_t.NumberOfBytes = CO2_LEN_W_ARG;
	//SendData_t.TxBytes = txbytes;
	memcpy(SendData_t.TxBytes,txbytes,CO2_LEN_W_ARG);	
	SendData_t.callback_function = &Callback;
		
	I2C_Error_t Error = I2C_Write(SendData_t);
	return Error;
}
void CO2_CB_ReceiveMeasurementInterval(I2C_TxRxError_t error)
{
	if(error == I2C_TXRXERROR) return; // No data available
	
	uint8_t rxBytes[CO2_LEN_R_VAL];
	if(I2C_Read_Data_From_Buffer(rxBytes,CO2_LEN_R_VAL) == I2C_ERROR) return;
	
	uint16_t MI = (uint16_t)(((uint16_t)rxBytes[0] << 8) | (uint16_t)rxBytes[1]); //Convert array to 16 Bit value
	if (calc_CRC_8(MI) != rxBytes[2]) return; // Checksum doesn't match
	
	SensorConfigDataCO2->meas_interval_in_sec_u16 = MI;
}
void CO2_GetMeasurementInterval(void)
{
	uint8_t txbytes[CO2_LEN_W_NOARG] = {0x46, 0x00};
	
	I2C_TxData_t SendData_st;
	SendData_st.Address = CO2_ADDRESS;
	SendData_st.NumberOfBytes = CO2_LEN_W_NOARG;
	//SendData_st.TxBytes = txbytes;
	memcpy(SendData_st.TxBytes,txbytes,CO2_LEN_W_NOARG);	
	SendData_st.callback_function = &CO2_CB_ReceiveMeasurementInterval;
	
	I2C_Read(SendData_st, CO2_DELAY_TIME,CO2_LEN_R_VAL);
	
}
void CO2_CB_ReceiveSensorData(I2C_TxRxError_t error)
{
	if(error == I2C_TXRXERROR) return; 
	
	uint8_t rxBytes[CO2_LEN_R_SENSORDATA];
	if(I2C_Read_Data_From_Buffer(rxBytes,CO2_LEN_R_SENSORDATA) == I2C_ERROR) return;
	//CRC Check
	for(uint8_t i = 0; i < 18; i++)
	{
		if((i+1) % 3 == 0) //Locations of CRC Bytes every 3rd Byte
		{
			uint16_t tempCRC = (rxBytes[i-2] << 8) | rxBytes[i-1];
			if (calc_CRC_8(tempCRC) != rxBytes[i]) return;
		}		
	}
	//All CRCs correct
	uint32_t tempCO2 = (uint32_t)((((uint32_t)rxBytes[0]) << 24) | (((uint32_t)rxBytes[1]) << 16) | (((uint32_t)rxBytes[3]) << 8) | ((uint32_t)rxBytes[4]));
	uint32_t tempTemp = (uint32_t)((((uint32_t)rxBytes[6]) << 24) | (((uint32_t)rxBytes[7]) << 16) | (((uint32_t)rxBytes[9]) << 8) | ((uint32_t)rxBytes[10]));
	uint32_t tempHum = (uint32_t)((((uint32_t)rxBytes[12]) << 24) | (((uint32_t)rxBytes[13]) << 16) | (((uint32_t)rxBytes[15]) << 8) | ((uint32_t)rxBytes[16]));
	
	//Write values to struct
	SensorDataCO2->co2_value_f = *(float*)&tempCO2;	
	SensorDataCO2->temperature_value_f = *(float*)&tempTemp;
	SensorDataCO2->humidity_value_f = *(float*)&tempHum;

}
CO2_Error_t CO2_GetMeasurementData(void)
{
	uint8_t txbytes[CO2_LEN_W_NOARG] = {0x03, 0x00};
	
	I2C_TxData_t SendData_st;
	SendData_st.Address = CO2_ADDRESS;
	SendData_st.NumberOfBytes = CO2_LEN_W_NOARG;
	//SendData_st.TxBytes = txbytes;
	memcpy(SendData_st.TxBytes,txbytes,CO2_LEN_W_NOARG);
	SendData_st.callback_function = CO2_CB_ReceiveSensorData;
	
	I2C_Error_t Error = I2C_Read(SendData_st, CO2_DELAY_TIME, CO2_LEN_R_SENSORDATA);
	return Error;
}
void CO2_CB_ReceiveDataReadyStatus(I2C_TxRxError_t error)
{				
	if(error == I2C_TXRXERROR) return; // No data available
	
	uint8_t rxBytes[CO2_LEN_R_VAL];
	if(I2C_Read_Data_From_Buffer(rxBytes,CO2_LEN_R_VAL) == I2C_ERROR) return;
	
	uint16_t DRS = (uint16_t)(((uint16_t)rxBytes[0] << 8) | (uint16_t)rxBytes[1]); //Convert array to 16 Bit value
	if (calc_CRC_8(DRS) != rxBytes[2]) return; // Checksum doesn't match
	
	SensorDataCO2->new_data_available_u16 = DRS;

}
CO2_Error_t CO2_GetDataReadyStatus(void)
{
	uint8_t txbytes[CO2_LEN_W_NOARG] = {0x02, 0x02};
	
	I2C_TxData_t SendData_st;
	SendData_st.Address = CO2_ADDRESS;
	SendData_st.NumberOfBytes = CO2_LEN_W_NOARG;
	//SendData_st.TxBytes = txbytes;
	memcpy(SendData_st.TxBytes,txbytes,CO2_LEN_W_NOARG);
	SendData_st.callback_function = CO2_CB_ReceiveDataReadyStatus;
	
	I2C_Error_t Error = I2C_Read(SendData_st, CO2_DELAY_TIME, CO2_LEN_R_VAL);
	return Error;
}
CO2_Error_t CO2_StartAutoCalibrationMode(void)
{
	uint8_t txbytes[CO2_LEN_W_ARG] = {0x53, 0x06, 0x00, CO2_AUTOCAL_ACTIVE,calc_CRC_8(0x0001)};
	
	I2C_TxData_t SendData_t;
	SendData_t.Address = CO2_ADDRESS;
	SendData_t.NumberOfBytes = CO2_LEN_W_ARG;
	//SendData_t.TxBytes = txbytes;
	memcpy(SendData_t.TxBytes,txbytes,CO2_LEN_W_ARG);
	SendData_t.callback_function = &Callback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	return Error;
}
CO2_Error_t CO2_StopAutoCalibrationMode(void)
{
	uint8_t txbytes[CO2_LEN_W_ARG] = {0x53, 0x06, 0x00, CO2_AUTOCAL_INACTIVE,calc_CRC_8(0x0000)};

	I2C_TxData_t SendData_t;
	SendData_t.Address = CO2_ADDRESS;
	SendData_t.NumberOfBytes = CO2_LEN_W_ARG;
	//SendData_t.TxBytes = txbytes;
	memcpy(SendData_t.TxBytes,txbytes,CO2_LEN_W_ARG);
	SendData_t.callback_function = &Callback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	return Error;
}
void CO2_CB_ReceiveAutoCalibrationMode(I2C_TxRxError_t error)
{
	if(error == I2C_TXRXERROR) return; // No data available
	
	uint8_t rxBytes[CO2_LEN_R_VAL];
	if(I2C_Read_Data_From_Buffer(rxBytes,CO2_LEN_R_VAL) == I2C_ERROR) return;
	
	uint16_t ASC = (uint16_t)(((uint16_t)rxBytes[0] << 8) | (uint16_t)rxBytes[1]); //Convert array to 16 Bit value
	if (calc_CRC_8(ASC) != rxBytes[2]) return; // Checksum doesn't match
	
	switch(ASC)
	{
		case CO2_AUTOCAL_ACTIVE:
			SensorDataCO2->AutocalibMode_en = CO2_AUTOCAL_ACTIVE;
			break;
		case CO2_AUTOCAL_INACTIVE:	
			SensorDataCO2->AutocalibMode_en = CO2_AUTOCAL_INACTIVE;
			break;
	}
}
void CO2_GetAutoCalibrationMode(void)
{
	uint8_t txbytes[CO2_LEN_W_NOARG] = {0x53, 0x06};
	
	I2C_TxData_t SendData_st;
	SendData_st.Address = CO2_ADDRESS;
	SendData_st.NumberOfBytes = CO2_LEN_W_NOARG;
	//SendData_st.TxBytes = txbytes;
	memcpy(SendData_st.TxBytes,txbytes,CO2_LEN_W_NOARG);
	SendData_st.callback_function = &CO2_CB_ReceiveAutoCalibrationMode;
	
	I2C_Read(SendData_st, CO2_DELAY_TIME, CO2_LEN_R_VAL);
	
}
CO2_Error_t CO2_SetCO2CalibrationValue(uint16_t Co2_concentration_in_ppm)
{
	if (Co2_concentration_in_ppm < 400 || Co2_concentration_in_ppm > 2000) //Check for valid range
	{
		return CO2_ERROR;
	}
	//Extract MSB & LSB from Co2_concentration_in_ppm
	uint8_t txbytes[CO2_LEN_W_ARG] = {0x52, 0x04, Co2_concentration_in_ppm >> 8, Co2_concentration_in_ppm & 0xFF, calc_CRC_8(Co2_concentration_in_ppm)};
		
	I2C_TxData_t SendData_t;
	SendData_t.Address = CO2_ADDRESS;
	SendData_t.NumberOfBytes = CO2_LEN_W_ARG;
	//SendData_t.TxBytes = txbytes;
	memcpy(SendData_t.TxBytes,txbytes,CO2_LEN_W_ARG);
	SendData_t.callback_function = &Callback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	return Error;
}
void CO2_CB_ReceiveCO2CalibrationValue(I2C_TxRxError_t error)
{
	if(error == I2C_TXRXERROR) return; // No data available
	
	uint8_t rxBytes[CO2_LEN_R_VAL];
	if(I2C_Read_Data_From_Buffer(rxBytes,CO2_LEN_R_VAL) == I2C_ERROR) return;
	
	uint16_t CCV = (uint16_t)(((uint16_t)rxBytes[0] << 8) | (uint16_t)rxBytes[1]); //Convert array to 16 Bit value
	if (calc_CRC_8(CCV) != rxBytes[2]) return; // Checksum doesn't match

	//TODO Handling of data
	
}
void CO2_GetCO2CalibrationValue(void)
{
	uint8_t txbytes[CO2_LEN_W_NOARG] = {0x52, 0x04};
	
	I2C_TxData_t SendData_st;
	SendData_st.Address = CO2_ADDRESS;
	SendData_st.NumberOfBytes = CO2_LEN_W_NOARG;
	//SendData_st.TxBytes = txbytes;
	memcpy(SendData_st.TxBytes,txbytes,CO2_LEN_W_NOARG);
	SendData_st.callback_function = &CO2_CB_ReceiveCO2CalibrationValue;
	
	I2C_Read(SendData_st, CO2_DELAY_TIME, CO2_LEN_R_VAL);	
}
//Temperature is in °C * 100
CO2_Error_t CO2_SetTemperatureOffset(uint16_t Temperature_in_C_times_100)
{
	uint8_t txbytes[CO2_LEN_W_ARG] = {0x54, 0x03, Temperature_in_C_times_100 >> 8, Temperature_in_C_times_100 & 0xFF, calc_CRC_8(Temperature_in_C_times_100)};
		
	I2C_TxData_t SendData_t;
	SendData_t.Address = CO2_ADDRESS;
	SendData_t.NumberOfBytes = CO2_LEN_W_ARG;
	//SendData_t.TxBytes = txbytes;	
	memcpy(SendData_t.TxBytes,txbytes,CO2_LEN_W_ARG);
	SendData_t.callback_function = &Callback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	return Error;
}
void CO2_CB_ReceiveTemperatureOffset(I2C_TxRxError_t error)
{
	if(error == I2C_TXRXERROR) return; // No data available
	
	uint8_t rxBytes[CO2_LEN_R_VAL];
	if(I2C_Read_Data_From_Buffer(rxBytes,CO2_LEN_R_VAL) == I2C_ERROR) return;		
	uint16_t TO = (uint16_t)(((uint16_t)rxBytes[0] << 8) | (uint16_t)rxBytes[1]); //Convert array to 16 Bit value
	if (calc_CRC_8(TO) != rxBytes[2]) return; // Checksum doesn't match
	
	SensorConfigDataCO2->temp_offset_u16 = TO;
}
void CO2_GetTemperatureOffset(void)
{
	uint8_t txbytes[CO2_LEN_W_NOARG] = {0x54, 0x03};
	
	I2C_TxData_t SendData_st;
	SendData_st.Address = CO2_ADDRESS;
	SendData_st.NumberOfBytes = CO2_LEN_W_NOARG;
	//SendData_st.TxBytes = txbytes;
	memcpy(SendData_st.TxBytes,txbytes,CO2_LEN_W_NOARG);
	SendData_st.callback_function = &CO2_CB_ReceiveTemperatureOffset;
	
	I2C_Read(SendData_st, CO2_DELAY_TIME, CO2_LEN_R_VAL);
	
}
CO2_Error_t CO2_SetAltitudeCompensation(uint16_t Altitude_in_m)
{
	uint8_t txbytes[CO2_LEN_W_ARG] = {0x51, 0x02, Altitude_in_m >> 8, Altitude_in_m & 0xFF, calc_CRC_8(Altitude_in_m)};
	I2C_TxData_t SendData_t;
	SendData_t.Address = CO2_ADDRESS;
	SendData_t.NumberOfBytes = CO2_LEN_W_ARG;
	//SendData_t.TxBytes = txbytes;
	memcpy(SendData_t.TxBytes,txbytes,CO2_LEN_W_ARG);
	SendData_t.callback_function = &Callback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	return Error;	
}
void CO2_CB_ReceiveAltitudeCompensation(I2C_TxRxError_t error)
{
	if(error == I2C_TXRXERROR) return; // No data available
	
	uint8_t rxBytes[CO2_LEN_R_VAL];
	if(I2C_Read_Data_From_Buffer(rxBytes,CO2_LEN_R_VAL) == I2C_ERROR) return;
	
	uint16_t AC = (uint16_t)(((uint16_t)rxBytes[0] << 8) | (uint16_t)rxBytes[1]); //Convert array to 16 Bit value
	if (calc_CRC_8(AC) != rxBytes[2]) return; // Checksum doesn't match
	
	SensorConfigDataCO2->altitude_in_m_u16 = AC;
}
void CO2_GetAltitudeCompensation(void)
{
	uint8_t txbytes[CO2_LEN_W_NOARG] = {0x51, 0x02};
	
	I2C_TxData_t SendData_st;
	SendData_st.Address = CO2_ADDRESS;
	SendData_st.NumberOfBytes = CO2_LEN_W_NOARG;
	//SendData_st.TxBytes = txbytes;
	memcpy(SendData_st.TxBytes,txbytes,CO2_LEN_W_NOARG);
	SendData_st.callback_function = &CO2_CB_ReceiveAltitudeCompensation;
	
	I2C_Read(SendData_st, CO2_DELAY_TIME, CO2_LEN_R_VAL);
	
}
void CO2_CB_ReceiveFirmwareVersion(I2C_TxRxError_t error)
{	
	if(error == I2C_TXRXERROR) return; // No data available

	uint8_t rxBytes[CO2_LEN_R_VAL];
	if(I2C_Read_Data_From_Buffer(rxBytes,CO2_LEN_R_VAL) == I2C_ERROR) return;

	uint16_t FV = (uint16_t)(((uint16_t)rxBytes[0] << 8) | (uint16_t)rxBytes[1]); //Convert array to 16 Bit value
	if (calc_CRC_8(FV) != rxBytes[2]) return; // Checksum doesn't match
	SensorDataCO2->firmware_version_u16 = FV;
}
void CO2_GetFirmwareVersion(void)
{
	uint8_t txbytes[CO2_LEN_W_NOARG] = {0xD1, 0x00};
	
	I2C_TxData_t SendData_st;
	SendData_st.Address = CO2_ADDRESS;
	SendData_st.NumberOfBytes = CO2_LEN_W_NOARG;
	//SendData_st.TxBytes = txbytes;
	memcpy(SendData_st.TxBytes,txbytes,CO2_LEN_W_NOARG);
	SendData_st.callback_function = &CO2_CB_ReceiveFirmwareVersion;

	I2C_Read(SendData_st, CO2_DELAY_TIME, CO2_LEN_R_VAL);
	
}
CO2_Error_t CO2_SoftReset(void)
{
	uint8_t txbytes[CO2_LEN_W_NOARG] = {0xD3, 0x04};
	
	I2C_TxData_t SendData_t;
	SendData_t.Address = CO2_ADDRESS;
	SendData_t.NumberOfBytes = CO2_LEN_W_NOARG;
	//SendData_t.TxBytes = txbytes;
	memcpy(SendData_t.TxBytes,txbytes,CO2_LEN_W_NOARG);
	SendData_t.callback_function = &Callback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	return Error;
}
CO2_Error_t CO2_UpdateSensorParameterData(void)
{
	CO2_GetFirmwareVersion();
	CO2_GetAutoCalibrationMode();
	return CO2_NOERROR;
}
CO2_Error_t CO2_ConfigSensor()
{
	
	//If Altitude is to be set, ambient pressure has to be 0
	if (SensorConfigDataCO2->ambient_pressure_in_mbar_u16 == 0)
	{
		CO2_StartMeasurement(SensorConfigDataCO2->ambient_pressure_in_mbar_u16);	
		CO2_SetAltitudeCompensation(SensorConfigDataCO2->altitude_in_m_u16);	
	}
	else if(SensorConfigDataCO2->ambient_pressure_in_mbar_u16 >= 700 && SensorConfigDataCO2->ambient_pressure_in_mbar_u16 <= 1400)
	{
		CO2_StartMeasurement(SensorConfigDataCO2->ambient_pressure_in_mbar_u16);	
	}
	
	if (SensorConfigDataCO2->meas_interval_in_sec_u16 >= 2 && SensorConfigDataCO2->meas_interval_in_sec_u16 <= 1800)
	{
		CO2_SetMeasurementInterval(SensorConfigDataCO2->meas_interval_in_sec_u16);	
	}

	CO2_SetTemperatureOffset(SensorConfigDataCO2->temp_offset_u16);
		
	return CO2_NOERROR;		
}
CO2_Error_t CO2_InitSensor(SensorData_t *SensorData_st, SensorConfigData_t *SensorConfigData_st)
{
	if(SensorData_st == 0) return CO2_ERROR;
	if(SensorConfigData_st == 0) return CO2_ERROR;

	
	// Set Pointer to local variable
	SensorDataCO2 = SensorData_st;
	SensorConfigDataCO2 = SensorConfigData_st;
	//I2C_InitModule();
	
	return CO2_NOERROR;
}
