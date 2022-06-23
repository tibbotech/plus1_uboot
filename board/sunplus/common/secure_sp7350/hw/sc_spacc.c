#include <common.h>
#include <cpu_func.h>
#include "../secure_config.h"
#include "../spacc/elpspacchw.h"
#include "../spacc/spacc_hw.h"

#ifdef prn_dword
#undef prn_dword
#endif
#define prn_dword(a)  printf("0x%x\n",(a))

struct spacc_ram   g_spacc_ram;


#ifndef SHA512_MAC_LEN
#define SHA512_MAC_LEN 64
#endif

static void warn_if_spacc_fifo_not_clean(void)
{
	if (!(SPACC_REG->fifo_stat & SPACC_FIFO_STAT_STAT_EMPTY)) {
		printf("warn: spacc fifo non-empty!\n");
	}
}

static void spacc_wait_fifo_empty(void)
{
	while (!(SPACC_REG->fifo_stat & SPACC_FIFO_STAT_STAT_EMPTY));
}

static void spacc_wait_fifo_nonempty(void)
{
	while (SPACC_REG->fifo_stat & SPACC_FIFO_STAT_STAT_EMPTY);
}

static void spacc_pop_fifo(int sw_id)
{
	spacc_wait_fifo_nonempty();

	SPACC_REG->stat_pop = 1;

	spacc_wait_fifo_empty();

	/* pdf p281 :
	 * The STAT_POP register must not be written if the FIFO_STAT.STAT_EMPTY flag is asserted. It
	 * takes two clock cycles between a write of the STAT_POP register and valid information in the STATUS
	 * register, so the read of the status register must be delayed by at least one clock cycle after the write of the
	 * STAT_POP register.
	 */

	/* wait until sw_id appears */

	while (SPACC_STATUS_SW_ID_GET(SPACC_REG->status) != sw_id);
}

static int warn_if_spacc_fail(void)
{
	int ret = SPACC_GET_STATUS_RET_CODE(SPACC_REG->status);

	if (ret != SPACC_OK) {
		printf("error: spacc ret_code=%d\n",ret);
	}

	return ret;
}

static int spacc_query_next_sw_id(void)
{
	/*
	 * Software tag of job ID. Will be returned with the packet in the
	 * status FIFO. This value automatically increments on every
	 * push to the command FIFO.
	 */
	return (SPACC_REG->sw_ctrl & 0xff);
}

static void ddt_init(struct boot_ddt *scd)
{
	scd->idx = 0;
	scd->len = 0;
	scd->ddt[0].ptr = 0;
	scd->ddt[0].len = 0;
}

static int pdu_ddt_add(struct boot_ddt *scd, u64 phys, u64 size)
{
	if (scd->idx >= SC_DDT_MAX) {
		printf("err: ddt full in ram\n");
		return -1;
	}

	scd->ddt[scd->idx].ptr = phys;
	scd->ddt[scd->idx].len = size;

	scd->len += size;
	++(scd->idx);

	// ended in a null entry
	scd->ddt[scd->idx].ptr = 0;
	scd->ddt[scd->idx].len = 0;

	return 0;
}

static u64 pdu_io_read32(const void *addr)
{
	const u8 *v = (const u8 *)addr;
	if ((u64)addr & 3) {
		return (v[3] << 24) | (v[2] << 16) | (v[1] << 8) | v[0];
	} else {
		return *(const u64 *)addr;
	}
}

/*
 * Reference source : spacc core/kernel/spacc_dev.c
 *
 *  static int sg_to_ddt(pdu_ddt *ddt, const struct scatterlist *sg)
 *
 * Add an SG entry to DDT, taking care to split entries which straddle
 * H/W addressing boundary.  Note that the addressing boundaries depend
 * on pCLUSTER_ADDR_WIDTH, which unfortunately is not part of the H/W
 * configuration registers.  The following assumes a value of 16 (64 KiB).
 */

static int ddt_add(struct boot_ddt *scd, u64 phys, u64 size)
{
	u64 baseaddr = phys;
	unsigned long totlen = size;
	int rc = 0;
	u64 curlen;

	while (totlen) {
		u64 maxaddr = (baseaddr | SPACC_MAX_MSG_SIZE) + 1; // up to boundary

		curlen = (totlen < (maxaddr - baseaddr)) ? totlen : (maxaddr - baseaddr);

		rc = pdu_ddt_add(scd, baseaddr, curlen);
		if (rc != 0) {
			break;
		}

		baseaddr += curlen;
		totlen -= curlen;
	}

	return rc;
}

// see sha512_vector()
static int spacc_shaX_512_vector(int num_elem, const u8 *addr[], const size_t *len, u8 out[64])
{
	int sw_id, ctrl_hash, i;
	u64 total_len = 0;

#ifdef CONFIG_USE_SHA2
	ctrl_hash = H_SHA512;
#else
	ctrl_hash = H_SHA3_512;
#endif

	if (num_elem > SC_DDT_MAX) {
		printf("err: too much elem=");
		prn_dword(num_elem);
		return -1;
	}

	/* src = message */
	ddt_init(&g_spacc_ram.src);
	for (i = 0; i < num_elem; i++) {
		if (len[i] > 0) {
			ddt_add(&g_spacc_ram.src, (u64)addr[i], (u64)len[i]);
			total_len += len[i];
		}
	}

	/* dst = hash output */
	ddt_init(&g_spacc_ram.dst);
	ddt_add(&g_spacc_ram.dst, (u64)out, 64);

	warn_if_spacc_fifo_not_clean();

	sw_id = spacc_query_next_sw_id();

	if (dcache_status())
		flush_dcache_all();

	// spacc databook, p288 Hash Mode
	//
	// CTRL.CIPH_ARG = NULL
	// PRE_AAD_LEN = PROC_LEN
	// POST_AAD_LEN = 0
	// CTRL.ENCRYPT = 1
	// CTRL.AAD_COPY = 0

	SPACC_REG->offset = 0;   // pre_aad offset
	SPACC_REG->pre_aad_len = total_len;
	SPACC_REG->proc_len = total_len;
	SPACC_REG->post_aad_len = 0;

	SPACC_REG->icv_len = 0; // 0=default length for the selected hash
	SPACC_REG->icv_offset = 0; // dst buf for encrypt

	// bus view = arm view for cb sram & dram
	SPACC_REG->src_ptr = (u64)g_spacc_ram.src.ddt;
	SPACC_REG->dst_ptr = (u64)g_spacc_ram.dst.ddt;

	// [31]=0 : iv is at context buffer
	SPACC_REG->iv_offset = 0;

	SPACC_REG->key_sz = (1 << _SPACC_KEY_SZ_CIPHER); // no cipher key
	SPACC_REG->key_sz = 0; // no hash key

	SPACC_REG->sw_ctrl  = sw_id; // auto incr after ctrl cmd
	SPACC_REG->aux_info = 0;

	// trigger hash
	SPACC_REG->ctrl =
		(0         << _SPACC_CTRL_CIPH_ALG)	|
		(ctrl_hash << _SPACC_CTRL_HASH_ALG)	|
		(HM_RAW    << _SPACC_CTRL_HASH_MODE)	|
		(1         << _SPACC_CTRL_ENCRYPT)	|
		(0         << _SPACC_CTRL_CTX_IDX)	|
		(0         << _SPACC_CTRL_AAD_COPY)	|
		(1         << _SPACC_CTRL_ICV_PT)	|
		(0         << _SPACC_CTRL_ICV_ENC)	|
		(0         << _SPACC_CTRL_ICV_APPEND)	|
		(1         << _SPACC_CTRL_MSG_BEGIN)	|
		(1         << _SPACC_CTRL_MSG_END);

	spacc_pop_fifo(sw_id);

	return warn_if_spacc_fail();
}

int spacc_shaX_512(const unsigned char *message, size_t message_len, unsigned char out[64])
{
	return spacc_shaX_512_vector(1, &message, &message_len, out);
}

// see ed25519_hash()
int spacc_ed25519_hash(const unsigned char *signature,
		const unsigned char *message, size_t message_len,
		const unsigned char *public_key, unsigned char h_val[64])
{
	int ctrl_hash, sw_id;

#ifdef CONFIG_USE_SHA2
	ctrl_hash = H_SHA512;
#else
	ctrl_hash = H_SHA3_512;
#endif

	/* src = message */
	ddt_init(&g_spacc_ram.src);
	ddt_add(&g_spacc_ram.src, (u64)signature, 32);
	ddt_add(&g_spacc_ram.src, (u64)public_key, 32);
	ddt_add(&g_spacc_ram.src, (u64)message, message_len);

	/* dst = hash output */
	ddt_init(&g_spacc_ram.dst);
	ddt_add(&g_spacc_ram.dst, (u64)h_val, 64);

	warn_if_spacc_fifo_not_clean();

	sw_id = spacc_query_next_sw_id();
	if (dcache_status())
		flush_dcache_all();

	// spacc databook, p288 Hash Mode
	//
	// CTRL.CIPH_ARG = NULL
	// PRE_AAD_LEN = PROC_LEN
	// POST_AAD_LEN = 0
	// CTRL.ENCRYPT = 1
	// CTRL.AAD_COPY = 0

	SPACC_REG->offset = 0;   // pre_aad offset
	SPACC_REG->pre_aad_len = g_spacc_ram.src.len;
	SPACC_REG->proc_len = g_spacc_ram.src.len;
	SPACC_REG->post_aad_len = 0;

	SPACC_REG->icv_len = 0; // 0=default length for the selected hash
	SPACC_REG->icv_offset = 0; // dst buf for encrypt

	// bus view = arm view for cb sram & dram
	SPACC_REG->src_ptr = (u64)g_spacc_ram.src.ddt;
	SPACC_REG->dst_ptr = (u64)g_spacc_ram.dst.ddt;

	// [31]=0 : iv is at context buffer
	SPACC_REG->iv_offset = 0;

	SPACC_REG->key_sz = (1 << _SPACC_KEY_SZ_CIPHER); // no cipher key
	SPACC_REG->key_sz = 0; // no hash key

	SPACC_REG->sw_ctrl  = sw_id; // auto incr after ctrl cmd
	SPACC_REG->aux_info = 0;

	// trigger hash
	SPACC_REG->ctrl =
		(0         << _SPACC_CTRL_CIPH_ALG)	|
		(ctrl_hash << _SPACC_CTRL_HASH_ALG)	|
		(HM_RAW    << _SPACC_CTRL_HASH_MODE)	|
		(1         << _SPACC_CTRL_ENCRYPT)	|
		(0         << _SPACC_CTRL_CTX_IDX)	|
		(0         << _SPACC_CTRL_AAD_COPY)	|
		(1         << _SPACC_CTRL_ICV_PT)	|
		(0         << _SPACC_CTRL_ICV_ENC)	|
		(0         << _SPACC_CTRL_ICV_APPEND)	|
		(1         << _SPACC_CTRL_MSG_BEGIN)	|
		(1         << _SPACC_CTRL_MSG_END);

	spacc_pop_fifo(sw_id);

	return warn_if_spacc_fail();
}

int spacc_aes_gcm_ad(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
		const u8 *crypt, size_t crypt_len,
		const u8 *aad, size_t aad_len, const u8 *tag, u8 *plain)
{
	int sw_id;
	volatile u64 *ctx = (void *)(SPACC_BASE + SPACC_CTX_CIPH_KEY);

	/* src = aad, message, tag */
	ddt_init(&g_spacc_ram.src);
	ddt_add(&g_spacc_ram.src, (u64)aad, (u64)aad_len);
	ddt_add(&g_spacc_ram.src, (u64)crypt, (u64)crypt_len);
	ddt_add(&g_spacc_ram.src, (u64)tag, 16);

	/* dst = decrypted output */
	ddt_init(&g_spacc_ram.dst);
	ddt_add(&g_spacc_ram.dst, (u64)plain, (u64)crypt_len);

	warn_if_spacc_fifo_not_clean();

	sw_id = spacc_query_next_sw_id();

	// aes context buffer
#if 0 // not work
	memcpy((void *)&ctx[0], (const void *)key, key_len);
	memcpy((void *)&ctx[8], (const void *)iv, iv_len);
	if (iv_len == 12) {
		ctx[11] = 0x01000000; // ctx offset=44B = iv[12~15]
	}
#else
	int i;
	for (i = 0; i < 8; i++) {
		ctx[i] = pdu_io_read32(&key[i * 4]);
	}

	if (iv_len >= 12) {
		ctx[8] = pdu_io_read32(&iv[0]);
		ctx[9] = pdu_io_read32(&iv[4]);
		ctx[10] = pdu_io_read32(&iv[8]);

		if (iv_len == 12) {
			ctx[11] = 0x01000000; // iv[12~15]
		} else if(iv_len >= 16) {
			ctx[11] = pdu_io_read32(&iv[12]);
		} else {
			// Not support IV != 12 or 16
			printf("warn: only acept 12/16 iv_len!\n");
		}
	}
#endif

	if (dcache_status())
		flush_dcache_all();

	// spacc databook, p289 AEAD Mode
	//
	// All combined mode algorithms have the following restrictions on the CTRL register fields:
	// ■ HASH_ALG must be NULL
	// ■ ICV_PT must be zero
	// ■ ICV_ENC must be zero
	// ■ KEY_EXP is not used
	// The combined mode algorithms do not support POST_AAD values.

	SPACC_REG->offset       = 0; // pre_aad offset
	SPACC_REG->pre_aad_len  = aad_len;
	SPACC_REG->proc_len     = crypt_len + aad_len;
	SPACC_REG->post_aad_len = 0;

	//SPACC_REG->icv_len = 0; // 0=default length for the selected hash
	SPACC_REG->icv_len = 16;  // gcm tag
	SPACC_REG->icv_offset = 0; // ignore since we set ICV_APPEND=1

	// bus view = arm view for cb sram & dram
	SPACC_REG->src_ptr = (u64)g_spacc_ram.src.ddt;
	SPACC_REG->dst_ptr = (u64)g_spacc_ram.dst.ddt;

	// [31]=0 : iv is at context buffer
	SPACC_REG->iv_offset = 0;

	SPACC_REG->key_sz = (1 << _SPACC_KEY_SZ_CIPHER) | key_len; // cipeher key
	SPACC_REG->key_sz = 0; // no hash key

	SPACC_REG->sw_ctrl  = sw_id; // auto incr after ctrl cmd
	SPACC_REG->aux_info = 0;

	// trigger aes-gcm
	SPACC_REG->ctrl =
		(C_AES     << _SPACC_CTRL_CIPH_ALG)	|
		(CM_GCM    << _SPACC_CTRL_CIPH_MODE)	|
		(0         <<  _SPACC_CTRL_HASH_ALG)	|
		(0         << _SPACC_CTRL_ENCRYPT)	| // decrypt
		(0         << _SPACC_CTRL_CTX_IDX)	|
		(0         << _SPACC_CTRL_AAD_COPY)	|
		(0         << _SPACC_CTRL_ICV_PT)	|
		(0         << _SPACC_CTRL_ICV_ENC)	|
		(1         << _SPACC_CTRL_ICV_APPEND)	| // icv is appended in src buf
		(1         << _SPACC_CTRL_MSG_BEGIN)	|
		(1         << _SPACC_CTRL_MSG_END);

	spacc_pop_fifo(sw_id);

	return warn_if_spacc_fail();
}


// see hmac_sha512_vector()
static int spacc_hmac_shaX_512_vector(const u8 *key, size_t key_len, size_t num_elem,
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

	/* if key is longer than 128 bytes reset it to key = SHA512(key) */
	if (key_len > 128) {
		if (spacc_shaX_512_vector(1, &key, &key_len, tk) < 0)
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
	memset(k_pad, 0, sizeof(k_pad));
	memcpy(k_pad, key, key_len);
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
	if (spacc_shaX_512_vector(1 + num_elem, _addr, _len, mac) < 0)
		return -1;

	memset(k_pad, 0, sizeof(k_pad));
	memcpy(k_pad, key, key_len);
	/* XOR key with opad values */
	for (i = 0; i < 128; i++)
		k_pad[i] ^= 0x5c;

	/* perform outer SHA512 */
	_addr[0] = k_pad;
	_len[0] = 128;
	_addr[1] = mac;
	_len[1] = SHA512_MAC_LEN;
	return spacc_shaX_512_vector(2, _addr, _len, mac);
}

// hmac_sha512()
int spacc_hmac_shaX_512(const u8 *key, size_t key_len, const u8 *data, size_t data_len, u8 *mac)
{
	return spacc_hmac_shaX_512_vector(key, key_len, 1, &data, &data_len, mac);
}

// see hmac_sha512_kdf()
int spacc_hmac_shaX_512_kdf(const u8 *secret, size_t secret_len, const u8 *seed, size_t seed_len, u8 *out, size_t outlen)
{
	__ALIGN4
	u8 T[SHA512_MAC_LEN];
	__ALIGN4
	u8 iter = 1;
	__ALIGN4
	const unsigned char *addr[4];
	__ALIGN4
	size_t len[4];
	size_t pos, clen;

	addr[0] = T;
	len[0] = SHA512_MAC_LEN;
	addr[1] = (const u8 *) ""; /* label = null */
	len[1] = 0;
	addr[2] = seed;
	len[2] = seed_len;
	addr[3] = &iter;
	len[3] = 1;

	if (spacc_hmac_shaX_512_vector(secret, secret_len, 3, &addr[1], &len[1], T) < 0)
		return -1;

	pos = 0;
	for (;;) {
		clen = outlen - pos;
		if (clen > SHA512_MAC_LEN)
			clen = SHA512_MAC_LEN;
		memcpy(out + pos, T, clen);
		pos += clen;

		if (pos == outlen)
			break;

		if (iter == 255) {
			memset(out, 0, outlen);
			memset(T, 0, SHA512_MAC_LEN);
			return -1;
		}
		iter++;

		if (spacc_hmac_shaX_512_vector(secret, secret_len, 4, addr, len, T) < 0)
		{
			memset(out, 0, outlen);
			memset(T, 0, SHA512_MAC_LEN);
			return -1;
		}
	}

	memset(T, 0, SHA512_MAC_LEN);
	return 0;
}

#if 0
int spacc_test_gcm(void)
{
	__attribute__((unused)) int res;
	int sw_id;
	volatile u64 *ctx = (void *)(SPACC_BASE + SPACC_CTX_CIPH_KEY);

	const u64 TEST_SRC_BUF = 0x9e8010f7; // 0x060061f7
	const u64 TEST_DST_BUF = 0x9e8049c5; // 0x080049c5

	const int source_ddt[4] = {
		TEST_SRC_BUF,0x1ce,0x0,0x0  //7bd2->0600
	};

	const int source[462] = {
		0x1a000000,0x365a1d5d,0xdb8063a8,0xb2a50579,0xd3ccfa6a,0x2576e744,0x361ea6a4,0xbc9ecb4f,
		0xff8c4484,0xfc3101dc,0x2382248a,0xf71f2187,0x4b1fb711,0xbacfc773,0x841483c0,0x75276a6a,
		0x5862ec07,0xf1775180,0x83dd307d,0x2abb9765,0x93161758,0x1013e0d9,0x81a8f44d,0xcf4934ff,
		0x686fb956,0x143eb0f0,0x6d16006b,0xe0d2ab4c,0x9a3a50db,0xc1c707f1,0x3d9da68b,0xf325c6b3,
		0x591d456f,0xc37e3877,0x2bd37445,0x74ccec57,0xda34dca4,0x641b851f,0xbf6f75e6,0x28e3aa03,
		0x0c5d3a1f,0xa35b4b8f,0xb0ec2bd2,0x5ef27363,0x55a180b7,0xbe1246bf,0xd406e337,0x52ce3f9d,
		0x19d1d6e2,0xc7386eaa,0x7a785dd9,0x9d3ba945,0xbb9b3473,0x3677f902,0xe777d1fb,0xaa0eef31,
		0x2aacd539,0x2e77765a,0x9363d680,0xb89848de,0x7c737fe9,0x903b931b,0x9b50ea2a,0xa20d5dcf,
		0xf447c911,0x438ae86e,0xe0667f5d,0xfe00f545,0x30127d95,0xdd203fc0,0x59ae1350,0xe0f369c4,
		0x22ab4f54,0xeb2707b3,0xfeb1d6f8,0xd8cdacc3,0xc1d935fd,0x73537299,0x4e9500f4,0x5b979368,
		0x19252fdf,0x7b648c8b,0x9b1385e3,0x66dfc2b5,0xab13cc27,0x242dfa34,0x585cc7d8,0xb70719b0,
		0xb76800df,0xd657c676,0x6f2cd397,0x72828b6c,0x5dcc0461,0x46f6fb47,0x569fe80b,0x3714d6c9,
		0x1db8022c,0x78befe19,0xe45ab171,0xdf35482a,0x366cfe40,0x606a6594,0xc434d4f6,0x2d865c60,
		0xfcafd319,0x8f115435,0xfafcd0ed,0x94aac3f1,0x35711c3d,0x8a568a4a,0xa6f0c4f0,0xc814c132,
		0x95fb1c09,0x0c60754e,0x75e82cf9,0x885cd833,0xb
	};
	const int destination_ddt[4] = {
		TEST_DST_BUF,0x11,0x0,0x0  //62fd->0800
	};
	const int destination[17] = {
		0xcf51f300,0x9987ee2d,0xcb751e33,0x8b3dea93,0xf3
	};

	const u64 key[] = { 0x78ae32eb, 0xef18ad1d, 0xcc4af1d7, 0xdb752a01 };
	const u64 iv[]  = { 0xac6f3aeb, 0x45eb9211, 0x5acbff40, 0x01000000 };
	int key_len = 16;

	CSTAMP(0x23500000);
	memcpy((void *)TEST_SRC_BUF - 3, source, sizeof(source));

	/* src = aad, message, tag */
	ddt_init(&g_spacc_ram.src);
	ddt_add(&g_spacc_ram.src, source_ddt[0], source_ddt[1]);

	/* dst = decrypted output */
	ddt_init(&g_spacc_ram.dst);
	ddt_add(&g_spacc_ram.dst, destination_ddt[0], destination_ddt[1]);

	CSTAMP(0x23500001);
	warn_if_spacc_fifo_not_clean();

	sw_id = spacc_query_next_sw_id();
	CSTAMP(sw_id);

	// aes context buffer
	ctx[0] = pdu_io_read32(&key[0]);
	ctx[1] = pdu_io_read32(&key[1]);
	ctx[2] = pdu_io_read32(&key[2]);
	ctx[3] = pdu_io_read32(&key[3]);
	ctx[8] = pdu_io_read32(&iv[0]);
	ctx[9] = pdu_io_read32(&iv[1]);
	ctx[10] = pdu_io_read32(&iv[2]);
	ctx[11] = pdu_io_read32(&iv[3]);

	CSTAMP(0x23500002);

	if (dcache_status())
		flush_dcache_all();

	CSTAMP(0x23500003);

	// spacc databook, p289 AEAD Mode
	//
	// All combined mode algorithms have the following restrictions on the CTRL register fields:
	// ■ HASH_ALG must be NULL
	// ■ ICV_PT must be zero
	// ■ ICV_ENC must be zero
	// ■ KEY_EXP is not used
	// The combined mode algorithms do not support POST_AAD values.

	SPACC_REG->offset       = 0;   // pre_aad offset
	SPACC_REG->pre_aad_len  = 445; // 0x1bd
	SPACC_REG->proc_len     = 462; // 0x1ce = 445 + 17
	SPACC_REG->post_aad_len = 0;

	//SPACC_REG->icv_len = 0; // 0=default length for the selected hash
	SPACC_REG->icv_len = 16;   // gcm tag
	SPACC_REG->icv_offset = 0; // dst buf for encrypt

	// bus view = arm view for cb sram & dram
	SPACC_REG->src_ptr = (u64)g_spacc_ram.src.ddt;
	SPACC_REG->dst_ptr = (u64)g_spacc_ram.dst.ddt;

	// [31]=0 : iv is at context buffer
	SPACC_REG->iv_offset = 0;

	SPACC_REG->key_sz = (1 << _SPACC_KEY_SZ_CIPHER) | key_len; // cipeher key
	SPACC_REG->key_sz = 0; // no hash key

	CSTAMP(0x23500004);
	CSTAMP(SPACC_REG->aux_info);

	SPACC_REG->sw_ctrl  = sw_id; // auto incr after ctrl cmd
	SPACC_REG->aux_info = 0;

	// trigger aes-gcm
	SPACC_REG->ctrl =
		(C_AES     << _SPACC_CTRL_CIPH_ALG)	|
		(CM_GCM    << _SPACC_CTRL_CIPH_MODE)	|
		(0         <<  _SPACC_CTRL_HASH_ALG)	|
		(1         << _SPACC_CTRL_ENCRYPT)	| // encrypt
		(0         << _SPACC_CTRL_CTX_IDX)	|
		(0         << _SPACC_CTRL_AAD_COPY)	|
		(0         << _SPACC_CTRL_ICV_PT)	|
		(0         << _SPACC_CTRL_ICV_ENC)	|
		(0         << _SPACC_CTRL_ICV_APPEND)	| // icv is at icv_offset in dst buf
		(1         << _SPACC_CTRL_MSG_BEGIN)	|
		(1         << _SPACC_CTRL_MSG_END);

	CSTAMP(0x23500005);
	spacc_pop_fifo(sw_id);

	CSTAMP(0x23500006);
	res = warn_if_spacc_fail();

	CSTAMP(0x23500007);
	CSTAMP(res);

	CSTAMP(0x23500008);
	//diag_printf("=======compare !! ========\n");
	{
		volatile int *ptr;
		volatile int read;
		volatile int right;
		int i;

		ptr = (void *)(TEST_DST_BUF - 1);
		right = destination[0];
		read = (*ptr)&0xffffff00;
		if(read != right) {//0xcf51f35e
			diag_printf("right = 0x%08x ,read = 0x%08x\n", right, read);
			CSTAMP(0x2350e001);
			return -1;
		}

		for(i=1;i<4;i++){
			ptr++;
			right = destination[i];
			read = (*ptr);
			if(read != right){
				diag_printf("right = 0x%08x ,read = 0x%08x\n", right, read);
				CSTAMP(0x2350e002);
				return -1;
			}
		}

		ptr++;
		right = destination[4];
		read=(*ptr)&0xff;
		if(read != right) {//0x8b3deaf3
			diag_printf("right = 0x%08x ,read = 0x%08x\n", right, read);
			CSTAMP(0x2350e003);
			return -1;
		}
	}
	CSTAMP(0x23500999);
	printf("test ok!!!!!!\n");
	return 0;
}
#endif
