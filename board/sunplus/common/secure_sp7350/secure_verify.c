#include <common.h>
#include <image.h>

#if (COMPILE_WITH_SECURE == 1)
#include <cpu_func.h>
#include "crypto_drv.h"

/***********************************
|---------------------------|
|        kernel data        |
|---------------------------|
|---------------------------|
|      sig data(256byte)    |
|---------------------------|
***********************************/

void prn_dump_buffer(unsigned char *buf, int len)
{
	static const char s[] = "       |       \n";
	u32 j = (uintptr_t)buf;
	int i;

	printf("\n%08x: ", j);
	i = j & 0x0f;
	while (i--)
		printf("   ");

	for (i = 0; i < len; i++, j++) {
		if (i && !(j & 0x0f))
			printf("%08x: ", j);
		printf("%02x%c",buf[i], s[j & 0x0f]);
	}
	puts("\n");
}

#define  prn_dump(s, buf, size) \
do { \
	printf("\n%s:", s); \
	prn_dump_buffer(buf, size); \
} while (0)

static void reverse_buf(u8 *buf, u32 len)
{
	int i;
	char temp;

	for (i = 0; i < len/2; ++i) {
		temp = buf[i];
		buf[i] = buf[len - 1 - i];
		buf[len - 1 - i] = temp;
	}
}

#define MIN(a, b)	((a) < (b) ? (a) : (b))

#define RSA_KEY_BITS	(2048)
#define RSA_KEY_SZ	(RSA_KEY_BITS / 8)

#define HASH_BUF_SZ	(200)

#if 0
#define HASH_MODE	M_SHA3_256
#define HASH_SZ		(32)
#else
#define HASH_MODE	M_SHA3_512
#define HASH_SZ		(64)
#endif
#define BLOCK_SZ	(HASH_BUF_SZ - (HASH_SZ << 1))
#define BATCH_SZ	(0xffff / (BLOCK_SZ * 4) * (BLOCK_SZ * 4))

static volatile struct sp_crypto_reg *reg = (void *)SP_CRYPTO_REG_BASE;
static u8 rsakey_E[RSA_KEY_SZ] = { 0x01, 0x00, 0x01 };
static u8 rsakey_N[RSA_KEY_SZ] = {
#include "rsakey_N.h"
};

static u8 dst[RSA_KEY_SZ];
static u8 p2[RSA_KEY_SZ];

static u8 *sp_hash(u8 *data, u32 len)
{
	u8 *buf = (u8 *)ALIGN((uintptr_t)p2, 32);
	u32 padding;

	//printf("HASH_MODE: %08x  %d  %d  %03x\n", HASH_MODE, HASH_SZ, BLOCK_SZ, BATCH_SZ);
	memset(buf, 0, HASH_BUF_SZ);

	/* last block, padding */
	padding = BLOCK_SZ - (len % BLOCK_SZ);
	memset(data + len, 0, padding);
	data[len] = 0x06;
	len += padding;
	data[len - 1] |= 0x80;
	//prn_dump_buffer(data + len - (BLOCK_SZ << 1), BLOCK_SZ << 1);

	flush_dcache_all();

	reg->HASHDPTR = (uintptr_t)buf;
	reg->HASHPAR0 = HASH_MODE;
	reg->HASHPAR1 = (uintptr_t)buf;

	while (len) {
		u32 bs = MIN(len, BATCH_SZ);

		reg->HASHSPTR = (uintptr_t)data;
		reg->HASHDMACS = (bs << 16) | 1;

		while (!(reg->SECIF & HASH_DMA_IF));
		reg->SECIF = HASH_DMA_IF; // clear hash dma finish flag

		//prn_dump_buffer(buf, HASH_SZ);
		data += bs;
		len -= bs;
	}

	return buf;
}

static long long mont_w(unsigned char *mod)
{
	long long t = 1;
	long long mode;
	int i;

	memcpy(&mode, mod, sizeof(mode));
	for (i = 1; i < 63; i++) {
		t = (t * t * mode);
	}

	return (-t);
}

static int sp_expmod(u8 *dst, u8 *src, u8 *e, u8 *n, u32 size)
{
	reg->RSADPTR  = (uintptr_t)dst;
	reg->RSASPTR  = (uintptr_t)src;
	reg->RSAYPTR  = (uintptr_t)e;
	reg->RSAP2PTR = (uintptr_t)p2;

	if (reg->RSANPTR != (uintptr_t)n) {
		long long w = mont_w(n);
		reg->RSANPTR  = (uintptr_t)n;
		reg->RSAWPTRL = w;
		reg->RSAWPTRH = w >> 32;
		reg->RSAPAR0  = RSA_SET_PARA_D(RSA_KEY_BITS) | RSA_PARA_PRECAL_P2;
	} else {
		reg->RSAPAR0  = RSA_SET_PARA_D(RSA_KEY_BITS) | RSA_PARA_FETCH_P2;
	}

	flush_dcache_all();
	reg->RSADMACS = RSA_DMA_SIZE(size) | RSA_DMA_ENABLE;

	while (!(reg->SECIF & RSA_DMA_IF));
	reg->SECIF = RSA_DMA_IF; // clear rsa dma finish flag

	return 0;
}

#define HEADER_SZ	(sizeof(struct image_header))

static int do_verify(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	u8 *k = (u8 *)simple_strtoul(argv[1], NULL, 0); // uImage start addr
	u32 imgsize = HEADER_SZ + image_get_data_size((image_header_t  *)k); // uImage size, without sign
	u8 *src = k + imgsize;
	ulong t0, t1;  /* unit:ms */
	int ret;

	//prn_dump("uImage header", k, 0x50);
	prn_dump("sign (encrypted hash)", src, RSA_KEY_SZ);

	t0 = get_timer(0);

	/* public key decrypt */
	reverse_buf(rsakey_N, RSA_KEY_SZ);
	reverse_buf(src, RSA_KEY_SZ);
	ret = sp_expmod(dst, src, rsakey_E, rsakey_N, RSA_KEY_SZ);
	reverse_buf(dst, HASH_SZ);

	/* hash uImage */
	src = sp_hash(k, imgsize);

 	/* verify sign */
	ret = memcmp(dst, src, HASH_SZ);

	t1 = get_timer(t0);

	prn_dump("decrypted hash", dst, HASH_SZ);
	prn_dump("hash", src, HASH_SZ);
	printf("Verify signature (%lu ms): %d\n", t1, ret);

	if (ret) while (1); // verify fail, stop boot kernel
	return ret;
}
#else
static int do_verify(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	return 0;
}
#endif

U_BOOT_CMD(
	verify, 3, 1, do_verify,
	"verify command",
	"verify kernel signature.\n"
	"\taddr: kernel addr, include uImage header.\n"
	"\tverify 0x307fc0\n"
);
