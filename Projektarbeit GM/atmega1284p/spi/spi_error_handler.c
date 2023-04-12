/*************************************************************************
* Title		: Error Handling Implementation
* Author	: Dimitri Dening
* Created	: 28.10.2021 08:10:39
* Software	: Microchip Studio V7
* Hardware	: Atmega1284P
    
USAGE:
	see <spi_error_handler.h>
	
NOTES:
	                   
*************************************************************************/
/* General libraries */
#include <util/delay.h>

/* User defined libraries */
#include "spi_error_handler.h"
#include "led_lib.h"
#include "uart.h"

#define LED_DEBUG_OUTPUT 1

#define SHORT_PULSE 1	// 1s
#define LONG_PULSE  3	// 3s
#define SEQ_LEN		3
#define REPEAT		2
#define DELAY		4	// 4s

typedef struct {
	spi_error_t err;
	uint16_t sequence[SEQ_LEN];
	char* error_string;
} table_t;

table_t error_table[] = {
	//         ERROR                                        SEQUENCE								   ERROR STRING
	//           |                                             |											|
	//           |                                             |											|
	//           |                                             |											|
	//-------------------------------------------------------------------------------------------------------------------------------
	{   SPI_NO_ERROR					,   {												}	,	"SPI_NO_ERROR"					},
	{   SPI_ERR_BUFFER_OVERFLOW         ,   { SHORT_PULSE ,   SHORT_PULSE   ,   SHORT_PULSE }   ,	"SPI_ERR_BUFFER_OVERFLOW"		},
	{   SPI_ERR_BUFFER_DATA_OVERWRITE   ,   { SHORT_PULSE ,   SHORT_PULSE   ,   LONG_PULSE  }   ,	"SPI_ERR_BUFFER_DATA_OVERWRITE" },
	{   SPI_ERR_DATA_OVERFLOW           ,   { SHORT_PULSE ,   LONG_PULSE    ,   SHORT_PULSE }   ,	"SPI_ERR_DATA_OVERFLOW"			},
	{   SPI_ERR_INVALID_PORT            ,   { SHORT_PULSE ,   LONG_PULSE    ,   LONG_PULSE  }   ,	"SPI_ERR_INVALID_PORT"			},
	{   SPI_ERR_WRITE_COLLISION         ,   { LONG_PULSE  ,   SHORT_PULSE   ,   SHORT_PULSE }   ,	"SPI_ERR_WRITE_COLLISION"		},
	{   SPI_ERR_FLUSH_FAILED            ,   { LONG_PULSE  ,   SHORT_PULSE   ,   LONG_PULSE  }   ,	"SPI_ERR_FLUSH_FAILED"			},
	{   SPI_ERR_RECV_BUSY               ,   { LONG_PULSE  ,   LONG_PULSE    ,   SHORT_PULSE }   ,	"SPI_ERR_RECV_BUSY"				},
	{   SPI_ERR_NOT_DEFINED             ,   { LONG_PULSE  ,   LONG_PULSE    ,   LONG_PULSE  }   ,	"SPI_ERR_NOT_DEFINED"			}
};

static void delay(int t) {
	while(t--) _delay_ms(1000);
}

static void error_led(spi_error_t error) {	
	
	uart_put("%s %s (%d)", "[spi error]:", error_table[error].error_string, error);

	#if LED_DEBUG_OUTPUT
	for (int loop = 0; loop < REPEAT; loop++){
		for (uint8_t seq = 0; seq < SEQ_LEN; seq++) {
			led_toggle(LED_ERROR); delay(error_table[error].sequence[seq]);
			led_toggle(LED_ERROR); delay(1);
		}
		delay(DELAY);
	}
	#endif
	
	return;
}

spi_error_t error_handler(spi_error_t error){
	switch(error){
		case SPI_NO_ERROR					:					return SPI_NO_ERROR;					break;
		case SPI_ERR_BUFFER_OVERFLOW		: error_led(error); return SPI_ERR_BUFFER_OVERFLOW;			break;
		case SPI_ERR_BUFFER_DATA_OVERWRITE	: error_led(error); return SPI_ERR_BUFFER_DATA_OVERWRITE;	break;
		case SPI_ERR_DATA_OVERFLOW			: error_led(error); return SPI_ERR_DATA_OVERFLOW;			break;
		case SPI_ERR_INVALID_PORT			: error_led(error); return SPI_ERR_INVALID_PORT;			break;
		case SPI_ERR_WRITE_COLLISION		: error_led(error); return SPI_ERR_WRITE_COLLISION;			break;
		case SPI_ERR_FLUSH_FAILED			: error_led(error); return SPI_ERR_FLUSH_FAILED;			break;
		case SPI_ERR_RECV_BUSY				: error_led(error); return SPI_ERR_RECV_BUSY;				break;
		default								: error_led(error);	return SPI_ERR_NOT_DEFINED;
	}
}