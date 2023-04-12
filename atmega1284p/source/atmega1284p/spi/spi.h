/*************************************************************************
* Title		: SPI Master Implementation
* Author	: Dimitri Dening
* Created	: 25.10.2021 17:56:35
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
* License	: MIT License
* Usage		: see Doxygen manual
*
*		Copyright (C) 2021 Dimitri Dening
*
*		Permission is hereby granted, free of charge, to any person obtaining a copy
*		of this software and associated documentation files (the "Software"), to deal
*		in the Software without restriction, including without limitation the rights
*		to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*		copies of the Software, and to permit persons to whom the Software is
*		furnished to do so, subject to the following conditions:
*
*		The above copyright notice and this permission notice shall be included in all
*		copies or substantial portions of the Software.
*
*		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*		IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*		FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*		AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*		LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*		OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*		SOFTWARE.
*
*************************************************************************/

/**
@file spi.h    
@author Dimitri Dening
@date 25.10.2021
@copyright (C) 2021 Dimitri Dening, MIT License
@brief Single Master implementation to communicate with spi devices using a ringbuffer. 
@note Modify the <spi_io.h> port declaration if using another MCU.
@bug The greater the physical distance from PIN4 of a slave device on the STK600, the greater the likelihood of failed transmission packets at higher speeds.
     e.g. : Device connected to PIN4 and PIN3 is able to operate at 8Mhz with SPI_CLOCK_DIV4 and lower.
			PIN0 to PIN2 are only able to operate at 8Mhz with SPI_CLOCK_DIV8 and lower.
@note Connect the error led (see <led_lib.h>) on the STK600 to monitor possible error codes. 
	  Occuring errors are described in <spi_error_handler.h>			
@usage The following code shows typical usage of this library.

@code
	#include "spi.h"
	
	void callback_function (void){ // Do stuff in callback function }
		
	int main(void){
		
		uint8_t tx_data[] = { 0x83, 0x00, 0x00, 0x00 };
			
		uint8_t* rx_data = (uint8_t*)malloc(sizeof(uint8_t) * 4);
		
		slave_info slave_device = spi_create_slave(PINB4, PORTB4, DDB4);
		
		spi_init(SPI_MSB, SPI_MODE0, SPI_CLOCK_DIV4);
		
		spi_write(&slave_device, tx_data, 4, NORMAL, DEFAULT, &callback_function);
		
		spi_read(&slave_device, rx_data, 4, 0x00, NORMAL, &callback_function); 
		
		free(rx_data);
		
		for(;;);
	}	
@endcode           
*/
#ifndef SPI_H_
#define SPI_H_

/* General libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* User defined libraries */
#include "spi_io.h"
#include "spi_buffer.h"
#include "spi_error_handler.h"

typedef void (*spi_callback)(void);

/* Task priority */
typedef enum priority_t{
	LOW,
	NORMAL,
	HIGH
} priority_t;

/* Optional arguments for spi_write */
typedef enum optional_t{
	DEFAULT,		// Default behaviour. Pulls CS high if task finished.
	CONTINUE_LOW,	// Keeps the CS line low after the transmission is done.
	STORE_DATA		// Reserved for internal operations in spi_read (DO NOT USE WITH spi_write)
} optional_t;

/* Describes a slave device */
typedef struct slave_info{
	uint8_t pin;
	uint8_t port;
	uint8_t ddr;
} slave_info;

typedef enum{
	SPI_MSB = 0x00,
	SPI_LSB = 0x01
} data_order_t;

typedef enum mode_t{
	SPI_MODE0 = 0x00, // CPOL = 0, CPHA = 0
	SPI_MODE1 = 0x01, // CPOL = 0, CPHA = 1
	SPI_MODE2 = 0x02, // CPOL = 1, CPHA = 0
	SPI_MODE3 = 0x03  // CPOL = 1, CPHA = 1
} mode_t;

typedef enum{
	SPI_CLOCK_DIV4 = 0x00,
	SPI_CLOCK_DIV16 = 0x01,
	SPI_CLOCK_DIV64 = 0x02,
	SPI_CLOCK_DIV128 = 0x03,
	// The following clock settings will set the SPI2X bit in the SPSR.
	// The SCK frequency will be doubled when the SPI is in master mode.
	SPI_CLOCK_DIV2 = 0x04,
	SPI_CLOCK_DIV8 = 0x05,
	SPI_CLOCK_DIV32 = 0x06,
	SPI_CLOCK_DIV64X = 0x07
} clock_rate_t;

/**
 * @brief	Initializes the SPI as master. 	
 * @param	Check the definitions above for the entire valid argument list.
 * @return	Returns an SPI error code if an operation fails.
 *			Otherwise SPI_NO_ERROR (0) is returned.
 */
spi_error_t spi_init(data_order_t data_order, mode_t mode, clock_rate_t clock_rate);

/**
 * @brief Transmits 8-bit data via SPI. 
 
 * The write function creates a single SPI task, which gets added to the current buffer.
 * The transmission then gets initialized by writing the first dataword to the SPDR.
 * When the task is finished the CS line is pulled high again.
 * It's recommended to check the return value for a possible error.
	
 * @param	slave			Transmit data to given slave.
 * @param	data			Array containing 8-bit datawords. The amount of datawords is limited (see <spi_buffer.h> DX_BUFFER_SIZE).
 * @param	number_of_bytes	Pass the amount of datawords that the array contains.
 *							Adding a wrong number of bytes can cause data loss when receiving 
 *							the data on another device.
 * @param	priority		Task priority. Higher priority tasks get executed earlier than lower priority tasks.
 * @param	opt				Optional argument to change behaviour. 
							1.  DEFAULT			: Default behaviour. CS line gets pulled up (high) once the task is completed.
							2.	CONTINUE_LOW	: Keeps the CS line low after the transmission is finished. 
							2.1	Use case example: spi_write command followed by a spi_read command, which requires the
												  CS line to be held low during this entire process.
							3.	STORE_DATA		: Reserved for internal operations in spi_read
 * @param	cb_func:		User defined callback function. Gets executed after a SPI task was completed.
 * @return					Returns an SPI error code if an operation fails. 
 *							Otherwise SPI_NO_ERROR (0) is returned.
 */
spi_error_t spi_write(slave_info* slave, uint8_t* data, uint8_t number_of_bytes, priority_t priority, optional_t opt, spi_callback cb_func);

/** 
 * @brief Receive 8-bit data. 
 * 
 * Each dataword is stored in the RX_BUFFER. The receive command is initiated
 * by transmitting a start byte e.g. dummy packet (0x00).
 *
 * @param	slave			Receive data from given slave.
 * @param	recv			Data from the RX_BUFFER gets extracted and stored in this array.
 * @param	number_of_bytes	Determine how many bytes to read and store in the RX_BUFFER.
 * @param	start_byte		Start communication with given start byte.
 * @param	priority		Task priority. Higher priority tasks get executed earlier than lower priority tasks.
 * @param	cb_func:		User defined callback function. Gets executed when all packets got received.
 * @return					Returns an SPI error code if an operation fails.
 *							Otherwise SPI_NO_ERROR (0) is returned.
 */
spi_error_t spi_read(slave_info* slave, uint8_t* recv, uint8_t number_of_bytes, uint8_t start_byte, priority_t priority, spi_callback cb_func);

/** 
 * @brief	Creates a new slave structure.
 * 
 * The user has to confirm a correct setup of the slave configuration. 
 * This function contains no error checking.
 * The new slave is configured as inactive.
 *
 * @return	Returns the new slave structure.
 */
slave_info spi_create_slave(uint8_t pin, uint8_t port, uint8_t ddr);

/**
 * @brief	Gets the next byte (8-bit dataword) in the RX_BUFFER.
 * @param	data			Read data from the RX_BUFFER and store them in the given array.
 * @param	number_of_bytes	Receive n amount of bytes from the RX_BUFFER.
 * @return	None
 */
void spi_get_byte(uint8_t* data, uint8_t len);

/**
 * @brief	Flushes the TX_BUFFER and RX_BUFFER by resetting the pointers to 
 *			the next data packet and changes the buffer state to empty.
 * @return	Returns an SPI error code if an operation fails.
 *			Otherwise SPI_NO_ERROR (0) is returned.
 */
spi_error_t spi_flush_buffer(void);

/**
 * @brief	Clears the remaining SPDR value from a previous session on a slave device.
 *
 * On restart any old data is still stored in the SPDR and gets shifted on 
 * MISO channel to the Master device possibly causing a wrong data sequence. 
 * This is a simple workaround to clear the previous remaining value in the SPDR.
 *
 * @param	slave	Clear SPDR from given slave.
 * @return	None
 */
void spi_clear(slave_info* slave);

#endif /* SPI_H_ */