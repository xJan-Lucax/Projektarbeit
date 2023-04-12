/*************************************************************************
* Title		: EEPROM driver
* Author	: Jan-Luca Nettingsmeier
* Created	: 11.08.2022 09:40:20
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
    
USAGE:
	
NOTES:
	                   
*************************************************************************/
  

 #include "uart.h"
 #include "eeprom.h"
 #include "timer_lib.h"
 #include <avr/eeprom.h>
 #include <stdio.h>
 #include <util/delay.h>
 
 //Identifier for driver. If you change these whole memory will be deleted 
 #define EEPROM_IDENTIFIER 122
 #define EEPROM_IDENTIFIER_2 123
 
 //max number of applications that can use the driver. Default is 10
 #define MAX_APPS_POSSIBLE 10

 //global count arrays 
 uint8_t apps_address[MAX_APPS_POSSIBLE];
 uint8_t apps_in_memory_space[MAX_APPS_POSSIBLE];

 //global count variables
 uint8_t apps_register_current_location_in_memory = 2;
 uint8_t apps_register_current_location_in_memory_for_next_variable = 3;
 uint8_t apps_registered = 0;
 uint8_t apps_found = 0;
 //important global arrays that contain all memory addresses and lenght of variables (array[application[variable]])
 uint8_t array_apps_with_addresses[MAX_APPS_POSSIBLE][16];
 uint8_t array_apps_with_lenght[MAX_APPS_POSSIBLE][16];


 eeprom_init(){
	if (eeprom_check_identifier() == false)
	{
		deleteall();
		eeprom_write(0, EEPROM_IDENTIFIER);
		eeprom_write(1, EEPROM_IDENTIFIER_2);
		WDTCSR = (1 << WDE);
	}
	else
	{
		eeprom_find_apps();
	}

 }
uint8_t nextapp = 2;

 //core function that scans eeeprom at startup for old applications
 void eeprom_find_apps(){

	//searching at eeprom location two for the first application
	
	uint8_t nextapp_value;
	
	//if location two is empty no old application in memory else searching for next
	nextapp_value = eeprom_read(nextapp);
	
	if (nextapp_value == 255)
	{
		uart_puts("Keine Applications registriert\r\n");
	} 
	else
	{
		apps_address[apps_found]=nextapp;
		apps_in_memory_space[apps_found]=nextapp_value;
		apps_found++;
		uart_puts("App gefunden !\r\n");

		nextapp = nextapp + nextapp_value +1;
		nextapp_value = eeprom_read(nextapp);

		while (nextapp_value != 255)
		{
			
			apps_address[apps_found]=nextapp;
			apps_in_memory_space[apps_found]=nextapp_value;
			apps_found++;
			uart_puts("App gefunden !\r\n");

			nextapp = nextapp + nextapp_value +1;
			nextapp_value = eeprom_read(nextapp);

		}
		uart_puts("Keine weitere App gefunden !\r\n");
	}

 }

//to get an eeprom storage area use this function
//gives back the app_number to read and write variables
//give this function an array of variablesizes and the number of variables
/************************************************************************/
/* For example your applications needs 3 Variables that have 16byte, 2byte and 1 byte.
   uint8_t array_of_variables[3]= {16, 2, 1};
   uint8_t number_of_variables = 3;
   app_number =   eeprom_register_app(array_of_variables, number_of_variables)
   IMPORTANT:
   If you need more than 254 Bytes for your application use two storage areas.
   Simple split your variables and call this function twice.

                                                                        */
/************************************************************************/  
 uint8_t eeprom_register_app(uint8_t array_of_variables[], uint8_t number_of_variables){
	 
	uint8_t space_of_memory_for_app = 0;
	space_of_memory_for_app = calculate_size_of_memory(array_of_variables, number_of_variables);
	
	
	if (apps_registered < apps_found)
	{
		if (space_of_memory_for_app == apps_in_memory_space[apps_registered])
		{
			apps_registered++;
			apps_register_current_location_in_memory = apps_register_current_location_in_memory+space_of_memory_for_app+1;
			calculate_apps_memory_for_variables(array_of_variables);
			uart_puts("App Registirert die mit alter uebereinstimmt\r\n");
		}
		else
		{
			uart_puts("App ist nicht die gleiche die an dieser Stelle im Speicher erwartet wurde.\r\n");
			deleteall_without_id();
			WDTCSR = (1 << WDE);
		}
	} 
	else
	{
		//app is same as in memory
		apps_register_current_location_in_memory = apps_register_current_location_in_memory+space_of_memory_for_app+1;
		apps_registered++;
		calculate_apps_memory_for_variables(array_of_variables);
		uart_puts("Neue App Registirert\r\n");
		
		//TEST
		uint8_t size_current_app = calculate_size_of_memory(array_of_variables, number_of_variables);
		uart_put("TEST");
		uart_putn(size_current_app);
		uart_putn(nextapp);
		eeprom_write(nextapp, size_current_app);
		
	}
	return apps_registered;
 }

 //function that calculates memory requirement of each application
 uint8_t calculate_size_of_memory(uint8_t array_of_variables[], uint8_t number_of_variables){
	uint8_t size_of_memory = 0;
	uint8_t n = number_of_variables;
	for (uint8_t i=0;i<n;i++)
	{
		size_of_memory = size_of_memory+array_of_variables[i];
	}
	return size_of_memory;
 }

 //function that calculates the memory address of each variable from each application
 void calculate_apps_memory_for_variables(uint8_t array_of_variables[]){
	uart_put("Stelle der Variablen berechnen");
		uart_puts("\r\n");
	for (uint8_t i=0; array_of_variables[i]!=0; i++)
	{
		//uart_puts("Variable: ");
		//uart_putn(array_of_variables[i]);
		//uart_puts("\r\n");

		array_apps_with_lenght[apps_registered][i] = array_of_variables[i];
		//uart_puts("Variable in Global: ");
		//uart_putn(array_apps_with_lenght[apps_registered][i]);
		//uart_puts("\r\n");

		array_apps_with_addresses[apps_registered][i] = apps_register_current_location_in_memory_for_next_variable;

		//uart_puts("Stelle im Speicher: ");
		//uart_putn(array_apps_with_addresses[apps_registered][i]);
		//uart_puts("\r\n");

		apps_register_current_location_in_memory_for_next_variable = apps_register_current_location_in_memory_for_next_variable+array_of_variables[i];

		//uart_puts("Naechste Variable: ");
		//uart_putn(apps_register_current_location_in_memory_for_next_variable);
		//uart_puts("\r\n");
	}

	apps_register_current_location_in_memory_for_next_variable++;
 }
 
 //function that checks the eeprom identifiers
 bool eeprom_check_identifier(){
	uint8_t id1 = eeprom_read(0);
	uint8_t id2 = eeprom_read(1);
	if (id1 == EEPROM_IDENTIFIER && id2 == EEPROM_IDENTIFIER_2)
	{
		return true;
	} 
	else
	{
		return false;
	}
 }

//simple write function using avr
void eeprom_write(uint8_t address, uint8_t *value) {
	eeprom_update_byte(address, value);
}

//function for writing an array
void eeprom_write_array(uint8_t address, uint8_t value[], uint8_t bytes) {
	for(uint8_t i = 0; i < bytes; i++){
		eeprom_update_byte(address+i, value[i]);
	}
}

//function to use in an application to write a variable
void eeprom_driver_write(uint8_t app_number, uint8_t value[], uint8_t number_of_variable){
	number_of_variable = number_of_variable-1;
	uint8_t address = array_apps_with_addresses[app_number][number_of_variable];
	eeprom_write_array(address, value, array_apps_with_lenght[app_number][number_of_variable]);
}

//simple read function using avr
uint8_t eeprom_read(uint8_t address){
	return eeprom_read_byte((const uint8_t*) address);
}

//function for reading one byte variables
uint8_t eeprom_driver_read_uint8_t(uint8_t app_number, uint8_t number_of_variable){
	uint8_t temp;
	number_of_variable = number_of_variable-1;
	temp = eeprom_read_byte((const uint8_t*) array_apps_with_addresses[app_number][number_of_variable]);
	return temp;
}

//function to use in an application to read a variable that is longer than one byte
void eeprom_driver_read(uint8_t* array_data, uint8_t app_number, uint8_t number_of_variable){
	//Damit Variablen 1-.... Variablen -1
	number_of_variable = number_of_variable-1;

	for(uint8_t i = 0; i < array_apps_with_lenght[app_number][number_of_variable]; i++){
		array_data[i] = eeprom_read_byte((const uint8_t*) array_apps_with_addresses[app_number][number_of_variable]+i);
	}
	//TEST
	/**
	uart_put("Lese Variable :");
	uart_putn(number_of_variable);
	uart_put("\r\n");
	uart_putc("array:");
	uint8_t ii = sizeof(array_data);
	for (uint8_t i=0;i<ii;i++)
	{
		uart_putn(array_data[i]);
	}
	uart_put("\r\n");
	uart_putc("Adresse:");
	uart_putn(array_apps_with_addresses[app_number][number_of_variable]);
	uart_put("\r\n");
	**/
}

//delete functions
 void deleteall() {
	for(uint16_t i = 0; i < 4096; i++){
		eeprom_update_byte(i, 255);
	}
	uart_put("! EEPROM alles geloescht !\r\n");
 }
 void deleteall_without_id() {
	  for(uint16_t i = 2; i < 4096; i++){
		  eeprom_update_byte(i, 255);
	  }
	  uart_put("! EEPROM alles geloescht !\r\n");
  }
  void deleteall_at_address(uint8_t address) {
	  for(uint16_t i = address; i < 4096; i++){
		  eeprom_update_byte(i, 255);
	  }
	  uart_put("! EEPROM alles geloescht !\r\n");
  }