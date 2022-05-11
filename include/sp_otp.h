#ifndef __SP_OTP_H
#define __SP_OTP_H
#include <common.h>

#if defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#define REG_BASE           0xF8000000
#elif (defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)) || \
		(defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C))
#define REG_BASE           0x9c000000
#endif
#define RF_GRP(_grp, _reg) ((((_grp) * 32 + (_reg)) * 4) + REG_BASE)

struct moon2_otp_regs {
	unsigned int sft_cfg[32];
};
#define MOON2_OTP_REG ((volatile struct moon2_otp_regs *)RF_GRP(2, 0))

struct hb_gp_regs {
	u32 hb_gpio_rgst_bus32[13];
};
#define HB_GP_REG ((volatile struct hb_gp_regs *)RF_GRP(350, 0))
#define KEY_HB_GP_REG ((volatile struct hb_gp_regs *)RF_GRP(779, 0))
#define CUSTOMER_HB_GP_REG ((volatile struct hb_gp_regs *)RF_GRP(79, 0))

struct otprx_regs {
	u32 sw_trim;
	u32 set_key;
	u32 otp_rsv;
	u32 otp_prog_ctl;
	u32 otp_prog_addr;
	u32 otp_prog_csb;
	u32 otp_prog_strobe;
	u32 otp_prog_load;
	u32 otp_prog_pgenb;
	u32 otp_prog_wr;
	u32 otp_prog_reg25;
	u32 otp_prog_state;
	u32 otp_usb_phy_trim;
	u32 otp_data2;
	u32 otp_pro_ps;
	u32 otp_rsv2;
	u32 key_srst;
	u32 otp_ctrl;
	u32 otp_cmd;
	u32 otp_cmd_status;
	u32 otp_addr;
	u32 otp_data;
};
#define SP_OTPRX_REG    ((volatile struct otprx_regs *)RF_GRP(351, 0))
#define KEY_OTPRX_REG   ((volatile struct otprx_regs *)RF_GRP(780, 0))
#define CUSTOMER_OTPRX_REG   ((volatile struct otprx_regs *)RF_GRP(80, 0))

/*
 * OTP memory
 * Each bank contains 4 words (4 * 32 bits).
 * Bank 0 starts at offset 0 from the base.
 *
 */
#define OTP_WORDS_PER_BANK		4
#define OTP_WORD_SIZE			sizeof(u32)
#define OTP_BIT_ADDR_OF_BANK		(8 * OTP_WORD_SIZE * \
						OTP_WORDS_PER_BANK)

#define QAC628_EFUSE_SIZE               128
#define I143_EFUSE_SIZE			128
#define QAK645_EFUSE0_SIZE		128
#define QAK645_EFUSE1_SIZE		1024
#define QAK645_EFUSE2_SIZE		128
#define QAK654_EFUSE_SIZE		128

#define OTP_READ_TIMEOUT                20000
#define OTP_WAIT_MICRO_SECONDS          100

/* OTP register map */
#define OTP_PROGRAM_CONTROL             0x0C
#define PIO_MODE                        0x07

#define OTP_PROGRAM_ADDRESS             0x10
#define PROG_OTP_ADDR                   0x1FFF

#define OTP_PROGRAM_PGENB               0x20
#define PIO_PGENB                       0x01

#define OTP_PROGRAM_ENABLE              0x24
#define PIO_WR                          0x01

#define OTP_PROGRAM_VDD2P5              0x28
#define PROGRAM_OTP_DATA                0xFF00
#define PROGRAM_OTP_DATA_SHIFT          8
#define REG25_PD_MODE_SEL               0x10
#define REG25_POWER_SOURCE_SEL          0x02
#define OTP2REG_PD_N_P                  0x01

#define OTP_PROGRAM_STATE               0x2C
#define OTPRSV_CMD3                     0xE0
#define OTPRSV_CMD3_SHIFT               5
#define TSMC_OTP_STATE                  0x1F

#define OTP_CONTROL                     0x44
#define PROG_OTP_PERIOD                 0xFFE0
#define PROG_OTP_PERIOD_SHIFT           5
#define OTP_EN_R                        0x01

#define OTP_CONTROL2                    0x48
#define OTP_RD_PERIOD                   0xFF00
#define OTP_RD_PERIOD_SHIFT             8
#define OTP_READ                        0x04

#define OTP_STATUS                      0x4C
#define OTP_READ_DONE                   0x10

#define OTP_READ_ADDRESS                0x50
#define RD_OTP_ADDRESS                  0x1F

int read_otp_data(volatile struct hb_gp_regs *otp_data, volatile struct otprx_regs *regs, int addr, char *value);

#endif /* __SP_OTP_H */

