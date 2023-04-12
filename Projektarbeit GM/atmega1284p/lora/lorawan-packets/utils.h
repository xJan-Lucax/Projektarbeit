#pragma once

#include <stdint.h>

// little endian
uint32_t uint8_array_to_uint32(const uint8_t *data);

// little endian
void uint32_to_uint8_array(const uint32_t data, uint8_t *array);

// little endian
uint16_t uint8_array_to_uint16(const uint8_t *data);

