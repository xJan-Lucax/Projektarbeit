/*************************************************************************
* Title		: outputs.c port from ATmeg16 to ATmega1284P
* Author	: Jonas Bruns (original author)
			  Dimitri Dening (ATmega1284P port)
* Created	: 17.05.2021 16:41:55 (ATmega16 version)
* Modified	: 23.11.2021 12:01:00 (ATmega1284P port)
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
    
USAGE:
	
NOTES:
               
*************************************************************************/
/* General libraries */
#include <avr/interrupt.h>

/* User defined libraries */
#include "outputs.h"
#include "led_lib.h"

sensor_data_t* SensorData;
Output_Data_t OutputData;
Output_AlignValueRight_t AlignSetting;

Output_Error_t output_init(sensor_data_t* SensorData_t, LCD_Settings_t* LCDSettings, Output_AlignValueRight_t align){
	LCDSettings->LineCount = LINE_COUNT;
	LCDSettings->Linelength = MAX_CHAR_COUNT;
	LCDSettings->LineList = (char*) &OutputData;
	LCD_Init(LCDSettings);
	SensorData = SensorData_t;
	AlignSetting = align;
	output_activate_interrupt();
	output_update_data();
	return Output_NoError;
}

Output_Error_t output_update_data() {
	Output_Error_t Error = Output_NoError;
	if (SensorData == 0)
	{
		return Output_Error;
	}
	char tmp[20];
	ConvertFloatToCharArray(tmp, SensorData->CO2_value);
	sprintf(OutputData.co2Value, "CO2: %s ppm", tmp);
	ConvertFloatToCharArray(tmp, SensorData->Humidty_value);
	sprintf(OutputData.humidityValue, "Humidity: %s%%", tmp);
	ConvertFloatToCharArray(tmp, SensorData->Temperature_value);
	sprintf(OutputData.temperatureValue, "Temp: %sßC", tmp);
	ConvertFloatToCharArray(tmp, SensorData->Pressure_value);
	sprintf(OutputData.pressureValue, "Pres: %shPA",tmp);
	sprintf(OutputData.firmwareVersion, "Firmware: %d.%d", (SensorDataCO2->firmware_version_u16 >> 8), (SensorDataCO2->firmware_version_u16 & 0xFF));
	if (SensorData->MeasState_en == 1) sprintf(OutputData.measState, "MeasState: On");
	else sprintf(OutputData.measState, "MeasState: Off");
	if (SensorData->AutocalibMode_en == 1) sprintf(OutputData.autocalibMode, "AutoCalib: On");
	else sprintf(OutputData.autocalibMode, "AutoCalib: Off");
	if (LCD_UpdateData((char*) &OutputData, MAX_CHAR_COUNT, 7)){Error=Output_Error;}
	Output_UpdateLEDs();
	return Error;
}

Output_Error_t Output_UpdateLEDs(){
	if (SensorData->CO2_value <= 1000){
		led_deactivate(LED1);
		led_deactivate(LED2);
		led_activate(LED0);
	}
	
	if (SensorData->CO2_value > 1000 && SensorData->CO2_value <= 2000){
		led_deactivate(LED0);
		led_deactivate(LED2);
		led_activate(LED1);
	}
	
	if (SensorData->CO2_value > 2000){
		led_deactivate(LED0);
		led_deactivate(LED1);
		led_activate(LED2);
	}
	
	return Output_NoError;
}

ISR(INT0_vect) {
	LCD_MoveDown();
	output_deactivate_interrupt();
}


ISR(INT1_vect) {
	LCD_MoveUp();
	output_deactivate_interrupt();
}

Output_Error_t output_activate_interrupt() {
	DDRD = 0b01100000;  //  SW 
	PORTD = 0b00001100; //  SW
	EICRA |= (1 << ISC11) | (1 << ISC01);
	EIMSK |= (1 << INT0) | (1 << INT1);
	return Output_NoError;
};

Output_Error_t output_deactivate_interrupt() {
	EIMSK &= ~(1 << INT0);
	EIMSK &= ~(1 << INT1);	
	EICRA &= ~(1 << ISC11); 
	EICRA &= ~(1 << ISC01); 
	return Output_NoError;
};