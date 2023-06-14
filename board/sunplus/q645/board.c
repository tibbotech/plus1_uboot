/*
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <version.h>
#include <common.h>
#include <asm/global_data.h>
#include <net.h>

#ifdef CONFIG_SP_SPINAND_Q645
extern void board_spinand_init(void);
#endif

#define Q645_REG_BASE			(0xf8000000)
#define Q645_RF_GRP(_grp, _reg)		((((_grp)*32+(_reg))*4)+Q645_REG_BASE)
#define Q645_RF_MASK_V_SET(_mask)       (((_mask) << 16) | (_mask))
#define Q645_RF_MASK_V_CLR(_mask)	(((_mask)<<16)| 0)
#define Q645_RF_AMBA(_grp, _reg) 	((((_grp) * 1024 + (_reg)) * 4) + Q645_REG_BASE)

struct Q645_moon0_regs {
	unsigned int stamp;             // 0.0
	unsigned int clken[5];          // 0.1 - 0.5
	unsigned int rsvd_1[5]; 	// 0.6 - 0.10
	unsigned int gclken[5];         // 0.11
	unsigned int rsvd_2[5]; 	// 0.16 - 0.20
	unsigned int reset[5];          // 0.21
	unsigned int rsvd_3[5];         // 0.26 - 030
	unsigned int hw_cfg;            // 0.31
};
#define Q645_MOON0_REG ((volatile struct Q645_moon0_regs *)Q645_RF_GRP(0,0))

struct Q645_moon1_regs{
	unsigned int sft_cfg[32];
};
#define Q645_MOON1_REG ((volatile struct Q645_moon1_regs *)Q645_RF_GRP(1,0))

#define Q645_MOON4_REG ((volatile struct Q645_moon1_regs *)Q645_RF_GRP(4,0))

struct uphy_u3_regs {
	unsigned int cfg[32];		       // 189.0
};

#define UPHY0_U3_REG ((volatile struct uphy_u3_regs *)Q645_RF_AMBA(189, 0))

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

#ifdef CONFIG_USB_ETHER
	usb_ether_init();
#endif

	return 0;
}
#endif

#if defined(CONFIG_USB_GADGET)
#include <usb.h>
#if defined(CONFIG_USB_DWC3_GADGET) && !defined(CONFIG_DM_USB_GADGET)
#include <dwc3-uboot.h>

static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_SUPER,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.base = 0xf80a1000,
	.index = 0,
	.dis_u2_susphy_quirk = 1,
	.hsphy_mode = USBPHY_INTERFACE_MODE_UTMIW,
};

int usb_gadget_handle_interrupts(void)
{
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

void uphy_init(void)
{
	volatile struct uphy_u3_regs *dwc3phy_reg;
	u32 result, i = 0;

	Q645_MOON0_REG->clken[3] = Q645_RF_MASK_V_SET(1 << 9);
	Q645_MOON0_REG->clken[3] = Q645_RF_MASK_V_SET(1 << 11);

	Q645_MOON0_REG->reset[3] = Q645_RF_MASK_V_SET(1 << 9);
	Q645_MOON0_REG->reset[3] = Q645_RF_MASK_V_SET(1 << 11);
	mdelay(1);
	Q645_MOON0_REG->reset[3] = Q645_RF_MASK_V_CLR(1 << 9);
	Q645_MOON0_REG->reset[3] = Q645_RF_MASK_V_CLR(1 << 11);

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
	//if (dm_gpio_get_value(gpiodir))
	//	dwc3phy_reg->cfg[5] = (dwc3phy_reg->cfg[5] & 0xFFE0) | 0x15;
	//else
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
}

int board_usb_init(int index, enum usb_init_type init)
{
	//const void *blob = gd->fdt_blob;

	/* find the snps,dwc3 node */
	//node = fdt_node_offset_by_compatible(blob, -1, "synopsys,dwc3");
	//printk("board_usb_init\n");
	//dwc3_device_data.base = fdtdec_get_addr(blob, node, "reg");
	uphy_init();
	return dwc3_uboot_init(&dwc3_device_data);
}
#endif /* CONFIG_USB_DWC3_GADGET */
#endif /* CONFIG_USB_GADGET */