#include "gcm.h"
#include "utils.h"
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdarg.h>

int debug_lvl = 0;

void dbg(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (debug_lvl)
		vprintf(fmt, args);
	va_end(args);
}

#if 0
//#define IN_PLACE

static int single_encryption(void) {
    mbedtls_gcm_context ctx;
#ifdef IN_PLACE
    unsigned char *buf;
#else
    unsigned char buf[64];
    unsigned char buf2[64];
    unsigned char tag_buf2[16];
#endif
    unsigned char tag_buf[16];
    int ret;
    mbedtls_cipher_id_t cipher = MBEDTLS_CIPHER_ID_AES;
    // 32 bytes.. that's 256 bits
    const unsigned char key[32] = { 0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
      0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
      0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
      0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08 };
    unsigned char plaintext[64] = { 0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
      0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
      0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
      0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
      0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
      0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
      0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
      0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55 };
    unsigned char expected_ciphertext[64] = { 0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24,
      0x4b, 0x72, 0x21, 0xb7, 0x84, 0xd0, 0xd4, 0x9c,
      0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0,
      0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e,
      0x21, 0xd5, 0x14, 0xb2, 0x54, 0x66, 0x93, 0x1c,
      0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05,
      0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97,
      0x3d, 0x58, 0xe0, 0x91, 0x47, 0x3f, 0x59, 0x85};
    const unsigned char initial_value[12] = { 0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
      0xde, 0xca, 0xf8, 0x88 };
    const unsigned char additional[] = {};

    mbedtls_gcm_init( &ctx );
    // 128 bits, not bytes!
    ret = mbedtls_gcm_setkey( &ctx, cipher, key, 128 );

#ifdef IN_PLACE
    buf = plaintext;
#endif

    ret = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, 64, initial_value, 12, additional, 0, plaintext, buf, 16, tag_buf);
    mbedtls_gcm_free( &ctx );
    if (memcmp(buf, expected_ciphertext, 64) == 0) {
        printf("My local test also works\n");
    } else {
        printf("local test failed\n");
    }

    // test decryption
#ifndef IN_PLACE
    mbedtls_gcm_init( &ctx );
    ret = mbedtls_gcm_setkey( &ctx, cipher, key, 128 );
    ret = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_DECRYPT, 64, initial_value, 12, additional, 0, expected_ciphertext, buf2, 16, tag_buf2);
    mbedtls_gcm_free( &ctx );
    if (memcmp(buf2, plaintext, 64) == 0) {
        printf("My local decryption works\n");
    } else {
        printf("local decryption failed\n");
    }
#endif

    return ret;
}
#endif

//int file_save_buf(FILE *out_fp, unsigned char *buf, int len) { int got; }

int aes_gcm_on_file(int encrypt, const char *iv_file, const char *key_file,
	const char *in_file, const char *out_file, const char *auth_file)
{
        struct stat st;
        FILE *fp = NULL, *out_fp = NULL;
	unsigned char iv_buf[256], key_buf[32], auth_buf[16];
	unsigned char in_buf[256], out_buf[256];
	int got, iv_len, key_len, auth_len, in_len, remain_rlen;
	mbedtls_gcm_context ctx;
	const unsigned char additional[] = {};
	int ret = -1;
	int mode, res;

	memset(iv_buf, 0, sizeof(iv_buf));
	memset(key_buf, 0, sizeof(key_buf));
	memset(auth_buf, 0, sizeof(auth_buf));

	// load iv
	dbg("%s: load iv\n", __func__);
	memset(&st, 0, sizeof(st));
	stat(iv_file, &st);
	iv_len = st.st_size;
	if (iv_len > sizeof(iv_buf)) {
		fprintf(stderr, "%s > max iv size=%ld\n", iv_file, sizeof(iv_buf));
		ret = -1;
		goto clr_out;
	}
	fp = fopen(iv_file, "rb");
	if (!fp) {
		fprintf(stderr, "fopen %s fail\n", iv_file);
		ret = -1;
		goto clr_out;
	}
	got = fread(iv_buf, 1, iv_len, fp);
	if (got != iv_len) {
		fprintf(stderr, "%s : only read %d bytes (total=%d)\n", iv_file, got, iv_len);
		ret = -1;
		goto clr_out;
	}
	fclose(fp);


	// load key
	dbg("%s: load key\n", __func__);
	memset(&st, 0, sizeof(st));
	stat(key_file, &st);
	key_len = st.st_size;
	if (key_len > sizeof(key_buf)) {
		fprintf(stderr, "%s > max key size=%ld\n", key_file, sizeof(key_buf));
		ret = -1;
		goto clr_out;
	}
	fp = fopen(key_file, "rb");
	if (!fp) {
		fprintf(stderr, "fopen %s fail\n", key_file);
		ret = -1;
		goto clr_out;
	}
	got = fread(key_buf, 1, key_len, fp);
	if (got != key_len) {
		fprintf(stderr, "%s : only read %d bytes (total=%d)\n", key_file, got, key_len);
		ret = -1;
		goto clr_out;
	}
	fclose(fp);

	// load auth (if decrypt)
	auth_len = sizeof(auth_buf);

	if (!encrypt) {
		dbg("%s: load auth tag\n", __func__);
		memset(&st, 0, sizeof(st));
		stat(auth_file, &st);
		auth_len = st.st_size;
		if (auth_len > sizeof(auth_buf)) {
			fprintf(stderr, "%s > max auth size=%ld\n", auth_file, sizeof(auth_buf));
			ret = -1;
			goto clr_out;
		}

		fp = fopen(auth_file, "rb");
		if (!fp) {
			fprintf(stderr, "fopen %s fail\n", auth_file);
			ret = -1;
			goto clr_out;
		}
		got = fread(auth_buf, 1, auth_len, fp);
		if (got != auth_len) {
			fprintf(stderr, "%s : only read %d bytes (total=%d)\n", auth_file, got, auth_len);
			ret = -1;
			goto clr_out;
		}
		fclose(fp);
	}

	mbedtls_gcm_init(&ctx);

	ret = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key_buf, key_len * 8); // key bits
	if (ret) {
		fprintf(stderr, "set key failed, ret=%d\n", ret);
		goto clr_out;
	}

	mode = encrypt ? MBEDTLS_GCM_ENCRYPT : MBEDTLS_GCM_DECRYPT;

	dbg("%s: mode=%d\n", __func__, mode);

	ret = mbedtls_gcm_starts(&ctx, mode, iv_buf, iv_len, additional, 0);
	if (ret) {
		fprintf(stderr, "gcm start failed, ret=%d\n", ret);
		goto clr_out;
	}

	//
	// open output file (write)
	//
	dbg("%s: open output file %s\n", __func__, out_file);
	out_fp = fopen(out_file, "wb");
	if (!out_fp) {
		fprintf(stderr, "fopen %s fail\n", out_file);
		ret = -1;
		goto clr_out;
	}

	//
	// open input file (read)
	//
	dbg("%s: open input file %s\n", __func__, in_file);
	fp = fopen(in_file, "rb");
	if (!fp) {
		fprintf(stderr, "fopen %s fail\n", in_file);
		ret = -1;
		goto clr_out;
	}

	memset(&st, 0, sizeof(st));
	stat(in_file, &st);
	remain_rlen = st.st_size;

	dbg("%s: input file size=%d\n", __func__, __LINE__, remain_rlen);

	got = 0; // current read length of input file
	while (remain_rlen > 0) {
		// load if read buffer is not full, or remain_rlen == 0
		if (got < sizeof(in_buf)) {
			in_len = sizeof(in_buf) - got;
			if (in_len > remain_rlen)
				in_len = remain_rlen;
			if (in_len > 0) {
				res = fread(&in_buf[got], 1, in_len, fp);
				got += res;
				remain_rlen -= res;
			}
		}

		dbg("%s: read got=%d bytes in buffer\n", __func__, got);

		// process only if read buffer is full, or it's the last round
		if (got >= sizeof(in_buf) || (0 == remain_rlen)) {
			in_len = sizeof(in_buf);
			if (in_len > got)
				in_len = got;

			ret = mbedtls_gcm_update(&ctx, in_len, in_buf, out_buf);
			if (ret) {
				fprintf(stderr, "gcm update failed, ret=%d\n", ret);
				ret = -1;
				goto clr_out;
			}

			// save output buffer to file
			dbg("%s: fwrite len=%d\n", __func__, in_len);
			res = fwrite(out_buf, 1, in_len, out_fp);
			if (res != in_len) {
				fprintf(stderr, "%s : fwrite return %d != %d (len), error=%d\n",
				out_file, got, in_len, ferror(out_fp));
				ret = -1;
				goto clr_out;
			}

			got -= in_len;
		}
	}
	
	dbg("%s: close input file and output file\n", __func__);
	fclose(fp);
	fclose(out_fp);

	ret = mbedtls_gcm_finish(&ctx, auth_buf, auth_len);
	if (ret) {
		fprintf(stderr, "gcm finish failed, ret=%d\n", ret);
		ret = -1;
		goto clr_out;
	}

	mbedtls_gcm_free(&ctx);

	// save auth tag (if encrypt)
	if (encrypt) {
		dbg("%s: save auth tag to file %s\n", __func__, auth_file);
		fp = fopen(auth_file, "wb");
		if (!fp) {
			fprintf(stderr, "fopen/wb %s fail\n", auth_file);
			ret = -1;
			goto clr_out;
		}
		got = fwrite(auth_buf, 1, auth_len, fp);
		if (got != auth_len) {
			fprintf(stderr, "%s : only write %d bytes (total=%d)\n", auth_file, got, auth_len);
			ret = -1;
			goto clr_out;
		}
		fclose(fp);
	}

	dbg("%s: well done\n", __func__);
	ret = 0;

clr_out:
	memset(key_buf, 0, sizeof(key_buf));
	memset(in_buf, 0, sizeof(in_buf));
	memset(out_buf, 0, sizeof(out_buf));

	return ret;
}

// argv[1] : 0=decrypt, 1=encrypt
// argv[2] : IV (bin file)
// argv[3] : aes key (bin file)
// argv[4] : input file name
// argv[5] : output file name
// argv[6] : auth tag (bin file)
void usage(void)
{
	printf("Usage:\n"
		"\targv[1] : 0=decrypt, 1=encrypt\n"
		"\targv[2] : IV (bin file)\n"
		"\targv[3] : aes key (bin file)\n"
		"\targv[4] : input file name\n"
		"\targv[5] : output file name\n"
		"\targv[6] : auth ta (bin file)\n"
		"\targv[7] : debug level 0(no debug), 1(verbose)\n\n");
}

int main(int argc, const char *argv[])
{
	int encrypt;
	const char *iv_file;
	const char *key_file;
	const char *in_file;
	const char *out_file;
	const char *auth_file;

	if (argc < 7) {
		usage();
		return -1;
	}

	encrypt = atoi(argv[1]);
	iv_file = argv[2];
	key_file = argv[3];
	in_file = argv[4];
	out_file = argv[5];
	auth_file = argv[6];

	if (argc >= 8)
		debug_lvl = atoi(argv[7]);

	printf("encrypt=%d iv=%s key=%s in=%s out=%s auth=%s\n",
		encrypt, iv_file, key_file, in_file, out_file, auth_file);

	if (debug_lvl) {
		printf("aes gcm self test:\n");
		if (mbedtls_gcm_self_test(1)){
			printf("aes gcm self test fail\n");
			return -1;
		}

		//single_encryption();
	}

	dbg("%s: L#%d\n", __func__, __LINE__);

	return aes_gcm_on_file(encrypt, iv_file, key_file, in_file, out_file, auth_file);
}
