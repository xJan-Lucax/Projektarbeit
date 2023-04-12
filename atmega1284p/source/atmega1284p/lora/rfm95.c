#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>

#include "rfm95.h"
#include "spi.h"
#include "timer_lib.h"
#include "uart.h"
#include "io_utils.h"

#include <stdlib.h>

#define RFM95_FREQUENCY_RESOLUTION 61.035
#define SPI_SEND_SIZE		0x02
#define SPI_DUMMY_SIZE		0x01

volatile rfm95_callback packet_sent_callback = NULL;
volatile rfm95_callback packet_received_callback = NULL;

static slave_info my_slave;

static uint8_t data_sent[SPI_SEND_SIZE];
static uint8_t spi_dummy[SPI_DUMMY_SIZE];

static void rfm95_write(uint8_t address, uint8_t data) {	
	data_sent[0] = address | (1 << 7);
	data_sent[1] = data;
	spi_write(&my_slave, data_sent, 2, HIGH, DEFAULT, NULL);
}

static uint8_t rfm95_read(uint8_t address) {
	
	uint8_t temp;
	uint8_t* rfm95_receive = (uint8_t*)malloc(sizeof(uint8_t) * 1);
	
	spi_dummy[0] = address & ~(1 << 7);	
	
	spi_write(&my_slave, spi_dummy, 1, HIGH, CONTINUE_LOW, NULL);
	spi_read(&my_slave, rfm95_receive, 1, 0x00, HIGH, NULL);
	
	temp = rfm95_receive[0];
	
	free(rfm95_receive);
	
	return temp;
}

void rfm95_init(uint8_t cs) {
	_delay_ms(10); // wait time after power-up
	
	EICRA |= (1 << ISC21) | (1 << ISC20); // rising edge generates asynchronously an interrupt request.
	EIFR  |= (1 << INTF2); // clearing any outstanding interrupt caused by the previous setting
	EIMSK |= (1 << INT2); // activating the interrupt service routine

	//spi_init(SPI_MSB, SPI_MODE0, SPI_CLOCK_DIV2);

	my_slave = spi_create_slave(cs, cs, cs);
	
	//sei(); // enabling interrupts globally
	
	// enable LoRa mode by enabling sleep mode, then LoRa mode
	rfm95_write(0x01, 0b00000000); // sleep mode
	rfm95_write(0x01, 0b10000000); // LoRa mode

	//rfm95_write(0x1D, 0x07); //set the Bandwidth to 125 kHz

	rfm95_write(0x09, 0xFF); // maximal radio power output
	rfm95_write(0x0C, 0x23); // LNA gain max

	// invert IQ for downlink data
	rfm95_write(0x33, 0b01010011);

	rfm95_write(0x39, 0x34); // sync word, 0x34 for TTN, not mentioned in the RFM95 datasheet, but in the SX1276 datasheet
}

void rfm95_set_frequency(float frequency) {	
	uint32_t frequency_factor = frequency / RFM95_FREQUENCY_RESOLUTION;
	
	rfm95_write(0x06, frequency_factor >> 16);
	rfm95_write(0x07, frequency_factor >> 8);
	rfm95_write(0x08, frequency_factor >> 0);
}

void rfm95_set_spreading_factor(uint8_t spreading_factor) {
	rfm95_write(0x1E, (spreading_factor << 4) | 0b00000100); // spreading factor & CRC on
}

void rfm95_send_packet(uint8_t *data, uint8_t length, rfm95_callback packet_sent) {
	packet_sent_callback = packet_sent;

	rfm95_write(0x01, 0b10000001); // standby mode

	rfm95_write(0x22, length); // set payload length
	rfm95_write(0x0D, 0x80); // set FIFO pointer to TX data start
	for (uint8_t i = 0; i < length; i++) { // write packet to the RFM95
		rfm95_write(0x00, data[i]);
	}

	rfm95_write(0x40, 0b01000000); // enable TxDone interrupt on DIO0
	rfm95_write(0x01, 0b10000011); // TX mode
}

void rfm95_start_listening(rfm95_callback packet_received) {
	packet_received_callback = packet_received;

	rfm95_write(0x40, 0b00000000); // enable RxDone interrupt on DIO0

	rfm95_write(0x0D, 0x00); // set FIFO pointer to RX data start
	rfm95_write(0x01, 0b10000110); // RX mode
}

uint8_t rfm95_read_packet(uint8_t **data) {
	uint8_t received_bytes = rfm95_read(0x13);
	//uart_put("%s %d %s", "[read packet]", received_bytes, "\r\n");
	if (received_bytes == 0) return 0;

	*data = malloc(sizeof(uint8_t) * received_bytes);
	if (*data == NULL) return 0;

	rfm95_write(0x0D, 0x00); // set FIFO pointer to RX data start
	for (uint8_t i = 0; i < received_bytes; i++) {
		(*data)[i] = rfm95_read(0x00);
	}

	return received_bytes;
}

ISR(INT2_vect, ISR_NOBLOCK) {
	rfm95_write(0x12, 0xFF); // clear RFM95 interrupt requests

	if (packet_sent_callback != NULL) {
		packet_sent_callback();
		packet_sent_callback = NULL;
		return;
	}

	if (packet_received_callback != NULL) {
		packet_received_callback();
		packet_received_callback = NULL;
		return;
	}
}
