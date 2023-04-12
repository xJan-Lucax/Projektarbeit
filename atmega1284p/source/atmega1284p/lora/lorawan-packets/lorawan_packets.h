#pragma once

#include "../lorawan.h"

#include <stdint.h>
#include <stdbool.h>

#define LORAWAN_MIC_LENGTH 4

typedef enum {
	LORAWAN_UNCONFIRMED_DATA_UP = 0b010,
	LORAWAN_UNCONFIRMED_DATA_DOWN = 0b011,
	LORAWAN_JOIN_REQUEST = 0b000,
	LOWAWAN_JOIN_ACCEPT = 0b001
} lorawan_message_type;

typedef enum {
	R1 = 0
} lorawan_major_version;

typedef struct {
	lorawan_message_type type;
	lorawan_major_version version;
} lorawan_mac_header;

typedef struct {
	uint8_t *device_address;
	uint32_t frame_counter;
} lorawan_frame_header;

typedef struct {
	lorawan_frame_header frame_header;
	uint8_t port;
	uint8_t* payload;
	uint8_t payload_length;
} lorawan_mac_payload;

typedef struct {
	lorawan_mac_header mac_header;
	lorawan_mac_payload mac_payload;
	uint8_t mic[LORAWAN_MIC_LENGTH];
} lorawan_packet;


lorawan_state* lorawan_packets_init(lorawan_device_config config);
void lorawan_packets_reset_counter(lorawan_state *state);
uint8_t lorawan_packets_calculate_needed_buffer_length(uint8_t payload_length);

lorawan_packet* lorawan_new_join_request(lorawan_state *state, const uint8_t *payload);

lorawan_packet* lorawan_packets_new_packet(lorawan_state *state, const uint8_t* payload, uint8_t length);

uint8_t lorawan_packets_write_packet(const lorawan_state *state, lorawan_packet* packet, uint8_t* buffer, uint8_t length);
uint8_t lorawan_packets_write_otaa(const lorawan_state *state, lorawan_packet *packet, uint8_t *buffer);
lorawan_packet* lorawan_packets_read_packet(lorawan_state *state, const uint8_t* buffer, uint8_t length);
void lorawan_packets_delete_packet(lorawan_packet* packet);
