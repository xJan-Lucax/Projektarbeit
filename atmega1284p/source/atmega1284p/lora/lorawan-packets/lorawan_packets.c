#include "lorawan_packets.h"

#include "crypto/lorawan_crypto.h"
#include "utils.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "../lorawan.h"

#define LORAWAN_MAC_HEADER_LENGTH 		1
#define LORAWAN_DEVICE_ADDRESS_LENGTH 	4
#define LORAWAN_FRAME_CONTROL_LENGTH 	1
#define LORAWAN_FRAME_COUNTER_LENGTH 	2
#define LORAWAN_FRAME_PORT_LENGTH 		1

#define LORAWAN_MAC_HEADER_POSITION 	0
#define LORAWAN_DEVICE_ADDRESS_POSITION (LORAWAN_MAC_HEADER_POSITION 		+ 	LORAWAN_MAC_HEADER_LENGTH)
#define LORAWAN_FRAME_CONTROL_POSITION 	(LORAWAN_DEVICE_ADDRESS_POSITION 	+ 	LORAWAN_DEVICE_ADDRESS_LENGTH)
#define LORAWAN_FRAME_COUNTER_POSITION 	(LORAWAN_FRAME_CONTROL_POSITION 	+ 	LORAWAN_FRAME_CONTROL_LENGTH)
#define LORAWAN_FRAME_PORT_POSITION 	(LORAWAN_FRAME_COUNTER_POSITION 	+ 	LORAWAN_FRAME_COUNTER_LENGTH)
#define LORAWAN_PAYLOAD_POSITION 		(LORAWAN_FRAME_PORT_POSITION 		+	LORAWAN_FRAME_PORT_LENGTH)

#define LORAWAN_MAX_FRAME_COUNTER_GAP 16384

lorawan_state* lorawan_packets_init(lorawan_device_config config) {
	lorawan_state* state = malloc(sizeof(lorawan_state));
	lorawan_packets_reset_counter(state);
	state->device_config = config;
	
	#if OTAA_ABP
		state->frame_counter.up = lorawan_read_otaa_information(62);

	#endif
	
	return state;
}

void lorawan_packets_reset_counter(lorawan_state *state) {
	state->frame_counter.up = 0;
	state->frame_counter.down = 0;
}

uint8_t lorawan_packets_calculate_needed_buffer_length(uint8_t payload_length) {
	return LORAWAN_MAC_HEADER_LENGTH + LORAWAN_DEVICE_ADDRESS_LENGTH + LORAWAN_FRAME_CONTROL_LENGTH
		 + LORAWAN_FRAME_COUNTER_LENGTH + LORAWAN_FRAME_PORT_LENGTH + payload_length + LORAWAN_MIC_LENGTH;
}

#if OTAA_ABP
lorawan_packet* lorawan_new_join_request(lorawan_state *state, const uint8_t *payload) {
	if (payload == NULL) {
		return NULL;
	}

	lorawan_packet *packet = malloc(sizeof(lorawan_packet));
	if (packet == NULL) return NULL;
	memset(packet, 0, sizeof(lorawan_packet));

	packet->mac_payload.payload = malloc(sizeof(uint8_t) * 22);
	if (packet->mac_payload.payload == NULL) {
		lorawan_packets_delete_packet(packet);
		return NULL;
	}
		
	memcpy(packet->mac_payload.payload, payload, 22);

	packet->mac_payload.payload_length = 22;

	packet->mac_header.version = R1;
	packet->mac_header.type = LORAWAN_JOIN_REQUEST;
	packet->mac_payload.port = 0;

	return packet;
}

lorawan_packet* lorawan_otaa_packet(lorawan_state *state){
	uint8_t *JoinFrame = malloc(sizeof(uint8_t)*22);
	
	if(JoinFrame == NULL)
		return NULL;
	
	for(uint8_t i = 0; i < 8; i++){
		JoinFrame[i] = state->device_config.app_eui[7-i];
	}
	
	for(uint8_t i = 0; i < 8; i++){
		JoinFrame[i+8] = state->device_config.dev_eui[7-i];
	}
	
	for(uint8_t i = 0; i < 2; i++){
		JoinFrame[i+16] = state->device_config.dev_nonce[1-i];
	}
	
	return lorawan_new_join_request(&state, JoinFrame);
}
#endif

lorawan_packet* lorawan_packets_new_packet(lorawan_state *state, const uint8_t *payload, uint8_t length) {
	assert(length <= 222); // max mac_payload size of lorawan EU868 (see 2.1.6 lorawan 1.1 regional parameters)

	if (payload == NULL || length == 0) {
		return NULL;
	}

	lorawan_packet *packet = malloc(sizeof(lorawan_packet));
	if (packet == NULL) return NULL;
	memset(packet, 0, sizeof(lorawan_packet));

	packet->mac_payload.payload = malloc(sizeof(uint8_t) * length);
	if (packet->mac_payload.payload == NULL) {
		lorawan_packets_delete_packet(packet);
		return NULL;
	}
	memcpy(packet->mac_payload.payload, payload, length);

	packet->mac_payload.payload_length = length;

	packet->mac_header.version = R1;
	packet->mac_header.type = LORAWAN_UNCONFIRMED_DATA_UP;
	packet->mac_payload.port = 1;
	packet->mac_payload.frame_header.device_address = state->device_config.device_address;
	packet->mac_payload.frame_header.frame_counter = state->frame_counter.up;

	state->frame_counter.up++;
	
	#if OTAA_ABP
	if((state->frame_counter.up%10) == 1)
		lorawan_write_otaa_information(62, state->frame_counter.up);
	#endif

	return packet;
}

void lorawan_packets_delete_packet(lorawan_packet *packet) {
	if (packet == NULL) return;

	if (packet->mac_payload.payload != NULL) {
		free(packet->mac_payload.payload);
	}

	free(packet);
}

// Returns the used buffer length
// Returns 0 if the buffer is too small or an unsupported packet configuration  was given
uint8_t lorawan_packets_write_packet(const lorawan_state *state, lorawan_packet *packet, uint8_t *buffer, uint8_t buffer_length) {
	if (packet->mac_payload.port == 0) return 0; // MAC commands (port = 0) are not supported
	if (packet->mac_header.type != LORAWAN_UNCONFIRMED_DATA_UP) return 0; // unsupported message type
	if (packet->mac_payload.payload_length == 0 && packet->mac_payload.payload == NULL) return 0; // no payload
	
	uint8_t needed_buffer_length = lorawan_packets_calculate_needed_buffer_length(packet->mac_payload.payload_length);
	if (buffer_length < needed_buffer_length) return 0;

	memset(buffer, 0, buffer_length);

	// mac_header
	buffer[LORAWAN_MAC_HEADER_POSITION] = (packet->mac_header.type << 5u) | packet->mac_header.version;

	// device address
	//uint32_to_uint8_array(packet->mac_payload.frame_header.device_address, buffer + LORAWAN_DEVICE_ADDRESS_POSITION);
	for (uint8_t i = 0; i < 4; i++)
	{
		buffer[i + LORAWAN_DEVICE_ADDRESS_POSITION] = packet->mac_payload.frame_header.device_address[i];

	}

	// frame control
	buffer[LORAWAN_FRAME_CONTROL_POSITION] = 0;

	// frame counter
	// 32 bit are set, but the MSB 16 bit are overwritten later (little endian)
	uint32_to_uint8_array(packet->mac_payload.frame_header.frame_counter, buffer + LORAWAN_FRAME_COUNTER_POSITION);


	// frame port
	buffer[LORAWAN_FRAME_PORT_POSITION] = packet->mac_payload.port;
	
	uint8_t payload_length = packet->mac_payload.payload_length;
	if (!lorawan_crypto_encrypt(
		buffer + LORAWAN_PAYLOAD_POSITION,
		packet->mac_payload.payload,
		packet->mac_payload.payload_length,
		state->device_config.app_session_key,
		packet->mac_payload.frame_header.device_address,
		LORAWAN_CRYPTO_UP,
		packet->mac_payload.frame_header.frame_counter)
	) {
		return 0;
	}
	
	lorawan_crypto_calculate_mic(
		buffer + (LORAWAN_PAYLOAD_POSITION + payload_length), // MIC destination
		buffer, // data for MIC calculation
		LORAWAN_PAYLOAD_POSITION + payload_length, // data length for MIC calculation
		state->device_config.network_session_key,
		packet->mac_payload.frame_header.device_address,
		LORAWAN_CRYPTO_UP,
		packet->mac_payload.frame_header.frame_counter
	);
	
	return needed_buffer_length;
}

#if OTAA_ABP
uint8_t lorawan_packets_write_otaa(const lorawan_state *state, lorawan_packet *packet, uint8_t *buffer) {
	if (packet->mac_payload.payload_length == 0 && packet->mac_payload.payload == NULL) return 0; // no payload
	
	memset(buffer, 0, 23);
	
	buffer[0] = 0b00000000; // MHDR
	
	for(uint8_t i = 0; i < 18; i++){
		buffer[i+1] = packet->mac_payload.payload[i];

	}
	
	uint8_t *mic = malloc(4*sizeof(uint8_t));
	
	if(mic == NULL)
		return NULL;
	
	lorawan_crypto_otaa_mic(buffer,state->device_config.app_key, mic);
	
	memcpy(buffer+19,mic,4);
	
	return 23;
}
#endif

lorawan_packet* lorawan_packets_read_packet(lorawan_state *state, const uint8_t *buffer, uint8_t length) {
	// the buffer length must be at least as big as expected for a 1 byte payload
	if (length < lorawan_packets_calculate_needed_buffer_length(1)) return NULL;

	lorawan_packet *packet = malloc(sizeof(lorawan_packet));
	if (packet == NULL) return NULL;
	memset(packet, 0, sizeof(lorawan_packet));

	// mac header
	packet->mac_header.type = (lorawan_message_type) (buffer[LORAWAN_MAC_HEADER_POSITION] >> 5u);
	packet->mac_header.version = (lorawan_major_version) (buffer[LORAWAN_MAC_HEADER_POSITION]);
	if (packet->mac_header.type != LORAWAN_UNCONFIRMED_DATA_DOWN && packet->mac_header.type != LOWAWAN_JOIN_ACCEPT) {
		// unsupported packet type
		lorawan_packets_delete_packet(packet);
		return NULL;
	}

	// device address
	packet->mac_payload.frame_header.device_address = uint8_array_to_uint32(buffer + LORAWAN_DEVICE_ADDRESS_POSITION);
	if (packet->mac_payload.frame_header.device_address != state->device_config.device_address) {
		uart_put("Packet wird verworfen");		
		lorawan_packets_delete_packet(packet);
		return NULL;
	}

	// port
	packet->mac_payload.port = buffer[LORAWAN_FRAME_PORT_POSITION];

	// frame counter
	uint32_t client_frame_counter_down = state->frame_counter.down;
	uint16_t server_frame_counter_down = uint8_array_to_uint16(buffer + LORAWAN_FRAME_COUNTER_POSITION);
	if (server_frame_counter_down - ((uint16_t) client_frame_counter_down) > LORAWAN_MAX_FRAME_COUNTER_GAP) {
		// gap is too large, too many packets have been dropped, don't allow further communication
		lorawan_packets_delete_packet(packet);
		return NULL;
	}
	uint32_t assumed_server_frame_counter_down = (client_frame_counter_down & 0xFFFF0000) + server_frame_counter_down;
	packet->mac_payload.frame_header.frame_counter = assumed_server_frame_counter_down;

	// calc & compare mic
	memcpy(packet->mic, buffer + sizeof(uint8_t) * (length - LORAWAN_MIC_LENGTH), LORAWAN_MIC_LENGTH);
	uint8_t calculated_mic[LORAWAN_MIC_LENGTH];
	lorawan_crypto_calculate_mic(
		calculated_mic,
		buffer,
		length - LORAWAN_MIC_LENGTH,
		state->device_config.network_session_key,
		state->device_config.device_address,
		LORAWAN_CRYPTO_MESSAGE_DOWN,
		assumed_server_frame_counter_down
	);

	bool mic_mismatch = memcmp(calculated_mic, packet->mic, sizeof(uint8_t) * LORAWAN_MIC_LENGTH);
	if (mic_mismatch) {
		lorawan_packets_delete_packet(packet);
		return NULL;
	}

	// MIC was correct, so the counter was also correct
	state->frame_counter.down = assumed_server_frame_counter_down + 1;

	// payload
	packet->mac_payload.payload_length = length - LORAWAN_PAYLOAD_POSITION - LORAWAN_MIC_LENGTH;
	packet->mac_payload.payload = (uint8_t*) malloc(sizeof(uint8_t) * packet->mac_payload.payload_length);
	if (packet->mac_payload.payload == NULL) {
		lorawan_packets_delete_packet(packet);
		return NULL;
	}

	// the XORed data is reversable by XORing again -> enrypting = decrypting
	// aes encrypted _known_ data is just used for the XORing
	if (!lorawan_crypto_encrypt(
			packet->mac_payload.payload,
			buffer + LORAWAN_PAYLOAD_POSITION,
			packet->mac_payload.payload_length,
			state->device_config.app_session_key,
			packet->mac_payload.frame_header.device_address,
			LORAWAN_CRYPTO_MESSAGE_DOWN,
			assumed_server_frame_counter_down)
	) {
		// decrypt failed
		lorawan_packets_delete_packet(packet);
		return NULL;
	}

	return packet;
}
