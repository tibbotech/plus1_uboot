/*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <version.h>
#include <common.h>
#include <asm/global_data.h>

#ifdef CONFIG_SP_SPINAND_Q645
extern void board_spinand_init(void);
#endif

#define SP7350_REG_BASE			(0xf8000000)
#define SP7350_RF_GRP(_grp, _reg)	((((_grp)*32+(_reg))*4)+SP7350_REG_BASE)
#define SP7350_REG_BASE_AO		(0xf8800000)
#define SP7350_RF_GRP_AO(_grp, _reg)	((((_grp)*32+(_reg))*4)+SP7350_REG_BASE_AO)
#define SP7350_RF_MASK_V_CLR(_mask)	(((_mask)<<16)| 0)

struct SP7350_moon0_regs_ao {
	unsigned int stamp;            // 0.0
	unsigned int reset[12];        // 0.1 -  0.12
	unsigned int rsvd[18];         // 0.13 - 0.30
	unsigned int hw_cfg;           // 0.31
};
#define SP7350_MOON0_REG_AO ((volatile struct SP7350_moon0_regs_ao *)SP7350_RF_GRP_AO(0,0))

struct SP7350_moon1_regs_ao{
	unsigned int sft_cfg[32];
};
#define SP7350_MOON1_REG_AO ((volatile struct SP7350_moon1_regs *)SP7350_RF_GRP_AO(1,0))

struct SP7350_moon2_regs_ao {
	unsigned int rsvd1;            // 2.0
	unsigned int clken[12];        // 2.1 - 2.12
	unsigned int rsvd2[2];         // 2.13 - 2.14
	unsigned int gclken[12];       // 2.15 - 2.26
	unsigned int rsvd3[5];         // 2.27 - 2.31
};
#define SP7350_MOON2_REG_AO ((volatile struct SP7350_moon2_regs_ao *)RF_GRP_AO(2, 0))

struct SP7350_moon4_regs_ao{
	unsigned int sft_cfg[32];
};
#define SP7350_MOON4_REG_AO ((volatile struct SP7350_moon4_regs_ao *)SP7350_RF_GRP_AO(4,0))


DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	return 0;
}

int misc_init_r(void)
{
	return 0;
}

void board_nand_init(void)
{
#ifdef CONFIG_SP_SPINAND_Q645
	board_spinand_init();
#endif
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_DM_VIDEO
	sp7021_video_show_board_info();
#endif
	return 0;
}
#endif