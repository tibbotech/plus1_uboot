// SPDX-License-Identifier: GPL-2.0+
/*
 * SUNPLUS SP7350 MIPI CSI/DSI TX display driver
 *
 * (C) Copyright 2022 hammer.hsieh <hammer.hsieh@sunplus.com>
 */

#include <common.h>
#include <display.h>
#include <dm.h>
#include <malloc.h>
#include <video.h>
#include <dm/uclass-internal.h>
#include <linux/io.h>
#include "display2.h"
#include "reg_disp.h"
#include "disp_dmix.h"
#include "disp_tgen.h"
#include "disp_tcon.h"
#include "disp_vpp.h"
#include "disp_osd.h"
#include "disp_mipitx.h"
#include <asm/gpio.h>

#if CONFIG_IS_ENABLED(DM_I2C) && defined(CONFIG_SP7350_LT8912B_BRIDGE)
#include "disp_i2c_lt8912b.h"
#endif

//#define debug printf

extern u32 osd0_header[];
struct sp7350_disp_priv *sp_gpio;

#if CONFIG_IS_ENABLED(DM_I2C) && defined(CONFIG_SP7350_LT8912B_BRIDGE)
extern void lt8912_write_init_config(struct udevice *p1);
extern void lt8912_write_mipi_basic_config(struct udevice *p2);
extern void lt8912_write_param_by_resolution(struct udevice *p2, int w, int h);
extern void lt8912_write_dds_config(struct udevice *p2);
extern void lt8912_write_rxlogicres_config(struct udevice *p1);
extern void lt8912_write_lvds_config(struct udevice *p2);
#endif

DECLARE_GLOBAL_DATA_PTR;

static int sp7350_display_probe(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	int max_bpp = 8; //default 8BPP format
	int is_mipi_dsi_tx = 1; //default MIPI_DSI_TX out
	int width, height;
	void *fb_alloc;
	u32 osd_base_addr;
#if CONFIG_IS_ENABLED(DM_I2C) && defined(CONFIG_SP7350_LT8912B_BRIDGE)
	int i;
#endif
	struct sp7350_disp_priv *priv = dev_get_priv(dev);
	int ret;

	width = CONFIG_VIDEO_SP7350_MAX_XRES;
	height = CONFIG_VIDEO_SP7350_MAX_YRES;

	printf("Disp: probe ... \n");
	printf("Disp: init %dx%d settings\n", width, height);

	fb_alloc = malloc((width*height*
					(CONFIG_VIDEO_SP7350_MAX_BPP >> 3)) + 64 );

	//printf("Disp: fb_alloc = %p\n", fb_alloc);
	//printf("Disp: fb_alloc = %ld\n", sizeof(fb_alloc));

	if (fb_alloc == NULL) {
		printf("Error: malloc in %s failed! \n",__func__);
		return -EINVAL;
	}

	if(((uintptr_t)fb_alloc & 0xffffffff00000000) != 0) {
		printf("Error: malloc addr over 4GB(%s)\n",__func__);
		return -EINVAL;
	}

	if(((uintptr_t)fb_alloc & 0x3f) != 0)
		fb_alloc = (void *)(((uintptr_t)fb_alloc + 64 ) & ~0x3f);

	DRV_DMIX_Init();
	DRV_TGEN_Init(width, height);
	DRV_TCON_Init(width, height);
	DRV_VPP_Init(width, height);
	DRV_OSD_Init(width, height);

#if CONFIG_IS_ENABLED(DM_I2C) && defined(CONFIG_SP7350_LT8912B_BRIDGE)
#else
	if (is_mipi_dsi_tx) {
		ret = gpio_request_by_name(dev, "reset-gpios", 0, &priv->reset, GPIOD_IS_OUT);
		if (ret) {
			printf("reset-gpios not found\n");
			if (ret != -ENOENT)
				return ret;
		}
	}
#endif

	sp_gpio = priv;

#if CONFIG_IS_ENABLED(DM_I2C) && defined(CONFIG_SP7350_LT8912B_BRIDGE)
	DRV_mipitx_Init_1(is_mipi_dsi_tx, width, height);
#else
	DRV_mipitx_Init(is_mipi_dsi_tx, width, height);
#endif

	//osd_base_addr = (u32)((uintptr_t)fb_alloc & 0x00000000ffffffff);
	osd_base_addr = (u32)((uintptr_t)fb_alloc);

	if(CONFIG_VIDEO_SP7350_MAX_BPP == 16) {
		API_OSD_UI_Init(width ,height, osd_base_addr, DRV_OSD_REGION_FORMAT_RGB_565);
		max_bpp = 16;
	}
	else if(CONFIG_VIDEO_SP7350_MAX_BPP == 32) {
		API_OSD_UI_Init(width ,height, osd_base_addr, DRV_OSD_REGION_FORMAT_ARGB_8888);
		max_bpp = 32;
	}
	else {
		API_OSD_UI_Init(width ,height, osd_base_addr, DRV_OSD_REGION_FORMAT_8BPP);
		max_bpp = 8;
	}

	//printf("G189.02 0x%08x\n", G189_OSD0_REG->osd_base_addr);

	uc_plat->base = (ulong)fb_alloc;
	uc_plat->size = width * height * (max_bpp >> 3);

	uc_priv->xsize = width;
	uc_priv->ysize = height;
	uc_priv->rot = CONFIG_VIDEO_SP7350_ROTATE;

	if(CONFIG_VIDEO_SP7350_MAX_BPP == 16) {
		uc_priv->bpix = VIDEO_BPP16;
	}
	else if(CONFIG_VIDEO_SP7350_MAX_BPP == 32) {
		uc_priv->bpix = VIDEO_BPP32;
	}
	else {
		uc_priv->bpix = VIDEO_BPP8;
	}

	#ifdef CONFIG_BMP_8BPP_UPDATE_CMAP
	if(uc_priv->bpix == VIDEO_BPP8) {
		uc_priv->cmap = (u32 *)(uintptr_t)(osd0_header+32);
	}
	#endif

#if CONFIG_IS_ENABLED(DM_I2C) && defined(CONFIG_SP7350_LT8912B_BRIDGE)
	for (i = 0; i < 8; i++) {
		ret = i2c_get_chip_for_busnum(i, LT8912B_I2C_ADDR_MAIN, 1, &priv->chip1);
		if (ret) {
			//printf("i2c bus%d chip1 scan ret %d\n", i, ret);
			if (i == 7) {
				printf("Disp: lt8912b bridge not found\n");
				goto skip_lt8912b;
			} else
				continue;
		}

		ret = i2c_get_chip_for_busnum(i, LT8912B_I2C_ADDR_CEC_DSI, 1, &priv->chip2);
		if (ret) {
			//printf("i2c bus%d chip2 scan ret %d\n", i, ret);
			return ret;
		} else {
			break;
		}
	}
	printf("Disp: init lt8912b bridge ic\n");

	lt8912_write_init_config(priv->chip1);

	lt8912_write_mipi_basic_config(priv->chip2);

	lt8912_write_param_by_resolution(priv->chip2, width, height);

	lt8912_write_dds_config(priv->chip2);

	lt8912_write_rxlogicres_config(priv->chip1);

	lt8912_write_lvds_config(priv->chip2);

skip_lt8912b:

#endif

	video_set_flush_dcache(dev, true);

	printf("Disp: probe done \n");

	return 0;
}

static int sp7350_display_bind(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	uc_plat->size = 0;

	return 0;
}

static const struct video_ops sp7350_display_ops = {
};

static const struct udevice_id sp7350_display_ids[] = {
	{ .compatible = "sunplus,sp7350-display" },
	{ }
};

U_BOOT_DRIVER(sp7350_display) = {
	.name	= "sp7350_display",
	.id	= UCLASS_VIDEO,
	.ops	= &sp7350_display_ops,
	.of_match = sp7350_display_ids,
	.bind	= sp7350_display_bind,
	.probe	= sp7350_display_probe,
	.priv_auto	= sizeof(struct sp7350_disp_priv),
};

