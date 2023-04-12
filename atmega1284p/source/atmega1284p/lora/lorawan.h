#pragma once

#include <stdint.h>
#include <stdbool.h>

#define OTAA_ABP 1 //1 -> OTAA, 0 -> ABP

typedef enum {
	LORAWAN_ANTENNA_IDLE,
	LORAWAN_ANTENNA_SENDING,
	LORAWAN_ANTENNA_RECEIVING
} lorawan_antenna_state;

#if OTAA_ABP

typedef struct {
	uint8_t device_address[4];
	uint8_t app_key[16];
	uint8_t app_eui[8];
	uint8_t app_session_key[16];
	uint8_t network_session_key[16];
	uint8_t dev_eui[8];
	uint8_t dev_nonce[2];
	uint8_t app_nonce[3];
	uint8_t network_id[3];
	uint8_t download_settings;
} lorawan_device_config;
#else

typedef struct {
	uint8_t device_address[4];
	uint8_t app_session_key[16];
	uint8_t network_session_key[16];
} lorawan_device_config;
#endif

typedef struct {
	uint32_t up;
	uint32_t down;
} lorawan_frame_counter;

typedef struct {
	lorawan_frame_counter frame_counter;
	lorawan_device_config device_config;
} lorawan_state;

typedef void (*downlink_callback)(void);
typedef void (*state_tmp)(void);

lorawan_state* lorawan_init(uint8_t chip_select);
bool lorawan_send_data(lorawan_state *state, const uint8_t *payload, const uint8_t length, downlink_callback callback);
bool lorawan_data_received();
uint8_t lorawan_read_received_data(lorawan_state *state, uint8_t **payload);
lorawan_antenna_state lorawan_get_antenna_state();
#if OTAA_ABP
bool lorawan_do_otaa();
void lorawan_otaa_downlink();
uint8_t lorawan_read_otaa_information(uint8_t address);
void lorawan_write_otaa_information(uint8_t address, uint8_t *value);
#endif
