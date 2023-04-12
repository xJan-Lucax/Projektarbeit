/*************************************************************************
* Title		: C include file for SPI Error Handling
* Author	: Dimitri Dening
* Created	: 28.10.2021 08:10:49
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
* ERROR LED SEQUENCE:
*
*	SPI_BUFFER_OVERFLOW			| SHORT, SHORT, SHORT
*	----------------------------|---------------------
*	SPI_BUFFER_DATA_OVERWRITE   | SHORT, SHORT, LONG 
*	----------------------------|---------------------
*	SPI_DATA_OVERFLOW           | SHORT, LONG , SHORT 
*	----------------------------|---------------------
*	SPI_INVALID_PORT            | SHORT, LONG , LONG
*	----------------------------|---------------------
*	SPI_WRITE_COLLISION			| LONG , SHORT, SHORT
*	----------------------------|---------------------
*	SPI_FLUSH_FAILED			| LONG , SHORT, LONG
*	----------------------------|---------------------
*	SPI_RECV_BUSY				| LONG , LONG , SHORT
*	----------------------------|---------------------
*	SPI_NOT_DEFINED				| LONG , LONG , LONG
*	
* ERRORS
*
*	SPI_BUFFER_OVERFLOW:
*		1.	More data written to the buffer than allowed.
*			Change TX_BUFFER_SIZE in the <spi_buffer.h> if needed.
*		2.  More bytes requested than the rx_buffer can store.
*			Change RX_BUFFER_SIZE in the <spi_buffer.h> if needed.
*
*	SPI_BUFFER_DATA_OVERWRITE:
*		If tasks are created faster than the MCU can process current tasks in the buffer, 
*		data will be overwritten. 	
*		Increase system frequency or TX_BUFFER size or SPI_CLOCK_DIVx.
*
*	SPI_DATA_OVERFLOW:
*		More data written to the data buffer than allowed.
*		Change DX_BUFFER_SIZE in the <spi_buffer.h> if needed.
*
*	SPI_INVALID_PORT:
*		Certain ports are reserved for SPI functionality. These are not allowed
*		for slave devices. These ports can change depending on the MCU used.
*		Refer to the <spi_io.h> to see which ports are used.
*
*	SPI_WRITE_COLLISION:
*		If the SPI Data Register (SPDR) is written to during a data transfer
*		the WCOL bit is set in the SPSR.
*
*	SPI_FLUSH_FAILED:
*		The flush command was executed while the SPI was still active.
*
*	SPI_RECV_BUSY:
*		The SPI is currently busy sending other data and
*		can't initiate another receive command.
*
*	SPI_NOT_DEFINED:
*		Errors that are either not defined yet or are general errors (e.g. memory allocation failure).
*************************************************************************/

/**
@file spi_error_handler.h
@author Dimitri Dening
@date 28.10.2021
@copyright (C) 2021 Dimitri Dening, MIT License
@brief Error handler for SPI.

Indicates the defined SPI errors on the STK600 by a unique led sequence.
The selected <ERROR_LED> can be changed in the <led_lib.h>.

@bug No known bugs.
@warning The led sequence is produced by using the <util/delay.h> library. This could have 
an effect on other peripherals used at the same time, which are not depended on the SPI.
@todo Implement an interrupt based delay with e.g. timer8.
*/
#ifndef SPI_ERROR_HANDLER_H_
#define SPI_ERROR_HANDLER_H_

/* Describes possible error states */
typedef enum spi_error_t {
	SPI_NO_ERROR,
	SPI_ERR_BUFFER_OVERFLOW,
	SPI_ERR_BUFFER_DATA_OVERWRITE,
	SPI_ERR_DATA_OVERFLOW,
	SPI_ERR_INVALID_PORT,
	SPI_ERR_WRITE_COLLISION,
	SPI_ERR_FLUSH_FAILED,
	SPI_ERR_RECV_BUSY,
	SPI_ERR_NOT_DEFINED
} spi_error_t;

/**
 * @brief	Prototype of an error handler.
 *
 * If an error occurs, a specific led sequence is shown on the STK600.
 *
 * @return	Returns an error code.
 */
spi_error_t error_handler(spi_error_t error);

#endif /* SPI_ERROR_HANDLER_H_ */