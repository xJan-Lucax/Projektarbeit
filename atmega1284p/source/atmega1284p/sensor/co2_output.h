/*
 * CO2_Output.h
 *
 * Created: 03.02.2021 14:29:59
 *  Author: Niklas Theis
 */

#pragma once

#include <avr/io.h>
#include <stdio.h>

#include "LCD.h"
#include "co2_sensor.h"
#include "utils.h"

#define LINE_COUNT 6

enum CO2Output_Error_t
{
	CO2Output_NoError = 0,
	CO2Output_Error = 1,
};
typedef enum CO2Output_Error_t CO2Output_Error_t;

enum CO2Output_AlignValueRight_t
{
	NoAlign = 0,
	AlignIfPossible = 1
};
typedef enum CO2Output_AlignValueRight_t CO2Output_AlignValueRight_t;

struct CO2Output_Data_t
{
	char co2Value[MAX_CHAR_COUNT];
	char humidityValue[MAX_CHAR_COUNT];
	char temperatureValue[MAX_CHAR_COUNT];
	char pressureValue[MAX_CHAR_COUNT];
	char autocalibMode[MAX_CHAR_COUNT];
	char measState[MAX_CHAR_COUNT];
	char firmwareVersion[MAX_CHAR_COUNT];
};

typedef struct CO2Output_Data_t CO2Output_Data_t;

SensorData_t* SensorData;

CO2Output_AlignValueRight_t AlignSetting;

CO2Output_Data_t OutputData;

CO2Output_Error_t CO2Output_Init(SensorData_t* sensorData, LCD_Settings_t* LCDSettings, CO2Output_AlignValueRight_t align);

CO2Output_Error_t CO2Output_UpdateData();
CO2Output_Error_t CO2Output_MoveUp();
CO2Output_Error_t CO2Output_MoveDown();
CO2Output_Error_t CO2Output_UpdateLEDs();