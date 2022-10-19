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

//#define debug printf

extern u32 osd0_header[];

DECLARE_GLOBAL_DATA_PTR;

struct sp7350_disp_priv {
	void __iomem *regs;
	struct display_timing timing;
};

void mipitx_pinmux_init(int is_mipi_dsi_tx)
{
	u32 value;

	value = G205_MIPITX_REG1->sft_cfg[8];
	value &= ~0x00000001;
	if(is_mipi_dsi_tx) {
		printf("mipitx_pinmux_init for mipi dsi tx\n");
		G205_MIPITX_REG1->sft_cfg[8] = value | 0x00000000; //MIPI DSI TX
	} else {
		printf("mipitx_pinmux_init for mipi csi tx\n");
		G205_MIPITX_REG1->sft_cfg[8] = value | 0x00000001; //MIPI CSI TX
	}

}

void mipitx_clk_init(int width, int height)
{
	;//TBD
	printf("G3.25 setting\n");
	DISP_MOON3_REG->sft_cfg[25] = 0x07800180;
}

void disp_set_output_resolution(int is_mipi_dsi_tx, int width, int height)
{
    int mode = 0;
    //DRV_VideoFormat_e fmt;
    //DRV_FrameRate_e fps;

	if(is_mipi_dsi_tx) { //mipitx output
		if((width == 720)&&(height == 480)) {
			mode = 0;
		}
		else if((width == 720)&&(height == 576)) {
			mode = 1;
		}
		else if((width == 800)&&(height == 480)) {
			mode = 7;
		}
		else if((width == 1024)&&(height == 600)) {
			mode = 7;
		}
		else if((width == 1280)&&(height == 720)) {
			mode = 2;
		}
		else if((width == 1920)&&(height == 1080)) {
			mode = 4;
		}
		else {
			mode = 0;
		}
		printf("mipi_dsi_tx output , mode = %d \n", mode);
	}
	else { //mipi_csi_tx output
		mode = 7;
		printf("mipi_csi_tx output , mode = %d \n", mode);
	}

}

static int sp7350_display_probe(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	int max_bpp = 8; //default 8BPP format
	int is_mipi_dsi_tx = 1; //default MIPI_DSI_TX out
	int width, height;
	void *fb_alloc;
	u32 osd_base_addr;

	#ifdef CONFIG_EN_SP7350_MIPITX_SW
	is_mipi_dsi_tx = 0; //Switch to MIPI_CSI_TX out
	#else	
	#endif
	width = CONFIG_VIDEO_SP7350_MAX_XRES;
	height = CONFIG_VIDEO_SP7350_MAX_YRES;

	printf("Disp: probe ... \n");
	printf("Disp: width = %d, height = %d\n", width, height);

	fb_alloc = malloc((width*height*
					(CONFIG_VIDEO_SP7350_MAX_BPP >> 3)) + 64 );

	printf("Disp: fb_alloc = %p\n", fb_alloc);
	printf("Disp: fb_alloc = %ld\n", sizeof(fb_alloc));

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

	mipitx_pinmux_init(is_mipi_dsi_tx);
	mipitx_clk_init(width, height);

	DRV_DMIX_Init();
	DRV_TGEN_Init(width, height);
	DRV_TCON_Init(width, height);
	DRV_VPP_Init(width, height);
	DRV_OSD_Init(width, height);
	DRV_mipitx_Init(is_mipi_dsi_tx, width, height);

	//DRV_DMIX_Layer_Init(DRV_DMIX_BG, DRV_DMIX_AlphaBlend, DRV_DMIX_PTG);
	//DRV_DMIX_Layer_Init(DRV_DMIX_L1, DRV_DMIX_Transparent, DRV_DMIX_VPP0);
	//DRV_DMIX_Layer_Init(DRV_DMIX_L3, DRV_DMIX_AlphaBlend, DRV_DMIX_OSD3);
	//DRV_DMIX_Layer_Init(DRV_DMIX_L4, DRV_DMIX_AlphaBlend, DRV_DMIX_OSD2);
	//DRV_DMIX_Layer_Init(DRV_DMIX_L5, DRV_DMIX_AlphaBlend, DRV_DMIX_OSD1);
	//DRV_DMIX_Layer_Init(DRV_DMIX_L6, DRV_DMIX_AlphaBlend, DRV_DMIX_OSD0);

	disp_set_output_resolution(is_mipi_dsi_tx, width, height);

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
		//uc_priv->cmap = (u32 *)(u32)(osd0_header+32);
		uc_priv->cmap = (u32 *)(uintptr_t)(osd0_header+32);
		//uc_priv->cmap = (u32 *)(osd0_header+32);
		//uc_priv->cmap = osd0_header+32;
	}
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

