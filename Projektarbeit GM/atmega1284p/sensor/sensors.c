/*
 * Board.c
 *
 * Created: 13.05.2021 20:28:35
 *  Author: Jonas Bruns
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
	daten_sensoren->AutocalibMode_en = co2Daten.AutocalibMode_en;
	daten_sensoren->firmware_version_u16 = co2Daten.firmware_version_u16;
	daten_sensoren->MeasState_en = co2Daten.MeasState_en;
	return Error;
}

Sens_Error_t sensor_get_data(void)
{
	Sens_Error_t Error = Sens_NOERROR;
	if (CO2_GetDataReadyStatus()){Error = Sens_ERROR;}
	if (Pres_GetSensorData()){Error = Sens_ERROR;}
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
	daten_sensoren->firmware_version_u16 = co2Daten.firmware_version_u16;
	daten_sensoren->MeasState_en = co2Daten.MeasState_en;
	return Error;
}