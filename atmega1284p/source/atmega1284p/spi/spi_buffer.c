/*************************************************************************
* Title		: spi_buffer.c
* Author	: Dimitri Dening
* Created	: 04.11.2021 15:54:29
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
    SPI transmit and receive buffer.
USAGE:
	see <spi_buffer.h>
               
*************************************************************************/
/* General libraries */
#include <stdlib.h>
#include <string.h>

/* User defined libraries */
#include "spi_buffer.h"

void spi_buffer_init(spi_tx* tx_buffer, spi_rx* rx_buffer){
	
	/* Initialize SPI TX Data Buffer */
	tx_buffer->tx_head = 0;
	tx_buffer->tx_tail = 0;
	tx_buffer->is_empty = 1;
	
	for (int i = 0; i < TX_BUFFER_SIZE; i++) {
		tx_buffer->task_list[i].data_head = 0;
		tx_buffer->task_list[i].data_tail = 0;
		tx_buffer->task_list[i].priority = 0;
	}

	/* Initialize SPI RX Data Buffer */
	for (int i = 0; i < RX_BUFFER_SIZE; i++) {
		rx_buffer->data[i] = 0; 
	}
	rx_buffer->rx_head = 0;
	rx_buffer->rx_tail = 0;
	rx_buffer->number_of_bytes_requested = 0;
}

spi_error_t spi_create_task(spi_tx* buffer, slave_info* slave, uint8_t* data, uint8_t number_of_bytes, uint8_t priority, uint8_t opt, spi_callback cb){
	
	if (number_of_bytes > DX_BUFFER_SIZE) return SPI_ERR_DATA_OVERFLOW;
	
	 spt_t spj = {
		 .data = (uint8_t*) malloc(sizeof(uint8_t) * number_of_bytes),
		 .data_head = 0,
	 	 .data_tail = number_of_bytes,
		 .priority = priority,
		 .opt = opt,
		 .slave = slave,
		 .callback = cb
	};
	
	if (spj.data == NULL) return SPI_ERR_NOT_DEFINED;
	
	memset(spj.data, 0, number_of_bytes);
	for (int idx = 0; idx < number_of_bytes; idx++) spj.data[idx] = data[idx];
	
	uint8_t tmppos; 
	uint8_t tmphead = buffer->tx_head;
	uint8_t tmptail = (buffer->tx_tail & TX_BUFFER_MASK);
	uint8_t tmpprev = (buffer->tx_tail - 1) & TX_BUFFER_MASK;	
	
	/* Search for the new position in the buffer if the new task has
	 * a higher priority than the previous task. */
	if (tmphead != tmptail && priority > buffer->task_list[tmpprev].priority) {
		// Search for new position to insert the task into
		while (tmphead != tmptail) {
			if (priority > buffer->task_list[tmphead].priority){
				tmppos = tmphead; // save position for the new task
				spt_t tmptask[TX_BUFFER_SIZE];
				
				// create a copy of the entire task list
				for (int i = 0; i < TX_BUFFER_SIZE; i++){
					tmptask[i] = buffer->task_list[i];
				}
				
				// shift the task list
				for (int i = tmphead; tmphead != tmptail; i = (++tmphead) & TX_BUFFER_MASK){
					buffer->task_list[i+1] = tmptask[i];
				}
				
				buffer->task_list[tmppos] = spj; // Insert new task
				break;
			}
			tmphead = (tmphead + 1) & TX_BUFFER_MASK;
		}
	} else{
		buffer->task_list[tmptail] = spj; // Append new task 
	}
	
	buffer->tx_tail = (++tmptail & TX_BUFFER_MASK);
	buffer->is_empty = 0;
	
	if (buffer->tx_tail == buffer->tx_head){
		return SPI_ERR_BUFFER_DATA_OVERWRITE;
	}
	
	if (abs(buffer->tx_tail - buffer->tx_head) > TX_BUFFER_SIZE - 1){ 
		return SPI_ERR_BUFFER_OVERFLOW;
	}
	
	return SPI_NO_ERROR;
}