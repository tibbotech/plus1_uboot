#include <common.h>
#include "hw/host_hpi.h"
#include "hpi_cmd.h"
#include "secure_config.h"
#include "hw/sc_spacc.h"
#include "sp_otp.h"
#include "sw/libeddsa/lib/eddsa.h"
#include "sw/linux/crypto/sha3.h"  
#include "sw/wpa_supplicant/src/crypto/aes-gcm.h"  
#include <asm/arch/sp_bootinfo.h>

void prn_dump_buffer(unsigned char *buf, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		if (i && !(i & 0xf)) {
			printf(" \n");
		}
		printf("0x%x ",buf[i]);
	}
	puts(" \n");
}
static int load_otp_pub_key(unsigned char *buf, int otp_byte_off, int bytes)
{
	int i;
	
	for (i = 0; i < bytes; i++) {
		read_otp_data(KEY_HB_GP_REG, KEY_OTPRX_REG, i+otp_byte_off,(char *)&buf[i]);   
	}
	//puts("uboot  OTP pub-key:\n");
	//prn_dump_buffer(buf,bytes);
	return 0;
}

int SC_shaX_512(const void *message, size_t message_len, unsigned char h_val[64])
{
	if (!IS_IC_HSM_DISABLE())   
		return spacc_shaX_512(message, message_len, h_val);
	else 
		return sha3_512(message, message_len, h_val);
}

int SC_ed25519_hash(const unsigned char *signature,
		const void *message, size_t message_len,
		const unsigned char *public_key, unsigned char h_val[64])
{
	if (!IS_IC_HSM_DISABLE())   
		return spacc_ed25519_hash(signature, message, message_len, public_key, h_val);
	else {
		ed25519_hash(signature, message, message_len, public_key, h_val);
		return 0;
	}
}


int SC_aes_gcm_ad(const u8 *key, size_t key_len, const u8 *iv, size_t iv_len,
		const u8 *crypt, size_t crypt_len,
		const u8 *aad, size_t aad_len, const u8 *tag, u8 *plain)
{
	if (!IS_IC_HSM_DISABLE())   
		return spacc_aes_gcm_ad(key, key_len, iv, iv_len, crypt, crypt_len, aad, aad_len, tag, plain);
	else
		return aes_gcm_ad(key, key_len, iv, iv_len, crypt, crypt_len, aad, aad_len, tag, plain);
}

int SC_ed25519_verify_hash(const unsigned char *signature, const unsigned char *public_key,
		const unsigned char h_val[64])
{
	int res = 0;
	if (!IS_IC_HSM_DISABLE()) {
		res = hsm_ed25519_verify_hash(signature, public_key, h_val);
		return res;
	}
	/* inverted return value of original ed25519 function */
	res = !ed25519_verify_hash(signature, public_key, h_val);

	return res;
}

int SC_x25519(unsigned char *shared_secret, const unsigned char *public_key,
		const unsigned char *private_key)
{
	int res = 0;

	// (x, y) = dQ
	// d = private_key (scalar)
	// Q = public key (point)
	// x = shared_secret

	if (!IS_IC_HSM_DISABLE()) {
		res = hsm_x25519(shared_secret, private_key, public_key);
		return res;
	}

	x25519(shared_secret, private_key, public_key);

	return res;
}

int SC_key_otp_load(unsigned char *buf, int otp_byte_off, int bytes)
{

	if (!IS_IC_HSM_DISABLE()) {
		return hsm_key_otp_load(buf, otp_byte_off, bytes);
	}
	return load_otp_pub_key(buf, otp_byte_off, bytes);
}
