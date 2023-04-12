/**************************************************************************
 Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.

 LICENSE TERMS

 The redistribution and use of this software (with or without changes)
 is allowed without the payment of fees or royalties provided that:

  1. source code distributions include the above copyright notice, this
	 list of conditions and the following disclaimer;

  2. binary distributions include the above copyright notice, this list
	 of conditions and the following disclaimer in their documentation;

  3. the name of the copyright holder is not used to endorse products
	 built using this software without specific written permission.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue 09/09/2006

 This is an AES implementation that uses only 8-bit byte operations on the
 cipher state (there are options to use 32-bit types if available).

 The combination of mix columns and byte substitution used here is based on
 that developed by Karl Malbrain. His contribution is acknowledged.

*****************************************************************************/

#include "aes.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* alternative versions (test for performance on your system) */
#if 1
#  define VERSION_1
#endif

/* functions for finite field multiplication in the AES Galois field */

#define WPOLY   0x011b
#define BPOLY     0x1b
#define DPOLY   0x008d

#define f1(x)   (x)
#define f2(x)   ((x << 1) ^ (((x >> 7) & 1) * WPOLY))
#define f4(x)   ((x << 2) ^ (((x >> 6) & 1) * WPOLY) ^ (((x >> 6) & 2) * WPOLY))
#define f8(x)   ((x << 3) ^ (((x >> 5) & 1) * WPOLY) ^ (((x >> 5) & 2) * WPOLY) \
						  ^ (((x >> 5) & 4) * WPOLY))
#define d2(x)   (((x) >> 1) ^ ((x) & 1 ? DPOLY : 0))

#define f3(x)   (f2(x) ^ x)
#define f9(x)   (f8(x) ^ x)
#define fb(x)   (f8(x) ^ f2(x) ^ x)
#define fd(x)   (f8(x) ^ f4(x) ^ x)
#define fe(x)   (f8(x) ^ f4(x) ^ f2(x))

/* this is the high bit of x right shifted by 1 */
/* position. Since the starting polynomial has  */
/* 9 bits (0x11b), this right shift keeps the   */
/* values of all top bits within a byte         */
static uint8_t hibit(const uint8_t x) {
	uint8_t r = (uint8_t)((x >> 1) | (x >> 2));
	r |= (r >> 2);
	r |= (r >> 4);
	return (r + 1) >> 1;
}

/* return the inverse of the finite field element x */
static uint8_t gf_inv(const uint8_t x) {
	uint8_t p1 = x, p2 = BPOLY, n1 = hibit(x), n2 = 0x80, v1 = 1, v2 = 0;

	if (x < 2) {
		return x;
	}

	for (; ;) {
		if (n1) {
			/* divide polynomial p2 by p1    */
			while(n2 >= n1) {
				n2 /= n1;               /* shift smaller polynomial left */
				p2 ^= (p1 * n2) & 0xff; /* and remove from larger one    */
				v2 ^= (v1 * n2);        /* shift accumulated value and   */
				n2 = hibit(p2);         /* add into result               */
			}
		} else {
			return v1;
		}

		if (n2)  {                      /* repeat with values swapped    */
			while (n1 >= n2) {
				n1 /= n2;
				p1 ^= p2 * n1;
				v1 ^= v2 * n1;
				n1 = hibit(p1);
			}
		} else {
			return v2;
		}
	}
}

static uint8_t fwd_affine(const uint8_t x) {
	return 0x63 ^ x ^ (x << 1) ^ (x << 2) ^ (x << 3) ^ (x << 4) ^ (x >> 7) ^ (x >> 6) ^ (x >> 5) ^ (x >> 4);
}

#define s_box(x)   fwd_affine(gf_inv(x))
#define gfm2_sb(x) f2(s_box(x))
#define gfm3_sb(x) f3(s_box(x))
#define gfm_9(x)   f9(x)
#define gfm_b(x)   fb(x)
#define gfm_d(x)   fd(x)
#define gfm_e(x)   fe(x)


#define block_copy_nn(d, s, l)    memcpy(d, s, l)
#define block_copy(d, s)          memcpy(d, s, N_BLOCK)

static void xor_block(void *d, const void *s) {
	((uint8_t*)d)[ 0] ^= ((uint8_t*)s)[ 0];
	((uint8_t*)d)[ 1] ^= ((uint8_t*)s)[ 1];
	((uint8_t*)d)[ 2] ^= ((uint8_t*)s)[ 2];
	((uint8_t*)d)[ 3] ^= ((uint8_t*)s)[ 3];
	((uint8_t*)d)[ 4] ^= ((uint8_t*)s)[ 4];
	((uint8_t*)d)[ 5] ^= ((uint8_t*)s)[ 5];
	((uint8_t*)d)[ 6] ^= ((uint8_t*)s)[ 6];
	((uint8_t*)d)[ 7] ^= ((uint8_t*)s)[ 7];
	((uint8_t*)d)[ 8] ^= ((uint8_t*)s)[ 8];
	((uint8_t*)d)[ 9] ^= ((uint8_t*)s)[ 9];
	((uint8_t*)d)[10] ^= ((uint8_t*)s)[10];
	((uint8_t*)d)[11] ^= ((uint8_t*)s)[11];
	((uint8_t*)d)[12] ^= ((uint8_t*)s)[12];
	((uint8_t*)d)[13] ^= ((uint8_t*)s)[13];
	((uint8_t*)d)[14] ^= ((uint8_t*)s)[14];
	((uint8_t*)d)[15] ^= ((uint8_t*)s)[15];
}

static void copy_and_key(void *d, const void *s, const void *k) {
	// copy & xor
	((uint8_t*)d)[ 0] = ((uint8_t*)s)[ 0] ^ ((uint8_t*)k)[ 0];
	((uint8_t*)d)[ 1] = ((uint8_t*)s)[ 1] ^ ((uint8_t*)k)[ 1];
	((uint8_t*)d)[ 2] = ((uint8_t*)s)[ 2] ^ ((uint8_t*)k)[ 2];
	((uint8_t*)d)[ 3] = ((uint8_t*)s)[ 3] ^ ((uint8_t*)k)[ 3];
	((uint8_t*)d)[ 4] = ((uint8_t*)s)[ 4] ^ ((uint8_t*)k)[ 4];
	((uint8_t*)d)[ 5] = ((uint8_t*)s)[ 5] ^ ((uint8_t*)k)[ 5];
	((uint8_t*)d)[ 6] = ((uint8_t*)s)[ 6] ^ ((uint8_t*)k)[ 6];
	((uint8_t*)d)[ 7] = ((uint8_t*)s)[ 7] ^ ((uint8_t*)k)[ 7];
	((uint8_t*)d)[ 8] = ((uint8_t*)s)[ 8] ^ ((uint8_t*)k)[ 8];
	((uint8_t*)d)[ 9] = ((uint8_t*)s)[ 9] ^ ((uint8_t*)k)[ 9];
	((uint8_t*)d)[10] = ((uint8_t*)s)[10] ^ ((uint8_t*)k)[10];
	((uint8_t*)d)[11] = ((uint8_t*)s)[11] ^ ((uint8_t*)k)[11];
	((uint8_t*)d)[12] = ((uint8_t*)s)[12] ^ ((uint8_t*)k)[12];
	((uint8_t*)d)[13] = ((uint8_t*)s)[13] ^ ((uint8_t*)k)[13];
	((uint8_t*)d)[14] = ((uint8_t*)s)[14] ^ ((uint8_t*)k)[14];
	((uint8_t*)d)[15] = ((uint8_t*)s)[15] ^ ((uint8_t*)k)[15];
}

static void add_round_key(uint8_t d[N_BLOCK], const uint8_t k[N_BLOCK]) {
	xor_block(d, k);
}

static void shift_sub_rows(uint8_t st[N_BLOCK]) {
	uint8_t tt;

	st[ 0] = s_box(st[ 0]); st[ 4] = s_box(st[ 4]);
	st[ 8] = s_box(st[ 8]); st[12] = s_box(st[12]);

	tt = st[1]; st[ 1] = s_box(st[ 5]); st[ 5] = s_box(st[ 9]);
	st[ 9] = s_box(st[13]); st[13] = s_box(tt);

	tt = st[2]; st[ 2] = s_box(st[10]); st[10] = s_box(tt);
	tt = st[6]; st[ 6] = s_box(st[14]); st[14] = s_box(tt);

	tt = st[15]; st[15] = s_box(st[11]); st[11] = s_box(st[ 7]);
	st[ 7] = s_box(st[ 3]); st[ 3] = s_box(tt);
}

#if defined(VERSION_1)
  static void mix_sub_columns(uint8_t dt[N_BLOCK]) {
	uint8_t st[N_BLOCK];
	block_copy(st, dt);
#else
  static void mix_sub_columns(uint8_t dt[N_BLOCK], uint8_t st[N_BLOCK]) {
#endif
	dt[ 0] = gfm2_sb(st[0]) ^ gfm3_sb(st[5]) ^ s_box(st[10]) ^ s_box(st[15]);
	dt[ 1] = s_box(st[0]) ^ gfm2_sb(st[5]) ^ gfm3_sb(st[10]) ^ s_box(st[15]);
	dt[ 2] = s_box(st[0]) ^ s_box(st[5]) ^ gfm2_sb(st[10]) ^ gfm3_sb(st[15]);
	dt[ 3] = gfm3_sb(st[0]) ^ s_box(st[5]) ^ s_box(st[10]) ^ gfm2_sb(st[15]);

	dt[ 4] = gfm2_sb(st[4]) ^ gfm3_sb(st[9]) ^ s_box(st[14]) ^ s_box(st[3]);
	dt[ 5] = s_box(st[4]) ^ gfm2_sb(st[9]) ^ gfm3_sb(st[14]) ^ s_box(st[3]);
	dt[ 6] = s_box(st[4]) ^ s_box(st[9]) ^ gfm2_sb(st[14]) ^ gfm3_sb(st[3]);
	dt[ 7] = gfm3_sb(st[4]) ^ s_box(st[9]) ^ s_box(st[14]) ^ gfm2_sb(st[3]);

	dt[ 8] = gfm2_sb(st[8]) ^ gfm3_sb(st[13]) ^ s_box(st[2]) ^ s_box(st[7]);
	dt[ 9] = s_box(st[8]) ^ gfm2_sb(st[13]) ^ gfm3_sb(st[2]) ^ s_box(st[7]);
	dt[10] = s_box(st[8]) ^ s_box(st[13]) ^ gfm2_sb(st[2]) ^ gfm3_sb(st[7]);
	dt[11] = gfm3_sb(st[8]) ^ s_box(st[13]) ^ s_box(st[2]) ^ gfm2_sb(st[7]);

	dt[12] = gfm2_sb(st[12]) ^ gfm3_sb(st[1]) ^ s_box(st[6]) ^ s_box(st[11]);
	dt[13] = s_box(st[12]) ^ gfm2_sb(st[1]) ^ gfm3_sb(st[6]) ^ s_box(st[11]);
	dt[14] = s_box(st[12]) ^ s_box(st[1]) ^ gfm2_sb(st[6]) ^ gfm3_sb(st[11]);
	dt[15] = gfm3_sb(st[12]) ^ s_box(st[1]) ^ s_box(st[6]) ^ gfm2_sb(st[11]);
  }

/*  Set the cipher key for the pre-keyed version */
uint8_t aes_set_key(const unsigned char *key, uint8_t keylen, aes_context *context) {
	uint8_t cc, rc, hi;

	switch(keylen) {
		case 16:
		case 24:
		case 32:
			break;
		default:
			context->rnd = 0;
			return -1;
	}
	block_copy_nn(context->ksch, key, keylen);
	hi = (keylen + 28) << 2;
	context->rnd = (hi >> 4) - 1;
	for (cc = keylen, rc = 1; cc < hi; cc += 4) {
		uint8_t tt, t0, t1, t2, t3;

		t0 = context->ksch[cc - 4];
		t1 = context->ksch[cc - 3];
		t2 = context->ksch[cc - 2];
		t3 = context->ksch[cc - 1];
		if (cc % keylen == 0) {
			tt = t0;
			t0 = s_box(t1) ^ rc;
			t1 = s_box(t2);
			t2 = s_box(t3);
			t3 = s_box(tt);
			rc = f2(rc);
		} else if (keylen > 24 && cc % keylen == 16) {
			t0 = s_box(t0);
			t1 = s_box(t1);
			t2 = s_box(t2);
			t3 = s_box(t3);
		}
		tt = cc - keylen;
		context->ksch[cc + 0] = context->ksch[tt + 0] ^ t0;
		context->ksch[cc + 1] = context->ksch[tt + 1] ^ t1;
		context->ksch[cc + 2] = context->ksch[tt + 2] ^ t2;
		context->ksch[cc + 3] = context->ksch[tt + 3] ^ t3;
	}
	return 0;
}

/*  Encrypt a single block of 16 bytes */
uint8_t aes_encrypt(const unsigned char in[N_BLOCK], unsigned char out[N_BLOCK], const aes_context *context) {
	if (context->rnd) {
		uint8_t s1[N_BLOCK], r;
		copy_and_key(s1, in, context->ksch);

		for (r = 1 ; r < context->rnd ; ++r)
#if defined(VERSION_1)
		{
			mix_sub_columns(s1);
			add_round_key(s1, context->ksch + r * N_BLOCK);
		}
#else
		{   uint8_t s2[N_BLOCK];
			mix_sub_columns(s2, s1);
			copy_and_key(s1, s2, context->ksch + r * N_BLOCK);
		}
#endif
		shift_sub_rows(s1);
		copy_and_key(out, s1, context->ksch + r * N_BLOCK);
	}
	else
		return -1;
	return 0;
}
