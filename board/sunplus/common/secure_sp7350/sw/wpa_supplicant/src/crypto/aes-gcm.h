/*
 * Common values for aes-gcm algorithms
 */
#ifndef __AES_GCM_INC_H__
#define __AES_GCM_INC_H__

int aes_gcm_ad(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
	       const u8 *crypt, size_t crypt_len,
	       const u8 *aad, size_t aad_len, const u8 *tag, u8 *plain);
	     
#endif