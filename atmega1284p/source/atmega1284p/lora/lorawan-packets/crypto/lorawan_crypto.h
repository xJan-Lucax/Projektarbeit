/**************************************************************************
 Copyright (c) <2016> <Jiapeng Li>
 https://github.com/JiapengLi/lorawan-parser

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

*****************************************************************************/

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    LORAWAN_CRYPTO_UP = 0,
    LORAWAN_CRYPTO_MESSAGE_DOWN = 1,
} lorawan_crypto_message_direction;

void lorawan_crypto_calculate_mic(
    uint8_t *mic,
    const uint8_t *data,
    uint8_t data_length,
    const uint8_t *aes_key,
    uint8_t* device_address,
    lorawan_crypto_message_direction direction,
    uint32_t frame_counter
);

// returns true for successful encryption, false if not
bool lorawan_crypto_encrypt(
    uint8_t *encrypted_data,
    const uint8_t *data,
    uint8_t data_length,
    const uint8_t *aes_key,
    uint8_t* device_address,
    lorawan_crypto_message_direction direction,
    uint32_t frame_counter
);
