/*
 * Sensor.h
 *
 * Created: 17.05.2021 16:41:38
 *  Author: Jonas Bruns
 *			Philipp Husemann (add SPS30)
 */ 

#pragma once

#include "pressure_sens.h"
#include "co2_sensor.h"
#include "particulate_matter_sensor.h"


struct sensor_data_t
{
	double Pressure_value;
	double Humidty_value;
	double Temperature_value;
	double CO2_value;
	CO2_AutocalibMode_t AutocalibMode_en;
	CO2_MeasurementState_t MeasState_en;
	uint16_t firmware_version_u16_SCD30;

	//FEINSTAUBSENSOR DATEN:
	double mcon_pm1_0;
	double mcon_pm2_5;
	double mcon_pm4_0;
	double mcon_pm10;
	double ncon_pm0_5;
	double ncon_pm1_0;
	double ncon_pm2_5;
	double ncon_pm4_0;
	double ncon_pm10;
	double typ_particle_size;
	char* serialNumber_SPS30;
	char* productTyp_SPS30;
	int error_hardware;
	SPS30_MeasurementState_t MeasStateSPS30_en;
	uint16_t firmware_version_u16_SPS30;
	uint32_t cleaningIntervall1;
};

typedef struct sensor_data_t sensor_data_t;

typedef enum Sens_Error_t
{
	Sens_NOERROR = 0,
	Sens_ERROR = 1
} Sens_Error_t;

Sens_Error_t sensor_init(sensor_data_t *BoardData);
Sens_Error_t sensor_start_measurement(void);
Sens_Error_t sensor_get_data(void);