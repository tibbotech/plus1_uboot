#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>


#define MAX_OUTPUT_SIZE    4096
#define MAX_INPUT_SIZE     4096
#define MAX_SALT_SIZE      4096

extern int hkdf_shaX_512(const unsigned char *salt, int salt_len, const unsigned char *ikm, int ikm_len,
		const unsigned char *info, int info_len, uint8_t okm[], int okm_len);

static void usage(void)
{
	printf("Usage:\n"
			"\targv[1] : output length (bytes)\n"
			"\targv[2] : output file (max size: %d)\n"
			"\targv[3] : input file (max size: %d)\n"
			"\targv[4] : optional input salt file (max size: %d)\n"
			"\t\n", MAX_OUTPUT_SIZE, MAX_INPUT_SIZE, MAX_SALT_SIZE);
}

int main(int argc, char *argv[])
{
	const char *in_file, *out_file, *salt_file = NULL;
	int out_len, in_len, salt_len = 0, got, res;
	struct stat st;
	FILE *fp, *out_fp;
	unsigned char *out_buf, *in_buf, *salt_buf = NULL;

	if (argc < 4) {
		usage();
		return -1;
	}

	out_len = atoi(argv[1]);
	out_file = argv[2];
	in_file = argv[3];

	if (argc > 4)
		salt_file = argv[4];

	if (out_len <= 0 || out_len > MAX_OUTPUT_SIZE) {
		fprintf(stderr, "not support output file size %d (max %d)\n", out_len, MAX_OUTPUT_SIZE);
		return -1;
	}

	memset(&st, 0, sizeof(st));
	stat(in_file, &st);
	in_len = st.st_size;

	if (in_len <= 0 || in_len > MAX_INPUT_SIZE) {
		fprintf(stderr, "not support input file size %d (max %d)\n", in_len, MAX_INPUT_SIZE);
		return -1;
	}

	in_buf = malloc(in_len);
	if (!in_buf) {
		fprintf(stderr, "fail to malloc(%d) for input\n", in_len);
		return -1;
	}

	// load input
	fp = fopen(in_file, "rb");
	if (!fp) {
		fprintf(stderr, "fopen %s fail\n", in_file);
		return -1;
	}
	got = fread(in_buf, 1, in_len, fp);
	if (got != in_len) {
		fprintf(stderr, " %s only read %d bytes (total=%d)\n", in_file, got, in_len);
		return -1;
	}
	fclose(fp);

	// load salt
	if (salt_file) {
		memset(&st, 0, sizeof(st));
		stat(salt_file, &st);
		salt_len = st.st_size;
		if (salt_len <= 0 || salt_len > MAX_SALT_SIZE) {
			fprintf(stderr, "not support salt file size %d (max %d)\n", salt_len, MAX_SALT_SIZE);
			return -1;
		}

		salt_buf = malloc(salt_len);
		if (!salt_buf) {
			fprintf(stderr, "fail to malloc(%d) for salt\n", salt_len);
			return -1;
		}

		fp = fopen(salt_file, "rb");
		if (!fp) {
			fprintf(stderr, "fopen %s fail\n", in_file);
			return -1;
		}
		got = fread(salt_buf, 1, salt_len, fp);
		if (got != salt_len) {
			fprintf(stderr, " %s only read %d bytes (total=%d)\n", salt_file, got, salt_len);
			return -1;
		}
		fclose(fp);
	}

	// output buffer
	out_buf = malloc(out_len);
	if (!out_buf) {
		fprintf(stderr, "fail to malloc(%d) for output\n", out_len);
		return -1;
	}

	// hkdf
	printf("hkdf_shaX_512 salt_len=%d in_len=%d out_len=%d\n", salt_len, in_len, out_len);
	res = hkdf_shaX_512(salt_buf, salt_len, in_buf, in_len, NULL, 0, out_buf, out_len);
	if (res) {
		fprintf(stderr, "fail in hkdf_shaX_512, ret=%d\n", res);
		return -1;
	}

	// save output
	out_fp = fopen(out_file, "wb");
	if (!out_fp) {
		fprintf(stderr, "fopen %s fail\n", out_file);
		return -1;
	}
	res = fwrite(out_buf, 1, out_len, out_fp);
	if (res != out_len) {
		fprintf(stderr, "%s : fwrite return %d != %d (len), error=%d\n",
				out_file, got, out_len, ferror(out_fp));
		return -1;
	}
	fclose(out_fp);

	return 0;
}
