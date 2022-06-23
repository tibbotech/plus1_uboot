/*
 * Common values for SHA-3 algorithms
 */
#ifndef __CRYPTO_SHA3_H__
#define __CRYPTO_SHA3_H__

#define SHA3_224_DIGEST_SIZE	(224 / 8)
#define SHA3_224_BLOCK_SIZE	(200 - 2 * SHA3_224_DIGEST_SIZE)

#define SHA3_256_DIGEST_SIZE	(256 / 8)
#define SHA3_256_BLOCK_SIZE	(200 - 2 * SHA3_256_DIGEST_SIZE)

#define SHA3_384_DIGEST_SIZE	(384 / 8)
#define SHA3_384_BLOCK_SIZE	(200 - 2 * SHA3_384_DIGEST_SIZE)

#define SHA3_512_DIGEST_SIZE	(512 / 8)
#define SHA3_512_BLOCK_SIZE	(200 - 2 * SHA3_512_DIGEST_SIZE)

struct sha3_state {
	__attribute__((aligned(8)))
	uint64_t	st[25];
	unsigned int	md_len;
	unsigned int	rsiz;
	unsigned int	rsizw;

	unsigned int	partial;

	__attribute__((aligned(8)))
	uint8_t		buf[SHA3_224_BLOCK_SIZE];
};


int sha3_256_init(struct sha3_state *sctx);
int sha3_512_init(struct sha3_state *sctx);
int sha3_update(struct sha3_state *sctx, const uint8_t *data, unsigned int len);
int sha3_final(struct sha3_state *sctx, uint8_t *out);

int sha3_256(const uint8_t *data, unsigned int len, uint8_t *out);
int sha3_512(const uint8_t *data, unsigned int len, uint8_t *out);

#endif
