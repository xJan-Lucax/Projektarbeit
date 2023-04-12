//#include <avr/eeprom.h>
#include "lorawan.h"

#include "rfm95.h"
#include "lorawan-packets/lorawan_packets.h"
#include "timer_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/io.h>
#include "uart.h"
//#include "eeprom.h"
#include "eeprom_driver/eeprom.h"

// TheThingsNetwork specification
// RX1 settings = TX settings
#define LORAWAN_TX_FREQUENCY 868.100 * 1000000
#define LORAWAN_RX1_FREQUENCY LORAWAN_TX_FREQUENCY
#define LORAWAN_RX2_FREQUENCY 869.525 * 1000000

#define LORAWAN_TX_SPREADING_FACTOR 7	//DR0 -> SF12, DR1 -> SF11, DR2 -> SF10, ..., DR5 -> SF7

#if OTAA_ABP
#define LORAWAN_JOINACCEPT1_DELAY_SECONDS 5	//TTN specific
#define LORAWAN_JOINACCEPT2_DELAY_SECONDS 6

//#define LORAWAN_NWKSKWY_ADDRESS 0      //16 Byte //Variable 1
//#define LORAWAN_APPSKEY_ADDRESS 16     //16 Byte //Variable 2
//#define LORAWAN_DEV_ADDRESS 32	     //4 Byte  //Variable 3
//#define LORAWAN_NWKID_ADDRESS 36       //3 Byte  //Variable 4
//#define LORAWAN_APPNONCE_ADDRESS 39    //3 Byte  //Variable 5
//#define LORAWAN_DLS_ADDRESS 42         //1 Byte  //Variable 6
//#define LORAWAN_RXD_ADDRESS 43         //1 Byte  //Variable 7
//#define LORAWAN_FORCE_OTAA_JOIN 80     //1 Byte  //Variable 8
//#define LORAWAN_DEVNONCE_ADDRESS 60    //2 Byte  //Variable 9
//#define LORAWAN_CHANNELLIST_ADDRESS 62 //14 Byte //Variable 10

//10 Variablen mit verschiedenen Größen im EEPROM Speicher anlegen
uint8_t array_of_memory[10] = {16,16,4,3,3,1,1,1,2,14};
uint8_t app_number = 0;

//2. App zum testen
//uint8_t array_of_memory2[8] = {16,16,4,4,4,1,1,1};
//uint8_t app_number2 = 0;

#endif

downlink_callback callback = NULL;

volatile lorawan_antenna_state antenna_state = LORAWAN_ANTENNA_IDLE;
volatile uint8_t *received_data = NULL; // only used to temporarily store the data after the callback
volatile uint8_t received_data_length = 0;
volatile uint8_t LORAWAN_RX1_DELAY_SECONDS = 1;
volatile uint8_t LORAWAN_RX2_DELAY_SECONDS = 2; //TTN specific -> RX1 + 1s
volatile uint8_t LORAWAN_RX1_SPREADING_FACTOR = LORAWAN_TX_SPREADING_FACTOR;
volatile uint8_t LORAWAN_RX2_SPREADING_FACTOR = 12;	//TThingsNetwork specific https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/
volatile float LORAWAN_FREQUENCY = 0.0;
volatile uint8_t LORAWAN_TIMEOUT_CONNECTION = 0;
volatile uint8_t LORAWAN_TIMEOUT_CONNECTION_LIMIT = 5;
volatile uint8_t LORAWAN_NWKSKWY_FF = 0;


union {
	uint32_t out;
	uint8_t bytes[4];
} freq;

#if OTAA_ABP
volatile uint8_t lorawan_channel = 0;

lorawan_device_config device_config = {	//Needed device configuration for OTAA
	.app_key = { 0x4D, 0x5D, 0xE7, 0x65, 0x4F, 0x2D, 0x1A, 0xD2, 0xE7, 0xF7, 0x65, 0x74, 0x1C, 0xF9, 0xC7, 0xB9 },
	.app_eui = { 0x07, 0x00, 0xB3, 0x0D, 0x50, 0x7E, 0x0D, 0x00 },
	.dev_eui = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0xA5, 0x70 },
	.dev_nonce = {0x00,0x00},
};
#else
lorawan_device_config device_config = {
	.app_session_key = { 0xB1, 0x06, 0x00, 0x48, 0xEC, 0x68, 0x9C, 0x2E, 0xBD, 0xD3, 0xA0, 0xF1, 0x90, 0x98, 0xD0, 0x84 },
	.network_session_key = { 0x1E, 0xB1, 0x0E, 0x1E, 0x3F, 0x1F, 0x89, 0x59, 0x7C, 0xF6, 0x37, 0x94, 0xDA, 0x2C, 0xB8, 0xC8 },
	.device_address = 0x260B8F77
};
#endif

lorawan_state* lorawan_init(uint8_t chip_select) {	
	#if OTAA_ABP

	uint8_t size_of_memory = 10;
	app_number = eeprom_register_app(array_of_memory, size_of_memory);

	//uint8_t networktest[16] = { 0x4D, 0x5D, 0xE7, 0x65, 0x4F, 0x2D, 0x1A, 0xD2, 0xE7, 0xF7, 0x65, 0x74, 0x1C, 0xF9, 0xC7, 0xB9 };
	//eeprom_driver_write(app_number, networktest, 1);
	//uint8_t networktest2[16];
	//eeprom_driver_read(networktest2, app_number, 1);
	//for (uint8_t i=0;i<16;i++)
	//{
	//	uart_putn(networktest2[0+i]);
	//}
	

	//Testen 2. APP
	//uint8_t size_of_memory2 = sizeof(array_of_memory2);
	//app_number2 = eeprom_register_app(array_of_memory2, size_of_memory2);

	//schreiben von Daten
	//uint8_t array_of_data[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	//eeprom_app_write(app_number, array_of_data, 0);


	_delay_ms(1000);

	uint8_t loraForceReJoin = 0;
	//loraForceReJoin = eeprom_read_uint8_t(app_number, 8);
	loraForceReJoin = eeprom_driver_read_uint8_t(app_number, 8);

	uart_put("ForceReJoin");
	uart_putn(loraForceReJoin);
	uart_put("");

	//uint8_t loraForceReJoin = lorawan_read_otaa_information(LORAWAN_FORCE_OTAA_JOIN);

	//LORAWAN_NWKSKWY_FF = lorawan_read_otaa_information(LORAWAN_NWKSKWY_ADDRESS);
	LORAWAN_NWKSKWY_FF = eeprom_driver_read_uint8_t(app_number, 1);
	//uart_puts("NWK: \n");
	//uart_putn(LORAWAN_NWKSKWY_FF);
	//uart_puts("\n");

	if (LORAWAN_NWKSKWY_FF == 255 && loraForceReJoin != LORAWAN_TIMEOUT_CONNECTION_LIMIT)//Abfrage  ob frischer EEPROM vorliegt. Wenn ja FORCE_OTAA_JOIN
	{

		//loraForceReJoin = LORAWAN_TIMEOUT_CONNECTION_LIMIT;

		//lorawan_write_otaa_information(LORAWAN_FORCE_OTAA_JOIN, loraForceReJoin);
		uint8_t temp[1] = {LORAWAN_TIMEOUT_CONNECTION_LIMIT};
		eeprom_driver_write(app_number,temp, 8);

		uart_put("New memory detected...");
		_delay_ms(1000);		//Waiting for UART
		WDTCSR = (1 << WDE);
		//_delay_ms(100);
		//asm("jmp 0x0000");	
	}
	
	
	DDRD &= ~(1 << PORTD2);
	PORTD |= 0x04;                      // Activate pullupressistor of PD2
	_delay_ms(100);

	if(((PIND & (1<<PORTD2)&&loraForceReJoin == LORAWAN_TIMEOUT_CONNECTION_LIMIT))||(!(PIND & (1<<PORTD2)))){
		uart_puts("Skip memory load...\r\n");
	}
	else
	{
		lorawan_restore_session();	//Restore keys and parameter from last session
		uart_puts("Restore config...\r\n");
	}

	eeprom_driver_read(device_config.dev_nonce, app_number, 9);
	//device_config.dev_nonce[0] = lorawan_read_otaa_information(LORAWAN_DEVNONCE_ADDRESS)+1;
	//device_config.dev_nonce[1] = lorawan_read_otaa_information(LORAWAN_DEVNONCE_ADDRESS+1)+1;
	

	
	#endif
	
	
	lorawan_state *state = lorawan_packets_init(device_config);
	rfm95_init(chip_select);
	uart_put("RFM init");
	
	#if OTAA_ABP	//If keys are not set, do OTAA
	if(device_config.network_session_key[0] == 0 || device_config.app_session_key[0] == 0 || device_config.device_address[0] == 0 || !(PIND & (1<<PORTD2))||loraForceReJoin == LORAWAN_TIMEOUT_CONNECTION_LIMIT){
		
		uint8_t temp = 0;
		//lorawan_write_otaa_information(LORAWAN_FORCE_OTAA_JOIN, 0);
		eeprom_driver_write(app_number, temp, 8);

		loraForceReJoin = 0;
		device_config.network_session_key[0] = 0;
		uart_puts("Doing OTAA...\r\n");
		_delay_ms(100);
		do{
			uart_puts(state);
			uart_put("%s %d", "[dev nonce]:", device_config.dev_nonce[1]);
			lorawan_do_otaa(state);
			_delay_ms(10000);	//wait 10s. When failing, try again
			
			if(device_config.network_session_key[0] == 0){
				if(device_config.dev_nonce[1] < 255){	//IMPORTANT!!! -> if the device needs more join-request to sent until a join-accept will be received, the device need to increment the devNonce.
					device_config.dev_nonce[1]++;		//				  otherwise the packages will not be answered by the application server.
				} else if (device_config.dev_nonce[1] == 255 && device_config.dev_nonce[0] < 255){
					device_config.dev_nonce[1] = 0;
					device_config.dev_nonce[0]++;
				} else{
					device_config.dev_nonce[1] = 0;
					device_config.dev_nonce[0] = 0;
				}
				
				eeprom_driver_write(app_number, device_config.dev_nonce, 9);
				//lorawan_write_otaa_information(LORAWAN_DEVNONCE_ADDRESS,device_config.dev_nonce[0]);	//store the increased devNonce
				//lorawan_write_otaa_information(LORAWAN_DEVNONCE_ADDRESS+1,device_config.dev_nonce[1]);
			}
		}while(device_config.network_session_key[0] == 0);
		
		state->device_config = device_config; //setting the new keys and settings to state
		
		uart_puts("OTAA done...\r\nRebooting...\r\n");
		
		_delay_ms(1000);		//Waiting for UART
		WDTCSR = (1 << WDE);
		//_delay_ms(100);		//Waiting for UART
		//asm("jmp 0x0000");	//Using the Watchdog to reboot the Atmega16
	}
	#endif
	
	return state;
}

static void packet_received() {
	timer_stop();
	LORAWAN_TIMEOUT_CONNECTION = 0;
	if (received_data != NULL){
		free((uint8_t*) received_data); // free old data, that was never read
	}
	
	received_data_length = rfm95_read_packet((uint8_t**) &received_data);

	antenna_state = LORAWAN_ANTENNA_IDLE;
	
	if (callback != NULL)
		callback();
	callback = NULL;
}

static void start_rx1() {
	if (LORAWAN_TIMEOUT_CONNECTION != LORAWAN_TIMEOUT_CONNECTION_LIMIT)
	{
	antenna_state = LORAWAN_ANTENNA_RECEIVING;

	#if OTAA_ABP															//if OTAA is choosen
		rfm95_set_frequency(LORAWAN_FREQUENCY);	//selecting random channel from Join Accept Message
	#else
		rfm95_set_frequency(LORAWAN_RX1_FREQUENCY);
	#endif
	
	rfm95_set_spreading_factor(LORAWAN_RX1_SPREADING_FACTOR);
	rfm95_start_listening(&packet_received);
	uart_puts("start RX1...\r\n");
	}
	else
	{
		//lorawan_write_otaa_information(LORAWAN_FORCE_OTAA_JOIN, LORAWAN_TIMEOUT_CONNECTION_LIMIT);
		eeprom_driver_write(app_number, LORAWAN_TIMEOUT_CONNECTION_LIMIT, 8);
		WDTCSR = (1 << WDE);
	}
	
}

static void start_rx2() {
	if (LORAWAN_TIMEOUT_CONNECTION != LORAWAN_TIMEOUT_CONNECTION_LIMIT)
	{
		antenna_state = LORAWAN_ANTENNA_RECEIVING;

		rfm95_set_frequency(LORAWAN_RX2_FREQUENCY);
	
		rfm95_set_frequency(LORAWAN_RX2_FREQUENCY);
		rfm95_set_spreading_factor(LORAWAN_RX2_SPREADING_FACTOR);
		rfm95_start_listening(&packet_received);
	
		uart_puts("start RX2...\r\n");
	}
	else
	{	
		//lorawan_write_otaa_information(LORAWAN_FORCE_OTAA_JOIN, LORAWAN_TIMEOUT_CONNECTION_LIMIT);
		eeprom_driver_write(app_number, LORAWAN_TIMEOUT_CONNECTION_LIMIT, 8);
		WDTCSR = (1 << WDE);
	}
	LORAWAN_TIMEOUT_CONNECTION = LORAWAN_TIMEOUT_CONNECTION +1;
	uart_put("%s[%i/%i]", "timeout", LORAWAN_TIMEOUT_CONNECTION, LORAWAN_TIMEOUT_CONNECTION_LIMIT);

}

#if OTAA_ABP
static void start_accept1() {	//analog to start_rx1/2, but with otaa settings
	antenna_state = LORAWAN_ANTENNA_RECEIVING;
	rfm95_set_frequency(LORAWAN_RX1_FREQUENCY);
	rfm95_set_spreading_factor(LORAWAN_RX1_SPREADING_FACTOR);
	rfm95_start_listening(&lorawan_otaa_downlink);
}

static void start_accept2() {
	antenna_state = LORAWAN_ANTENNA_RECEIVING;
	rfm95_set_frequency(LORAWAN_RX2_FREQUENCY);
	rfm95_set_spreading_factor(LORAWAN_RX2_SPREADING_FACTOR);
	rfm95_start_listening(&lorawan_otaa_downlink);
}
#endif

static void packet_sent() {
	antenna_state = LORAWAN_ANTENNA_IDLE;
	
	timer_start(LORAWAN_RX1_DELAY_SECONDS, &start_rx1, LORAWAN_RX2_DELAY_SECONDS, &start_rx2);
}

#if OTAA_ABP
static void join_sent() { //analog to packet sent
	antenna_state = LORAWAN_ANTENNA_IDLE;
	
	timer_start(LORAWAN_JOINACCEPT1_DELAY_SECONDS, &start_accept1, LORAWAN_JOINACCEPT2_DELAY_SECONDS, &start_accept2);
}

void lorawan_otaa_downlink(){	//function will be called, when join-accept message will be received
	timer_stop();
	uint8_t payload[32] = { 5 };
	received_data_length = rfm95_read_packet((uint8_t**) &received_data);

	antenna_state = LORAWAN_ANTENNA_IDLE;
	
	uart_puts("Packet: ");
	for(uint8_t i = 0; i < received_data_length; i++){
		uint8_t *tmp = malloc(4);
		itoa(received_data[i],tmp,16);
		uart_puts(tmp);
		uart_putc(' ');
		free(tmp);
	}
	uart_puts("\r\n");
	
	_delay_ms(100);
	
	lorawan_otaa_encrypt(payload,received_data+1,device_config.app_key,received_data_length-1);	//decrypt the message with the app key
	
	uint8_t mic[4];
	
	lorawan_crypto_otaa_mic_answer(payload, device_config.app_key, mic, 29);
	
	if((payload[received_data_length-5] == mic[0]) && (payload[received_data_length-4] == mic[1]) && (payload[received_data_length-3] == mic[2]) && (payload[received_data_length-2] == mic[3])){	
		if((received_data_length-1 == 32) || (received_data_length-1 == 16)){	//check join_accept length
		
			uint8_t appNonce[3];
			for(uint8_t i = 0; i < 3; i++){
				//lorawan_write_otaa_information(LORAWAN_APPNONCE_ADDRESS+i, payload[i]);		//write information to eeprom
				appNonce[i] = payload[i];
			}
			uint8_t nwkid[3];
			for(uint8_t i = 0; i < 3; i++){
				nwkid[i] = payload[3+i];
				//lorawan_write_otaa_information(LORAWAN_NWKID_ADDRESS+i, payload[3+i]);
			}
			uint8_t devaddr[4];
			for(uint8_t i = 0; i < 4; i++){
				devaddr[i] = payload[9-i];
				//lorawan_write_otaa_information(LORAWAN_DEV_ADDRESS+i, payload[9-i]);
			}
			eeprom_driver_write(app_number, appNonce, 5);
			eeprom_driver_write(app_number, nwkid, 4);
			eeprom_driver_write(app_number, devaddr, 3);
			/**
			uint8_t appNonce[3];
			for(uint8_t i = 0; i < 3; i++){
				lorawan_write_otaa_information(LORAWAN_APPNONCE_ADDRESS+i, payload[i]);		//write information to eeprom
				appNonce[i] = payload[i];
			}
		
			uint8_t nwkid[3];
			for(uint8_t i = 0; i < 3; i++){
				nwkid[i] = payload[3+i];
				lorawan_write_otaa_information(LORAWAN_NWKID_ADDRESS+i, payload[3+i]);
			}
			
			uint8_t devaddr[4];
			for(uint8_t i = 0; i < 4; i++){
				devaddr[i] = payload[9-i];
				lorawan_write_otaa_information(LORAWAN_DEV_ADDRESS+i, payload[9-i]);
				
			}
			**/

			lorawan_compute_keys(appNonce, nwkid, device_config.dev_nonce, device_config.app_key, device_config.network_session_key, device_config.app_session_key);	//compute the network and app session keys
		
			uart_puts("NwkSKey: ");
			for(uint8_t i = 0; i < 16; i++){
				uint8_t *tmp = malloc(4);
				itoa(device_config.network_session_key[i],tmp,16);
				uart_puts(tmp);
				uart_putc(' ');
				free(tmp);
			}
			uart_puts("\r\n");
			
			uart_puts("AppSKey: ");
				for(uint8_t i = 0; i < 16; i++){
					uint8_t *tmp = malloc(4);
					itoa(device_config.app_session_key[i],tmp,16);
					uart_puts(tmp);
					uart_putc(' ');
					free(tmp);
				}
			uart_puts("\r\n");
			
			uart_puts("DeviceAddr: ");
			for(uint8_t i = 0; i < 4; i++){
				uint8_t *tmp = malloc(4);
				itoa(devaddr[i],tmp,16);
				uart_puts(tmp);
				uart_putc(' ');
				free(tmp);
			}
			uart_puts("\r\n");
	
			_delay_ms(1000);	
			


			eeprom_driver_write(app_number, device_config.network_session_key, 1);
			eeprom_driver_write(app_number, device_config.app_session_key, 2);

			/**
			for(uint8_t i = 0; i < 16; i++){
				lorawan_write_otaa_information(LORAWAN_NWKSKWY_ADDRESS+i, device_config.network_session_key[i]);
			}
		
			for(uint8_t i = 0; i < 16; i++){
				lorawan_write_otaa_information(LORAWAN_APPSKEY_ADDRESS+i, device_config.app_session_key[i]);
			}
			**/

			eeprom_driver_write(app_number, payload[10], 6);
			LORAWAN_RX2_SPREADING_FACTOR = (12 - (payload[10] & 0b00001111));									//setting the from ttn transmitted download settings
			LORAWAN_RX1_SPREADING_FACTOR = LORAWAN_RX1_SPREADING_FACTOR + ((payload[10] &  0b01110000) >> 4);

			eeprom_driver_write(app_number, payload[11], 7);
			LORAWAN_RX1_DELAY_SECONDS = payload[11];	//set new rx delay. ttn normaly sends rx1 delay 1s

			/**
			lorawan_write_otaa_information(LORAWAN_DLS_ADDRESS, payload[10]);
		
			LORAWAN_RX2_SPREADING_FACTOR = (12 - (payload[10] & 0b00001111));									//setting the from ttn transmitted download settings
			LORAWAN_RX1_SPREADING_FACTOR = LORAWAN_RX1_SPREADING_FACTOR + ((payload[10] &  0b01110000) >> 4);
		
			lorawan_write_otaa_information(LORAWAN_RXD_ADDRESS, payload[11]);
			LORAWAN_RX1_DELAY_SECONDS = payload[11];	//set new rx delay. ttn normaly sends rx1 delay 1s
			**/
		}
	
		if(received_data_length == 33){
			uint8_t temp[14];
			for (uint8_t i = 0; i < 15; i++){
				temp[i] = payload[12+i];	//the channellist will be stored
			}
			eeprom_driver_write(app_number, temp, 10);
			
			/**														//if channellist is send
			for (uint8_t i = 0; i < 15; i++){
				lorawan_write_otaa_information(LORAWAN_CHANNELLIST_ADDRESS+i,payload[12+i]);	//the channellist will be stored
			}
			**/
		} 
		else{
			//Variable 10
			uint8_t temp[14] = { 0x18, 0x4f, 0x84, 0xE8, 0x56, 0x84, 0xB8, 0x5E, 0x84 };
			eeprom_driver_write(app_number, temp, 10);
			
			/**																			//otherwise
			lorawan_write_otaa_information(LORAWAN_CHANNELLIST_ADDRESS,0x18);			//the three default frequencies will be stored
			lorawan_write_otaa_information(LORAWAN_CHANNELLIST_ADDRESS+1,0x4f);
			lorawan_write_otaa_information(LORAWAN_CHANNELLIST_ADDRESS+2,0x84);
		
			lorawan_write_otaa_information(LORAWAN_CHANNELLIST_ADDRESS+3,0xE8);
			lorawan_write_otaa_information(LORAWAN_CHANNELLIST_ADDRESS+4,0x56);
			lorawan_write_otaa_information(LORAWAN_CHANNELLIST_ADDRESS+5,0x84);
			//Hier vermutlich Fehler im alten CODE
			lorawan_write_otaa_information(LORAWAN_CHANNELLIST_ADDRESS+3,0xB8);
			lorawan_write_otaa_information(LORAWAN_CHANNELLIST_ADDRESS+4,0x5E);
			lorawan_write_otaa_information(LORAWAN_CHANNELLIST_ADDRESS+5,0x84);
			**/
		}
	}
};

bool lorawan_do_otaa(lorawan_state *state) {
	uint8_t buffer_length = 23;	//join-request length is fixed
	uint8_t *buffer = malloc(sizeof(uint8_t) * buffer_length);
	if (buffer == NULL)
		return false;
		
	state->device_config = device_config;	

	lorawan_packet *packet = lorawan_otaa_packet(state);	//make the join-request package
	if (packet == NULL){
		free(buffer);
		return false;
	}

	uint8_t packet_write_successful = lorawan_packets_write_otaa(state, packet, buffer);	//fill the buffer with the package
	if (!packet_write_successful) {
		lorawan_packets_delete_packet(packet);
		free(buffer);
		return false;
	}

	antenna_state = LORAWAN_ANTENNA_SENDING;
	callback = NULL;

	rfm95_set_frequency(LORAWAN_TX_FREQUENCY);
	rfm95_set_spreading_factor(LORAWAN_TX_SPREADING_FACTOR);
	rfm95_send_packet(buffer, buffer_length, &join_sent);

	lorawan_packets_delete_packet(packet);
	free(buffer);
	
	lorawan_packets_reset_counter(state);
	lorawan_write_otaa_information(62, 0);

	return true;
}
#endif

bool lorawan_send_data(lorawan_state *state, const uint8_t *payload, const uint8_t length, downlink_callback downlink_received) {
	uint8_t buffer_length = lorawan_packets_calculate_needed_buffer_length(length);
	uint8_t *buffer = malloc(sizeof(uint8_t) * buffer_length);
	if (buffer == NULL) return false;
	
	lorawan_packet *packet = lorawan_packets_new_packet(state, payload, length);
	if (packet == NULL) {
		free(buffer);
		return false;
	}

	uint8_t packet_write_successful = lorawan_packets_write_packet(state, packet, buffer, buffer_length);
	if (!packet_write_successful) {
		lorawan_packets_delete_packet(packet);
		free(buffer);
		return false;
	}

	antenna_state = LORAWAN_ANTENNA_SENDING;
	callback = downlink_received;

#if OTAA_ABP
	//uint8_t ran = (rand() % 3); //3 because of the used TheThingsIndoorGateway witch only uses channel 1 to 3

	lorawan_channel = (62 + (3 * (rand() % 3)));		//calculate the right address in eeprom

	freq.bytes[0] = lorawan_read_otaa_information(lorawan_channel);		//build a uint32_t with the help of a union
	freq.bytes[1] = lorawan_read_otaa_information(lorawan_channel+1);
	freq.bytes[2] = lorawan_read_otaa_information(lorawan_channel+2);
	freq.bytes[3] = 0x00;
		
	LORAWAN_FREQUENCY = 100.0f;					//prepare the float
	LORAWAN_FREQUENCY = LORAWAN_FREQUENCY * (float)freq.out;		//do the calculation of the frequency. The cast from uint32_t to float is the most important thing at this point.

	rfm95_set_frequency(LORAWAN_FREQUENCY);	//selecting random channel from Join Accept Message
#else
	rfm95_set_frequency(LORAWAN_TX_FREQUENCY);
#endif
	rfm95_set_spreading_factor(LORAWAN_TX_SPREADING_FACTOR);
	rfm95_send_packet(buffer, buffer_length, &packet_sent);

	lorawan_packets_delete_packet(packet);
	free(buffer);

	return true;
}

uint8_t lorawan_read_received_data(lorawan_state *state, uint8_t **payload) {
	if (!lorawan_data_received()) return 0;

	lorawan_packet *packet = lorawan_packets_read_packet(state, (uint8_t*) received_data, received_data_length);
	free((uint8_t*) received_data);
	received_data = NULL;
	received_data_length = 0;
	if (packet == NULL) return 0;

	uint8_t payload_length = packet->mac_payload.payload_length;
	*payload = malloc(sizeof(uint8_t) * payload_length);
	if (*payload == NULL) {
		lorawan_packets_delete_packet(packet);
		return 0;
	}

	memcpy(*payload, packet->mac_payload.payload, payload_length);
	
	lorawan_packets_delete_packet(packet);

	return payload_length;
}

bool lorawan_data_received() {
	return received_data_length != 0;
}

lorawan_antenna_state lorawan_get_antenna_state() {
	return antenna_state;
}

#if OTAA_ABP
uint8_t lorawan_read_otaa_information(uint8_t address){

	return eeprom_read_byte((const uint8_t*) address);
}

void lorawan_write_otaa_information(uint8_t address, uint8_t *value){
	//uart_put("%d",value);
	eeprom_update_byte((uint8_t*) address, value);
}

void lorawan_restore_session(){
	//device_config.app_nonce lesen
	eeprom_driver_read(device_config.app_nonce, app_number, 5);
	//device_config.network_id lesen
	eeprom_driver_read(device_config.network_id, app_number, 4);
	//device_config.network_session_key lesen
	eeprom_driver_read(device_config.network_session_key, app_number, 1);
	//device_config.app_session_key lesen
	eeprom_driver_read(device_config.app_session_key, app_number, 2);
	//device_address lesen ACHTUNG wird im Orginal Code von hinten nach vorne gelesen
	eeprom_driver_read(device_config.device_address, app_number, 3);
	//dev_nonce lesen
	eeprom_driver_read(device_config.dev_nonce, app_number, 9);
	//device_config.download_settings lesen
	device_config.download_settings = eeprom_driver_read_uint8_t(app_number, 6);
	LORAWAN_RX2_SPREADING_FACTOR = (12 - (device_config.download_settings & 0b00001111));
	LORAWAN_RX1_SPREADING_FACTOR = LORAWAN_RX1_SPREADING_FACTOR + ((device_config.download_settings &  0b01110000) >> 4);
	
	LORAWAN_RX1_DELAY_SECONDS = eeprom_driver_read_uint8_t(app_number, 7);
	LORAWAN_RX2_DELAY_SECONDS = LORAWAN_RX1_DELAY_SECONDS + 1;
	//LORAWAN_RX1_DELAY_SECONDS = 5;
	//LORAWAN_RX2_DELAY_SECONDS = LORAWAN_RX1_DELAY_SECONDS + 1;
	
	uart_put("App_Nonce");
	uart_putn(device_config.app_nonce);
	uart_put("Restored");
	
	/**CODE mit festgeschriebenen Addressen
	for(uint8_t i = 0; i < 3; i++){
		device_config.app_nonce[i] = lorawan_read_otaa_information(LORAWAN_APPNONCE_ADDRESS+i);
	}
	for(uint8_t i = 0; i < 3; i++){
		device_config.network_id[i] = lorawan_read_otaa_information(LORAWAN_NWKID_ADDRESS+i);
	}
	for(uint8_t i = 0; i < 16; i++){
		device_config.network_session_key[i] = lorawan_read_otaa_information(LORAWAN_NWKSKWY_ADDRESS+i);
	}
	for(uint8_t i = 0; i < 16; i++){
		device_config.app_session_key[i] = lorawan_read_otaa_information(LORAWAN_APPSKEY_ADDRESS+i);
	}
	for(uint8_t i = 0; i < 4; i++){
		device_config.device_address[3-i] = lorawan_read_otaa_information(LORAWAN_DEV_ADDRESS+i);
	}
	
	device_config.dev_nonce[0] = lorawan_read_otaa_information(LORAWAN_DEVNONCE_ADDRESS);
	device_config.dev_nonce[1] = lorawan_read_otaa_information(LORAWAN_DEVNONCE_ADDRESS+1);
	
	device_config.download_settings = lorawan_read_otaa_information(LORAWAN_DLS_ADDRESS);
	LORAWAN_RX2_SPREADING_FACTOR = (12 - (device_config.download_settings & 0b00001111));
	LORAWAN_RX1_SPREADING_FACTOR = LORAWAN_RX1_SPREADING_FACTOR + ((device_config.download_settings &  0b01110000) >> 4);
	LORAWAN_RX1_DELAY_SECONDS = lorawan_read_otaa_information(LORAWAN_RXD_ADDRESS);
	LORAWAN_RX2_DELAY_SECONDS = LORAWAN_RX1_DELAY_SECONDS + 1;
}
**/
}
#endif