/*
 * CO2_Output.c
 *
 * Created: 03.02.2021 14:28:42
 *  Author: Niklas Theis
 */

#include "co2_output.h"
#include "led_lib.h"

CO2Output_Error_t CO2Output_Init(SensorData_t* sensorData, LCD_Settings_t* LCDSettings, CO2Output_AlignValueRight_t align)
{
	LCDSettings->LineCount = LINE_COUNT;
	LCDSettings->Linelength = MAX_CHAR_COUNT;
	LCDSettings->LineList = (char*) &OutputData;
	LCD_Init(LCDSettings);
	SensorData = sensorData;
	AlignSetting = align;
	CO2Output_UpdateData();
	
	return CO2Output_NoError;
}

CO2Output_Error_t CO2Output_UpdateData()
{
	if (SensorData == 0)
	{
		return CO2Output_Error;	
	}
	char tmp[20];
	ConvertFloatToCharArray(tmp, SensorData->co2_value_f);
	sprintf(OutputData.co2Value, "CO2: %s ppm", tmp);
	ConvertFloatToCharArray(tmp, SensorData->humidity_value_f);
	sprintf(OutputData.humidityValue, "Humidity: %s%%", tmp);
	ConvertFloatToCharArray(tmp, SensorData->temperature_value_f);
	sprintf(OutputData.temperatureValue, "Temp: %sßC", tmp); //ß wegen ROM Code A00 (°)
	sprintf(OutputData.firmwareVersion, "Firmware: %d.%d", (SensorData->firmware_version_u16 >> 8), (SensorData->firmware_version_u16 & 0xFF));
	if (SensorData->MeasState_en == 1) sprintf(OutputData.measState, "MeasState: On");
	else sprintf(OutputData.measState, "MeasState: Off");
	if (SensorData->AutocalibMode_en == 1) sprintf(OutputData.autocalibMode, "AutoCalib: On");
	else sprintf(OutputData.autocalibMode, "AutoCalib: Off");
	LCD_UpdateData((char*) &OutputData, MAX_CHAR_COUNT, 6);
	CO2Output_UpdateLEDs();
	return CO2Output_NoError;
}

CO2Output_Error_t CO2Output_MoveUp()
{
	LCD_MoveUp();
	return CO2Output_NoError;
}

CO2Output_Error_t CO2Output_MoveDown()
{
	LCD_MoveDown();
	return CO2Output_NoError;
}

CO2Output_Error_t CO2Output_UpdateLEDs(){
	if (SensorData->co2_value_f <= 1000){
		led_deactivate(LED1);
		led_deactivate(LED2);
		led_activate(LED0);
	} else if (SensorData->co2_value_f > 1000 && SensorData->co2_value_f <= 2000){
		led_deactivate(LED0);
		led_deactivate(LED2);
		led_activate(LED1);
	} else{
		led_deactivate(LED0);
		led_deactivate(LED1);
		led_activate(LED2);
	}
	
	return CO2Output_NoError;
}