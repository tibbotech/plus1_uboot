#ifndef _AES_INC_
#define _AES_INC_

#include <common.h>

#define AES_BLOCK_SIZE          16

#define AES_KEYSIZE_128         16
#define AES_KEYSIZE_192         24
#define AES_KEYSIZE_256         32

#define AES_MAXNR 14 /* be fixed for asm code access */

struct AES_KEY {
        __ALIGN8
        unsigned int rd_key[4 * (AES_MAXNR + 1)]; // 4 * 15 * 4 = 240
        int rounds;
};

static inline int aes_num_rounds(int key_bytes)
{
	/*
	 * # of rounds specified by AES:
	 * 128 bit key          10 rounds
	 * 192 bit key          12 rounds
	 * 256 bit key          14 rounds
	 * => n byte key        => 6 + (n/4) rounds
	 */
	return 6 + key_bytes / 4;
}

void AES_encrypt(const u8 *in, u8 *out, struct AES_KEY *ctx);
void AES_decrypt(const u8 *in, u8 *out, struct AES_KEY *ctx);

int private_AES_set_encrypt_key(const unsigned char *userKey, const int bits, struct AES_KEY *key);
int private_AES_set_decrypt_key(const unsigned char *userKey, const int bits, struct AES_KEY *key);

int aes_gcm_ae(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
		const u8 *plain, size_t plain_len,
		const u8 *aad, size_t aad_len, u8 *crypt, u8 *tag);
int aes_gcm_ad(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
		const u8 *crypt, size_t crypt_len,
		const u8 *aad, size_t aad_len, const u8 *tag, u8 *plain);

#endif
