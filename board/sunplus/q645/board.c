/*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <version.h>
#include <common.h>


#ifdef CONFIG_SP_SPINAND
extern void board_spinand_init(void);
#endif

#define Q645_REG_BASE 				(0xf8000000)
#define Q645_RF_GRP(_grp, _reg)		((((_grp)*32+(_reg))*4)+Q645_REG_BASE)
#define Q645_RF_MASK_V_CLR(_mask)	(((_mask)<<16)| 0)

struct Q645_moon0_regs{
	unsigned int stamp;
	unsigned int clken[10];
	unsigned int gclken[10];
	unsigned int reset[10];
	unsigned int hw_cfg;
};
#define Q645_MOON0_REG ((volatile struct Q645_moon0_regs *)Q645_RF_GRP(0,0))

struct Q645_moon1_regs{
	unsigned int sft_cfg[32];
};
#define Q645_MOON1_REG ((volatile struct Q645_moon1_regs *)Q645_RF_GRP(1,0))

#define Q645_MOON4_REG ((volatile struct Q645_moon1_regs *)Q645_RF_GRP(4,0))
enum Device_table{
	DEVICE_SPI_NAND = 0,
	DEVICE_MAX
};

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int board_eth_init(bd_t *bis)
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
#ifdef CONFIG_SP_SPINAND
		case DEVICE_SPI_NAND:
			/* module release reset pin */
			Q645_MOON0_REG->reset[8]=Q645_RF_MASK_V_CLR(1<<10);//spi nand 
			Q645_MOON0_REG->reset[4]=Q645_RF_MASK_V_CLR(1<<4);//bch
			/* nand pll level set */
			Q645_MOON4_REG->sft_cfg[27] |= (0x00040004);
			break;
#endif			
		default:
			printf("unknowm \n");
			break;
	}
}

void board_nand_init(void)
{
#ifdef CONFIG_SP_SPINAND
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