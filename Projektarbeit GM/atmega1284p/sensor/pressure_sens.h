/*
 * PressureSensor.h
 *
 * Created: 17.05.2021 16:41:38
 *  Author: Jonas Bruns
 */ 

#pragma once

#include <avr/io.h>
#include <string.h>
#include "i2c_driver.h"

struct PressureSensorData_t 
{
	double Pressure_value;
	double Humidty_value;
	double Temperature_value;
};
typedef struct PressureSensorData_t PressureSensorData_t;

typedef enum Pres_Error_t
{
	Pres_NOERROR = 0,
	Pres_ERROR = 1
} Pres_Error_t;

Pres_Error_t Pres_InitSensor(PressureSensorData_t *PressureSensorData);
Pres_Error_t Pres_StartMeasurement(void);
Pres_Error_t Pres_GetSensorData(void);
Pres_Error_t Pres_Reset(void);