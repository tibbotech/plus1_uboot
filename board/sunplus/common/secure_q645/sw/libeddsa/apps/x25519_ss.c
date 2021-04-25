#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "eddsa.h"

#define KEY_LEN 32
#define SHARED_SECRET_LEN 32

void prn_dump(char *title, const uint8_t *buf, int len)
{
	unsigned i;

	if (title)
		printf("%s", title);

	if (len < 1)
		return;

	for(i = 0; i < len - 1; ++i) {
		printf("0x%02X, ", (unsigned)buf[i]);
	}
	printf("0x%02X\n", (unsigned)buf[i]);
}

static int save_ss(const char *fname, uint8_t ss[SHARED_SECRET_LEN])
{
	int fd, res, size;

	/* save bin */
	fd = open(fname, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		fprintf(stderr, "can't open file %s\n", fname);
		return -1;
	}

	size = SHARED_SECRET_LEN;
	res = write(fd, ss, SHARED_SECRET_LEN);
	if (res != size) {
		fprintf(stderr, "can't write r to %s\n", fname);
		return -1;
	}
	close(fd);

	return 0;
}

static int hex2byte(char val)
{
	if (val >= '0' && val <= '9')
		return (val - '0');
	if (val >= 'a' && val <= 'f')
		return 10 + (val - 'a');
	if (val >= 'A' && val <= 'F')
		return 10 + (val - 'A');
	return -1;
}

static int hex2bytes(uint8_t out_buf[KEY_LEN], int size, const char *hex_in)
{
	int i;
	int val0, val1;

	for (i = 0; i < size; i++) {
		val0 = hex2byte(hex_in[i * 2]);
		val1 = hex2byte(hex_in[i * 2 + 1]);
		if (val0 < 0 || val1 < 0) {
			fprintf(stderr, "input %d/%d-th char isn't hexdigit: \"%c%c\"",
				i * 2, i * 2 + 1, hex_in[i * 2], hex_in[i * 2 + 1]);
			return -1;
		}
		out_buf[i] = (val0 << 4) | val1;
	}
	return 0;
}

static int load_file_key_hex(uint8_t *key, int key_len, const char *fname)
{
	int i, fd, res;
	char buf[256]; /* hex chars -> 128 bytes max */

	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "can't open %s\n", fname);
		return -1;
	}

	/* load hex chars */
	res = read(fd, buf, sizeof(buf));
	if (res < key_len * 2) {
		fprintf(stderr, "read failed, key file is too short < %d\n", key_len * 2);
		close(fd);
		return -1;
	}

	/* convert to byte array */
	if (hex2bytes(key, key_len, buf)) {
		fprintf(stderr, "key file is not hex\n");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

static int load_file_key_bin(uint8_t *key, int key_len, const char *fname)
{
	int i, fd, res;

	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "can't open %s\n", fname);
		return -1;
	}

	/* load binary */
	res = read(fd, key, key_len);
	if (res < key_len) {
		fprintf(stderr, "read failed, key file %s read is too short %d < %d\n",
			fname,  res, key_len);
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

/* Usage:
 * ./x25519_ss -p priv_key_file -b pub_key_file -o shared_secret_file
 *
 * Eg:
 * ./sign -p key_priv_0.hex -b key_pub_0.hex -o ss.bin
 */
static void usage(const char *prog)
{
	fprintf(stderr,
		"Usage:\n"
		"%s\n"
		"-p priv_key_file   : Our private key file (binary)\n"
		"-b pub_key_file    : Other's public key file (binary)\n"
		"-o ss_file         : output shared secret file (binary)\n"
		, prog);
}

/*
 * Input: see usage()
 *
 * Output: output signature to output bin
 */
int main(int argc, char **argv)
{
	uint8_t private_key[KEY_LEN];
	uint8_t public_key[KEY_LEN];
	uint8_t ss[SHARED_SECRET_LEN];
	char *f_priv = NULL, *f_pub = NULL;
	char *out_ss = "ss.bin";
	unsigned char *msg;
	int len;
	char c;
	int ret;
	uint8_t h_val[64];

	while ((c = getopt(argc, argv, "p:b:o:")) != -1) {
		switch (c) {
		case 'p':
			f_priv = optarg;
			break;
		case 'b':
			f_pub = optarg;
			break;
		case 'o':
			out_ss = optarg;
			break;
		case '?':
		default:
			fprintf(stderr, "Unknown argument\n");
			usage(argv[0]);
			return -1;
		}
	}

	if (!f_priv || !f_pub) {
		fprintf(stderr, "miss arguments\n");
		usage(argv[0]);
		return -1;
	}

	if (load_file_key_bin(private_key, KEY_LEN, f_priv)) {
		fprintf(stderr, "failed to load priv key from %s\n", f_priv);
		ret = -1;
		goto clr_out;
	}

	if (load_file_key_bin(public_key, KEY_LEN, f_pub)) {
		fprintf(stderr, "failed to load public key from %s\n", f_priv);
		ret = -1;
		goto clr_out;
	}

	memset(ss, 0, sizeof(ss));

	x25519(ss, private_key, public_key);

	//prn_dump("key_pub       : ", public_key, sizeof(public_key));
	//prn_dump("key_priv   : ", private_key, sizeof(private_key));
	//prn_dump("shared secret : ", ss, SHARED_SECRET_LEN);

	printf("\nSave shared secret to file: %s\n", out_ss);
	if (save_ss(out_ss, ss)) {
		fprintf(stderr, "failed to write file %s\n", out_ss);
		return -1;
	}

clr_out:
	memset(private_key, 0, KEY_LEN);

	return ret;
}
