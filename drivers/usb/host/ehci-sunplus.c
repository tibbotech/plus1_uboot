/*
 * This is a driver for the ehci controller found in Sunplus Gemini SoC
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <usb.h>
#include <asm/io.h>
#include <usb/ehci-ci.h>
#include <usb/ulpi.h>
#include <linux/delay.h>

#include "ehci.h"


#if defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_Q645) && \
	!defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#define REG_BASE		0x9c000000
#elif defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#define REG_BASE		0xf8000000
#endif

#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#define REG_BASE_AO		0xf8800000
#endif

#define RF_GRP(_grp, _reg) ((((_grp) * 32 + (_reg)) * 4) + REG_BASE)

#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#define RF_GRP_AO(_grp, _reg) ((((_grp) * 32 + (_reg)) * 4) + REG_BASE_AO)
#endif

#define RF_MASK_V(_mask, _val)       (((_mask) << 16) | (_val))
#define RF_MASK_V_SET(_mask)         (((_mask) << 16) | (_mask))
#define RF_MASK_V_CLR(_mask)         (((_mask) << 16) | 0)

#if defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
// usb spec 2.0 Table 7-3  VHSDSC (min, max) = (525, 625)
// default = 577 mV (374 + 7 * 29)
#define DEFAULT_UPHY_DISC	0x7   // 7 (=577mv)
#define DEFAULT_SQ_CT		0x3
#else
// usb spec 2.0 Table 7-3  VHSDSC (min, max) = (525, 625)
// default = 586.5 mV (405 + 11 * 16.5)
// update  = 619.5 mV (405 + 13 * 16.5)
#define DEFAULT_UPHY_DISC	0xd   // 13 (=619.5mV)
#define ORIG_UPHY_DISC		0xb   // 11 (=586.5mV)
#endif

#if defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C) && \
	!defined(CONFIG_TARGET_PENTAGRAM_Q645) && !defined(CONFIG_TARGET_PENTAGRAM_SP7350)
struct uphy_rn_regs {
       unsigned int cfg[22];
};
#elif defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C) || \
	defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
struct uphy_rn_regs {
	u32 cfg[28];		       // 150.0
	u32 gctrl[3];		       // 150.28
	u32 gsts;		       // 150.31
};
#endif
#define UPHY0_RN_REG ((volatile struct uphy_rn_regs *)RF_GRP(149, 0))
#define UPHY1_RN_REG ((volatile struct uphy_rn_regs *)RF_GRP(150, 0))

#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
struct moon0_regs {
	unsigned int stamp;            // 0.0
	unsigned int reset[15];        // 0.1 -  0.12
	unsigned int rsvd[15];         // 0.13 - 0.30
	unsigned int hw_cfg;           // 0.31
};
#define MOON0_REG ((volatile struct moon0_regs *)RF_GRP_AO(0, 0))
#else
struct moon0_regs {
	unsigned int stamp;            // 0.0
	unsigned int clken[10];        // 0.1
	unsigned int gclken[10];       // 0.11
	unsigned int reset[10];        // 0.21
	unsigned int hw_cfg;           // 0.31
};
#define MOON0_REG ((volatile struct moon0_regs *)RF_GRP(0, 0))
#endif

struct moon1_regs {
	unsigned int sft_cfg[32];
};
#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#define MOON1_REG ((volatile struct moon1_regs *)RF_GRP_AO(1, 0))
#else
#define MOON1_REG ((volatile struct moon1_regs *)RF_GRP(1, 0))
#endif

struct moon2_regs {
	unsigned int sft_cfg[32];
};
#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#define MOON2_REG ((volatile struct moon2_regs *)RF_GRP_AO(2, 0))
#else
#define MOON2_REG ((volatile struct moon2_regs *)RF_GRP(2, 0))
#endif

struct moon3_regs {
	unsigned int sft_cfg[32];
};
#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#define MOON3_REG ((volatile struct moon3_regs *)RF_GRP_AO(3, 0))
#else
#define MOON3_REG ((volatile struct moon3_regs *)RF_GRP(3, 0))
#endif

#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
struct moon4_regs {
	unsigned int sft_cfg[32];
};
#define MOON4_REG ((volatile struct moon4_regs *)RF_GRP_AO(4, 0))
#else
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
#endif

struct moon5_regs {
	unsigned int sft_cfg[32];
};
#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#define MOON5_REG ((volatile struct moon5_regs *)RF_GRP_AO(5, 0))
#else
#define MOON5_REG ((volatile struct moon5_regs *)RF_GRP(5, 0))
#endif

#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
struct hb_gp_regs {
	unsigned int hb_otp_data[8];
	unsigned int reserved_8[24];
};
#define OTP_REG	((volatile struct hb_gp_regs *)RF_GRP_AO(71, 0))
#else
struct hb_gp_regs {
        unsigned int hb_otp_data0;
        unsigned int hb_otp_data1;
        unsigned int hb_otp_data2;
        unsigned int hb_otp_data3;
        unsigned int hb_otp_data4;
        unsigned int hb_otp_data5;
        unsigned int hb_otp_data6;
        unsigned int hb_otp_data7;
        unsigned int hb_otp_ctl;
        unsigned int hb_otp_data;
        unsigned int g7_reserved[22];
};
#define HB_GP_REG ((volatile struct hb_gp_regs *)RF_GRP(350, 0))
#endif

struct sunplus_ehci_priv {
	struct ehci_ctrl ehcictrl;
	struct usb_ehci *ehci;
};

static void uphy_init(int port_num)
{
#if defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C) && \
	!defined(CONFIG_TARGET_PENTAGRAM_Q645) && !defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	unsigned int val, set;

	// 1. Default value modification
	if(0 == port_num) {
		MOON4_REG->uphy0_ctl[0] = RF_MASK_V(0xffff, 0x4002);
		MOON4_REG->uphy0_ctl[1] = RF_MASK_V(0xffff, 0x8747);
	} else if (1 == port_num) {
		MOON4_REG->uphy1_ctl[0] = RF_MASK_V(0xffff, 0x4004);
		MOON4_REG->uphy1_ctl[1] = RF_MASK_V(0xffff, 0x8747);
	}

	// 2. PLL power off/on twice
	if(0 == port_num){
		MOON4_REG->uphy0_ctl[3] = RF_MASK_V(0xffff, 0x88);
		mdelay(1);
		MOON4_REG->uphy0_ctl[3] = RF_MASK_V(0xffff, 0x80);
		mdelay(1);
		MOON4_REG->uphy0_ctl[3] = RF_MASK_V(0xffff, 0x88);
		mdelay(1);
		MOON4_REG->uphy0_ctl[3] = RF_MASK_V(0xffff, 0x80);
		mdelay(1);
		MOON4_REG->uphy0_ctl[3] = RF_MASK_V(0xffff, 0);
	} else if (1 == port_num){
		MOON4_REG->uphy1_ctl[3] = RF_MASK_V(0xffff, 0x88);
		mdelay(1);
		MOON4_REG->uphy1_ctl[3] = RF_MASK_V(0xffff, 0x80);
		mdelay(1);
		MOON4_REG->uphy1_ctl[3] = RF_MASK_V(0xffff, 0x88);
		mdelay(1);
		MOON4_REG->uphy1_ctl[3] = RF_MASK_V(0xffff, 0x80);
		mdelay(1);
		MOON4_REG->uphy1_ctl[3] = RF_MASK_V(0xffff, 0);
	}
	mdelay(1);

	// 3. reset UPHY0/1
	if (0 == port_num) {
		MOON0_REG->reset[2] = RF_MASK_V_SET(1 << 13);
		MOON0_REG->reset[2] = RF_MASK_V_CLR(1 << 13);
	} else if (1 == port_num) {
		MOON0_REG->reset[2] = RF_MASK_V_SET(1 << 14);
		MOON0_REG->reset[2] = RF_MASK_V_CLR(1 << 14);
	}
	mdelay(1);

	// 4. UPHY 0 internal register modification
	if (0 == port_num) {
		UPHY0_RN_REG->cfg[7] = 0x8b;
	} else if (1 == port_num){
		UPHY1_RN_REG->cfg[7] = 0x8b;
	}

	// 5. USBC 0 reset
	if (0 == port_num) {
		MOON0_REG->reset[2] = RF_MASK_V_SET(1 << 10);
		MOON0_REG->reset[2] = RF_MASK_V_CLR(1 << 10);
	} else if (1 == port_num) {
		MOON0_REG->reset[2] = RF_MASK_V_SET(1 << 11);
		MOON0_REG->reset[2] = RF_MASK_V_CLR(1 << 11);
	}

	// Backup solution to workaround real IC USB clock issue
	// (issue: hang on reading EHCI_USBSTS after EN_ASYNC_SCHEDULE)
	if(0 == port_num) {
		if (HB_GP_REG->hb_otp_data2 & 0x1) { // G350.2 bit[0]
			printf("uphy0 rx clk inv\n");
			MOON4_REG->uphy0_ctl[2] = RF_MASK_V_SET(1 << 6);
		}
	} else if (1 == port_num) {
		if (HB_GP_REG->hb_otp_data2 & 0x2) { // G350.2 bit[1]
			printf("uphy1 rx clk inv\n");
			MOON4_REG->uphy1_ctl[2] = RF_MASK_V_SET(1 << 6);
		}
	}

    // OTP for USB DISC (disconnect voltage)
	val = HB_GP_REG->hb_otp_data6;
	if(0 == port_num) {
		set = val & 0x1F; // UPHY0 DISC
	    if (!set) {
	            set = DEFAULT_UPHY_DISC;
	    } else if (set <= ORIG_UPHY_DISC) {
	            set += 2;
	    }
	    UPHY0_RN_REG->cfg[7] = (UPHY0_RN_REG->cfg[7] & ~0x1F) | set;
	} else if (1 == port_num) {
		set = (val >> 5) & 0x1F; // UPHY1 DISC
	    if (!set) {
	            set = DEFAULT_UPHY_DISC;
	    } else if (set <= ORIG_UPHY_DISC) {
	            set += 2;
	    }
	    UPHY1_RN_REG->cfg[7] = (UPHY1_RN_REG->cfg[7] & ~0x1F) | set;
	}
#elif defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C)
	// 1. enable UPHY 0/1 & USBC 0/1 HW CLOCK */
	if (0 == port_num) {
		MOON0_REG->clken[2] = RF_MASK_V_SET(1 << 13);
		MOON0_REG->clken[2] = RF_MASK_V_SET(1 << 10);
	} else if (1 == port_num) {
		MOON0_REG->clken[2] = RF_MASK_V_SET(1 << 14);
		MOON0_REG->clken[2] = RF_MASK_V_SET(1 << 11);
	}
	mdelay(1);

	// 2. reset UPHY 0/1
	if (0 == port_num) {
		MOON0_REG->reset[2] = RF_MASK_V_SET(1 << 13);
		mdelay(1);
		MOON0_REG->reset[2] = RF_MASK_V_CLR(1 << 13);
	} else if (1 == port_num) {
		MOON0_REG->reset[2] = RF_MASK_V_SET(1 << 14);
		mdelay(1);
		MOON0_REG->reset[2] = RF_MASK_V_CLR(1 << 14);
	}
	mdelay(1);

	// 3. Default value modification
	if (0 == port_num) {
		UPHY0_RN_REG->gctrl[0] = 0x18888002;
	} else if (1 == port_num) {
		UPHY1_RN_REG->gctrl[0] = 0x18888002;
	}
	mdelay(1);

	// 4. PLL power off/on twice
	if (0 == port_num) {
		UPHY0_RN_REG->gctrl[2] = 0x88;
		mdelay(1);
		UPHY0_RN_REG->gctrl[2] = 0x80;
		mdelay(1);
		UPHY0_RN_REG->gctrl[2] = 0x88;
		mdelay(1);
		UPHY0_RN_REG->gctrl[2] = 0x80;;
		mdelay(20);
		UPHY0_RN_REG->gctrl[2] = 0x0;
	} else if (1 == port_num) {
		UPHY1_RN_REG->gctrl[2] = 0x88;
		mdelay(1);
		UPHY1_RN_REG->gctrl[2] = 0x80;
		mdelay(1);
		UPHY1_RN_REG->gctrl[2] = 0x88;
		mdelay(1);
		UPHY1_RN_REG->gctrl[2] = 0x80;
		mdelay(20);
		UPHY1_RN_REG->gctrl[2] = 0x0;
	}

	// 5. USBC 0/1 reset
	if (0 == port_num) {
		MOON0_REG->reset[2] = RF_MASK_V_SET(1 << 10);
		mdelay(1);
		MOON0_REG->reset[2] = RF_MASK_V_CLR(1 << 10);
	} else if (1 == port_num){
		MOON0_REG->reset[2] = RF_MASK_V_SET(1 << 11);
		mdelay(1);
		MOON0_REG->reset[2] = RF_MASK_V_CLR(1 << 11);
	}
	mdelay(1);

	// 6. HW workaround
	if (0 == port_num) {
		UPHY0_RN_REG->cfg[19] |= 0x0f;
	} else if(1 == port_num){
		UPHY1_RN_REG->cfg[19] |= 0x0f;
	}
	// 7. USB DISC (disconnect voltage)
	if (0 == port_num) {
		UPHY0_RN_REG->cfg[7] = 0x8b;
	} else if (1 == port_num) {
		UPHY1_RN_REG->cfg[7] = 0x8b;
	}
	// 8. RX SQUELCH LEVEL
	if (0 == port_num) {
		UPHY0_RN_REG->cfg[25] = 0x4;
	} else if(1 == port_num){
		UPHY1_RN_REG->cfg[25] = 0x4;
	}
#elif defined(CONFIG_TARGET_PENTAGRAM_Q645)
	unsigned int val, set;

	if (0 == port_num) {
		/* enable clock for UPHY, USBC and OTP */
		MOON0_REG->clken[3] = RF_MASK_V_SET(1 << 8);
		MOON0_REG->clken[3] = RF_MASK_V_SET(1 << 13);
		MOON0_REG->clken[2] = RF_MASK_V_SET(1 << 2);

		/* disable reset for OTP */
		MOON0_REG->reset[2] = RF_MASK_V_CLR(1 << 2);

		/* reset UPHY */
		MOON0_REG->reset[3] = RF_MASK_V_SET(1 << 8);
		mdelay(1);
		MOON0_REG->reset[3] = RF_MASK_V_CLR(1 << 8);
		mdelay(1);

		/* Default value modification */
		UPHY0_RN_REG->gctrl[0] = 0x08888101;

		/* PLL power off/on twice */
		UPHY0_RN_REG->gctrl[2] = 0x88;
		mdelay(1);
		UPHY0_RN_REG->gctrl[2] = 0x80;
		mdelay(1);
		UPHY0_RN_REG->gctrl[2] = 0x88;
		mdelay(1);
		UPHY0_RN_REG->gctrl[2] = 0x80;
		mdelay(20); /*  experience */
		UPHY0_RN_REG->gctrl[2] = 0;

		/* USBC 0 reset */
		MOON0_REG->reset[3] = RF_MASK_V_SET(1 << 13);
		mdelay(1);
		MOON0_REG->reset[3] = RF_MASK_V_CLR(1 << 13);
		mdelay(1);

		/* fix rx-active question */
		UPHY0_RN_REG->cfg[19] |= 0xf;

		/* OTP for USB phy tx clock invert */
		val = HB_GP_REG->hb_otp_data2;
		if ((val >> 1) & 1)
			UPHY0_RN_REG->gctrl[1] |= (1 << 5);

		/* OTP for USB phy rx clock invert */
		val = HB_GP_REG->hb_otp_data2;
		if (val & 1)
			UPHY0_RN_REG->gctrl[1] |= (1 << 6);

		/* OTP for USB DISC (disconnect voltage) */
		val = HB_GP_REG->hb_otp_data6;
	        set = val & 0x1f;
	        if (!set)
	                set = DEFAULT_UPHY_DISC;

	        UPHY0_RN_REG->cfg[7] = (UPHY0_RN_REG->cfg[7] & ~0x1F) | set;

		/* OTP for USB phy current source adjustment */
		MOON3_REG->sft_cfg[20] = RF_MASK_V_CLR(1 << 5);

		/* OTP for RX squelch level control to APHY */
		val = HB_GP_REG->hb_otp_data6;
	        set = (val >> 5) & 0x7;
	        if (!set)
			set = DEFAULT_SQ_CT;

	        UPHY0_RN_REG->cfg[25] = (UPHY0_RN_REG->cfg[25] & ~0x7) | set;
	}
#elif defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	unsigned int val, set;

	if (0 == port_num) {
		/* enable clock for UPHY, USBC and OTP */
		MOON2_REG->sft_cfg[6] = RF_MASK_V_SET(1 << 12); // UPHY0_CLKEN=1
		MOON2_REG->sft_cfg[6] = RF_MASK_V_SET(1 << 15); // USBC0_CLKEN=1
		MOON2_REG->sft_cfg[5] = RF_MASK_V_SET(1 << 13); // OTPRX_CLKEN=1

		/* disable reset for OTP */
		MOON0_REG->reset[0] = RF_MASK_V_CLR(1 << 9);  // RBUS_BLOCKB_RESET=0
		mdelay(1);
		MOON0_REG->reset[4] = RF_MASK_V_CLR(1 << 13); // OTPRX_RESET=0
		mdelay(1);

		/* reset UPHY0 */
		/* UPHY0_RESET : 1->0 */
		MOON0_REG->reset[5] = RF_MASK_V_SET(1 << 12);
		mdelay(1);
		MOON0_REG->reset[5] = RF_MASK_V_CLR(1 << 12);
		mdelay(1);

		/* Default value modification */
		/* G149.28 uphy0_gctr0 */
		UPHY0_RN_REG->gctrl[0] = 0x08888101;

		/* PLL power off/on twice */
		/* G149.30 uphy0_gctrl2 */
		UPHY0_RN_REG->gctrl[2] = 0x88;
		mdelay(1);
		UPHY0_RN_REG->gctrl[2] = 0x80;
		mdelay(1);
		UPHY0_RN_REG->gctrl[2] = 0x88;
		mdelay(1);
		UPHY0_RN_REG->gctrl[2] = 0x80;
		mdelay(1);
		UPHY0_RN_REG->gctrl[2] = 0;
		mdelay(20); /*  experience */

		/* USBC 0 reset */
		/* USBC0_RESET : 1->0 */
		MOON0_REG->reset[5] = RF_MASK_V_SET(1 << 15);
		mdelay(1);
		MOON0_REG->reset[5] = RF_MASK_V_CLR(1 << 15);
		mdelay(1);

		/* fix rx-active question */
		/* G149.19 */
		UPHY0_RN_REG->cfg[19] |= 0xf;

		/* OTP for USB phy rx clock invert */
		/* G149.29[6] */
		val = OTP_REG->hb_otp_data[2];
		if (val & 0x1)
			UPHY0_RN_REG->gctrl[1] |= (1 << 6);

		/* OTP for USB phy tx clock invert */
		/* G149.29[5] */
		val = OTP_REG->hb_otp_data[2];
		if ((val >> 1) & 0x1)
			UPHY0_RN_REG->gctrl[1] |= (1 << 5);

		/* OTP for USB DISC (disconnect voltage) */
		/* G149.7[4:0] */
		val = OTP_REG->hb_otp_data[6];
	        set = val & 0x1f;
	        if (!set)
	                set = DEFAULT_UPHY_DISC;

	        UPHY0_RN_REG->cfg[7] = (UPHY0_RN_REG->cfg[7] & 0xffffffe0) | set;
	}
#endif
}

static void usb_power_init(int is_host, int port_num)
{
#if defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C) && \
	!defined(CONFIG_TARGET_PENTAGRAM_Q645) && !defined(CONFIG_TARGET_PENTAGRAM_SP7350)
   	/* a. enable pin mux control (sft_cfg_8, bit2/bit3)	*/
	/*    Host: enable					*/
    	/*    Device: disable					*/
	if (is_host) {
		MOON1_REG->sft_cfg[3] = RF_MASK_V_SET(1 << (2 + port_num));
	} else {
		MOON1_REG->sft_cfg[3] = RF_MASK_V_CLR(1 << (2 + port_num));
	}

    	/* b. USB control register:			*/
    	/*    Host:   ctrl=1, host sel=1, type=1	*/
    	/*    Device  ctrl=1, host sel=0, type=0	*/
	if (is_host) {
		if(0 == port_num){
			MOON4_REG->usbc_ctl = RF_MASK_V_SET(7 << 4);
		} else if (1 == port_num){
			MOON4_REG->usbc_ctl = RF_MASK_V_SET(7 << 12);
		}
	} else {
		if(0 == port_num){
			MOON4_REG->usbc_ctl = RF_MASK_V_SET(1 << 4);
			MOON4_REG->usbc_ctl = RF_MASK_V_CLR(3 << 5);
		} else if (1 == port_num){
			MOON4_REG->usbc_ctl = RF_MASK_V_SET(1 << 12);
			MOON4_REG->usbc_ctl = RF_MASK_V_CLR(3 << 13);
		}
	}
#elif defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C)
    	/* a. enable pin mux control (sft_cfg_8, bit2/bit3)	*/
    	/*    Host: enable					*/
    	/*    Device: disable					*/
	if (is_host) {
	    MOON1_REG->sft_cfg[2] = RF_MASK_V_SET(1 << (12 + port_num));
	} else {
	    MOON1_REG->sft_cfg[2] = RF_MASK_V_CLR(1 << (12 + port_num));
	}

    	/* b. USB control register:			*/
    	/*    Host:   ctrl=1, host sel=1, type=1	*/
    	/*    Device  ctrl=1, host sel=0, type=0	*/
	if (is_host) {
	    if (0 == port_num) {
		    MOON5_REG->sft_cfg[17] = RF_MASK_V_SET(7 << 4);
	    } else if (1 == port_num) {
		    MOON5_REG->sft_cfg[17] = RF_MASK_V_SET(7 << 12);
	    }
	} else {
	    if (0 == port_num) {
		    MOON5_REG->sft_cfg[17] = RF_MASK_V_SET(1 << 4);
		    MOON5_REG->sft_cfg[17] = RF_MASK_V_CLR(3 << 5);
	    } else if (1 == port_num) {
		    MOON5_REG->sft_cfg[17] = RF_MASK_V_SET(1 << 12);
		    MOON5_REG->sft_cfg[17] = RF_MASK_V_CLR(3 << 13);
	    }
	}
#elif defined(CONFIG_TARGET_PENTAGRAM_Q645)
	/* USB control register:		*/
    	/* Host:   ctrl=1, host sel=1, type=1	*/
    	/* Device  ctrl=1, host sel=0, type=0	*/
	if (is_host) {
		if (0 == port_num) {
			MOON3_REG->sft_cfg[22] = RF_MASK_V_SET(7 << 0);
		}
	} else {
		if (0 == port_num) {
			MOON3_REG->sft_cfg[22] = RF_MASK_V_SET(1 << 0);
			MOON3_REG->sft_cfg[22] = RF_MASK_V_CLR(3 << 1);
		}
	}
#elif defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	/* a. enable pin mux control	*/
	/*    Host: enable		*/
	/*    Device: disable		*/
	if (is_host) {
		MOON1_REG->sft_cfg[1] = RF_MASK_V_SET(1 << 7);
	} else {
		MOON1_REG->sft_cfg[1] = RF_MASK_V_CLR(1 << 7);
	}

	/* b. USB control register: 			*/
	/*    Host:   ctrl=1, host sel=1, type=1 	*/
	/*    Device  ctrl=1, host sel=0, type=0 	*/
	if (is_host) {
		MOON4_REG->sft_cfg[10] = RF_MASK_V_SET(7 << 0);
	} else {
		MOON4_REG->sft_cfg[10] = RF_MASK_V_SET(1 << 0);
		MOON4_REG->sft_cfg[10] = RF_MASK_V_CLR(3 << 1);
	}
#endif
}

static int ehci_sunplus_ofdata_to_platdata(struct udevice *dev)
{
	struct sunplus_ehci_priv *priv = dev_get_priv(dev);

	priv->ehci = (struct usb_ehci *)devfdt_get_addr_ptr(dev);
	if (!priv->ehci)
		return -EINVAL;

	return 0;
}

static int ehci_sunplus_probe(struct udevice *dev)
{
	struct usb_plat *plat = dev_get_plat(dev);
	struct sunplus_ehci_priv *priv = dev_get_priv(dev);
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;

#if defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C) && \
	!defined(CONFIG_TARGET_PENTAGRAM_Q645) && !defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	hccr = (struct ehci_hccr *)((uint32_t)&priv->ehci->ehci_len_rev);
	hcor = (struct ehci_hcor *)((uint32_t)&priv->ehci->ehci_usbcmd);
#elif defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C) || \
	defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	hccr = (struct ehci_hccr *)((uint64_t)&priv->ehci->ehci_len_rev);
	hcor = (struct ehci_hcor *)((uint64_t)&priv->ehci->ehci_usbcmd);
#endif

	printf("%s.%d, dev_name:%s,port_num:%d\n",__FUNCTION__, __LINE__, dev->name, dev->seq_);

	uphy_init(dev->seq_);
	usb_power_init(1, dev->seq_);

	return ehci_register(dev, hccr, hcor, NULL, 0, plat->init_type);
}

static int ehci_usb_remove(struct udevice *dev)
{
	printf("%s.%d, dev_name:%s,port_num:%d\n",__FUNCTION__, __LINE__, dev->name, dev->seq_);
	usb_power_init(0, dev->seq_);

	return ehci_deregister(dev);
}

static const struct udevice_id ehci_sunplus_ids[] = {
#if defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C) && \
	!defined(CONFIG_TARGET_PENTAGRAM_Q645) && !defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	{ .compatible = "sunplus,sunplus-q628-usb-ehci0" },
	{ .compatible = "sunplus,sp7021-usb-ehci0" },
	{ .compatible = "sunplus,sp7021-usb-ehci1" },
	{ .compatible = "sunplus,sunplus-q628-usb-ehci1" },
#elif defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C)
	{ .compatible = "sunplus,sunplus-i143-usb-ehci0" },
	{ .compatible = "sunplus,sunplus-i143-usb-ehci1" },
#elif defined(CONFIG_TARGET_PENTAGRAM_Q645)
	{ .compatible = "sunplus,q645-usb-ehci" },
#elif defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	{ .compatible = "sunplus,sp7350-usb-ehci" },
#endif
	{ }
};

U_BOOT_DRIVER(ehci_sunplus) = {
	.name	= "ehci_sunplus",
	.id	= UCLASS_USB,
	.of_match = ehci_sunplus_ids,
	.of_to_plat = ehci_sunplus_ofdata_to_platdata,
	.probe = ehci_sunplus_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.plat_auto = sizeof(struct usb_plat),
	.priv_auto = sizeof(struct sunplus_ehci_priv),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

