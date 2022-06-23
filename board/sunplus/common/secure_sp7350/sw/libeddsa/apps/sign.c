#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "eddsa.h"
#include <auto_config.h>

#define KEY_LEN 32
#define HASH_LEN 64
#define SIGNATURE_LEN 64

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

static int save_bin(const char *fname, uint8_t *buf, int size)
{
	int fd, res;

	/* save bin */
	fd = open(fname, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		fprintf(stderr, "can't open file %s\n", fname);
		return -1;
	}

	res = write(fd, buf, size);
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

static int hex2bytes(uint8_t l_hash[KEY_LEN], int size, const char *hex_hash)
{
	int i;
	int val0, val1;

	for (i = 0; i < size; i++) {
		val0 = hex2byte(hex_hash[i * 2]);
		val1 = hex2byte(hex_hash[i * 2 + 1]);
		if (val0 < 0 || val1 < 0) {
			fprintf(stderr, "input %d/%d-th char isn't hexdigit: \"%c%c\"",
				i * 2, i * 2 + 1, hex_hash[i * 2], hex_hash[i * 2 + 1]);
			return -1;
		}
		l_hash[i] = (val0 << 4) | val1;
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
		fprintf(stderr, "read failed, key file is too short < %d\n", key_len);
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

size_t infile(const char *filename, unsigned char **bp)
{
	size_t len;
	FILE *fp;

	fp = fopen(filename, "rb");
        if (!fp) {
                fprintf(stderr, "open %s failed!\n", filename);
		return -1;
        }

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	*bp = malloc(len);
	if (*bp == NULL) {
		fprintf(stderr, "failed to malloc len=%d !\n", (int)len);
		return -1;
	}
	len = fread(*bp, 1, len, fp);
	fclose(fp);
	return len;
}

/* Usage:
 * ./sign -p priv_key_file -b pub_key_file -s file_to_sign -o sig_file
 *
 * Eg:
 * ./sign -p key_priv_0.bin -b key_pub_0.bin -s xboot.bin -o hash.sig
 */
static void usage(const char *prog)
{
	fprintf(stderr,
		"ed25519-sha%d sign tool\n"
		"Usage:\n"
		"%s\n"
		"-p priv_key_file   : private key file (binary)\n"
		"-b pub_key_file    : public key file (binary)\n"
		"-s file_to_sign    : input file to sign\n"
		"-h hash_file       : output hash file (binary) \n"
		"-o sig_file        : output signature file (binary) \n",
#ifdef CONFIG_USE_SHA2
                        2
#else
                        3
#endif
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
	uint8_t sig[SIGNATURE_LEN];
	char *f_priv = NULL, *f_pub = NULL, *f_in;
	char *out_hash = "hash.bin";
	char *out_sig = "hash.sig";
	unsigned char *msg;
	int len;
	char c;
	int res;
	uint8_t h_val[64];

	while ((c = getopt(argc, argv, "p:b:s:h:o:")) != -1) {
		switch (c) {
		case 'p':
			f_priv = optarg;
			break;
		case 'b':
			f_pub = optarg;
			break;
		case 's':
			f_in = optarg;
			break;
		case 'h':
			out_hash = optarg;
			break;
		case 'o':
			out_sig = optarg;
			break;
		case '?':
		default:
			fprintf(stderr, "Unknown argument\n");
			usage(argv[0]);
			return -1;
		}
	}

	if (!f_priv || !f_pub || !f_in) {
		fprintf(stderr, "miss arguments\n");
		usage(argv[0]);
		return -1;
	}

	if (load_file_key_bin(private_key, KEY_LEN, f_priv)) {
		fprintf(stderr, "failed to load priv key from %s\n", f_priv);
		return -1;
	}

	if (load_file_key_bin(public_key, KEY_LEN, f_pub)) {
		fprintf(stderr, "failed to load public key from %s\n", f_priv);
		return -1;
	}

	memset(sig, 0, sizeof(sig));

	len = infile(f_in, &msg);
	if (len <= 0) {
		fprintf(stderr, "failed to load input file to sign!\n");
		return -1;
	}
	ed25519_sign(sig, private_key, public_key, msg, len);

	ed25519_hash(sig, msg, len, public_key, h_val);
	if (!ed25519_verify_hash(sig, public_key, h_val)) {
                fprintf(stderr, "Verify: failed. Please check your key pair!\n");
		return -1;
	}

	printf("Sign and verify signature: OK\n\n");

	//prn_dump("key_priv   : ", private_key, sizeof(private_key));
	prn_dump("key_pub    : ", public_key, sizeof(public_key));
	prn_dump("Hash       : ", h_val, HASH_LEN);
	prn_dump("Signature  : ", sig, SIGNATURE_LEN);

	printf("\nSave signature to file: %s\n", out_sig);
	if (save_bin(out_sig, sig, SIGNATURE_LEN)) {
		fprintf(stderr, "failed to write file %s\n", out_sig);
		return -1;
	}

	printf("\nSave hash to file: %s\n", out_hash);
	if (save_bin(out_hash, h_val, HASH_LEN)) {
		fprintf(stderr, "failed to write file %s\n", out_hash);
		return -1;
	}

	return 0;
}
