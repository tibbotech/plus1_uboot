#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include "eddsa.h"
#include <auto_config.h>

#define KEY_LEN 32

#define SAVE_AS_HEX
#define SAVE_AS_BIN
#define SAVE_AS_C_INC

#define SAVE_PUB_AS_OTP_CSV

//#define PRINT_PRIVATE_KEY // define to print private key

void vli_print(uint8_t *p_vli, int len)
{
	unsigned i;
	for (i = 0; i < len - 1; ++i) {
		printf("0x%02X, ", (unsigned)p_vli[i]);
	}
	printf("0x%02X", (unsigned)p_vli[i]);
}

int randfd;

void getRandomBytes(void *p_dest, unsigned p_size)
{
	if(read(randfd, p_dest, p_size) != (int)p_size) {
		printf("Failed to get random bytes.\n");
		exit(1);
	}
}

#define TEST_COUNT_PER_KEY 10

/* return 0 if ok */
static int test_key_pair(int count, uint8_t public_key[KEY_LEN], uint8_t private_key[KEY_LEN])
{
	int i;
	const int len = 128;
	uint8_t msg[len];
	uint8_t sig[64];
	uint8_t h_val[64];

	for(i=0; i < count; ++i) {
		getRandomBytes(msg, sizeof(msg));

                ed25519_sign(sig, private_key, public_key, msg, len);

		ed25519_hash(sig, msg, len, public_key, h_val);
		if (!ed25519_verify_hash(sig, public_key, h_val))
			return -1;
	}

	return 0;
}

static int save_key(char *keyname, uint8_t *key, int size)
{
	int fd, res, i;
	char fname[512];

	if (size < 1) {
		fprintf(stderr, "empty key size\n");
		return -1;
	}

#ifdef SAVE_AS_HEX
	sprintf(fname, "keys/%s.hex", keyname);
	fd = open(fname, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		fprintf(stderr, "can't open file %s\n", fname);
		return -1;
	}

	for(i = 0; i < size; ++i) {
		dprintf(fd, "%02X", (unsigned)key[i]);
	}
	close(fd);
	printf("Generated: %s\n", fname);
#endif

#ifdef SAVE_AS_BIN
	sprintf(fname, "keys/%s.bin", keyname);
	fd = open(fname, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		fprintf(stderr, "can't open file %s\n", fname);
		return -1;
	}
	res = write(fd, key, size);
	if (res != size) {
		fprintf(stderr, "can't write %s, size=%d, return %d\n", fname, size, res);
		return -1;
	}
	close(fd);
	printf("Generated: %s\n", fname);
#endif

#ifdef SAVE_AS_C_INC
	sprintf(fname, "keys/%s.inc", keyname);
	fd = open(fname, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		fprintf(stderr, "can't open file %s\n", fname);
		return -1;
	}

	dprintf(fd, "const uint8_t %s[%u] = { ", keyname, size);
	for(i = 0; i < size - 1; ++i) {
		dprintf(fd, "0x%02X, ", (unsigned)key[i]);
	}
	dprintf(fd, "0x%02X };\n", (unsigned)key[i]);
	close(fd);
	printf("Generated: %s\n", fname);
#endif

	return 0;
}

#ifdef SAVE_PUB_AS_OTP_CSV
static int save_pub_key_otp_csv(const char *keyname, uint8_t *key, int size)
{
	int fd, res, i;
	char fname[512];

	if (size < 1) {
		fprintf(stderr, "empty key size\n");
		return -1;
	}

	sprintf(fname, "keys/%s", keyname);
	fd = open(fname, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		fprintf(stderr, "can't open file %s\n", fname);
		return -1;
	}

	// q642 OTP[SB_KPUB] = G779.0~7 (256 bits)
	for(i = 0; i < size; ++i) {
		dprintf(fd, "%d,%u\n", (0 * 4) + i, (unsigned)key[i]);
	}
	close(fd);
	printf("Generated OTP: %s\n", fname);
}
#endif

int main(int argc, char **argv)
{
	unsigned char public_key[KEY_LEN];
	unsigned char private_key[KEY_LEN];
	unsigned char signature[64];
	unsigned l_num = 1;
	unsigned i;
	char keyname[256];
	int res;
	struct stat st = {0};

	if(argc > 1) {
		l_num = strtoul(argv[1], NULL, 10);
	}

	randfd = open("/dev/urandom", O_RDONLY);
	if(randfd == -1) {
		printf("No access to urandom\n");
		return -1;
	}

	if (stat("keys", &st) == -1) {
		res = mkdir("keys", 0700);
		if (res < 0) {
			fprintf(stderr, "can't create dir for keys\n");
			return -1;
		}
	}

	for(i = 0; i < l_num; ++i) {
		getRandomBytes(private_key, KEY_LEN);

		ed25519_genpub(public_key, private_key);

		if (test_key_pair(TEST_COUNT_PER_KEY, public_key, private_key)) {
			fprintf(stderr, "WARN: key_%u test error! Re-generate key pair.\n", i);
			i--;
			continue;
		}

		printf("Test ed25519-sha%d key pair: OK\n",
#ifdef CONFIG_USE_SHA2
			2
#else
			3
#endif
			);

#ifdef PRINT_PRIVATE_KEY
		printf("uint8_t priv_%u[%u] = { ", i, KEY_LEN);
		vli_print(private_key, KEY_LEN);
		printf("};\n");
#endif

		printf("uint8_t pub_%u[%u] = { ", i, KEY_LEN);
		vli_print(public_key, KEY_LEN);
		printf("};\n\n");

		sprintf(keyname, "ed_priv_%u", i);
		save_key(keyname, private_key, KEY_LEN);
		sprintf(keyname, "ed_pub_%u", i);
		save_key(keyname, public_key, KEY_LEN);

#ifdef SAVE_PUB_AS_OTP_CSV
		sprintf(keyname, "ed_pub_%d.csv", i);
		save_pub_key_otp_csv(keyname, public_key, KEY_LEN);
#endif
	}

	return 0;
}
