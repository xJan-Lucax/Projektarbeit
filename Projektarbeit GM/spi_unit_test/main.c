/*************************************************************************
* Title		: SPI Unit Test
* Author	: Dimitri Dening
* Created	: 13.02.2022 19:30:22
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
* Usage		: see Doxygen manual
*
* This is a modified version of Microchip Studios' SPI Unit Test.
* Following the license from Microchip used in the original file.
*
* Subject to your compliance with these terms, you may use Microchip
* software and any derivatives exclusively with Microchip products.
* It is your responsibility to comply with third party license terms applicable
* to your use of third party software (including open source software) that
* may accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
* INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
* AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
* LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
* LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
* SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
* POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
* ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
* RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*
*************************************************************************/

/**
@file main.c  
@author Dimitri Dening
@date 13.02.2022
@brief SPI Unit Test
@note Connect the flash memory (AT45DB041B) on the STK600 to the SPI PORT.
@note Connect two STK600s to run the entire unit test.       
*/
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "suite.h"
#include "uart.h"
#include "../atmega1284p/spi/spi.h"
#include "../atmega1284p/led_lib.h"

/* Define CPU frequency in Hz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#define DATA_BUFFER_SIZE	0x05
	
static uint8_t data_buffer_write[]	= { 0x84, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
static uint8_t data_flash_write[]	= { 0x83, 0x00, 0x00, 0x00 };
static uint8_t data_flash_read[]	= { 0xd2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static uint8_t data_sent[]			= { 0x01, 0x02, 0x03, 0x04, 0x05 };
// static uint8_t data_dummy[]			= { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static slave_info data_flash_slave;
static slave_info test_slave;	  
static slave_info dummy_slave;	  

// static uint8_t data_received[DATA_BUFFER_SIZE]; 
	
void call_tx(void){ uart_put("doing tx"); }
void call_rx(void){ uart_put("doing rx"); }
	
static int flash_read_data(slave_info slave, uint8_t* recv_arr) {

	bool ret = 0;
	
	/* Send the flash read command */
	ret = spi_write(&slave, data_flash_read, 8, NORMAL, CONTINUE_LOW, NULL);
		
	if (ret != 0) return TEST_ERROR;
		
	/* Get the data from flash */
	ret = spi_read(&slave, recv_arr, DATA_BUFFER_SIZE, 0xFF, NORMAL, NULL);
		
	for (volatile uint16_t i = 0; i < 30000; i++) {}
		
	if (ret != 0) return TEST_ERROR;
	
	return 0;
}

/************************************************************************/
/*                             Test 0                                   */
/************************************************************************/
static int run_spi_flash_read_test(const struct test_case* test) {
	
	bool ret = false;
	
	uint8_t* spi_receive = (uint8_t*)malloc(sizeof(uint8_t) * DATA_BUFFER_SIZE);

	if (spi_receive == NULL) { return TEST_ERROR; }
	
	/* Read the data from Page 0 of flash. This is to avoid write of same
	 * data again and to reduce the flash write during testing */
	ret = flash_read_data(data_flash_slave, spi_receive);
	if (ret != 0) { 
		free(spi_receive); 
		return ret; 
	}
	
	/* Check the read data */
	for (uint8_t i = 0; i < DATA_BUFFER_SIZE; i++) {
		uart_put("%s %d", "[device 1]: read spi data", spi_receive[i]);
		/* Clear receive data buffer */
		spi_receive[i] = 0;
	}
	
	free(spi_receive);
	
	return TEST_PASS;
}

/************************************************************************/
/*                             Test 1                                   */
/************************************************************************/
static int run_spi_transfer_test(const struct test_case* test) {
	
	bool write_enable = false;
	bool ret = false;
	
	uint8_t* spi_receive = (uint8_t*)malloc(sizeof(uint8_t) * DATA_BUFFER_SIZE);

	if (spi_receive == NULL) { return TEST_ERROR; }
	
	/* Read the data from Page 0 of flash. This is to avoid write of same
	 * data again and to reduce the flash write during testing */
	ret = flash_read_data(data_flash_slave, spi_receive);
	if (ret != 0) {
		free(spi_receive);
		return ret;
	}
	
	/* Check the read data */
	for (uint8_t i = 0; i < DATA_BUFFER_SIZE; i++) {
		if (spi_receive[i] != data_sent[i]) {
			write_enable = true;
		}

		/* Clear receive data buffer */
		spi_receive[i] = 0;
	}
	
	/* If first time flash is getting used data will be 0xff. So write known data */
	if (write_enable) {

		uart_put("%s", "[device 1]: write mode");
		
		/* Send the buffer write command followed by data to written */
		ret = spi_write(&data_flash_slave, data_buffer_write, 9, NORMAL, NONE, NULL);
		if (ret != 0) {
			free(spi_receive);
			return TEST_ERROR;
		}

		/* Flash delay */
		for (volatile uint16_t i = 0; i < 30000; i++) {}

		/* Send the flash write command to write the previously sent data to flash */
		ret = spi_write(&data_flash_slave, data_flash_write, 4, NORMAL, NONE, NULL);
		if (ret != 0) { 
			free(spi_receive);
			return TEST_ERROR;
		}

		/* Flash delay */
		for (volatile uint16_t i = 0; i < 30000; i++) {}
	}
	
	/* Read the data from flash. */
	ret = flash_read_data(data_flash_slave, spi_receive);
	if (ret != 0) {
		free(spi_receive);
		return ret;
	}

	/* Check the read data */
	for (uint8_t i = 0; i < DATA_BUFFER_SIZE; i++) {
		
		uart_put("%s %d %s %d", "[device 1]: read spi data", spi_receive[i], "expected", data_sent[i]);
		
		if (spi_receive[i] != data_sent[i]) {
			free(spi_receive);
			return TEST_FAIL;
		}
	}
	
	free(spi_receive);
	
	return TEST_PASS;
}

/************************************************************************/
/*                             Test 2                                   */
/************************************************************************/
static int run_spi_multi_transfer_test(const struct test_case* test) {
	
	bool write_enable_a = false;
	bool write_enable_b = false;
	bool state_a = false;
	bool state_b = false;
	bool ret = false;
	
	uint8_t* spi_receive = (uint8_t*)malloc(sizeof(uint8_t) * DATA_BUFFER_SIZE);

	if (spi_receive == NULL) { return TEST_ERROR; }
	
	/******************************/
	/*          DEVICE 1		  */
	/******************************/
	ret = flash_read_data(data_flash_slave, spi_receive);
	if (ret != 0) return ret;

	/* Check the read data */
	for (uint8_t i = 0; i < DATA_BUFFER_SIZE; i++) {
		if (spi_receive[i] != data_sent[i]) {
			write_enable_a = true;
		}
		
		// uart_put("%s %d", "[device 1]: first flash read", spi_receive[i]);

		/* Clear receive data buffer */
		spi_receive[i] = 0;
	}
	
	/******************************/
	/*          DEVICE 2		  */
	/******************************/
	ret = flash_read_data(test_slave, spi_receive);
	if (ret != 0) {
		free(spi_receive);
		return ret;
	}

	/* Check the read data */
	for (uint8_t i = 0; i < DATA_BUFFER_SIZE; i++) {
		if (spi_receive[i] != data_sent[i]) {
			write_enable_b = true;
		}
		
		// uart_put("%s %d", "[device 2]: first flash read", spi_receive[i]);

		/* Clear receive data buffer */
		spi_receive[i] = 0;
	}
	
	/******************************/
	/*          DEVICE 1		  */
	/******************************/
	if (write_enable_a) {
		
		uart_put("%s", "[device 1]: write mode");

		/* Send the buffer write command followed by data to written */
		ret = spi_write(&data_flash_slave, data_buffer_write, 9, NORMAL, NONE, NULL);
		if (ret != 0) {
			free(spi_receive);
			return TEST_ERROR;
		}

		/* Flash delay */
		for (volatile uint16_t i = 0; i < 30000; i++) {}

		/* Send the flash write command to write the previously sent data to flash */
		ret = spi_write(&data_flash_slave, data_flash_write, 4, NORMAL, NONE, NULL);
		if (ret != 0) {
			free(spi_receive);
			return TEST_ERROR;
		}

		/* Flash delay */
		for (volatile uint16_t i = 0; i < 30000; i++) {}
	}
	
	/******************************/
	/*          DEVICE 2		  */
	/******************************/
	if (write_enable_b) {
		
		uart_put("%s", "[device 2]: write mode");

		/* Send the buffer write command followed by data to written */
		ret = spi_write(&test_slave, data_buffer_write, 9, NORMAL, NONE, NULL);
		if (ret != 0) {
			free(spi_receive);
			return TEST_ERROR;
		}

		/* Flash delay */
		for (volatile uint16_t i = 0; i < 30000; i++) {}

		/* Send the flash write command to write the previously sent data to flash */
		ret = spi_write(&test_slave, data_flash_write, 4, NORMAL, NONE, NULL);
		if (ret != 0) {
			free(spi_receive);
			return TEST_ERROR;
		}

		/* Flash delay */
		for (volatile uint16_t i = 0; i < 30000; i++) {}
	}
	
	/******************************/
	/*          DEVICE 1		  */
	/******************************/
	ret = flash_read_data(data_flash_slave, spi_receive);
	if (ret != 0) {
		free(spi_receive);
		return ret;
	}

	/* Check the read data */
	for (uint8_t i = 0; i < DATA_BUFFER_SIZE; i++) {
		
		uart_put("%s %d %s %d", "[device 1]: read spi data", spi_receive[i], "expected", data_sent[i]);
		
		if (spi_receive[i] != data_sent[i]) {
			state_a = true;
		}
		
		spi_receive[i] = 0;
	}
	
	/******************************/
	/*          DEVICE 2		  */
	/******************************/
	ret = flash_read_data(test_slave, spi_receive);
	if (ret != 0) {
		free(spi_receive);
		return ret;
	}

	/* Check the read data */
	for (uint8_t i = 0; i < DATA_BUFFER_SIZE; i++) {
		
		uart_put("%s %d %s %d", "[device 2]: read spi data", spi_receive[i], "expected", data_sent[i]);
		
		if (spi_receive[i] != data_sent[i]) {
			state_b = true;
		}
		
		spi_receive[i] = 0;
	}
	
	free(spi_receive);
	
	if (state_a || state_b) {
		return TEST_FAIL;
	}

	return TEST_PASS;
}

/************************************************************************/
/*                             Test 3                                   */
/************************************************************************/
static uint16_t big_data_count = 0;

static void callback_big_data(void) { big_data_count++; }; 
	
/* Test large data packets.

   Tested with:
	System freq @8MHz, SPI Clock Divider 2
	
	TX_BUFFER_SIZE 32 
		++ higher values should yield better and more stable results ++
		++ balance between performance and avaiable memory space ++
		++ => 32 is the target size ++
	RX_BUFFER_SIZE 32
		++ irrelevant for this test ++
	DX_BUFFER_SIZE 64 
		++ usually send packets contain only 2 datawords (rfm95) ++
		++ tested with very big values ++
*/
static int run_spi_big_data_test(const struct test_case* test) {
	
	bool ret = 0;
	
	uint8_t number_of_bytes = DX_BUFFER_SIZE;
	
	uint16_t number_of_tasks = 5000; // without spi_wait() 255 is the limit
									 // one task missing on values e.g. 256 and 512
									 // value of 257 is fine
									 // missing passed task gets added to the counter of the next unit test (???)
	
	uint8_t* big_data = (uint8_t*)calloc(number_of_bytes, sizeof(uint8_t));
	
	if (big_data == NULL) {
		return TEST_ERROR;
	}
	
	for (int i = 0; i < number_of_tasks; i++){
		ret = spi_write(&dummy_slave, big_data, number_of_bytes, NORMAL, NONE, &callback_big_data);
		if (ret != 0) {
			free(big_data);
			return TEST_ERROR;
		}
		
		//spi_wait();	// <-- If too many spi_write requests are sent a small delay is needed.
					    //	   Either spi_wait() or _delay_us(1)
	}
	
	/* Delay */
	for (volatile uint32_t i = 0; i < 60000; i++) {}
	
	free(big_data);
	
	uart_put("%s %d %s %d", "tasks passed", big_data_count, "expected", number_of_tasks);
	
	if ( big_data_count != number_of_tasks) return TEST_FAIL;
	
	return TEST_PASS;
}

/************************************************************************/
/*                             Test 4                                   */
/************************************************************************/
static uint16_t buffer_flood_count = 0;

void callback_buffer_flood(void) { buffer_flood_count++; }; 
	
/* Flood the buffer with alot of spi_write requests.

   Tested with:
	System freq @8MHz, SPI Clock Divider 2
	
	TX_BUFFER_SIZE 32 
		++ higher values should yield better and more stable results ++
		++ balance between performance and avaiable memory space ++
		++ => 32 is the target size ++
	RX_BUFFER_SIZE 32
		++ irrelevant for this test ++
	DX_BUFFER_SIZE 64 
		++ test with only two datawords (rfm95 usecase) ++
*/
static int run_spi_buffer_flood_test(const struct test_case* test) {
	
	bool ret = 0;
	
	uint8_t number_of_bytes = 2; 
	
	uint16_t number_of_tasks = 32000; // (2^15) - 1 = 32767
	
	uint8_t* big_data = (uint8_t*)calloc(number_of_bytes, sizeof(uint8_t));
	
	if (big_data == NULL) {
		return TEST_ERROR;
	}
	
	for (int i = 0; i < number_of_tasks; i++){
		memset(big_data, i, number_of_bytes);
		ret = spi_write(&dummy_slave, big_data, number_of_bytes, NORMAL, NONE, &callback_buffer_flood);
		if (ret != 0) {
			free(big_data);
			return TEST_ERROR;
		}
	}
	
	/* Delay */
	for (volatile uint16_t i = 0; i < 30000; i++) {}
	
	free(big_data);
	
	uart_put("%s %d %s %d", "tasks passed", buffer_flood_count, "expected", number_of_tasks);
	
	if ( buffer_flood_count != number_of_tasks) return TEST_FAIL;
	
	return TEST_PASS;
}

/************************************************************************/
/*                             Test 5                                   */
/************************************************************************/
bool memory_return_success = 0;
void callback_memory_leak(void) { memory_return_success = 1; }; 

/* If somewhere in the spi routine a free() command is missing (memory leak)
 * an allocation error should occur and fail this test. */
static int run_spi_memory_leak_test(const struct test_case* test) {
	
	bool ret = 0;
	
	uint8_t number_of_bytes = DX_BUFFER_SIZE; 
	
	int number_of_tasks = 10000; // <-- increase value to provoke possible memory leak
	for (int i = 0; i < number_of_tasks; i++){
		uint8_t* receive_data = (uint8_t*)malloc(sizeof(uint8_t) * number_of_bytes);
		
		ret = spi_read(&dummy_slave, receive_data, number_of_bytes, 0x00, NORMAL, &callback_memory_leak);
		
		if (ret != 0) { 
			free(receive_data);
			return TEST_ERROR;
		}
		
		while(memory_return_success != 1);
		
		memory_return_success = 0;
		
		free(receive_data);
	}
	
	return TEST_PASS;
}

/************************************************************************/
/*                             Main                                     */
/************************************************************************/
int main(void) {
	
	cli();		
	
	led_init();
	
	uart_init();
	
	spi_init(SPI_MSB, SPI_CPOL_LOW, SPI_CPHA_LEADING, SPI_CLOCK_DIV2);
	
	sei();
	
	data_flash_slave = spi_create_slave(4, 4, 4);
	test_slave		 = spi_create_slave(3, 3, 3);
	dummy_slave		 = spi_create_slave(0, 0, 0); // do not connect device to PIN0
	
	DEFINE_TEST_CASE(data_flash_read_test, NULL, run_spi_flash_read_test, NULL, "SPI data flash read test");
	DEFINE_TEST_CASE(data_transfer_test, NULL, run_spi_transfer_test, NULL, "SPI data transfer test");
	DEFINE_TEST_CASE(data_multi_transfer_test, NULL, run_spi_multi_transfer_test, NULL, "SPI data multi transfer test");
	DEFINE_TEST_CASE(big_data_test, NULL, run_spi_big_data_test, NULL, "SPI big data test");
	DEFINE_TEST_CASE(buffer_flood_test, NULL, run_spi_buffer_flood_test, NULL, "SPI buffer flood test");
	DEFINE_TEST_CASE(memory_leak_test, NULL, run_spi_memory_leak_test, NULL, "SPI memory leak test");

	/* Put test case addresses in an array */
	DEFINE_TEST_ARRAY(spi_tests) = {
		&data_flash_read_test,
		&data_transfer_test,
		&data_multi_transfer_test, // <- should fail if only one device is connected
		&big_data_test,
		&buffer_flood_test,
		&memory_leak_test
	};
	
	/* Define the test suite */
	DEFINE_TEST_SUITE(spi_suite, spi_tests, "SPI driver test suite");

	/* Run all tests in the test suite */
	test_suite_run(&spi_suite);
	
	while (1) { /* Busy-wait forever. */ }
}