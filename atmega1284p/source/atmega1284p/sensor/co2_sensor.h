  
  #pragma once

  #include "i2c_driver.h"
  #include "crc_sps_scd.h"
  #define CO2_ADDRESS 0x61
  #define CO2_DELAY_TIME 4

  //Constants for data length
  #define CO2_LEN_W_NOARG 2
  #define CO2_LEN_W_ARG 5
  #define CO2_LEN_R_VAL 3
  #define CO2_LEN_R_SENSORDATA 18


  enum CO2_Error_t
  {
	  CO2_NOERROR = 0,
	  CO2_ERROR = 1
  };
  typedef enum CO2_Error_t CO2_Error_t;

  enum CO2_AutocalibMode_t
  {
	  CO2_AUTOCAL_INACTIVE = 0x00,
	  CO2_AUTOCAL_ACTIVE = 0x01
  };
  typedef enum CO2_AutocalibMode_t CO2_AutocalibMode_t;

  enum CO2_MeasurementState_t
  {
	  CO2_MEAS_STOPPED = 0,
	  CO2_MEAS_RUNNING = 1
  };
  typedef enum CO2_MeasurementState_t CO2_MeasurementState_t;

  struct SensorData_t
  {
	  uint16_t new_data_available_u16;
	  float co2_value_f;
	  float humidity_value_f;
	  float temperature_value_f;
	  CO2_AutocalibMode_t AutocalibMode_en;
	  CO2_MeasurementState_t MeasState_en;
	  uint16_t firmware_version_u16;
  };
  typedef struct SensorData_t SensorData_t;

  struct SensorConfigData_t
  {
	  uint16_t meas_interval_in_sec_u16;
	  uint16_t ambient_pressure_in_mbar_u16;
	  uint16_t altitude_in_m_u16;
	  uint16_t temp_offset_u16;
  };
  typedef struct SensorConfigData_t SensorConfigData_t;


  SensorData_t *SensorDataCO2;
  SensorConfigData_t *SensorConfigDataCO2;

  CO2_Error_t CO2_InitSensor(SensorData_t *SensorData_st, SensorConfigData_t *SensorConfigData_st);
  CO2_Error_t CO2_ConfigSensor(void);
  CO2_Error_t CO2_GetDataReadyStatus(void);
  CO2_Error_t CO2_GetMeasurementData(void);
  CO2_Error_t CO2_UpdateSensorParameterData(void);
  CO2_Error_t CO2_StartMeasurement(uint16_t Ambient_Pressure_in_mBar);
  CO2_Error_t CO2_StopMeasurement(void);
  CO2_Error_t CO2_StartAutoCalibrationMode(void);
  CO2_Error_t CO2_StopAutoCalibrationMode(void);
  CO2_Error_t CO2_SetCO2CalibrationValue(uint16_t Co2_concentration_in_ppm);
  CO2_Error_t CO2_SoftReset(void);
  void CO2_GetTemperatureOffset(void);