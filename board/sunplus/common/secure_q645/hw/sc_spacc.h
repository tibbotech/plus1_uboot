#ifndef __SC_SPACC_INC_H__
#define __SC_SPACC_INC_H__

#include <config.h>

int spacc_shaX_512(const unsigned char *message, size_t message_len, unsigned char out[64]);
int spacc_hkdf_shaX_512(const unsigned char *salt, int salt_len, const unsigned char *ikm, int ikm_len,
		const unsigned char *info, int info_len, uint8_t okm[], int okm_len);

int spacc_ed25519_hash(const unsigned char *signature, const unsigned char *message, size_t message_len,
		const unsigned char *public_key, unsigned char h_val[64]);



int spacc_aes_gcm_ad(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
		const u8 *crypt, size_t crypt_len,
		const u8 *aad, size_t aad_len, const u8 *tag, u8 *plain);

#endif /* __SC_SPACC_INC_H__ */
