#pragma once

#include "lorawan.h"

#include <stdint.h>

typedef void (*rfm95_callback)(void);

void rfm95_init(uint8_t cs);

void rfm95_set_frequency(float frequency);
void rfm95_set_spreading_factor(uint8_t spreading_factor);

void rfm95_send_packet(uint8_t *data, uint8_t length, rfm95_callback packet_sent);
void rfm95_start_listening(rfm95_callback packet_received);

uint8_t rfm95_read_packet(uint8_t **data_pointer);