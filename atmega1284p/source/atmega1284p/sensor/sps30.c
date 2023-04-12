/************************************************************************
* Title: SPS30 Driver for I2C
* Author: Philipp Husemann
* Created: 24.04.2022
*
************************************************************************/
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include "particulate_matter_sensor.h"


uint8_t CalcCrc(uint8_t data[2]) {
	uint16_t bytes = (uint16_t)(((uint16_t)data[0] << 8) | (uint16_t)data[1]);
	uint8_t crc = calc_CRC_8(bytes);
	return crc;
}




void noCallback(I2C_TxRxError_t error) // For functions that do not need callback
{
	return;
}


SPS30_Error_t SPS30_StartMeasurement(void){ 
	uint8_t dt[2]= {0x03,0x00}; //Address for values in big-endian IEEE-754 Float with dummy-byte
	uint8_t txbytes[SPS30_LEN_W_ARG] = {0x00,0x10,0x03,0x00,CalcCrc(dt)};
	I2C_TxData_t SendData_t;
	SendData_t.Address = SPS30_ADDRESS;
	SendData_t.NumberOfBytes = SPS30_LEN_W_ARG;
	memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_ARG);
	SendData_t.callback_function = &noCallback;
	I2C_Error_t Error = I2C_Write(SendData_t);
	if (Error == I2C_NOERROR) SensorDataSPS30->MeasState_en = SPS30_MEAS_RUNNING;//Set measurement start in struct
	return Error;
}



SPS30_Error_t SPS30_StopMeasurement(void){
	uint8_t txbytes[SPS30_LEN_W_NOARG] = {0x01, 0x04};
	
	I2C_TxData_t SendData_t;
	SendData_t.Address = SPS30_ADDRESS;
	SendData_t.NumberOfBytes = SPS30_LEN_W_NOARG;
	memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_NOARG);
	SendData_t.callback_function = &noCallback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	if (Error == I2C_NOERROR) SensorDataSPS30->MeasState_en = SPS30_MEAS_STOPPED;//Set measurement stop in struct
	return Error;
}

void SPS30_CB_ReceiveDataReadyStatus(I2C_TxRxError_t error)//Callback for getDataReadyStatus
{
	if(error == I2C_TXRXERROR) return; // No data available
	
	uint8_t rxBytes[SPS30_LEN_R_VAL];
	if(I2C_Read_Data_From_Buffer(rxBytes,SPS30_LEN_R_VAL) == I2C_ERROR) return; // Check that buffer data is correct
	
	uint16_t DRS = (uint16_t)(((uint16_t)rxBytes[0] << 8) | (uint16_t)rxBytes[1]); //Convert array to 16 Bit value
	uint8_t temp[2] = {rxBytes[0],rxBytes[1]};
	if (CalcCrc(temp) != rxBytes[2]) return; // Checksum doesn't match
	
	SensorDataSPS30->new_data_available_u16 = DRS; // Set data-ready status in struct
}

SPS30_Error_t SPS30_GetDataReadyStatus(void){
	uint8_t txbytes[SPS30_LEN_W_NOARG] = {0x02, 0x02};
	
	I2C_TxData_t SendData_st;
	SendData_st.Address = SPS30_ADDRESS;
	SendData_st.NumberOfBytes = SPS30_LEN_W_NOARG;
	memcpy(SendData_st.TxBytes,txbytes,SPS30_LEN_W_NOARG);
	SendData_st.callback_function = &SPS30_CB_ReceiveDataReadyStatus;
	
	I2C_Error_t Error = I2C_Read(SendData_st, SPS30_DELAY_TIME, SPS30_LEN_R_VAL);
	return Error;
	
}

void SPS30_CB_ReceiveSensorData(I2C_TxRxError_t error)// Callback for getMeasurementData
{
	if(error == I2C_TXRXERROR) return; // No data available
	
	uint8_t rxBytes[SPS30_LEN_R_SENSORDATA];
	if(I2C_Read_Data_From_Buffer(rxBytes,SPS30_LEN_R_SENSORDATA) == I2C_ERROR) return;// Check that buffer data is correct
	
	for (int i = 0;i<59;i=i+3) //run over the howl received data
	{
		uint8_t temp [2] = {rxBytes[i],rxBytes[i+1]};
		
		if (CalcCrc(temp)!=rxBytes[i+2])// Check checksum with received checksum
		{
			return;// Checksum doesn't match
		}
	}
	
	uint32_t ui32_mcon_pm1_0 = (uint32_t)((((uint32_t)rxBytes[0]) << 24) | (((uint32_t)rxBytes[1]) << 16) | (((uint32_t)rxBytes[3]) << 8) | ((uint32_t)rxBytes[4]));//Convert array to 32 Bit value
	uint32_t ui32_mcon_pm2_5 = (uint32_t)((((uint32_t)rxBytes[6]) << 24) | (((uint32_t)rxBytes[7]) << 16) | (((uint32_t)rxBytes[9]) << 8) | ((uint32_t)rxBytes[10]));//Convert array to 32 Bit value
	uint32_t ui32_mcon_pm4_0 = (uint32_t)((((uint32_t)rxBytes[12]) << 24) | (((uint32_t)rxBytes[13]) << 16) | (((uint32_t)rxBytes[15]) << 8) | ((uint32_t)rxBytes[16]));//Convert array to 32 Bit value
	uint32_t ui32_mcon_pm10 = (uint32_t)((((uint32_t)rxBytes[18]) << 24) | (((uint32_t)rxBytes[19]) << 16) | (((uint32_t)rxBytes[21]) << 8) | ((uint32_t)rxBytes[22]));//Convert array to 32 Bit value
	uint32_t ui32_ncon_pm0_5 = (uint32_t)((((uint32_t)rxBytes[24]) << 24) | (((uint32_t)rxBytes[25]) << 16) | (((uint32_t)rxBytes[27]) << 8) | ((uint32_t)rxBytes[28])); //Convert array to 32 Bit value
	uint32_t ui32_ncon_pm1_0 = (uint32_t)((((uint32_t)rxBytes[30]) << 24) | (((uint32_t)rxBytes[31]) << 16) | (((uint32_t)rxBytes[33]) << 8) | ((uint32_t)rxBytes[34]));//Convert array to 32 Bit value
	uint32_t ui32_ncon_pm2_5 = (uint32_t)((((uint32_t)rxBytes[36]) << 24) | (((uint32_t)rxBytes[37]) << 16) | (((uint32_t)rxBytes[39]) << 8) | ((uint32_t)rxBytes[40]));//Convert array to 32 Bit value
	uint32_t ui32_ncon_pm4_0 = (uint32_t)((((uint32_t)rxBytes[42]) << 24) | (((uint32_t)rxBytes[43]) << 16) | (((uint32_t)rxBytes[45]) << 8) | ((uint32_t)rxBytes[46]));//Convert array to 32 Bit value
	uint32_t ui32_ncon_pm10 = (uint32_t)((((uint32_t)rxBytes[48]) << 24) | (((uint32_t)rxBytes[49]) << 16) | (((uint32_t)rxBytes[51]) << 8) | ((uint32_t)rxBytes[52]));//Convert array to 32 Bit value
	uint32_t ui32_typ_particle_size = (uint32_t)((((uint32_t)rxBytes[54]) << 24) | (((uint32_t)rxBytes[55]) << 16) | (((uint32_t)rxBytes[57]) << 8) | ((uint32_t)rxBytes[58]));//Convert array to 32 Bit value
	
	SensorDataSPS30->mcon_pm1_0 = *(float*)&ui32_mcon_pm1_0;//Convert 32 Bit value to float and save it in struct
	SensorDataSPS30->mcon_pm2_5=*(float*)&ui32_mcon_pm2_5;//Convert 32 Bit value to float and save it in struct
	SensorDataSPS30->mcon_pm4_0=*(float*)&ui32_mcon_pm4_0;//Convert 32 Bit value to float and save it in struct
	SensorDataSPS30->mcon_pm10=*(float*)&ui32_mcon_pm10;//Convert 32 Bit value to float and save it in struct
	SensorDataSPS30->ncon_pm0_5=*(float*)&ui32_ncon_pm0_5;//Convert 32 Bit value to float and save it in struct
	SensorDataSPS30->ncon_pm1_0=*(float*)&ui32_ncon_pm1_0;//Convert 32 Bit value to float and save it in struct
	SensorDataSPS30->ncon_pm2_5=*(float*)&ui32_ncon_pm2_5;//Convert 32 Bit value to float and save it in struct
	SensorDataSPS30->ncon_pm4_0=*(float*)&ui32_ncon_pm4_0;//Convert 32 Bit value to float and save it in struct
	SensorDataSPS30->ncon_pm10=*(float*)&ui32_ncon_pm10;//Convert 32 Bit value to float and save it in struct
	SensorDataSPS30->typ_particle_size=*(float*)&ui32_typ_particle_size;//Convert 32 Bit value to float and save it in struct
	

}


SPS30_Error_t SPS30_GetMeasurementData(void){
		uint8_t txbytes[SPS30_LEN_W_NOARG] = {0x03, 0x00};
		
		I2C_TxData_t SendData_st;
		SendData_st.Address = SPS30_ADDRESS;
		SendData_st.NumberOfBytes = SPS30_LEN_W_NOARG;
		memcpy(SendData_st.TxBytes,txbytes,SPS30_LEN_W_NOARG);
		SendData_st.callback_function = &SPS30_CB_ReceiveSensorData;
		
		I2C_Error_t Error = I2C_Read(SendData_st, SPS30_DELAY_TIME, SPS30_LEN_R_SENSORDATA);
		return Error;
}


SPS30_Error_t SPS30_Sleep(void){
	uint8_t txbytes[SPS30_LEN_W_NOARG] = {0x10, 0x01};
	I2C_TxData_t SendData_t;
	SendData_t.Address = SPS30_ADDRESS;
	SendData_t.NumberOfBytes = SPS30_LEN_W_NOARG;
	memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_NOARG);
	SendData_t.callback_function = &noCallback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	
	if(Error==I2C_ERROR)
	{
		return Error;
	}
	
	if (Error == I2C_NOERROR)
	{
		 SensorDataSPS30->SleepState_en = SPS30_SLEEP_ON;
	}//Save sleep-state in struct
	return Error;
}
void SPS30_CB_WakeUp(I2C_TxRxError_t error){ //Callback for WakeUp
	if (!error)
	{
		uint8_t txbytes[SPS30_LEN_W_NOARG] = {0x11, 0x03};
		
		I2C_TxData_t SendData_t;
		SendData_t.Address = SPS30_ADDRESS;
		SendData_t.NumberOfBytes = SPS30_LEN_W_NOARG;
		memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_NOARG);
		SendData_t.callback_function = &noCallback;
		I2C_Error_t Error = I2C_Write(SendData_t);
	}else return;
}
SPS30_Error_t SPS30_Wake_Up(void){
	//First run will be ignored, cant be other implemented
	uint8_t txbytes[SPS30_LEN_W_NOARG] = {0x11, 0x03};
		
	I2C_TxData_t SendData_t;
	SendData_t.Address = SPS30_ADDRESS;
	SendData_t.NumberOfBytes = SPS30_LEN_W_NOARG;
	memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_NOARG);
	SendData_t.callback_function = &SPS30_CB_WakeUp;//Second run in Callback
		
	I2C_Error_t Error = I2C_Write(SendData_t);
	

	
	if (Error == I2C_NOERROR) {SensorDataSPS30->SleepState_en = SPS30_SLEEP_ON;}
	return Error;
}



SPS30_Error_t SPS30_StartFanCleaning(void){
	uint8_t txbytes[SPS30_LEN_W_NOARG] = {0x56, 0x07};
	
	I2C_TxData_t SendData_t;
	SendData_t.Address = SPS30_ADDRESS;
	SendData_t.NumberOfBytes = SPS30_LEN_W_NOARG;
	memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_NOARG);
	SendData_t.callback_function = &noCallback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	
	return Error;


}
void SPS30_CB_CleaningIntervall(I2C_TxRxError_t error)
{
	if(error == I2C_TXRXERROR) return;// No data available

	uint8_t rxBytes[SPS30_LEN_R_2VAL];
	if(I2C_Read_Data_From_Buffer(rxBytes,SPS30_LEN_R_2VAL) == I2C_ERROR) return;// Check that buffer data is correct
	
	
	
	for (int i = 0;i<5;i=i+3) //run over the howl received data
	{
		uint8_t temp [2] = {rxBytes[i],rxBytes[i+1]};
		
		if (CalcCrc(temp)!=rxBytes[i+2])// Check checksum with received checksum
		{
			return;// Checksum doesn't match
		}
	}
	uint32_t CI = (uint32_t)((((uint32_t)rxBytes[0]) << 24) | (((uint32_t)rxBytes[1]) << 16) | (((uint32_t)rxBytes[3]) << 8) | ((uint32_t)rxBytes[4]));//Convert array to 32 Bit value	
	SensorDataSPS30->cleaning_intervall = CI; // Set CleaningIntervall to struct
}
void SPS30_GetAutoCleaningIntervall(void){
	uint8_t txbytes[SPS30_LEN_W_NOARG] = {0x80, 0x04};
	
	I2C_TxData_t SendData_t;
	SendData_t.Address = SPS30_ADDRESS;
	SendData_t.NumberOfBytes = SPS30_LEN_W_NOARG;
	memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_NOARG);
	SendData_t.callback_function = &SPS30_CB_CleaningIntervall;
	
	I2C_Read(SendData_t, SPS30_DELAY_TIME,SPS30_LEN_R_2VAL);
	
}

SPS30_Error_t SPS30_SetAutoCleaningIntervall(uint32_t cleaningIntervall){
	
	
	uint8_t temp [4]={cleaningIntervall>>24,(cleaningIntervall>>16),(cleaningIntervall>>8),cleaningIntervall};// Convert 32 Bit value to array
		
	uint8_t t1[2] = {temp[0],temp[1]};
	uint8_t t2[2] = {temp[2],temp[3]};
	uint8_t txbytes[SPS30_LEN_W_2ARG] = {0x80, 0x04,temp[0],temp[1],CalcCrc(t1),temp[2],temp[3],CalcCrc(t2)};
	
	I2C_TxData_t SendData_t;
	SendData_t.Address = SPS30_ADDRESS;
	SendData_t.NumberOfBytes = SPS30_LEN_W_2ARG;
	memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_2ARG);
	SendData_t.callback_function = &noCallback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	

	
	if (Error == I2C_NOERROR) SensorDataSPS30->cleaning_intervall = cleaningIntervall; // Set CleaningIntervall to struct
	return Error;
}
void SPS30_CB_ProductTyp(I2C_TxRxError_t error)
{

	if(error == I2C_TXRXERROR) return;// No data available

	uint8_t rxBytes[SPS30_LEN_R_PRODUCTTYP];

	if(I2C_Read_Data_From_Buffer(rxBytes,SPS30_LEN_R_PRODUCTTYP) == I2C_ERROR){return;}// Check that buffer data is correct
	
	for (int i = 0;i<SPS30_LEN_R_PRODUCTTYP;i=i+3)//run over the howl received data
	{
		uint8_t temp [2] = {rxBytes[i],rxBytes[i+1]};
		
		if (CalcCrc(temp)!=rxBytes[i+2])// Check checksum with received checksum
		{
			return;// Checksum doesn't match
			
		}
	}
		int lauf1 = 0;
	for (int i = 1 ;i<=SPS30_LEN_R_PRODUCTTYP;i++)
	{
		if (i%3!=0 && i!=0)
		{
			SensorDataSPS30->productTyp[lauf1] = rxBytes[i-1];
			lauf1++;
		}
		SensorDataSPS30->productTyp[i] = rxBytes[i];
		
	}
	SensorDataSPS30->productTyp[lauf1] = '\0';
}
void SPS30_GetProductType(void){
				uint8_t txbytes[SPS30_LEN_W_NOARG] = {0xD0, 0x02};
				I2C_TxData_t SendData_t;
				
				SendData_t.Address = SPS30_ADDRESS;
				SendData_t.NumberOfBytes = SPS30_LEN_W_NOARG;
				memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_NOARG);
				
				SendData_t.callback_function = &SPS30_CB_ProductTyp;
				
				I2C_Read(SendData_t, SPS30_DELAY_TIME,SPS30_LEN_R_PRODUCTTYP);
								
}
void SPS30_CB_SerialNumber(I2C_TxRxError_t error)
{
	if(error == I2C_TXRXERROR) return;// No data available

	uint8_t rxBytes[SPS30_LEN_R_SENSORINFO];
	if(I2C_Read_Data_From_Buffer(rxBytes,SPS30_LEN_R_SENSORINFO) == I2C_ERROR) return;// Check that buffer data is correct
	
	for (int i = 0;i<48;i=i+3)//run over the howl received data
	{
		uint8_t temp [2] = {rxBytes[i],rxBytes[i+1]};
		
		if (CalcCrc(temp)!=rxBytes[i+2])// Check checksum with received checksum
		{
			return;// Checksum doesn't match 	
		}
	}

	int lauf1 = 0;
	for (int i = 0 ;i<=32;i++)
	{
		if (i%3!=0&&i!=0)
		{
			SensorDataSPS30->serialNumber[lauf1] = rxBytes[i-1];
			lauf1++;
		}
	}
SensorDataSPS30->serialNumber[lauf1] = '\0';
	
}
void SPS30_GetSerialNumber(void){
			uint8_t txbytes[SPS30_LEN_W_NOARG] = {0xD0, 0x33};
			
			I2C_TxData_t SendData_t;
			SendData_t.Address = SPS30_ADDRESS;
			SendData_t.NumberOfBytes = SPS30_LEN_W_NOARG;
			memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_NOARG);
			SendData_t.callback_function = &SPS30_CB_SerialNumber;
			
			I2C_Read(SendData_t, SPS30_DELAY_TIME,SPS30_LEN_R_SENSORINFO);
}

void SPS30_CB_Version(I2C_TxRxError_t error)
{
	if(error == I2C_TXRXERROR) return;// No data available
	uint8_t rxBytes[SPS30_LEN_R_VAL];
	if(I2C_Read_Data_From_Buffer(rxBytes,SPS30_LEN_R_VAL) == I2C_ERROR) return;// Check that buffer data is correct
	
	uint8_t temp [2]= {rxBytes[0],rxBytes[1]};
	if (rxBytes[2]!=CalcCrc(temp)) return;// Check checksum with received checksum and return if not correct
	uint16_t FV = (uint16_t)(((uint16_t)rxBytes[0] << 8) | (uint16_t)rxBytes[1]); //Convert array to 16 Bit value
	SensorDataSPS30->firmware_version= FV;//First Byte is main version, save to struct
}
void SPS30_getVersion(void){
		uint8_t txbytes[SPS30_LEN_W_NOARG] = {0xD1, 0x00};
		
		I2C_TxData_t SendData_t;
		SendData_t.Address = SPS30_ADDRESS;
		SendData_t.NumberOfBytes = SPS30_LEN_W_NOARG;
		memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_NOARG);
		SendData_t.callback_function = &SPS30_CB_Version;
		
		I2C_Read(SendData_t, SPS30_DELAY_TIME,SPS30_LEN_R_2VAL);
	
}

void SPS30_CB_StatusRegister(I2C_TxRxError_t error){
	if(error == I2C_TXRXERROR) return;// No data available

	uint8_t rxBytes[SPS30_LEN_R_2VAL];
	if(I2C_Read_Data_From_Buffer(rxBytes,SPS30_LEN_R_2VAL) == I2C_ERROR) return;// Check that buffer data is correct
	
	for (int i = 0;i<5;i=i+3)//run over the howl received data
	{
		uint8_t temp [2] = {rxBytes[i],rxBytes[i+1]};
		
		if (CalcCrc(temp)!=rxBytes[i+2])// Check checksum with received checksum
		{
			return;// Checksum doesn't match
		}
	}
	uint32_t SR = (uint32_t)((((uint32_t)rxBytes[0]) << 24) | (((uint32_t)rxBytes[1]) << 16) | (((uint32_t)rxBytes[3]) << 8) | ((uint32_t)rxBytes[4]));//Convert array to 32 Bit value
	
	
	uint8_t t = ((SR << 10)>> 31);//Bitshift for bit 21 (SPEED)
	if (t == 1)
	{
		SensorDataSPS30->Hardware_e = SPS30_FAN_SPEED;//Save SPEED error to struct
	}

	t = 0;
	t = ((SR << 26) >> 31);//Bitshift for bit 5 (Laser)
	if (t == 1)
	{
		SensorDataSPS30->Hardware_e = SPS30_LASER;//Save LASER error to struct
	}
	t = 0;
	t = ((SR << 27) >> 31);//Bitshift for bit 4 (FAN)
	if (t == 1)
	{
		SensorDataSPS30->Hardware_e = SPS30_FAN;//Save FAN error to struct
	}
}
void SPS30_getDeviceStatusRegister(void){
	uint8_t txbytes[SPS30_LEN_W_NOARG] = {0xD2, 0x06};
	
	I2C_TxData_t SendData_t;
	SendData_t.Address = SPS30_ADDRESS;
	SendData_t.NumberOfBytes = SPS30_LEN_W_NOARG;
	memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_NOARG);
	SendData_t.callback_function = &SPS30_CB_StatusRegister;
	
	I2C_Read(SendData_t, SPS30_DELAY_TIME,SPS30_LEN_R_2VAL);
}


SPS30_Error_t SPS30_clearDeviceStatusRegister(void){
	uint8_t txbytes[SPS30_LEN_W_NOARG] = {0xD2, 0x10};
	
	I2C_TxData_t SendData_t;
	SendData_t.Address = SPS30_ADDRESS;
	SendData_t.NumberOfBytes = SPS30_LEN_W_NOARG;
	memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_NOARG);
	SendData_t.callback_function = &noCallback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	return Error;
}




SPS30_Error_t SPS30_InitSensor(SensorDataSPS30_t *SensorData_st, SensorConfigData1_t *SensorConfigData_st){
	if(SensorData_st == 0) return SPS30_ERROR;//Check whether Sensor object exist
	if(SensorConfigData_st == 0) return SPS30_ERROR;//Check whether ConfigData object exist

	
	// Set Pointer to local variable
	SensorDataSPS30 = SensorData_st;
	SensorConfigDataSPS30 = SensorConfigData_st;
	
	if(SPS30_StartFanCleaning()==SPS30_ERROR) return SPS30_ERROR; // Start Fan Cleaning at Start
	
	return SPS30_NOERROR;
}

SPS30_Error_t SPS30_SoftReset(void){
	uint8_t txbytes[SPS30_LEN_W_NOARG] = {0xD3, 0x04};
	
	I2C_TxData_t SendData_t;
	SendData_t.Address = SPS30_ADDRESS;
	SendData_t.NumberOfBytes = SPS30_LEN_W_NOARG;
	memcpy(SendData_t.TxBytes,txbytes,SPS30_LEN_W_NOARG);
	SendData_t.callback_function = &noCallback;
	
	I2C_Error_t Error = I2C_Write(SendData_t);
	return Error;
}
SPS30_Error_t SPS30_UpdateSensorParameterData(void) // Set all parameters in struct
{
	SPS30_GetAutoCleaningIntervall();
	SPS30_GetProductType();
	SPS30_GetSerialNumber();
	SPS30_getVersion();
	SPS30_getDeviceStatusRegister();
	return SPS30_NOERROR;
}
