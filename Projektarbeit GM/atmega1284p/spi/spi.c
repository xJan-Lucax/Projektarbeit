/*************************************************************************
* Title		: SPI Master Implementation
* Author	: Dimitri Dening
* Created	: 25.10.2021 17:56:24
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
        
DESCRIPTION:
	Single master implementation to communicate with spi devices using a ringbuffer.  
USAGE:
	see <spi.h>
NOTES:
	                   
*************************************************************************/
/* General libraries */
#include <avr/interrupt.h>
#include <util/delay.h>

/* User defined libraries */
#include "spi.h"
#include "uart.h"
#include "led_lib.h"

typedef enum {
	SPI_ACTIVE,
	SPI_INACTIVE
} SPI_STATE_T;

static spi_tx tx_buffer;
static spi_rx rx_buffer;
static SPI_STATE_T SPI_STATE;
static slave_info slave;

typedef struct {
	uint8_t** array_ptr;
	uint8_t number_of_bytes;
} data_t;

static data_t* data_packet;
	
typedef void (*data_callback_t)(data_t*);
	
volatile spi_callback task_finished_callback = NULL;
volatile data_callback_t data_callback = NULL;

static void get_byte(data_t* data); // Forward declaration

spi_error_t spi_init(data_order_t data_order, clock_polarity_t clock_polarity, clock_phase_t clock_phase, clock_rate_t clock_rate){
		
	/* Set MOSI and SCK output, all others input */
	SPI_DDR = (1 << SPI_SCK) | (1 << SPI_MOSI);
	
	/* Make sure the MISO pin is input */
	SPI_DDR &= ~(1 << SPI_MISO);
	
	/* This is the default slave configured on PORTB4 */
	slave = spi_create_slave(PINB4, PORTB4, DDB4);
	
	/* Enable SPI Interrupt Flag, SPI, Data Order, Master Mode, 
	 * Clock Polarity, Clock Phase */	
	SPCR = (1 << SPIE) | (1 << SPE) | (data_order << DORD) | (1 << MSTR) |
	(clock_polarity << CPOL) | (clock_phase << CPHA);
	
	/* Set Clock Rate */
	if (clock_rate == SPI_CLOCK_DIV2  || clock_rate == SPI_CLOCK_DIV8 ||
		clock_rate == SPI_CLOCK_DIV32 || clock_rate == SPI_CLOCK_DIV64X) {
			SPCR |= ((clock_rate - 0x04) << SPR0);
			SPSR |= (1 << SPI2X);
	} else {
			SPCR |= (clock_rate << SPR0);
			SPSR &= ~(1 << SPI2X);
	}
	
	SPI_STATE = SPI_INACTIVE;
	
	spi_buffer_init(&tx_buffer, &rx_buffer);
	
	data_packet = (data_t*)malloc(sizeof(data_t));
	
	if (data_packet == NULL) { return SPI_ERR_NOT_DEFINED; }
		
	data_callback = &get_byte;

	// sei(); // global interrupt enable
	
	return SPI_NO_ERROR;
}

void spi_get_byte(uint8_t* data, uint8_t len){
	
	uint8_t tmphead;
	
	if (rx_buffer.rx_head == rx_buffer.rx_tail) {
		// uart_put("spi buffer empty...");
		// return;
	}
	
	while (SPI_STATE == SPI_ACTIVE);
	
	for (uint8_t i = 0; i < len; i++) {
		tmphead = (rx_buffer.rx_head++) & RX_BUFFER_MASK;
		data[i] = rx_buffer.data[tmphead];
	}
}

static void get_byte(data_t* data){
	
	uint8_t tmphead;
	
	if (rx_buffer.rx_head == rx_buffer.rx_tail) {
		// uart_put("spi buffer empty...");
		// return;
	}
	
	while (SPI_STATE == SPI_ACTIVE);
	
	for (uint8_t i = 0; i < data->number_of_bytes; i++) {
		tmphead = (rx_buffer.rx_head++) & RX_BUFFER_MASK;
		(*data->array_ptr)[i] = rx_buffer.data[tmphead];
	}
}

static spi_error_t spi_enable_slave(slave_info* _slave){
	
	if (_slave->port == SPI_SCK || _slave->port == SPI_MOSI || _slave->port == SPI_MISO) {
		return error_handler(SPI_ERR_INVALID_PORT);
	}

	if (_slave->port == slave.port) return SPI_NO_ERROR;
	
	/* Replace the current slave with the new slave */
	slave = *_slave;
	
	/* Pin configuration for the new slave */
	SPI_DDR  |= (1 << slave.ddr);  // @Output
	SPI_PORT |= (1 << slave.port); // Pull up := inactive
	
	/* Re-enable Master Mode again if it got reset by setting a slave pin as input by accident. */
	if (!(SPCR & (1 << MSTR))) { SPCR |= (1 << MSTR); }

	return SPI_NO_ERROR;
}

spi_error_t spi_write(slave_info* _slave, uint8_t* data, uint8_t number_of_bytes, priority_t priority, optional_t opt, spi_callback cb_func){
	
	spi_error_t err;
	
	err = spi_create_task(&tx_buffer, _slave, data, number_of_bytes, priority, opt, cb_func);
	
	if (err != SPI_NO_ERROR) { return error_handler(err); }
	
	/* If the SPI is not active right now, it is save to transmit the next dataword from the tx_buffer. */
	if (SPI_STATE == SPI_INACTIVE) {
		
		SPI_STATE = SPI_ACTIVE;
		
		task_finished_callback = cb_func; // load callback function for current task
		
		err = spi_enable_slave(tx_buffer.task_list[tx_buffer.tx_head].slave);
		
		if (err != SPI_NO_ERROR) return error_handler(err);
		
		SPI_PORT &= ~(1 << slave.port);  /* Pull down := active */
		
		SPDR = tx_buffer.task_list[tx_buffer.tx_head].data[tx_buffer.task_list[tx_buffer.tx_head].data_head++];
		
		if (SPSR & (1 << WCOL)) return error_handler(SPI_ERR_WRITE_COLLISION);
	} else {
		// Do nothing.
	}
	
	return SPI_NO_ERROR;
}

spi_error_t spi_read(slave_info* _slave, uint8_t* _recv, uint8_t number_of_bytes, uint8_t start_byte, priority_t priority, spi_callback cb_func){
	
	spi_error_t err;
	
	data_packet->array_ptr = &_recv;
	
	data_packet->number_of_bytes = number_of_bytes;
	
	uint8_t* dummy_packet = (uint8_t*) malloc(sizeof(uint8_t) * number_of_bytes);

	if (dummy_packet == NULL) return SPI_ERR_NOT_DEFINED;
	
	memset(dummy_packet, start_byte, number_of_bytes);
	
	err = spi_create_task(&tx_buffer, _slave, dummy_packet, number_of_bytes, priority, STORE_DATA, cb_func);
	
	if (err != SPI_NO_ERROR) {
		free(dummy_packet);
		return error_handler(err);
	}
	
	/* If the SPI is not active right now, it is save to transmit the next dataword from the tx_buffer. */
	if (SPI_STATE == SPI_INACTIVE) {
		
		SPI_STATE = SPI_ACTIVE;
		
		task_finished_callback = cb_func; // load callback function for current task
		
		err = spi_enable_slave(tx_buffer.task_list[tx_buffer.tx_head].slave);
		
		if (err != SPI_NO_ERROR) {
			free(dummy_packet);
			return error_handler(err);
		}
		
		SPI_PORT &= ~(1 << slave.port); /* Pull down := active */
		
		SPDR = tx_buffer.task_list[tx_buffer.tx_head].data[tx_buffer.task_list[tx_buffer.tx_head].data_head++];
		
		if (SPSR & (1 << WCOL)) {
			free(dummy_packet);
			return error_handler(SPI_ERR_WRITE_COLLISION);
		}
	} else {
		// Do nothing.
	}
	
	free(dummy_packet);
	
	return SPI_NO_ERROR;
}

slave_info spi_create_slave(uint8_t pin, uint8_t port, uint8_t ddr){
	slave_info s = { .pin = pin, .port = port, .ddr = ddr };
	SPI_PORT |= (1 << port); // Pull up := inactive
	SPI_DDR  |= (1 << ddr);  // @Output
	
	return s;
}

spi_error_t spi_flush_buffer(void){
	
	if (SPI_STATE == SPI_ACTIVE) return error_handler(SPI_ERR_FLUSH_FAILED);
		
	tx_buffer.tx_head = 0;
	tx_buffer.tx_tail = 0;
	tx_buffer.is_empty = 1;
	
	for (int i = 0; i < TX_BUFFER_SIZE; i++) {
		tx_buffer.task_list[i].data_head = 0;
		tx_buffer.task_list[i].data_tail = 0;
		tx_buffer.task_list[i].priority = 0;
	}
		
	for (int i = 0; i < RX_BUFFER_SIZE; i++) {
		rx_buffer.data[i] = 0;
	}
	
	rx_buffer.rx_head = 0;
	rx_buffer.rx_tail = 0;
	rx_buffer.number_of_bytes_requested = 0;
		
	return SPI_NO_ERROR;
}

static void spi_wait(void){
	while(SPI_STATE == SPI_ACTIVE);
}

static void _spi_clear(void){
	SPI_STATE = SPI_INACTIVE;
}

void spi_clear(slave_info* _slave){
	uint8_t recv_dummy[RX_BUFFER_SIZE];
	spi_read(_slave, recv_dummy, 1, 0x00, NORMAL, &_spi_clear);
	spi_wait();
	spi_flush_buffer();
}

uint8_t dump, tmphead, tmptail, tmp;
slave_info* tmpslave;

ISR(SPI_STC_vect){
	
	// =================================================
	//						RECEIVE
	// =================================================
	tmptail = rx_buffer.rx_tail & RX_BUFFER_MASK;

	if (tx_buffer.task_list[tx_buffer.tx_head].opt == STORE_DATA){
		rx_buffer.data[tmptail] = SPDR;
		rx_buffer.rx_tail++;
	} else {
		dump = SPDR; 
	}

	// =================================================
	//						SEND
	// =================================================	
	if (tx_buffer.tx_head != tx_buffer.tx_tail){
		
		tmpslave = tx_buffer.task_list[tx_buffer.tx_head].slave;

		if (tmpslave->port != slave.port) spi_enable_slave(tmpslave);
		
		// Send next dataword from current task
		if (tx_buffer.task_list[tx_buffer.tx_head].data_head != tx_buffer.task_list[tx_buffer.tx_head].data_tail){	
			SPI_PORT &= ~(1 << slave.port); // Pull down := active		
			SPDR = tx_buffer.task_list[tx_buffer.tx_head].data[tx_buffer.task_list[tx_buffer.tx_head].data_head++];
		} else { 
			
			// Task finished 
			
			SPI_STATE = SPI_INACTIVE;
			
			if (tx_buffer.task_list[tx_buffer.tx_head].opt == STORE_DATA){
				
				if (data_callback != NULL) {
					data_callback(data_packet);
				}
			}
			
			if (task_finished_callback != NULL) {
				task_finished_callback();
				task_finished_callback = NULL;
			}
			
			free(tx_buffer.task_list[tx_buffer.tx_head].data);
			
			/* Move to next task in tx_buffer */
			tmp = (tx_buffer.tx_head & TX_BUFFER_MASK);
			tmphead = ((tx_buffer.tx_head + 1) & TX_BUFFER_MASK);
			tx_buffer.tx_head = tmphead;
			
			if (tx_buffer.task_list[tmp].opt == CONTINUE_LOW) {
				/* Keep CS low after the task finished. */
			} else {
				SPI_PORT |= (1 << slave.port); // Pull up := inactive
			}
			
			/* Start the next task in the buffer if available. */
			if (tx_buffer.tx_head != tx_buffer.tx_tail) {	
				SPI_STATE = SPI_ACTIVE;
				task_finished_callback = tx_buffer.task_list[tx_buffer.tx_head].callback; // load callback function for next task 
				SPI_PORT &= ~(1 << slave.port); // Pull down := active
				SPDR = tx_buffer.task_list[tx_buffer.tx_head].data[tx_buffer.task_list[tx_buffer.tx_head].data_head++];
			} else {
				SPI_STATE = SPI_INACTIVE;
				tx_buffer.is_empty = 1;
			}
		}		
	} else {
		SPI_PORT |= (1 << slave.port); // Pull up := inactive
		SPI_STATE = SPI_INACTIVE;	
		tx_buffer.is_empty = 1;	
	}
}