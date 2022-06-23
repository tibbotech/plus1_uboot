#ifndef __WPASUP_BOOT_PORT_INC_H__
#define __WPASUP_BOOT_PORT_INC_H__

#if defined(__i386__) || defined(__x86_64__)
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <auto_config.h>
#include <types.h>
#else
#include <common.h>
#endif

#define SHA512_MAC_LEN 64

#define os_memset       memset
#define os_memcpy       memcpy
#define forced_memzero(_buf,_bytes) memset(_buf,0,_bytes)

int hmac_shaX_512_vector(const u8 *key, size_t key_len, size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac);
int hmac_shaX_512(const u8 *key, size_t key_len, const u8 *data, size_t data_len, u8 *mac);

int hmac_shaX_512_kdf(const u8 *secret, size_t secret_len, const char *label, const u8 *seed, size_t seed_len, u8 *out, size_t outlen);

#endif
