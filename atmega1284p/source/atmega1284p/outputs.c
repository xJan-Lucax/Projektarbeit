/*
 * Outputs.c
 *
 * Created: 17.05.2021 16:41:55
 *  Author: Jonas Bruns
 *			Philipp Husemann (add SPS30)
 */ 

#include <avr/interrupt.h>

#include "outputs.h"
#include "led_lib.h"

sensor_data_t* SensorData1;
Output_Data_t OutputData1;
Output_AlignValueRight_t AlignSetting;

Output_Error_t output_init(sensor_data_t* SensorData_t, LCD_Settings_t* LCDSettings, Output_AlignValueRight_t align){
	LCDSettings->LineCount = LINE_COUNT;
	LCDSettings->Linelength = MAX_CHAR_COUNT;
	LCDSettings->LineList = (char*) &OutputData1;
	LCD_Init(LCDSettings);
	SensorData1 = SensorData_t;
	AlignSetting = align;
	output_activate_interrupt();
	output_update_data();
	return Output_NoError;
}

Output_Error_t output_update_data() {
	Output_Error_t Error = Output_NoError;
	if (SensorData1 == 0)
	{
		return Output_Error;
	}
	char tmp[20];
	//SPS30:
	ConvertFloatToCharArray(tmp, SensorData1->mcon_pm1_0);
	sprintf(OutputData1.c_mcon_pm1_0, "PM1,0:%säg/m3",tmp);
	//uart_puts(OutputData1.c_mcon_pm1_0);
	//uart_puts("\r\n");
	ConvertFloatToCharArray(tmp, SensorData1->mcon_pm2_5);
	sprintf(OutputData1.c_mcon_pm2_5, "PM2,5:%säg/m3", tmp);//ä für my
	//uart_puts(OutputData1.c_mcon_pm2_5);
	//uart_puts("\r\n");
	ConvertFloatToCharArray(tmp, SensorData1->mcon_pm4_0);
	sprintf(OutputData1.c_mcon_pm4_0, "PM4,0:%säg/m3", tmp);
	//uart_puts(OutputData1.c_mcon_pm4_0);
	//uart_puts("\r\n");
	ConvertFloatToCharArray(tmp, SensorData1->mcon_pm10);
	sprintf(OutputData1.c_mcon_pm10, "PM10:%säg/m3", tmp);
	//uart_puts(OutputData1.c_mcon_pm10);
	//uart_puts("\r\n");
	ConvertFloatToCharArray(tmp, SensorData1->ncon_pm0_5);
	sprintf(OutputData1.c_ncon_pm0_5, "PM0,5:%s#/cm3", tmp);
	//uart_puts(OutputData1.c_ncon_pm0_5);
	//uart_puts("\r\n");
	ConvertFloatToCharArray(tmp, SensorData1->ncon_pm1_0);
	sprintf(OutputData1.c_ncon_pm1_0, "PN1,0:%s#/cm3", tmp);
	//uart_puts(OutputData1.c_ncon_pm1_0);
	//uart_puts("\r\n");
	ConvertFloatToCharArray(tmp, SensorData1->ncon_pm2_5);
	sprintf(OutputData1.c_ncon_pm2_5, "PN2,5:%s#/cm3", tmp);
	//uart_puts(OutputData1.c_ncon_pm2_5);
	//uart_puts("\r\n");
	ConvertFloatToCharArray(tmp, SensorData1->ncon_pm4_0);
	sprintf(OutputData1.c_ncon_pm4_0, "PN4,0:%s#/cm3", tmp);
	//uart_puts(OutputData1.c_ncon_pm4_0);
	//uart_puts("\r\n");
	ConvertFloatToCharArray(tmp, SensorData1->ncon_pm10);
	sprintf(OutputData1.c_ncon_pm10, "PN10:%s#/cm3", tmp);
	//uart_puts(OutputData1.c_ncon_pm10);
	//uart_puts("\r\n");
	ConvertFloatToCharArray(tmp, SensorData1->typ_particle_size);
	sprintf(OutputData1.c_typ_particle_size, "TYP:%säg", tmp);
	//uart_puts(OutputData1.c_typ_particle_size);
	//uart_puts("\r\n");
	//uart_puts("\r\n");uart_puts("\r\n");uart_puts("\r\n");uart_puts("\r\n");uart_puts("\r\n");uart_puts("\r\n");uart_puts("\r\n");uart_puts("\r\n");uart_puts("\r\n");uart_puts("\r\n");

	ConvertFloatToCharArray(tmp, SensorData1->CO2_value);
	sprintf(OutputData1.co2Value, "CO2: %s ppm", tmp);
	ConvertFloatToCharArray(tmp, SensorData1->Humidty_value);
	sprintf(OutputData1.humidityValue, "Humidity: %s%%", tmp);
	ConvertFloatToCharArray(tmp, SensorData1->Temperature_value);
	sprintf(OutputData1.temperatureValue, "Temp: %sßC", tmp);
	ConvertFloatToCharArray(tmp, SensorData1->Pressure_value);
	sprintf(OutputData1.pressureValue, "Pres: %shPA",tmp);

	sprintf(OutputData1.firmwareVersion_SCD30, "FirmwareC: %d.%d", (SensorData1->firmware_version_u16_SCD30 >> 8), (SensorData1->firmware_version_u16_SCD30 & 0xFF));
	sprintf(OutputData1.firmwareVersion_SPS30, "FirmwareF: %d.%d", (SensorData1->firmware_version_u16_SPS30>> 8), (SensorData1->firmware_version_u16_SPS30 & 0xFF));

	if (SensorData1->MeasState_en == 1) sprintf(OutputData1.measState_SCD30, "MeasStateC: On");
	else sprintf(OutputData1.measState_SCD30, "MeasStateC: Off");
	if (SensorData1->AutocalibMode_en == 1) sprintf(OutputData1.autocalibMode, "AutoCalib: On");
	else sprintf(OutputData1.autocalibMode, "AutoCalib: Off");

	if (SensorData1->MeasStateSPS30_en == 1) sprintf(OutputData1.measState_SPS30, "MeasStateF: On");
	else sprintf(OutputData1.measState_SPS30, "MeasStateF: Off");


	sprintf(OutputData1.serialNumber,"N:%s",SensorData1->serialNumber_SPS30);
	sprintf(OutputData1.productType,"T:%s",SensorData1->productTyp_SPS30);

	if(SensorData1->error_hardware==0){sprintf(OutputData1.hardware_errors, "Status: NoError");}
	if(SensorData1->error_hardware==1){sprintf(OutputData1.hardware_errors, "Status:FANSPEED");}
	if(SensorData1->error_hardware==2){sprintf(OutputData1.hardware_errors, "Status: LASER");}
	if(SensorData1->error_hardware==3){sprintf(OutputData1.hardware_errors, "Status: FAN");}


	sprintf(OutputData1.cleaningIntervall,"CI: %lu",SensorData1->cleaningIntervall1);


	if (LCD_UpdateData((char*) &OutputData1, MAX_CHAR_COUNT, 17)){Error=Output_Error;}
	Output_UpdateLEDs();
	return Error;
}

Output_Error_t Output_UpdateLEDs(){
	if (SensorData1->CO2_value <= 1000){
		led_deactivate(LED1);
		led_deactivate(LED2);
		led_activate(LED0);
	}
	
	if (SensorData1->CO2_value > 1000 && SensorData1->CO2_value <= 2000){
		led_deactivate(LED0);
		led_deactivate(LED2);
		led_activate(LED1);
	}
	
	if (SensorData1->CO2_value > 2000){
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