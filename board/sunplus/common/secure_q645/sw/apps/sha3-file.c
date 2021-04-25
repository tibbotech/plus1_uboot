#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

//defined(__i386__) || defined(__x86_64__)

#include "../../../include/types.h"
#include "../linux/crypto/sha3.h"


int debug_lvl = 0;

struct sha3_state g_sctx;

void dbg(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (debug_lvl)
		vprintf(fmt, args);
	va_end(args);
}

int sha3_on_file(int sha3_bits, const char *out_file, const char *in_file)
{
        struct stat st;
        FILE *fp = NULL;
	unsigned char in_buf[256], out_buf[256];
	int got, in_len, out_len, remain_rlen;
	int ret = -1;
	int mode, res;

	memset(&g_sctx, 0, sizeof(g_sctx));

	dbg("%s: sha3_bits=%d in_file=%s out_file=%s\n", __func__, sha3_bits, in_file, out_file);

	switch (sha3_bits) {
	case 256:
		out_len = 32;
		sha3_256_init(&g_sctx);
		break;
	case 512:
		out_len = 64;
		sha3_512_init(&g_sctx);
		break;
	default:
		fprintf(stderr, "not support \"sha3-%d\"\n", sha3_bits);
		return -1;
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

	dbg("%s: input file size=%d\n", __func__, remain_rlen);

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

			ret = sha3_update(&g_sctx, in_buf, in_len);
			if (ret) {
				fprintf(stderr, "sha3 update failed, ret=%d\n", ret);
				ret = -1;
				goto clr_out;
			}

			got -= in_len;
		}
	}
	
	dbg("%s: close input file and output file\n", __func__);
	fclose(fp);

	ret = sha3_final(&g_sctx, out_buf);
	if (ret) {
		fprintf(stderr, "sha3 final failed, ret=%d\n", ret);
		ret = -1;
		goto clr_out;
	}

	// save hash to file
	dbg("%s: save hash to file %s\n", __func__, out_file);
	fp = fopen(out_file, "wb");
	if (!fp) {
		fprintf(stderr, "fopen/wb %s fail\n", out_file);
		ret = -1;
		goto clr_out;
	}
	got = fwrite(out_buf, 1, out_len, fp);
	if (got != out_len) {
		fprintf(stderr, "%s : only write %d bytes (total=%d)\n", out_file, got, out_len);
		ret = -1;
		goto clr_out;
	}
	fclose(fp);

	dbg("%s: well done\n", __func__);
	ret = 0;

clr_out:
	memset(in_buf, 0, sizeof(in_buf));
	memset(out_buf, 0, sizeof(out_buf));

	return ret;
}

void usage(void)
{
	printf("Usage:\n"
		"\targv[1] : 256/512\n"
		"\targv[2] : hash output file (binary)\n"
		"\targv[3] : input file \n"
		"\targv[4] : debug level 0(no debug), 1(verbose)\n\n");
}

int main(int argc, const char *argv[])
{
	int sha_bits;
	const char *in_file;
	const char *out_file;
	int in_bytes;

	if (argc < 4) {
		usage();
		return -1;
	}

	sha_bits = atoi(argv[1]);
	out_file = argv[2];
	in_file = argv[3];

	if (argc >= 5)
		debug_lvl = atoi(argv[4]);

	printf("sha bits=%d in=%s out=%s debug=%d\n",
		sha_bits, in_file, out_file, debug_lvl);

	return sha3_on_file(sha_bits, out_file, in_file);
}
