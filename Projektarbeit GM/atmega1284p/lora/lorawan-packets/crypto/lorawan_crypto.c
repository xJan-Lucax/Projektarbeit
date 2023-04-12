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

#include "lorawan_crypto.h"
#include "../lorawan_packets.h"
#include "../utils.h"
#include "cmac.h"

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define LW_KEY_LEN 16
#define LORAWAN_BLOCK_LENGTH 16

void lorawan_crypto_otaa_mic(const uint8_t *buffer, const uint8_t *aes_key, uint8_t mic[4]){
	uint8_t b0[19]; //malloc(19*sizeof(uint8_t));
	
	memcpy(b0, buffer, 19*sizeof(uint8_t));//19
	
	aes_cmac_context cmacctx;
	aes_cmac_init(&cmacctx);
	
	aes_cmac_set_key(&cmacctx, aes_key);

	aes_cmac_update(&cmacctx, b0, 19);

	//free(b0);

	uint8_t temp[19];
	aes_cmac_final(temp, &cmacctx);	

	memcpy(mic, temp, sizeof(uint8_t) * LORAWAN_MIC_LENGTH); // the MIC is only 4 bytes long
}

void lorawan_crypto_otaa_mic_answer(const uint8_t *buffer, const uint8_t *aes_key, uint8_t mic[4], uint8_t length){
	uint8_t b0[length];
	
	b0[0] = 0x20;
	memcpy(b0+1, buffer, (length-1)*sizeof(uint8_t));
	
	aes_cmac_context cmacctx;
	aes_cmac_init(&cmacctx);
	
	aes_cmac_set_key(&cmacctx, aes_key);

	aes_cmac_update(&cmacctx, b0, length);

	//free(b0);

	uint8_t temp[length];
	aes_cmac_final(temp, &cmacctx);

	memcpy(mic, temp, sizeof(uint8_t) * LORAWAN_MIC_LENGTH); // the MIC is only 4 bytes long
}

void lorawan_crypto_calculate_mic(uint8_t mic[LORAWAN_MIC_LENGTH], const uint8_t *data, uint8_t data_length, const uint8_t *aes_key, uint8_t* device_address, lorawan_crypto_message_direction direction, uint32_t frame_counter) {
    uint8_t b0[LW_KEY_LEN];
    memset(b0, 0, LW_KEY_LEN);

    b0[0] = 0x49;
    b0[5] = direction;
    //	uint32_to_uint8_array(device_address, b0 + 6);
	
	for (uint8_t i = 0; i < 4; i++){
		b0[6+i] = device_address[i];
	}
	
    uint32_to_uint8_array(frame_counter, b0 + 10);
    b0[15] = data_length;

	aes_cmac_context cmacctx;
	aes_cmac_init(&cmacctx);
	
	aes_cmac_set_key(&cmacctx, aes_key);

	aes_cmac_update(&cmacctx, b0, LW_KEY_LEN);
	aes_cmac_update(&cmacctx, data, data_length);

	uint8_t temp[LW_KEY_LEN];
	aes_cmac_final(temp, &cmacctx);

	memcpy(mic, temp, sizeof(uint8_t) * LORAWAN_MIC_LENGTH); // the MIC is only 4 bytes long
}

static void lorawan_crypto_xor(uint8_t const l[], uint8_t const r[], uint8_t out[], uint16_t bytes) {
	uint8_t const* lptr = l;
	uint8_t const* rptr = r;
	uint8_t* optr = out;
	uint8_t const* const end = out + bytes;

	for (;optr < end; lptr++, rptr++, optr++) *optr = *lptr ^ *rptr;
}

bool lorawan_crypto_encrypt(uint8_t *encrypted_data, const uint8_t *data, uint8_t data_length, const uint8_t *aes_key, uint8_t* device_address, lorawan_crypto_message_direction direction, uint32_t frame_counter) {
    if (data_length == 0) return false;

	uint8_t temp_block[LORAWAN_BLOCK_LENGTH];

	uint16_t const over_hang_bytes = data_length % LORAWAN_BLOCK_LENGTH;
    int blocks = data_length / LORAWAN_BLOCK_LENGTH;
    if (over_hang_bytes) {
    	++blocks;
    }

	memset(temp_block, 0, LORAWAN_BLOCK_LENGTH);

	temp_block[0] = 0x01;
	temp_block[5] = direction;

	// uint32_to_uint8_array(device_address, temp_block + 6);
	
	for (uint8_t i = 0; i < 4; i++){
		temp_block[6+i] = device_address[i];
	}
	
	uint32_to_uint8_array(frame_counter, temp_block + 10);

	for (uint8_t i = 1; i <= blocks; i++, data += LORAWAN_BLOCK_LENGTH, encrypted_data += LORAWAN_BLOCK_LENGTH){
		temp_block[15] = i;

		aes_context aesContext;
		aes_set_key(aes_key, LW_KEY_LEN, &aesContext);

		uint8_t encrypted_block[LORAWAN_BLOCK_LENGTH];
		aes_encrypt(temp_block, encrypted_block, &aesContext);

		uint16_t bytes_to_encrypt;
		if ((i < blocks) || (over_hang_bytes == 0)) {
			bytes_to_encrypt = LORAWAN_BLOCK_LENGTH;
		} else {
			bytes_to_encrypt = over_hang_bytes;
		}

		lorawan_crypto_xor(encrypted_block, data, encrypted_data, bytes_to_encrypt);
	}

	return true;
}

void lorawan_otaa_encrypt(uint8_t *payload, const uint8_t *buffer, const uint8_t *key, const uint8_t payload_length){
	if (payload_length == 0) return false;

	aes_context aesContext;
	aes_set_key(key, LW_KEY_LEN, &aesContext);
	aes_encrypt(buffer, payload, &aesContext);

	if (payload_length > 16) {
		aes_encrypt(buffer+16, payload+16, &aesContext);
	}
}

void lorawan_compute_keys(const uint8_t *appNonce, const uint8_t *netID, const uint8_t *devNonce, const uint8_t *appkey, uint8_t *nwkskey, uint8_t *appskey){
	uint8_t nonce[16];		//Building the Nonce to generate the Network and Application session Keys
	
	memset(nonce, 0, 16);
	nonce[0] = 0x01;	//The Network session Key requiers a 0x01 at the first position
	memcpy(nonce + 1, appNonce, 3);
	memcpy(nonce + 4, netID, 3);
	nonce[7] = devNonce[1];
	nonce[8] = devNonce[0];
	
	aes_context aesContext;

	aes_set_key(appkey, 16, &aesContext);	//The Keys are generated with the Application Key
	aes_encrypt(nonce, nwkskey, &aesContext);
	
	nonce[0] = 0x02;	//The Application session Key requiers a 0x02 at the first position
	aes_encrypt(nonce, appskey, &aesContext);
}