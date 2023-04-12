/*
 * Board.c
 *
 * Created: 13.05.2021 20:28:35
 *  Author: Jonas Bruns
			Philipp Husemann (add SPS30)
 */ 
#include "sensors.h"

// Choose Board: 0 = SCD30, 1= BME280
#define Humidity_Sensor 0
#define Temperatur_Sensor 0

char ausgabe_temperature[20];
char ausgabe_pressure[20];
char ausgabe_humdity[20];

sensor_data_t *daten_sensoren;
PressureSensorData_t pressDaten;
SensorData_t co2Daten;

//SPS30:
SensorDataSPS30_t SensorData_st;
SensorConfigData1_t SensorConfigData_st;

SensorConfigData_t CO2_Config =
	{
		.altitude_in_m_u16 = 0,
		.meas_interval_in_sec_u16 = 2,
		.ambient_pressure_in_mbar_u16 = 0,
		.temp_offset_u16 = 0
	};

Sens_Error_t sensor_init(sensor_data_t *SensorData)
{
	daten_sensoren = SensorData;
	Sens_Error_t Error = Sens_NOERROR;
	if (I2C_InitModule()){Error = Sens_ERROR;}
	if (SPS30_InitSensor(&SensorData_st, &SensorConfigData_st))	{Error = Sens_ERROR;}
	if (Pres_InitSensor(&pressDaten)){Error = Sens_ERROR;}
	if (CO2_InitSensor(&co2Daten, &CO2_Config)){Error = Sens_ERROR;}
	return Error;
}

Sens_Error_t sensor_start_measurement(void)
{
	Sens_Error_t Error = Sens_NOERROR;
	if (CO2_StartMeasurement(1000)){Error = Sens_ERROR;}
	if (Pres_StartMeasurement()){Error = Sens_ERROR;}
	if (CO2_UpdateSensorParameterData()){Error = Sens_ERROR;}
	if(SPS30_StartMeasurement()){Error = Sens_ERROR;}

	if(SPS30_UpdateSensorParameterData()){Error = Sens_ERROR;}

	daten_sensoren->AutocalibMode_en = co2Daten.AutocalibMode_en;
	daten_sensoren->firmware_version_u16_SCD30 = co2Daten.firmware_version_u16;
	daten_sensoren->MeasState_en = co2Daten.MeasState_en;

	//SPS30:
	daten_sensoren->MeasStateSPS30_en=SensorData_st.MeasState_en;
	daten_sensoren->productTyp_SPS30 = SensorData_st.productTyp;
	daten_sensoren->serialNumber_SPS30 = SensorData_st.serialNumber;
	daten_sensoren->firmware_version_u16_SPS30 = SensorData_st.firmware_version;
	daten_sensoren->error_hardware = SensorData_st.Hardware_e;
	daten_sensoren->cleaningIntervall1 = SensorData_st.cleaning_intervall;
	return Error;
}

Sens_Error_t sensor_get_data(void)
{
	Sens_Error_t Error = Sens_NOERROR;
	if (CO2_GetDataReadyStatus()){Error = Sens_ERROR;}
	if (Pres_GetSensorData()){Error = Sens_ERROR;}
	//SPS30:
	if(SPS30_GetDataReadyStatus()){Error = Sens_ERROR;}
	if(SensorData_st.new_data_available_u16==1 )
	{

		if(SPS30_GetMeasurementData()){Error = Sens_ERROR;}
		
	}
	
	if (co2Daten.new_data_available_u16 ==1)
	{

		if (CO2_GetMeasurementData()){Error = Sens_ERROR;}
		if (CO2_UpdateSensorParameterData()){Error = Sens_ERROR;}
	}
	if (CO2_StartMeasurement(daten_sensoren->Pressure_value)){Error = Sens_ERROR;}
	daten_sensoren->Pressure_value = pressDaten.Pressure_value;
	daten_sensoren->CO2_value = co2Daten.co2_value_f;
	if (Temperatur_Sensor == 0) daten_sensoren->Temperature_value = co2Daten.temperature_value_f;
	if(Temperatur_Sensor == 1) daten_sensoren->Temperature_value = pressDaten.Temperature_value;
	if (Humidity_Sensor == 0) daten_sensoren->Humidty_value = co2Daten.humidity_value_f;
	if (Humidity_Sensor == 1) daten_sensoren->Humidty_value = pressDaten.Humidty_value;
	daten_sensoren->AutocalibMode_en = co2Daten.AutocalibMode_en;
	daten_sensoren->firmware_version_u16_SCD30 = co2Daten.firmware_version_u16;
	daten_sensoren->firmware_version_u16_SPS30 = SensorData_st.firmware_version;
	daten_sensoren->MeasState_en = co2Daten.MeasState_en;
	//SPS30:
	daten_sensoren->MeasStateSPS30_en=SensorData_st.MeasState_en;
	daten_sensoren->mcon_pm1_0 = SensorData_st.mcon_pm1_0;
	daten_sensoren->mcon_pm2_5 = SensorData_st.mcon_pm2_5;
	daten_sensoren->mcon_pm4_0 = SensorData_st.mcon_pm4_0;
	daten_sensoren->mcon_pm10 = SensorData_st.mcon_pm10;
	daten_sensoren->ncon_pm0_5 = SensorData_st.ncon_pm0_5;
	daten_sensoren->ncon_pm1_0 = SensorData_st.ncon_pm1_0;
	daten_sensoren->ncon_pm2_5 = SensorData_st.ncon_pm2_5;
	daten_sensoren->ncon_pm4_0 = SensorData_st.ncon_pm4_0;
	daten_sensoren->ncon_pm10 = SensorData_st.ncon_pm10;
	daten_sensoren->typ_particle_size = SensorData_st.typ_particle_size;
	daten_sensoren->cleaningIntervall1 = SensorData_st.cleaning_intervall;
	daten_sensoren->MeasStateSPS30_en=SensorData_st.MeasState_en;
	return Error;
}