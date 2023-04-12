/**************************************************************************
Copyright (C) 2009 Lander Casado, Philippas Tsigas

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files
(the "Software"), to deal with the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimers. Redistributions in
binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimers in the documentation and/or
other materials provided with the distribution.

In no event shall the authors or copyright holders be liable for any special,
incidental, indirect or consequential damages of any kind, or any damages
whatsoever resulting from loss of use, data or profits, whether or not
advised of the possibility of damage, and on any theory of liability,
arising out of or in connection with the use or performance of this software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS WITH THE SOFTWARE

*****************************************************************************/

#include "aes.h"
#include "cmac.h"

#include <string.h>

#define LSHIFT(v, r) do {                                       \
		int i;                                                  \
		for (i = 0; i < 15; i++)                                \
			(r)[i] = (v)[i] << 1 | (v)[i + 1] >> 7;             \
		(r)[15] = (v)[15] << 1;                                 \
	} while (0)

#define XOR(v, r) do {                                          \
		int i;                                                  \
		for (i = 0; i < 16; i++)                                \
		{	                                                    \
					(r)[i] = (r)[i] ^ (v)[i];                   \
		}                                                       \
	} while (0)                                                 \


#define MIN(a,b) (((a)<(b))?(a):(b))

void aes_cmac_init(aes_cmac_context *context) {
	memset(context->x, 0, sizeof context->x);
	context->m_n = 0;
	memset(context->context.ksch, '\0', sizeof(context->context.ksch));
}

void aes_cmac_set_key(aes_cmac_context *context, const uint8_t key[AES_CMAC_KEY_LENGTH]) {
	aes_set_key(key, AES_CMAC_KEY_LENGTH, &context->context);
}

void aes_cmac_update(aes_cmac_context *context, const uint8_t *data, u_int length) {
	u_int mlen;
	unsigned char in[16];

	if (context->m_n > 0) {
		mlen = MIN(16 - context->m_n, length);
		memcpy(context->m_last + context->m_n, data, mlen);
		context->m_n += mlen;
		if (context->m_n < 16 || length == mlen) return;
		XOR(context->m_last, context->x);
		aes_encrypt( context->x, context->x, &context->context);
		data += mlen;
		length -= mlen;
	}
	while (length > 16) {
		/* not last block */

		XOR(data, context->x);

		memcpy(in, &context->x[0], 16);
		aes_encrypt( in, in, &context->context);
		memcpy(&context->x[0], in, 16);

		data += 16;
		length -= 16;
	}
	/* potential last block, save it */
	memcpy(context->m_last, data, length);
	context->m_n = length;
}

void aes_cmac_final(uint8_t digest[AES_CMAC_DIGEST_LENGTH], aes_cmac_context *context) {
	uint8_t K[16];
	unsigned char in[16];
	/* generate subkey K1 */
	memset(K, '\0', 16);

	aes_encrypt( K, K, &context->context);

	if (K[0] & 0x80) {
		LSHIFT(K, K);
		K[15] ^= 0x87;
	} else {
		LSHIFT(K, K);
	}

	if (context->m_n == 16){
		/* last block was a complete block */
		XOR(K, context->m_last);
	} else {
		/* generate subkey K2 */
		if (K[0] & 0x80) {
			LSHIFT(K, K);
			K[15] ^= 0x87;
		} else {
			LSHIFT(K, K);
		}

		/* padding(m_last) */
		context->m_last[context->m_n] = 0x80;
		while (++context->m_n < 16) {
			context->m_last[context->m_n] = 0;
		}

		XOR(K, context->m_last);

	}
	XOR(context->m_last, context->x);

	memcpy(in, &context->x[0], 16);
	aes_encrypt(in, digest, &context->context);
	memset(K, 0, sizeof K);
}
