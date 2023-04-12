#include "utils.h"

#include <stdint.h>

uint32_t uint8_array_to_uint32(const uint8_t *data) {
    uint32_t output = 0;

	output |= ((uint32_t) data[0]) << 0;
    output |= ((uint32_t) data[1]) << 8;
    output |= ((uint32_t) data[2]) << 16;
    output |= ((uint32_t) data[3]) << 24;
	
    return output;
}

void uint32_to_uint8_array(const uint32_t data, uint8_t array[4]) {
    array[0] = (data >> 0);
    array[1] = (data >> 8);
    array[2] = (data >> 16);
    array[3] = (data >> 24);
}

uint16_t uint8_array_to_uint16(const uint8_t *data) {
    uint32_t output = 0;

	output |= ((uint16_t) data[0]) << 0;
    output |= ((uint16_t) data[1]) << 8;
	
    return output;
}
