/*************************************************************************
* Title		: spi_buffer.h
* Author	: Dimitri Dening
* Created	: 04.11.2021 15:54:43
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
* NOTES:
*	This file should only be included from <spi.h>, never directly.
*************************************************************************/

/**
@file spi_buffer.h
@author Dimitri Dening
@date 04.11.2021
@copyright (C) 2021 Dimitri Dening, GNU General Public License Version 3
@brief SPI transmit and receive buffer.
@note This file should only be included from <spi.h>, never directly.
@bug No known bugs.
*/
#ifndef SPI_BUFFER_H_
#define SPI_BUFFER_H_

/* General libraries */
#include <stdint.h>

/* User defined libraries */
#include "spi_error_handler.h"

#define TX_BUFFER_SIZE 64
#define TX_BUFFER_MASK (TX_BUFFER_SIZE - 1)
#define RX_BUFFER_SIZE 64
#define RX_BUFFER_MASK (RX_BUFFER_SIZE - 1)
#define DX_BUFFER_SIZE 16
#define DX_BUFFER_MASK (DX_BUFFER_SIZE - 1)

#if (TX_BUFFER_SIZE & TX_BUFFER_MASK)
#	error "TX_Buffer must be a power of 2"
#endif
#if (RX_BUFFER_SIZE & RX_BUFFER_MASK)
#	error "RX_Buffer must be a power of 2"
#endif
 
typedef void (*spi_callback)(void); // Forward declaration

typedef struct slave_info slave_info;  // Forward declaration

/* Describes a SPI task */
typedef struct spt_t {
	uint8_t* data;
	uint8_t data_head;
	uint8_t data_tail;
	uint8_t priority;
	uint8_t opt;
	slave_info* slave;
	spi_callback callback;
} spt_t;

/* Describes the SPI TX Buffer */
typedef struct spi_tx {
	spt_t task_list[TX_BUFFER_SIZE];
	uint8_t tx_head;
	uint8_t tx_tail;
	uint8_t is_empty;
} spi_tx;

/* Describes the SPI RX Buffer */
typedef struct spi_rx {
	uint8_t data[RX_BUFFER_SIZE];
	uint8_t number_of_bytes_requested;
	uint8_t rx_head;
	uint8_t rx_tail;
} spi_rx;

/**
 * @brief	Initializes the transmit and receive buffer for SPI transmits.
 *
 *			The tx_buffer contains a task list of limited size.
 *			The rx_buffer can receive a limited size of data.
 *			The created tx_buffer can be used to store new SPI tasks e.g. spi_create_task(buffer, ...)
 *
 * @return	None
 */
void spi_buffer_init(spi_tx* tx_buffer, spi_rx* rx_buffer);

/** 
 * @brief	Create a new task to transmit via SPI.
 *
 *			This function is used by the <spi.h> file during the transmit operation.
 *			Unlike the spi_write() function, this function only stores a new task
 *			in the buffer, but does NOT initiate a transmit by writing the first
 *			dataword to the SPDR.
 *			A task can be linked to a specific buffer, if more than one buffer is used.
 *
 * @param	buffer			Specifies the buffer that a task is linked to.	
 * @param	slave			Specifies a slave that a task is linked to.
 * @param	data			Array containing 8-bit datawords.
 * @param	number_of_bytes	You must pass the amount of datawords that the array contains.
 *							Adding a wrong number of bytes can cause data loss when receiving the data on another device.
 * @param	priority		Task priority. Higher priority tasks get executed earlier than lower priority tasks.
 * @param	opt				Optional argument to change task behaviour.
 * @param	spi_callback cb User defined callback function. Linked to a specific task.
 * @return					Returns an SPI error code if an operation fails.
 *							Otherwise SPI_NO_ERROR (0) is returned.
 */
spi_error_t spi_create_task(spi_tx* buffer, slave_info* slave, uint8_t* data, uint8_t number_of_bytes, uint8_t priority, uint8_t opt, spi_callback cb);

#endif /* SPI_BUFFER_H_ */