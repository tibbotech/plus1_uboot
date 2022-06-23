#ifndef __HOST_HPI_INC_H__
#define __HOST_HPI_INC_H__

#define HSM_TIMEOUT_MAX_MS   10000  // 10s tolerance for any ROM's HPI cmd

int host_hpi_read(uint32_t *pcmd, uint32_t *pm0, uint32_t *pm1, uint32_t timeout_ms);
int host_hpi_write(uint32_t cmd, uint32_t m0, uint32_t m1, uint32_t timeout_ms);

int hsm_ping(uint32_t data);

int hsm_ed25519_verify_hash(const unsigned char *signature, const unsigned char *public_key,
                const unsigned char h_val[64]);

int hsm_x25519(unsigned char *shared_secret, const unsigned char *private_key,
		const unsigned char *public_key);

int hsm_key_otp_load(unsigned char *buf, int otp_byte_off, int bytes);

int hsm_preload_key(uint32_t key_type, const unsigned char *src_key_buf, uint32_t key_len);

#endif /* __HOST_HPI_INC_H__ */
