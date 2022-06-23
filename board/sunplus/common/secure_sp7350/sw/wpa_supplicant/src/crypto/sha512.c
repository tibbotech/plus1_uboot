/*
 * SHA-512 hash implementation and interface functions
 * Copyright (c) 2003-2018, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#if 0
#include "includes.h"

#include "common.h"
#include "sha512.h"
#include "crypto.h"
#endif

#include "wpasup_boot_port.h"
#ifdef CONFIG_USE_SHA2
//#include "../../../ed25519/sha512.h" // sha512_init
#include "../../../libeddsa/lib/sha512.h"
#else
#include "../../../linux/crypto/sha3.h" // sha3_512_init
#endif

/**
 * sha512_vector - SHA512 hash for data vector
 * @num_elem: Number of elements in the data vector
 * @addr: Pointers to the data areas
 * @len: Lengths of the data blocks
 * @mac: Buffer for the hash
 * Returns: 0 on success, -1 of failure
 */
static int shaX_512_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
        size_t i;
#ifdef CONFIG_USE_SHA2
        struct sha512 ctx;

        sha512_init(&ctx);
	for (i = 0; i < num_elem; i++) {
		if (!addr[i] || !len[i])
			continue;
		sha512_add(&ctx, addr[i], len[i]);
	}
        sha512_final(&ctx, mac);
#else
	struct sha3_state ctx;
        sha3_512_init(&ctx);
	for (i = 0; i < num_elem; i++) {
		if (!addr[i] || !len[i])
			continue;
		if (sha3_update(&ctx, addr[i], len[i]))
			return -1;
	}
        if (sha3_final(&ctx, mac))
                return -1;
#endif
        return 0;
}


/**
 * hmac_sha512_vector - HMAC-SHA512 over data vector (RFC 2104)
 * @key: Key for HMAC operations
 * @key_len: Length of the key in bytes
 * @num_elem: Number of elements in the data vector
 * @addr: Pointers to the data areas
 * @len: Lengths of the data blocks
 * @mac: Buffer for the hash (64 bytes)
 * Returns: 0 on success, -1 on failure
 */
int hmac_shaX_512_vector(const u8 *key, size_t key_len, size_t num_elem,
		       const u8 *addr[], const size_t *len, u8 *mac)
{
	__ALIGN4
	unsigned char k_pad[128]; /* padding - key XORd with ipad/opad */
	__ALIGN4
	unsigned char tk[64];
	__ALIGN4
	const u8 *_addr[6];
	__ALIGN4
	size_t _len[6], i;

	if (num_elem > 5) {
		/*
		 * Fixed limit on the number of fragments to avoid having to
		 * allocate memory (which could fail).
		 */
		return -1;
	}

	/* if key is longer than 128 bytes reset it to key = SHA512(key) */
	if (key_len > 128) {
		if (shaX_512_vector(1, &key, &key_len, tk) < 0)
			return -1;
		key = tk;
		key_len = 64;
	}

	/* the HMAC_SHA512 transform looks like:
	 *
	 * SHA512(K XOR opad, SHA512(K XOR ipad, text))
	 *
	 * where K is an n byte key
	 * ipad is the byte 0x36 repeated 128 times
	 * opad is the byte 0x5c repeated 128 times
	 * and text is the data being protected */

	/* start out by storing key in ipad */
	os_memset(k_pad, 0, sizeof(k_pad));
	os_memcpy(k_pad, key, key_len);
	/* XOR key with ipad values */
	for (i = 0; i < 128; i++)
		k_pad[i] ^= 0x36;

	/* perform inner SHA512 */
	_addr[0] = k_pad;
	_len[0] = 128;
	for (i = 0; i < num_elem; i++) {
		_addr[i + 1] = addr[i];
		_len[i + 1] = len[i];
	}
	if (shaX_512_vector(1 + num_elem, _addr, _len, mac) < 0)
		return -1;

	os_memset(k_pad, 0, sizeof(k_pad));
	os_memcpy(k_pad, key, key_len);
	/* XOR key with opad values */
	for (i = 0; i < 128; i++)
		k_pad[i] ^= 0x5c;

	/* perform outer SHA512 */
	_addr[0] = k_pad;
	_len[0] = 128;
	_addr[1] = mac;
	_len[1] = SHA512_MAC_LEN;
	return shaX_512_vector(2, _addr, _len, mac);
}


/**
 * hmac_sha512 - HMAC-SHA512 over data buffer (RFC 2104)
 * @key: Key for HMAC operations
 * @key_len: Length of the key in bytes
 * @data: Pointers to the data area
 * @data_len: Length of the data area
 * @mac: Buffer for the hash (64 bytes)
 * Returns: 0 on success, -1 on failure
 */
int hmac_shaX_512(const u8 *key, size_t key_len, const u8 *data,
		size_t data_len, u8 *mac)
{
	return hmac_shaX_512_vector(key, key_len, 1, &data, &data_len, mac);
}
