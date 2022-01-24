/*
 * Common values for aes-gcm algorithms
 */
#ifndef __CRYPTO_SHA3_H__
#define __CRYPTO_SHA3_H__

extern AES_encrypt(_in, _out, _key);

int aes_gcm_ad(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
	       const u8 *crypt, size_t crypt_len,
	       const u8 *aad, size_t aad_len, const u8 *tag, u8 *plain);
int aes_gcm_ae(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
	       const u8 *plain, size_t plain_len,
	       const u8 *aad, size_t aad_len, u8 *crypt, u8 *tag);
static void aes_gcm_gctr(void *aes, const u8 *J0, const u8 *in, size_t len,
			 u8 *out);
static void aes_gcm_ghash(const u8 *H, const u8 *aad, size_t aad_len,
			  const u8 *crypt, size_t crypt_len, u8 *S);
static void aes_gcm_prepare_j0(const u8 *iv, size_t iv_len, const u8 *H, u8 *J0);
static void aes_gctr(void *aes, const u8 *icb, const u8 *x, size_t xlen, u8 *y);
int aes_gmac(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
	     const u8 *aad, size_t aad_len, u8 *tag);

#endif