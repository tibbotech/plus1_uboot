#ifndef __HPI_CMD_INC_H__
#define __HPI_CMD_INC_H__

#define PACK_A4 __attribute__((packed)) __attribute__((aligned(4)))

enum ctrl_cmd  {

	// host send
	HROM_CMD_PING             = 0x370001,
	HROM_CMD_ED25519_V_H      = 0x370002,
	HROM_CMD_X25519           = 0x370003,
	HROM_CMD_KOTP_READ        = 0x370004,
	HROM_CMD_LOAD             = 0x370005,
	HROM_CMD_JUMP             = 0x370007,
	HROM_CMD_SEMC_START       = 0x370008,
	HROM_CMD_PRELOAD_KEY      = 0x370009,

	// hsm send
	HROM_CMD_REP_SUCCESS      = 0x870000,
	HROM_CMD_REP_FAIL         = 0x870001,
	HROM_CMD_REP_UNKNOWN      = 0x870002,

	// Only 24-bit CMD is available in HPI HOST_MB_CTRL and CPU_MB_CTRL
	HROM_CMD_END              = 0xFFFFFF,
};

struct hcmd_ed25519_v_h {
	uint32_t addr_sig;
	uint32_t addr_pk;
	uint32_t addr_hash;
} PACK_A4;

struct hcmd_x25519 {
	uint32_t addr_ss;
	uint32_t addr_pk;
	uint32_t addr_sk;
} PACK_A4;

struct hcmd_kotp_read {
	uint32_t addr_dst;
	uint32_t byte_off;
	uint32_t bytes;
} PACK_A4;

struct hcmd_load {
	uint32_t addr_dst;
	uint32_t addr_src;
	uint32_t addr_len;
} PACK_A4;

enum {
	HSMK_TYPE_ALL   = 0,
	HSMK_TYPE_DUK   = 1,
	HSMK_TYPE_BBR   = 2,
	HSMK_TYPE_APP0  = 3,
	HSMK_TYPE_APP1  = 4,
	HSMK_TYPE_ADC   = 5,
};

struct hcmd_preload_key {
	uint32_t hsmk_type;
	uint32_t key_len;
	uint32_t addr_src;
} PACK_A4;

struct hcmd_semc_boot {
	uint32_t aic_type;  // 0=bbr, 1=app0, 2=app1
	uint32_t code_size; // aic code size
	uint32_t aic_ibar;  // aic image internal address
	uint32_t aic_ebar;  // aic image external address
} PACK_A4;

int hpi_read(uint32_t *pcmd, uint32_t *pm0, uint32_t *pm1);
int hpi_write(uint32_t cmd, uint32_t m0, uint32_t m1);

#endif /* __HPI_CMD_INC_H__ */
