// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Sunplus, Inc.
 *
 * DWC3 controller driver
 *
 * Author: ChingChouHuang<chingchou.huangh@sunplus.com>
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <log.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <linux/delay.h>
#include <asm/gpio.h>
#include <usb/xhci.h>
#include <asm/io.h>
#include <linux/usb/dwc3.h>
#include <linux/usb/otg.h>

struct xhci_dwc3_plat {
	struct phy_bulk phys;
};
#if 1
#define RF_GRP(_grp, _reg) ((((_grp) * 32 + (_reg)) * 4) + REG_BASE)
#define RF_AMBA(_grp, _reg) ((((_grp) * 1024 + (_reg)) * 4) + REG_BASE)

#define RF_MASK_V(_mask, _val)       (((_mask) << 16) | (_val))
#define RF_MASK_V_SET(_mask)         (((_mask) << 16) | (_mask))
#define RF_MASK_V_CLR(_mask)         (((_mask) << 16) | 0)
#if defined(CONFIG_TARGET_PENTAGRAM_Q645) ||  defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#define REG_BASE           0xF8000000
struct moon0_regs {
	unsigned int stamp;            // 0.0
	unsigned int clken[5];         // 0.1 - 0.5
	unsigned int rsvd_1[5]; 	   	   // 0.6 - 0.10
	unsigned int gclken[5];        // 0.11
	unsigned int rsvd_2[5]; 	   	   // 0.16 - 0.20
	unsigned int reset[5];         // 0.21
	unsigned int rsvd_3[5];          // 0.26 - 030
	unsigned int hw_cfg;           // 0.31
};
#define MOON0_REG ((volatile struct moon0_regs *)RF_GRP(0, 0))

/* start of xhci */
struct uphy_u3_regs {
	unsigned int cfg[32];		       // 189.0
};

struct gpio_desc *gpiodir;
struct udevice *phydev;

#define UPHY0_U3_REG ((volatile struct uphy_u3_regs *)RF_AMBA(189, 0))
#define UPHY1_U3_REG ((volatile struct uphy_u3_regs *)RF_AMBA(190, 0))
#elif defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C)
#define REG_BASE           0x9c000000

struct uphy_rn_regs {
	u32 cfg[28];		       // 150.0
	u32 gctrl[3];		       // 150.28
	u32 gsts;		       // 150.31
};

struct uphy_u3_regs {
	u32 cfg[160];		       // 265.0
};


#define UPHY2_RN_REG ((volatile struct uphy_rn_regs *)RF_GRP(151, 0))
#define UPHY3_U3_REG ((volatile struct uphy_u3_regs *)RF_AMBA(265, 0))

struct moon0_regs {
	unsigned int stamp;            // 0.0
	unsigned int clken[10];        // 0.1
	unsigned int gclken[10];       // 0.11
	unsigned int reset[10];        // 0.21
	unsigned int hw_cfg;           // 0.31
};
#define MOON0_REG ((volatile struct moon0_regs *)RF_GRP(0, 0))

struct moon1_regs {
	unsigned int sft_cfg[32];
};
#define MOON1_REG ((volatile struct moon1_regs *)RF_GRP(1, 0))

struct moon2_regs {
	unsigned int sft_cfg[32];
};
#define MOON2_REG ((volatile struct moon2_regs *)RF_GRP(2, 0))

struct moon3_regs {
	unsigned int sft_cfg[32];
};
#define MOON3_REG ((volatile struct moon3_regs *)RF_GRP(3, 0))

struct moon4_regs {
	unsigned int pllsp_ctl[7];	// 4.0
	unsigned int plla_ctl[5];	// 4.7
	unsigned int plle_ctl;		// 4.12
	unsigned int pllf_ctl;		// 4.13
	unsigned int plltv_ctl[3];	// 4.14
	unsigned int usbc_ctl;		// 4.17
	unsigned int uphy0_ctl[4];	// 4.18
	unsigned int uphy1_ctl[4];	// 4.22
	unsigned int pllsys;		// 4.26
	unsigned int clk_sel0;		// 4.27
	unsigned int probe_sel;		// 4.28
	unsigned int misc_ctl_0;	// 4.29
	unsigned int uphy0_sts;		// 4.30
	unsigned int otp_st;		// 4.31
};
#define MOON4_REG ((volatile struct moon4_regs *)RF_GRP(4, 0))

struct moon5_regs {
	unsigned int sft_cfg[32];
};
#define MOON5_REG ((volatile struct moon5_regs *)RF_GRP(5, 0))
#endif
static void uphy_init(void)
{
#if defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	volatile struct uphy_u3_regs *dwc3phy_reg;
	u32 result, i = 0;

	MOON0_REG->clken[3] = RF_MASK_V_SET(1 << 11);

	MOON0_REG->reset[3] = RF_MASK_V_SET(1 << 9);
	MOON0_REG->reset[3] = RF_MASK_V_SET(1 << 11);
	mdelay(1);
	MOON0_REG->reset[3] = RF_MASK_V_CLR(1 << 9);
	MOON0_REG->reset[3] = RF_MASK_V_CLR(1 << 11);

	dwc3phy_reg = (volatile struct uphy_u3_regs *) UPHY0_U3_REG;
	dwc3phy_reg->cfg[1] |= 0x03;
	for (;;)
	{
		result = dwc3phy_reg->cfg[2] & 0x3;
		if (result == 0x01)
			break;

		if (i++ > 10) {
			debug("PHY0_TIMEOUT_ERR0 ");
			i = 0;
			break;
		}
		mdelay(1);
	}

	dwc3phy_reg->cfg[2] |= 0x01;
	if (dm_gpio_get_value(gpiodir))
		dwc3phy_reg->cfg[5] = (dwc3phy_reg->cfg[5] & 0xFFE0) | 0x15;
	else
		dwc3phy_reg->cfg[5] = (dwc3phy_reg->cfg[5] & 0xFFE0) | 0x11;

	for (;;)
	{
		result = dwc3phy_reg->cfg[2] & 0x3;
		if (result == 0x01)
			break;

		if (i++ > 10) {
			debug("PHY0_TIMEOUT_ERR1 ");
			i = 0;
			break;
		}
		mdelay(1);
	}

#elif defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C)
// 1. enable UPHY 2/3 */
	MOON0_REG->clken[2] = RF_MASK_V_SET(1 << 15);
	MOON0_REG->clken[2] = RF_MASK_V_SET(1 << 9);

	mdelay(1);

	// 2. reset UPHY 2/3
	MOON0_REG->reset[2] = RF_MASK_V_SET(1 << 15);
	MOON0_REG->reset[2] = RF_MASK_V_SET(1 << 9);
	mdelay(1);
	MOON0_REG->reset[2] = RF_MASK_V_CLR(1 << 15);
	MOON0_REG->reset[2] = RF_MASK_V_CLR(1 << 9);

	mdelay(1);

	// 3. Default value modification
	UPHY2_RN_REG->gctrl[0] = 0x18888002;

	mdelay(1);

	// 4. PLL power off/on twice
	UPHY2_RN_REG->gctrl[2] = 0x88;
	mdelay(1);
	UPHY2_RN_REG->gctrl[2] = 0x80;
	mdelay(1);
	UPHY2_RN_REG->gctrl[2] = 0x88;
	mdelay(1);
	UPHY2_RN_REG->gctrl[2] = 0x80;;
	mdelay(20);
	UPHY2_RN_REG->gctrl[2] = 0x0;

	// 5. USBC 0/1 reset
	//MOON0_REG->reset[2] = RF_MASK_V_SET(1 << 12);
	//mdelay(1);
	//MOON0_REG->reset[2] = RF_MASK_V_CLR(1 << 12);
	//mdelay(1);

	// 6. HW workaround
	UPHY2_RN_REG->cfg[19] |= 0x0f;

	// 7. USB DISC (disconnect voltage)
	UPHY2_RN_REG->cfg[7] = 0x8b;

	// 8. RX SQUELCH LEVEL
	UPHY2_RN_REG->cfg[25] = 0x4;

	//UPHY2_RN_REG->cfg[16] = 0x19;
	//UPHY2_RN_REG->cfg[17] = 0x92;
	//UPHY2_RN_REG->cfg[3]  = 0x17;
	//U3 phy settings
	UPHY3_U3_REG->cfg[0]   = 0x43;
	UPHY3_U3_REG->cfg[11]  = 0x21;
	UPHY3_U3_REG->cfg[13]  = 0x5;
	UPHY3_U3_REG->cfg[17]  = 0x1f;
	UPHY3_U3_REG->cfg[18]  = 0x0;
	UPHY3_U3_REG->cfg[95]  = 0x33;
	UPHY3_U3_REG->cfg[112] = 0x11;
	UPHY3_U3_REG->cfg[113] = 0x0;
	UPHY3_U3_REG->cfg[114] = 0x1;
	UPHY3_U3_REG->cfg[115] = 0x0;
	UPHY3_U3_REG->cfg[128] = 0x9;
	//eq settings
	UPHY3_U3_REG->cfg[75]  = 0x0;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x1f;
	UPHY3_U3_REG->cfg[79]  = 0x0e;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x1;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x1f;
	UPHY3_U3_REG->cfg[79]  = 0x0e;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x2;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x4f;
	UPHY3_U3_REG->cfg[79]  = 0x0e;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x3;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x4f;
	UPHY3_U3_REG->cfg[79]  = 0x0e;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x4;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x7f;
	UPHY3_U3_REG->cfg[79]  = 0x0e;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x5;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x7f;
	UPHY3_U3_REG->cfg[79]  = 0x0e;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x6;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xaf;
	UPHY3_U3_REG->cfg[79]  = 0x0e;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x7;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xaf;
	UPHY3_U3_REG->cfg[79]  = 0x0e;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x8;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xdf;
	UPHY3_U3_REG->cfg[79]  = 0x0e;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x9;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xdf;
	UPHY3_U3_REG->cfg[79]  = 0x0e;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0xa;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x0f;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0xb;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x0f;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0xc;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x3f;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0xd;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x3f;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0xe;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x6f;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0xf;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x6f;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x10;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x9f;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x11;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0x9f;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x12;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xcf;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x13;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xcf;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x14;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xef;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x15;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xef;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x16;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x30;
	UPHY3_U3_REG->cfg[78]  = 0xef;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x17;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x30;
	UPHY3_U3_REG->cfg[78]  = 0xef;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x18;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x70;
	UPHY3_U3_REG->cfg[78]  = 0xee;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x19;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x70;
	UPHY3_U3_REG->cfg[78]  = 0xee;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x1a;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xb0;
	UPHY3_U3_REG->cfg[78]  = 0xed;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x1b;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xb0;
	UPHY3_U3_REG->cfg[78]  = 0xed;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x1c;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xec;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x1d;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xec;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x1e;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x30;
	UPHY3_U3_REG->cfg[78]  = 0xec;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x1f;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x30;
	UPHY3_U3_REG->cfg[78]  = 0xec;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x20;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x70;
	UPHY3_U3_REG->cfg[78]  = 0xeb;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x21;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x70;
	UPHY3_U3_REG->cfg[78]  = 0xeb;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x22;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xb0;
	UPHY3_U3_REG->cfg[78]  = 0xea;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x23;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xb0;
	UPHY3_U3_REG->cfg[78]  = 0xea;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x24;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xe9;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x25;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xe9;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x26;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x30;
	UPHY3_U3_REG->cfg[78]  = 0xe9;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x27;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x30;
	UPHY3_U3_REG->cfg[78]  = 0xe9;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x28;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x70;
	UPHY3_U3_REG->cfg[78]  = 0xe8;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x29;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x70;
	UPHY3_U3_REG->cfg[78]  = 0xe8;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x2a;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xb0;
	UPHY3_U3_REG->cfg[78]  = 0xe7;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x2b;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xb0;
	UPHY3_U3_REG->cfg[78]  = 0xe7;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x2c;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xe6;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x2d;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xe6;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x2e;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x30;
	UPHY3_U3_REG->cfg[78]  = 0xe6;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x2f;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x30;
	UPHY3_U3_REG->cfg[78]  = 0xe6;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x30;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x70;
	UPHY3_U3_REG->cfg[78]  = 0xe5;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x31;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x70;
	UPHY3_U3_REG->cfg[78]  = 0xe5;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x32;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xb0;
	UPHY3_U3_REG->cfg[78]  = 0xe4;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x33;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xb0;
	UPHY3_U3_REG->cfg[78]  = 0xe4;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x34;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xe3;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x35;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xe3;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x36;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x30;
	UPHY3_U3_REG->cfg[78]  = 0xe3;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x37;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x30;
	UPHY3_U3_REG->cfg[78]  = 0xe3;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x38;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x70;
	UPHY3_U3_REG->cfg[78]  = 0xe2;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x39;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x70;
	UPHY3_U3_REG->cfg[78]  = 0xe2;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x3a;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xb0;
	UPHY3_U3_REG->cfg[78]  = 0xe1;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x3b;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xb0;
	UPHY3_U3_REG->cfg[78]  = 0xe1;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x3c;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xe0;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x3d;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0xf0;
	UPHY3_U3_REG->cfg[78]  = 0xe0;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x3e;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x30;
	UPHY3_U3_REG->cfg[78]  = 0xe0;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;

	UPHY3_U3_REG->cfg[80]  = 0x00;
	UPHY3_U3_REG->cfg[75]  = 0x3f;
	UPHY3_U3_REG->cfg[76]  = 0xbe;
	UPHY3_U3_REG->cfg[77]  = 0x30;
	UPHY3_U3_REG->cfg[78]  = 0xe0;
	UPHY3_U3_REG->cfg[79]  = 0x0f;
	UPHY3_U3_REG->cfg[80]  = 0x01;
#endif
}
#endif

void dwc3_set_mode(struct dwc3 *dwc3_reg, u32 mode)
{
	clrsetbits_le32(&dwc3_reg->g_ctl,
			DWC3_GCTL_PRTCAPDIR(DWC3_GCTL_PRTCAP_OTG),
			DWC3_GCTL_PRTCAPDIR(mode));
}

static void dwc3_phy_reset(struct dwc3 *dwc3_reg)
{
	/* Assert USB3 PHY reset */
	setbits_le32(&dwc3_reg->g_usb3pipectl[0], DWC3_GUSB3PIPECTL_PHYSOFTRST);

	/* Assert USB2 PHY reset */
	setbits_le32(&dwc3_reg->g_usb2phycfg, DWC3_GUSB2PHYCFG_PHYSOFTRST);

	mdelay(100);

	/* Clear USB3 PHY reset */
	clrbits_le32(&dwc3_reg->g_usb3pipectl[0], DWC3_GUSB3PIPECTL_PHYSOFTRST);

	/* Clear USB2 PHY reset */
	clrbits_le32(&dwc3_reg->g_usb2phycfg, DWC3_GUSB2PHYCFG_PHYSOFTRST);
}

void dwc3_core_soft_reset(struct dwc3 *dwc3_reg)
{
	/* Before Resetting PHY, put Core in Reset */
	setbits_le32(&dwc3_reg->g_ctl, DWC3_GCTL_CORESOFTRESET);

	/* reset USB3 phy - if required */
	dwc3_phy_reset(dwc3_reg);

	mdelay(100);

	/* After PHYs are stable we can take Core out of reset state */
	clrbits_le32(&dwc3_reg->g_ctl, DWC3_GCTL_CORESOFTRESET);
}

int dwc3_core_init(struct dwc3 *dwc3_reg)
{
	u32 reg;
	u32 revision;
	unsigned int dwc3_hwparams1;

	revision = readl(&dwc3_reg->g_snpsid);
	/* This should read as U3 followed by revision number */
	if ((revision & DWC3_GSNPSID_MASK) != 0x55330000) {
		puts("this is not a DesignWare USB3 DRD Core\n");
		return -1;
	}

	dwc3_core_soft_reset(dwc3_reg);

	dwc3_hwparams1 = readl(&dwc3_reg->g_hwparams1);

	reg = readl(&dwc3_reg->g_ctl);
	reg &= ~DWC3_GCTL_SCALEDOWN_MASK;
	reg &= ~DWC3_GCTL_DISSCRAMBLE;
	switch (DWC3_GHWPARAMS1_EN_PWROPT(dwc3_hwparams1)) {
	case DWC3_GHWPARAMS1_EN_PWROPT_CLK:
		reg &= ~DWC3_GCTL_DSBLCLKGTNG;
		break;
	default:
		debug("No power optimization available\n");
	}

	/*
	 * WORKAROUND: DWC3 revisions <1.90a have a bug
	 * where the device can fail to connect at SuperSpeed
	 * and falls back to high-speed mode which causes
	 * the device to enter a Connect/Disconnect loop
	 */
	if ((revision & DWC3_REVISION_MASK) < 0x190a)
		reg |= DWC3_GCTL_U2RSTECN;

	writel(reg, &dwc3_reg->g_ctl);

	return 0;
}

#if CONFIG_IS_ENABLED(DM_USB)
static int xhci_dwc3_probe(struct udevice *dev)
{
	struct xhci_hcor *hcor;
	struct xhci_hccr *hccr;
	struct dwc3 *dwc3_reg;
	enum usb_dr_mode dr_mode;
	//struct xhci_dwc3_plat *plat = dev_get_plat(dev);
	const char *phy;
	u32 reg;
#if defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	ofnode node;
	int ret = 0;
#endif
	hccr = (struct xhci_hccr *)((uintptr_t)dev_remap_addr(dev));
	hcor = (struct xhci_hcor *)((uintptr_t)hccr +
			HC_LENGTH(xhci_readl(&(hccr)->cr_capbase)));
#if defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	node = ofnode_by_compatible(ofnode_null(), "sunplus,usb3-phy");
	if (!ofnode_valid(node))
		debug("%s phy node failed\n", __func__);

	ret = uclass_get_device_by_ofnode(UCLASS_PHY, node, &phydev);

	gpiodir = devm_gpiod_get(phydev, "typec", GPIOD_IS_IN);
#endif
	uphy_init();

	dwc3_reg = (struct dwc3 *)((char *)(hccr) + DWC3_REG_OFFSET);

	dwc3_core_init(dwc3_reg);

	/* Set dwc3 usb2 phy config */
	reg = readl(&dwc3_reg->g_usb2phycfg[0]);

	phy = dev_read_string(dev, "phy_type");
	if (phy && strcmp(phy, "utmi_wide") == 0) {
		reg |= DWC3_GUSB2PHYCFG_PHYIF;
		reg &= ~DWC3_GUSB2PHYCFG_USBTRDTIM_MASK;
		reg |= DWC3_GUSB2PHYCFG_USBTRDTIM_16BIT;
	}

	if (dev_read_bool(dev, "snps,dis_enblslpm-quirk"))
		reg &= ~DWC3_GUSB2PHYCFG_ENBLSLPM;

	if (dev_read_bool(dev, "snps,dis-u2-freeclk-exists-quirk"))
		reg &= ~DWC3_GUSB2PHYCFG_U2_FREECLK_EXISTS;

	if (dev_read_bool(dev, "snps,dis_u2_susphy_quirk"))
		reg &= ~DWC3_GUSB2PHYCFG_SUSPHY;

	writel(reg, &dwc3_reg->g_usb2phycfg[0]);

	dr_mode = usb_get_dr_mode(dev_ofnode(dev));
	if (dr_mode == USB_DR_MODE_UNKNOWN)
		/* by default set dual role mode to HOST */
		dr_mode = USB_DR_MODE_HOST;

	dwc3_set_mode(dwc3_reg, dr_mode);

	return xhci_register(dev, hccr, hcor);
}

static int xhci_dwc3_remove(struct udevice *dev)
{
	//struct xhci_dwc3_plat *plat = dev_get_plat(dev);

	//dwc3_shutdown_phy(dev, &plat->phys);
#if defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	dm_gpio_free(phydev, gpiodir);
#endif
	return xhci_deregister(dev);
}

static const struct udevice_id xhci_dwc3_ids[] = {
	{ .compatible = "snps,dwc3" },
	{ .compatible = "synopsys,dwc3" },
	{ }
};

U_BOOT_DRIVER(xhci_dwc3) = {
	.name = "xhci-dwc3",
	.id = UCLASS_USB,
	.of_match = xhci_dwc3_ids,
	.probe = xhci_dwc3_probe,
	.remove = xhci_dwc3_remove,
	.ops = &xhci_usb_ops,
	.priv_auto	= sizeof(struct xhci_ctrl),
	.plat_auto	= sizeof(struct xhci_dwc3_plat),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
#endif
