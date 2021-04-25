#include <common.h>
#include "host_hpi.h"
#include "../hpi_cmd.h"
#include "../secure_config.h"
#include "../hsm/dss_config.h"

#ifdef prn_dword
#undef prn_dword
#endif
#define prn_dword(a)  printf("0x%x\n",(a))
//#define DEBUG

static unsigned int delta_ticks(unsigned int t_s, unsigned int t_e)
{
	if (t_e >= t_s)
		return t_e - t_s;

	return (0xffffffffU - t_s) + t_e + 1;
}

int host_hpi_read(uint32_t *pcmd, uint32_t *pm0, uint32_t *pm1, uint32_t timeout_ms)
{
	uint32_t got;
	uint32_t ts;

	printf("R\n");

#ifdef DEBUG
	printf("host_hpi_read@");
	prn_dword((int)&HPI_REG->cpu_mb_ctrl);
#endif

	// Wait for mb ownership
	ts = get_ticks();
	while ((HPI_REG->cpu_mb_own & 1) == 0) {
		if (timeout_ms && (delta_ticks(ts, get_ticks()) > (timeout_ms * TIMER_KHZ))) {
			printf("R timeout\n");
			return -1; // timeout
		}
		asm volatile ("nop;"); // optional
	}
	got = HPI_REG->cpu_mb_ctrl;

	if (pcmd)
		*pcmd = got;
	if (pm0)
		*pm0 = HPI_REG->cpu_mb_p[0];
	if (pm1)
		*pm1 = HPI_REG->cpu_mb_p[1];

#ifdef DEBUG
	prn_dword(got);
	if (pm0)
		prn_dword(*pm0);
	if (pm1)
		prn_dword(*pm1);
#endif

	// Return host_mb ownership
	HPI_REG->cpu_mb_own = 1;
#ifdef FAKE_HPI
	HPI_REG->cpu_mb_own = 0; // emulate W1C
#endif

#ifdef DEBUG
        printf("done\n");
#endif

	return 0;
}

int host_hpi_write(uint32_t cmd, uint32_t m0, uint32_t m1, uint32_t timeout_ms)
{
	uint32_t ts;

	printf("W");

#ifdef DEBUG
        printf("host_hpi_write@");
	prn_dword((int)&HPI_REG->host_mb_ctrl);
        prn_dword(cmd);
        prn_dword(m0);
        prn_dword(m1);
#endif

	HPI_REG->host_mb_p[0] = m0;
	HPI_REG->host_mb_p[1] = m1;

#ifdef FAKE_HPI
	HPI_REG->host_mb_own = 1; // emulate auto-set by writing ctrl
#endif
	HPI_REG->host_mb_ctrl = cmd;

	// Wait for mb ownership
	ts = get_ticks();
	while (HPI_REG->host_mb_own) {
		if (timeout_ms && (delta_ticks(ts, get_ticks()) > (timeout_ms * TIMER_KHZ))) {
			printf("W timeout\n");
			return -1; // timeout
		}
		asm volatile ("nop;"); // optional
	}

#ifdef DEBUG
	printf("done\n");
#endif

	return 0;
}

/* convert ram view : arm -> hsm
 *  - cbdma sram
 *  - dram
 */
static uint32_t arm2hsm_view(const void *addr)
{
	uint32_t caddr, hview;
#if (DRAM_MAX_SIZE != 0x80000000U)   
	volatile unsigned int *reg;
	int i;
#endif

	// only 32b address is concerned in our SOC
	caddr = (uint64_t)addr;

	// default to original view
	hview = caddr;
#if (DRAM_MAX_SIZE == 0x80000000U)   //DRAM=2GB case
	if (caddr >= CB_SRAM0_BASE && caddr < CB_SRAM0_END) {
		// cbdma sram
		hview = (caddr & ~0xfff00000U) | (0x3a200000U); // hsm cbsram
	} else if (caddr < 0x80000000) { // boot coe's dram VA (mmu off)
		hview = caddr + 0x50000000U; // hsm dram
	} else if (caddr >= 0xC0000000U) { // boot code's dram VA (mmu on)
		hview = (caddr - 0xC0000000U) + 0x50000000U; // hsm dram
	} else {
		printf("err: no conv arm2hsm=");
		prn_dword((uint64_t)addr);
	}
#else   //DRAM reach 3.5GB case
	if (caddr >= CB_SRAM0_BASE && caddr < CB_SRAM0_END) {
		// cbdma sram
		hview = (caddr & ~0xfff00000U) | (0x3a200000U); // hsm cbsram
	} else if (caddr < 0x80000000) { // boot coe's dram VA (mmu off)
		hview = caddr + 0x50000000U; // hsm dram
	  for (i = 5; i <= 0xC; i++) {
		  reg = (void *)DSS_SYSMST_REG_BASE + (i * 4); // SYS_REG i
		  *reg = (i - 5) << 28;
	  }
	} else if (caddr < 0xE0000000) { // boot coe's dram VA (mmu off)
		hview = (caddr - 0x80000000U) + 0x50000000U; // hsm dram
	  for (i = 5; i <= 0xA; i++) {
		  reg = (void *)DSS_SYSMST_REG_BASE + (i * 4); // SYS_REG i
		  *reg = (i - 5 + 8) << 28;
	  }
	} else if (caddr >= 0xC0000000U) { // boot code's dram VA (mmu on)
		hview = (caddr - 0xC0000000U) + 0x50000000U; // hsm dram
	} else {
		printf("err: no conv arm2hsm=%p",addr);
	}
#endif
	return hview;
}

int hsm_ping(uint32_t data)
{
	uint32_t cmd = 0;
	uint32_t got = 0;
	int res;

	// trigger
	res = host_hpi_write(HROM_CMD_PING, data, 0, 100); // hsm should be up within 100ms
	if (res) {

		return res;
	}

	// wait result
	res = host_hpi_read(&cmd, &got, 0, 100);
	if (res) {

		return res;
	}

	if (got != data) {
		printf("bad data="); prn_dword(got);
		return -1;
	}

	if (cmd == HROM_CMD_REP_SUCCESS)
		return 0;

	return cmd;
}

int hsm_ed25519_verify_hash(const unsigned char *signature, const unsigned char *public_key,
		const unsigned char h_val[64])
{
	struct hcmd_ed25519_v_h hd;
	uint32_t hview;
	uint32_t cmd = 0;
	int res;

	hd.addr_sig   = arm2hsm_view(signature);
	hd.addr_pk    = arm2hsm_view(public_key);
	hd.addr_hash  = arm2hsm_view(h_val);

	hview = arm2hsm_view(&hd);

	flush_dcache_all();

	// trigger
	res = host_hpi_write(HROM_CMD_ED25519_V_H, hview, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	// wait result
	res = host_hpi_read(&cmd, 0, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	if (cmd == HROM_CMD_REP_SUCCESS)
		return 0;

	printf("hsm ed25519 err=");
	prn_dword(cmd);

	return cmd;
}

int hsm_x25519(unsigned char *shared_secret, const unsigned char *private_key,
		const unsigned char *public_key)
{
	struct hcmd_x25519 hd;
	uint32_t hview;
	uint32_t cmd = 0;
	int res;

	hd.addr_ss    = arm2hsm_view(shared_secret);
	hd.addr_pk    = arm2hsm_view(public_key);
	hd.addr_sk    = arm2hsm_view(private_key);

	hview = arm2hsm_view(&hd);

	flush_dcache_all();

	// trigger
	res = host_hpi_write(HROM_CMD_X25519, hview, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	// wait result
	res = host_hpi_read(&cmd, 0, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	flush_dcache_all();

	if (cmd == HROM_CMD_REP_SUCCESS)
		return 0;

	printf("hsm x25519 err=%d ",cmd);
	return -1;
}

int hsm_load(u32 dst_hview, u32 src_hview, u32 len)
{
	uint32_t hview;
	uint32_t cmd = 0;
	struct hcmd_load hd;
	int res;

	hd.addr_dst = dst_hview;
	hd.addr_src = src_hview;
	hd.addr_len = len;

	hview = arm2hsm_view(&hd);

	flush_dcache_all();

	// trigger
	res = host_hpi_write(HROM_CMD_LOAD, hview, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	// wait result
	res = host_hpi_read(&cmd, 0, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	if (cmd == HROM_CMD_REP_SUCCESS)
		return 0;

	printf("hsm load err=%d",cmd);
	return -1;
}

int hsm_jump(u32 dst_hview)
{
	uint32_t cmd = 0;
	int res;

	flush_dcache_all();

	// trigger
	res = host_hpi_write(HROM_CMD_JUMP, dst_hview, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	// wait result
	res = host_hpi_read(&cmd, 0, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	if (cmd == HROM_CMD_REP_SUCCESS)
		return 0;

	printf("hsm jump err=%d",cmd);
	return -1;
}

int hsm_key_otp_load(unsigned char *buf, int otp_byte_off, int bytes)
{
	uint32_t hview;
	uint32_t cmd = 0;
	struct hcmd_kotp_read hd;
	int res;

	hd.addr_dst = arm2hsm_view(buf);
	hd.byte_off = otp_byte_off;
	hd.bytes = bytes;

	hview = arm2hsm_view(&hd);

	flush_dcache_all();

	// trigger
	res = host_hpi_write(HROM_CMD_KOTP_READ, hview, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	// wait result
	res = host_hpi_read(&cmd, 0, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	flush_dcache_all();

	if (cmd == HROM_CMD_REP_SUCCESS)
		return 0;

	printf("hsm kotp err=%d",cmd);
	return -1;
}

int hsm_preload_key(uint32_t key_type, const unsigned char *src_key_buf, uint32_t key_len)
{
	uint32_t hview;
	uint32_t cmd = 0;
	struct hcmd_preload_key hd;
	int res;

	hd.hsmk_type = key_type;
	hd.key_len = key_len;
	hd.addr_src = arm2hsm_view(src_key_buf);

	hview = arm2hsm_view(&hd);

	flush_dcache_all();

	// trigger
	res = host_hpi_write(HROM_CMD_PRELOAD_KEY, hview, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	// wait result
	res = host_hpi_read(&cmd, 0, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	if (cmd == HROM_CMD_REP_SUCCESS)
		return 0;

	printf("hsm preload key err=%d",cmd);
	return -1;
}

int hsm_semc_start(uint32_t aic_type, uint32_t code_size, uint32_t aic_ibar, uint32_t aic_ebar)
{
	uint32_t hview;
	uint32_t cmd = 0;
	struct hcmd_semc_boot hd;
	int res;

	hd.aic_type = aic_type;
	hd.code_size = code_size;
	hd.aic_ibar = aic_ibar;
	hd.aic_ebar = aic_ebar;

	hview = arm2hsm_view(&hd);

	flush_dcache_all();

	// trigger
	res = host_hpi_write(HROM_CMD_SEMC_START, hview, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	// wait result
	res = host_hpi_read(&cmd, 0, 0, HSM_TIMEOUT_MAX_MS);
	if (res) {

		return res;
	}

	if (cmd == HROM_CMD_REP_SUCCESS)
		return 0;

	printf("hsm semc boot err=%d",cmd);
	return -1;
}
