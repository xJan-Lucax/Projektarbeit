/*************************************************************************
* Title		: EEPROM driver
* Author	: Jan-Luca Nettingsmeier
* Created	: 11.08.2022 09:30:12
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
    
USAGE:
	
NOTES:
	                   
*************************************************************************/
#pragma once
#include <stdbool.h>

eeprom_init();
bool eeprom_find_start();
void eeprom_find_apps();


//check and calculate functions
bool eeprom_check_identifier();
uint8_t eeprom_register_app(uint8_t array_of_variables[], uint8_t number_of_variables);
uint8_t calculate_size_of_memory(uint8_t array_of_variables[], uint8_t number_of_variables);
void calculate_apps_memory_for_variables(uint8_t array_of_variables[]);

//write functions
void eeprom_write(uint8_t address, uint8_t *value);
void eeprom_write_array(uint8_t address, uint8_t value[], uint8_t bytes);
void eeprom_driver_write(uint8_t app_number, uint8_t value[], uint8_t number_of_variable);

//read functions
uint8_t eeprom_read(uint8_t address);
uint8_t eeprom_driver_read_uint8_t(uint8_t app_number, uint8_t number_of_variable);
void eeprom_driver_read(uint8_t* array_data, uint8_t app_number, uint8_t number_of_variable);

//delete functions
void deleteall();
void deleteall_without_id();
void deleteall_at_address(uint8_t address);

//debug functions
void print_variable_from_eeprom(uint8_t app_number, uint8_t number_of_variable);
