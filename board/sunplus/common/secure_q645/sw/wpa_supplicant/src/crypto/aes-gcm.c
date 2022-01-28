#define PORT_IN_BOOT

/*
 * Galois/Counter Mode (GCM) and GMAC with AES
 *
 * Copyright (c) 2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef PORT_IN_BOOT
#include "includes.h"

#include "common.h"
#include "aes.h"
#include "aes_wrap.h"
#endif

/* support in-place aes gcm */
#define SUPPORT_IN_PLACE

#ifdef PORT_IN_BOOT
#include <common.h>
#include <aes.h>
//#include <arm/lib/utils_def.h>

#define os_memset memset
#define os_memcpy memcpy
#define os_memcmp_const(_s1,_s2,_num) memcmp(_s1, _s2, _num)
#define wpa_printf(_type,_msg) printf(_msg)
#define wpa_hexdump_key(...)
#define aes_encrypt(_key,_in,_out) AES_encrypt(_in, _out, _key)
#define aes_encrypt_deinit(...)

static inline u32 WPA_GET_BE32(const u8 *a)
{
        return ((u32) a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3];
}
static inline void WPA_PUT_BE32(u8 *a, u32 val)
{
        a[0] = (val >> 24) & 0xff;
        a[1] = (val >> 16) & 0xff;
        a[2] = (val >> 8) & 0xff;
        a[3] = val & 0xff;
}
static inline void WPA_PUT_BE64(u8 *a, u64 val)
{
        a[0] = val >> 56;
        a[1] = val >> 48;
        a[2] = val >> 40;
        a[3] = val >> 32;
        a[4] = val >> 24;
        a[5] = val >> 16;
        a[6] = val >> 8;
        a[7] = val & 0xff;
}
#endif

static void inc32(u8 *block)
{
	u32 val;
	val = WPA_GET_BE32(block + AES_BLOCK_SIZE - 4);
	val++;
	WPA_PUT_BE32(block + AES_BLOCK_SIZE - 4, val);
}


static void xor_block(u8 *dst, const u8 *src)
{
	u32 *d = (u32 *) dst;
	u32 *s = (u32 *) src;

#ifdef SUPPORT_IN_PLACE
//	if (((u32)dst & 3) || ((u32)src & 3)) {
//		prn_string("unaligned:\n");
//		prn_dword((u32)dst);
//		prn_dword((u32)src);
//	}
#endif

	*d++ ^= *s++;
	*d++ ^= *s++;
	*d++ ^= *s++;
	*d++ ^= *s++;
}


static void shift_right_block(u8 *v)
{
	u32 val;

	val = WPA_GET_BE32(v + 12);
	val >>= 1;
	if (v[11] & 0x01)
		val |= 0x80000000;
	WPA_PUT_BE32(v + 12, val);

	val = WPA_GET_BE32(v + 8);
	val >>= 1;
	if (v[7] & 0x01)
		val |= 0x80000000;
	WPA_PUT_BE32(v + 8, val);

	val = WPA_GET_BE32(v + 4);
	val >>= 1;
	if (v[3] & 0x01)
		val |= 0x80000000;
	WPA_PUT_BE32(v + 4, val);

	val = WPA_GET_BE32(v);
	val >>= 1;
	WPA_PUT_BE32(v, val);
}


/* Multiplication in GF(2^128) */
static void gf_mult(const u8 *x, const u8 *y, u8 *z)
{
	__ALIGN4
	u8 v[16];
	int i, j;

	os_memset(z, 0, 16); /* Z_0 = 0^128 */
	os_memcpy(v, y, 16); /* V_0 = Y */

	for (i = 0; i < 16; i++) {
		for (j = 0; j < 8; j++) {
			if (x[i] & BIT(7 - j)) {
				/* Z_(i + 1) = Z_i XOR V_i */
				xor_block(z, v);
			} else {
				/* Z_(i + 1) = Z_i */
			}

			if (v[15] & 0x01) {
				/* V_(i + 1) = (V_i >> 1) XOR R */
				shift_right_block(v);
				/* R = 11100001 || 0^120 */
				v[0] ^= 0xe1;
			} else {
				/* V_(i + 1) = V_i >> 1 */
				shift_right_block(v);
			}
		}
	}
}


static void ghash_start(u8 *y)
{
	/* Y_0 = 0^128 */
	os_memset(y, 0, 16);
}


static void ghash(const u8 *h, const u8 *x, size_t xlen, u8 *y)
{
	size_t m, i;
	const u8 *xpos = x;
	__ALIGN4
	u8 tmp[16];

	m = xlen / 16;

	for (i = 0; i < m; i++) {
		/* Y_i = (Y^(i-1) XOR X_i) dot H */
		xor_block(y, xpos);
		xpos += 16;

		/* dot operation:
		 * multiplication operation for binary Galois (finite) field of
		 * 2^128 elements */
		gf_mult(y, h, tmp);
		os_memcpy(y, tmp, 16);
	}

	if (x + xlen > xpos) {
		/* Add zero padded last block */
		size_t last = x + xlen - xpos;
		os_memcpy(tmp, xpos, last);
		os_memset(tmp + last, 0, sizeof(tmp) - last);

		/* Y_i = (Y^(i-1) XOR X_i) dot H */
		xor_block(y, tmp);

		/* dot operation:
		 * multiplication operation for binary Galois (finite) field of
		 * 2^128 elements */
		gf_mult(y, h, tmp);
		os_memcpy(y, tmp, 16);
	}

	/* Return Y_m */
}


static void aes_gctr(void *aes, const u8 *icb, const u8 *x, size_t xlen, u8 *y)
{
	size_t i, n, last;
	__ALIGN4
	u8 cb[AES_BLOCK_SIZE], tmp[AES_BLOCK_SIZE];
	const u8 *xpos = x;
	u8 *ypos = y;

	if (xlen == 0)
		return;

	n = xlen / 16;

	os_memcpy(cb, icb, AES_BLOCK_SIZE);
	/* Full blocks */
	for (i = 0; i < n; i++) {
#ifdef SUPPORT_IN_PLACE
		if (x == y)  {
			aes_encrypt(aes, cb, tmp);
			xor_block(ypos, tmp);
		} else {
			aes_encrypt(aes, cb, ypos);
			xor_block(ypos, xpos);
		}
#else
		aes_encrypt(aes, cb, ypos);
		xor_block(ypos, xpos);
#endif
		xpos += AES_BLOCK_SIZE;
		ypos += AES_BLOCK_SIZE;
		inc32(cb);
	}

	last = x + xlen - xpos;
	if (last) {
		/* Last, partial block */
		aes_encrypt(aes, cb, tmp);
		for (i = 0; i < last; i++)
			*ypos++ = *xpos++ ^ tmp[i];
	}
}


#ifdef PORT_IN_BOOT
static void aes_gcm_init_hash_subkey(struct AES_KEY *aes_key, size_t key_len, u8 *H)
{
	/* Generate hash subkey H = AES_K(0^128) */
	memset(H, 0, AES_BLOCK_SIZE);
	aes_encrypt(aes_key, H, H);
}
#else
static void * aes_gcm_init_hash_subkey(const u8 *key, size_t key_len, u8 *H)
{
	void *aes;

	aes = aes_encrypt_init(key, key_len);
	if (aes == NULL)
		return NULL;

	/* Generate hash subkey H = AES_K(0^128) */
	os_memset(H, 0, AES_BLOCK_SIZE);
	aes_encrypt(aes, H, H);
	wpa_hexdump_key(MSG_EXCESSIVE, "Hash subkey H for GHASH",
			H, AES_BLOCK_SIZE);
	return aes;
}
#endif

static void aes_gcm_prepare_j0(const u8 *iv, size_t iv_len, const u8 *H, u8 *J0)
{
	__ALIGN4
	u8 len_buf[16];

	if (iv_len == 12) {
		/* Prepare block J_0 = IV || 0^31 || 1 [len(IV) = 96] */
		os_memcpy(J0, iv, iv_len);
		os_memset(J0 + iv_len, 0, AES_BLOCK_SIZE - iv_len);
		J0[AES_BLOCK_SIZE - 1] = 0x01;
	} else {
		/*
		 * s = 128 * ceil(len(IV)/128) - len(IV)
		 * J_0 = GHASH_H(IV || 0^(s+64) || [len(IV)]_64)
		 */
		ghash_start(J0);
		ghash(H, iv, iv_len, J0);
		WPA_PUT_BE64(len_buf, 0);
		WPA_PUT_BE64(len_buf + 8, iv_len * 8);
		ghash(H, len_buf, sizeof(len_buf), J0);
	}
}


static void aes_gcm_gctr(void *aes, const u8 *J0, const u8 *in, size_t len,
			 u8 *out)
{
	__ALIGN4
	u8 J0inc[AES_BLOCK_SIZE];

	if (len == 0)
		return;

	os_memcpy(J0inc, J0, AES_BLOCK_SIZE);
	inc32(J0inc);
	aes_gctr(aes, J0inc, in, len, out);
}


static void aes_gcm_ghash(const u8 *H, const u8 *aad, size_t aad_len,
			  const u8 *crypt, size_t crypt_len, u8 *S)
{
	__ALIGN4
	u8 len_buf[16];

	/*
	 * u = 128 * ceil[len(C)/128] - len(C)
	 * v = 128 * ceil[len(A)/128] - len(A)
	 * S = GHASH_H(A || 0^v || C || 0^u || [len(A)]64 || [len(C)]64)
	 * (i.e., zero padded to block size A || C and lengths of each in bits)
	 */
	ghash_start(S);
	ghash(H, aad, aad_len, S);
	ghash(H, crypt, crypt_len, S);
	WPA_PUT_BE64(len_buf, aad_len * 8);
	WPA_PUT_BE64(len_buf + 8, crypt_len * 8);
	ghash(H, len_buf, sizeof(len_buf), S);

	wpa_hexdump_key(MSG_EXCESSIVE, "S = GHASH_H(...)", S, 16);
}


/**
 * aes_gcm_ae - GCM-AE_K(IV, P, A)
 */
int aes_gcm_ae(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
	       const u8 *plain, size_t plain_len,
	       const u8 *aad, size_t aad_len, u8 *crypt, u8 *tag)
{
	__ALIGN4
	u8 H[AES_BLOCK_SIZE];
	__ALIGN4
	u8 J0[AES_BLOCK_SIZE];
	__ALIGN4
	u8 S[16];
	void *aes;
#ifdef PORT_IN_BOOT
	struct AES_KEY aes_key;
	aes = (void *)&aes_key;

        private_AES_set_encrypt_key(key, key_len * 8, &aes_key);
        aes_key.rounds = aes_num_rounds(key_len);
	aes_gcm_init_hash_subkey(&aes_key, key_len, H);
#else
	aes = aes_gcm_init_hash_subkey(key, key_len, H);
	if (aes == NULL)
		return -1;
#endif

	aes_gcm_prepare_j0(iv, iv_len, H, J0);

	/* C = GCTR_K(inc_32(J_0), P) */
	aes_gcm_gctr(aes, J0, plain, plain_len, crypt);

	aes_gcm_ghash(H, aad, aad_len, crypt, plain_len, S);

	/* T = MSB_t(GCTR_K(J_0, S)) */
	aes_gctr(aes, J0, S, sizeof(S), tag);

	/* Return (C, T) */

	aes_encrypt_deinit(aes);

	return 0;
}


/**
 * aes_gcm_ad - GCM-AD_K(IV, C, A, T)
 */
int aes_gcm_ad(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
	       const u8 *crypt, size_t crypt_len,
	       const u8 *aad, size_t aad_len, const u8 *tag, u8 *plain)
{
	__ALIGN4
	u8 H[AES_BLOCK_SIZE];
	__ALIGN4
	u8 J0[AES_BLOCK_SIZE];
	__ALIGN4
	u8 S[16], T[16];
	void *aes;
#ifdef PORT_IN_BOOT
	struct AES_KEY aes_key;
	aes = (void *)&aes_key;

//	if ((u32)iv & 3) {
//		prn_string("ad: err, unalign iv@");
//		prn_dword((u32)iv);
//		return -1;
//	}

        private_AES_set_encrypt_key(key, key_len * 8, &aes_key);
        aes_key.rounds = aes_num_rounds(key_len);
	aes_gcm_init_hash_subkey(&aes_key, key_len, H);
#else
	aes = aes_gcm_init_hash_subkey(key, key_len, H);
	if (aes == NULL)
		return -1;
#endif

	aes_gcm_prepare_j0(iv, iv_len, H, J0);

	/* in place : ghash first */
#ifdef SUPPORT_IN_PLACE
	aes_gcm_ghash(H, aad, aad_len, crypt, crypt_len, S);
#endif

	/* P = GCTR_K(inc_32(J_0), C) */
	aes_gcm_gctr(aes, J0, crypt, crypt_len, plain);

#ifndef SUPPORT_IN_PLACE
	aes_gcm_ghash(H, aad, aad_len, crypt, crypt_len, S);
#endif

	/* T' = MSB_t(GCTR_K(J_0, S)) */
	aes_gctr(aes, J0, S, sizeof(S), T);

	aes_encrypt_deinit(aes);

	if (os_memcmp_const(tag, T, 16) != 0) {
		wpa_printf(MSG_EXCESSIVE, "GCM: Tag mismatch");
//		prn_dump("\ntag e: ", (u8 *)tag, 16);
//		prn_dump("tag d: ", (u8 *)T, 16);
		return -1;
	}

	return 0;
}


int aes_gmac(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
	     const u8 *aad, size_t aad_len, u8 *tag)
{
	return aes_gcm_ae(key, key_len, iv, iv_len, NULL, 0, aad, aad_len, NULL,
			  tag);
}
