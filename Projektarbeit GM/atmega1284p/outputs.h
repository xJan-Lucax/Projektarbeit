/*
 * Outputs.h
 *
 * Created: 17.05.2021 16:41:38
 *  Author: Jonas Bruns
 */ 

#pragma once

#include <avr/io.h>
#include <stdio.h>

#include "LCD.h"
#include "utils.h"
#include "sensors.h"

#define LINE_COUNT 7

enum Output_Error_t
{
	Output_NoError = 0,
	Output_Error = 1,
};
typedef enum Output_Error_t Output_Error_t;

enum Output_AlignValueRight_t
{
	no_align = 0,
	AlignIfPossible = 1
};
typedef enum Output_AlignValueRight_t Output_AlignValueRight_t;

struct Output_Data_t
{
	char co2Value[MAX_CHAR_COUNT];
	char humidityValue[MAX_CHAR_COUNT];
	char temperatureValue[MAX_CHAR_COUNT];
	char pressureValue[MAX_CHAR_COUNT];
	char autocalibMode[MAX_CHAR_COUNT];
	char measState[MAX_CHAR_COUNT];
	char firmwareVersion[MAX_CHAR_COUNT];
};
typedef struct Output_Data_t Output_Data_t;

Output_Error_t output_init(sensor_data_t* SensorData, LCD_Settings_t* LCDSettings, Output_AlignValueRight_t align);
Output_Error_t output_activate_interrupt();
Output_Error_t output_deactivate_interrupt();
Output_Error_t Output_UpdateLEDs();
Output_Error_t output_update_data();
