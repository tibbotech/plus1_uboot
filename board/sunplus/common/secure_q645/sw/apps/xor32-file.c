#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <types.h>


//#include "../../../include/types.h"


int debug_lvl = 0;


void dbg(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (debug_lvl)
		vprintf(fmt, args);
	va_end(args);
}

// assume len % 4 == 0
int xor32(unsigned char *buf, int len)
{
	int val = 0;
	int tmp;
	int i;

	for (i = 0; i < len / 4; i++ ) {
		val ^= ((uint32_t *)buf)[i];
	}

	return val;
}

int xor32_on_file(const char *out_file, const char *in_file)
{
        struct stat st;
        FILE *fp = NULL;
	unsigned char in_buf[256];
	int got, in_len, out_len, remain_rlen;
	int ret = -1;
	int res;
	int xval = 0;

	out_len = 4;

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

	if (remain_rlen % 4) {
		fprintf(stderr, "input file (%s) size must be a multiple of 4\n", in_file);
		ret = -1;
		goto clr_out;
	}

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

			xval ^= xor32(in_buf, in_len);

			got -= in_len;
		}
	}
	
	dbg("%s: close input file and output file\n", __func__);
	fclose(fp);

	// save hash to file
	dbg("%s: save hash to file %s\n", __func__, out_file);
	fp = fopen(out_file, "wb");
	if (!fp) {
		fprintf(stderr, "fopen/wb %s fail\n", out_file);
		ret = -1;
		goto clr_out;
	}
	got = fwrite(&xval, 1, out_len, fp);
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

	return ret;
}

void usage(void)
{
	printf("Usage:\n"
		"\targv[1] : output file (binary)\n"
		"\targv[2] : input file (binary) \n"
		"\targv[3] : debug level 0(no debug), 1(verbose)\n\n");
}

int main(int argc, const char *argv[])
{
	const char *in_file;
	const char *out_file;
	int in_bytes;

	if (argc < 3) {
		usage();
		return -1;
	}

	out_file = argv[1];
	in_file = argv[2];

	if (argc > 3)
		debug_lvl = atoi(argv[3]);

	printf("xor32 in=%s out=%s debug=%d\n", in_file, out_file, debug_lvl);

	return xor32_on_file(out_file, in_file);
}
