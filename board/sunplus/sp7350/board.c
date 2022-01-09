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
#define SP7350_RF_MASK_V_CLR(_mask)	(((_mask)<<16)| 0)

struct SP7350_moon0_regs {
	unsigned int stamp;             // 0.0
	unsigned int clken[5];          // 0.1 - 0.5
	unsigned int rsvd_1[5]; 	// 0.6 - 0.10
	unsigned int gclken[5];         // 0.11
	unsigned int rsvd_2[5]; 	// 0.16 - 0.20
	unsigned int reset[5];          // 0.21
	unsigned int rsvd_3[5];         // 0.26 - 030
	unsigned int hw_cfg;            // 0.31
};
#define SP7350_MOON0_REG ((volatile struct SP7350_moon0_regs *)SP7350_RF_GRP(0,0))

struct SP7350_moon1_regs{
	unsigned int sft_cfg[32];
};
#define SP7350_MOON1_REG ((volatile struct SP7350_moon1_regs *)SP7350_RF_GRP(1,0))

#define SP7350_MOON4_REG ((volatile struct SP7350_moon1_regs *)SP7350_RF_GRP(4,0))

enum Device_table{
	DEVICE_SPI_NAND = 0,
	DEVICE_MAX
};

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

void SetBootDev(unsigned int bootdev, unsigned int pin_x)
{
	switch(bootdev)
	{
#ifdef CONFIG_SP_SPINAND_Q645
		case DEVICE_SPI_NAND:
			/* module release reset pin */
			SP7350_MOON0_REG->reset[2] = SP7350_RF_MASK_V_CLR(3<<11);   // spi nand & bch
			/* nand pll level set */
			//SP7350_MOON4_REG->sft_cfg[27] |= (0x00040004);
			break;
#endif
		default:
			printf("unknowm \n");
			break;
	}
}

void board_nand_init(void)
{
#ifdef CONFIG_SP_SPINAND_Q645
	SetBootDev(DEVICE_SPI_NAND,1);
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