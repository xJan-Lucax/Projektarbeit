/************************************************************************
* Title: SPS30 Driver for I2C
* Author: Philipp Husemann
* Created: 24.04.2022
*                                                                     
************************************************************************/




#pragma once

#include "I2C_Driver.h"
#include "crc_sps_scd.h" //Use the same checksum calculation like the SCD30
 
#define SPS30_ADDRESS 0x69
#define SPS30_DELAY_TIME 4

#define SPS30_LEN_W_NOARG 2
#define SPS30_LEN_W_ARG 5
#define SPS30_LEN_W_2ARG 8
#define SPS30_LEN_R_VAL 3
#define SPS30_LEN_R_SENSORDATA 60
#define SPS30_LEN_R_SENSORINFO 48
#define SPS30_LEN_R_2VAL 6
#define SPS30_LEN_R_PRODUCTTYP 12

enum SPS30_Error_t
{
	SPS30_NOERROR = 0,
	SPS30_ERROR = 1
};
typedef enum SPS30_Error_t SPS30_Error_t;


enum SPS30_MeasurementState_t
{
	SPS30_MEAS_STOPPED = 0,
	SPS30_MEAS_RUNNING = 1
};
typedef enum SPS30_MeasurementState_t SPS30_MeasurementState_t;

enum SPS30_SleepState_t
{	
	SPS30_SLEEP_OFF = 0,
	SPS30_SLEEP_ON = 1
};
typedef enum SPS30_SleepState_t SPS30_SleepState_t;


enum SPS30_Hardware_Errors_t
{
	SPS30_NO_ERROR = 0,
	SPS30_FAN_SPEED = 1,
	SPS30_LASER = 2,
	SPS30_FAN = 3
	
};
typedef enum SPS30_Hardware_Errors_t SPS30_Hardware_Errors_t;


struct SensorDataSPS30_t
{
	uint16_t new_data_available_u16;
	float mcon_pm1_0;
	float mcon_pm2_5;
	float mcon_pm4_0;
	float mcon_pm10;
	float ncon_pm0_5;
	float ncon_pm1_0;
	float ncon_pm2_5;
	float ncon_pm4_0;
	float ncon_pm10;
	float typ_particle_size;
	SPS30_MeasurementState_t MeasState_en;
	SPS30_SleepState_t SleepState_en;
	SPS30_Hardware_Errors_t Hardware_e;
	uint16_t firmware_version;
	uint32_t cleaning_intervall;
	char serialNumber[48];
	char productTyp[9];
};
typedef struct SensorDataSPS30_t SensorDataSPS30_t;

struct SensorConfigData1_t
{
	uint32_t auto_cleaning_intervall_u16;
};
typedef struct SensorConfigData1_t SensorConfigData1_t;

SensorDataSPS30_t *SensorDataSPS30;
SensorConfigData1_t *SensorConfigDataSPS30;

/**
*	@brief	Initializes the SPS30 sensor.
*	@param	Check the definitions above for the entire valid argument list.
*	@return Returns a SPS30 error code if the initialization fails, otherwise an zero for no error returns
*/
SPS30_Error_t SPS30_InitSensor(SensorDataSPS30_t *SensorData_st, SensorConfigData1_t *SensorConfigData_st);

/**
*	@brief Check whether new data is available
*	@param none
*	@Returns a SPS30 error code if query fails, otherwise an zero for no error returns
*/
SPS30_Error_t SPS30_GetDataReadyStatus(void);

/**
*	@brief Get available data from SPS30
*	@param none
*	@Returns a SPS30 error code if query fails, otherwise an zero for no error returns
*/
SPS30_Error_t SPS30_GetMeasurementData(void);

/*
*	@brief Update all values whitch are not part of continuous measurement 
*	@param none
*	@return Returns a SPS30 error code if query fails, otherwise an zero for no error returns
*/
SPS30_Error_t SPS30_UpdateSensorParameterData(void);

/*
*	@brief Starts continuous measurement
*	@param none
*	@return Returns a SPS30 error code if query fails, otherwise an zero for no error returns
*/
SPS30_Error_t SPS30_StartMeasurement(void);

/*
*	@brief Stops continuous measurement
*	@param none
*	@return Returns a SPS30 error code if query fails, otherwise an zero for no error returns
*/
SPS30_Error_t SPS30_StopMeasurement(void);

/*
*	@brief Makes a soft reset
*	@param none
*	@return Returns a SPS30 error code if query fails, otherwise an zero for no error returns
*/
SPS30_Error_t SPS30_SoftReset(void);

/*
*	@brief Brings SPS30 to sleep mode if continuous measurement is stopped 
*	@param none
*	@return Returns a SPS30 error code if query fails, otherwise an zero for no error returns
*/
SPS30_Error_t SPS30_Sleep(void);

/*
*	@brief Awakes SPS30 out of sleep mode
*	@param none
*	@return Returns a SPS30 error code if query fails, otherwise an zero for no error returns
*/
SPS30_Error_t SPS30_Wake_Up(void);

/*
*	@brief Get the auto cleaning interval of the SPS30
*	@param none
*	@return Returns a SPS30 error code if query fails, otherwise an zero for no error returns
*/
void SPS30_GetAutoCleaningIntervall(void);



/*
*	@brief Set the auto cleaning interval of the SPS30
*	@param cleaningIntervall in seconds
*	@return Returns a SPS30 error code if query fails, otherwise an zero for no error returns
*/
SPS30_Error_t SPS30_SetAutoCleaningIntervall(uint32_t cleaningIntervall);

/*
*	@brief Get the product type of the SPS30
*	@param none
*	@return none
*/
void SPS30_GetProductType(void);

/*
*	@brief Get the serial number of the SPS30
*	@param none
*	@return none
*/
void SPS30_GetSerialNumber(void);

/*
*	@brief Get the version number of the SPS30
*	@param none
*	@return none
*/
void SPS30_getVersion(void);
/*
*	@brief Get the device status register of the SPS30
*	@param none
*	@return none
*/
void SPS30_getDeviceStatusRegister(void);

/*
*	@brief Clear the device status register of the SPS30
*	@param none
*	@return Returns a SPS30 error code if query fails, otherwise an zero for no error returns
*/
SPS30_Error_t SPS30_clearDeviceStatusRegister(void);

/*
*	@brief Update the sensor parameters of the SPS30
*	@param none
*	@return Returns a SPS30 error code if query fails, otherwise an zero for no error returns
*/
SPS30_Error_t SPS30_UpdateSensorParameterData(void);