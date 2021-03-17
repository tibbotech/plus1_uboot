// SPDX-License-Identifier: GPL-2.0+
/*
 * SUNPLUS SP7021 HDMI/TTL display driver
 *
 * (C) Copyright 2020 hammer.hsieh <hammer.hsieh@sunplus.com>
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
#include "disp_dve.h"
#include "disp_vpp.h"
#include "disp_osd.h"
#include "disp_hdmitx.h"

//#define debug printf

extern u32 osd0_header[];

DECLARE_GLOBAL_DATA_PTR;

struct sp7021_disp_priv {
	void __iomem *regs;
	struct display_timing timing;
};

void ttl_pinmux_init(int is_hdmi)
{
    if(!is_hdmi)
        DISP_MOON1_REG->sft_cfg[4] = 0x00400040; //enable lcdif
    else 
        DISP_MOON1_REG->sft_cfg[4] = 0x00400000; //disable lcdif
}

void ttl_clk_init(int method, int width, int height)
{
    if (method == 0) {
        DISP_MOON4_REG->plltv_ctl[0] = 0x80418041; //en pll , clk = 27M //G4.14
        //DISP_MOON4_REG->plltv_ctl[1] = 0x00000000; //G4.15
        DISP_MOON4_REG->plltv_ctl[2] = 0xFFFF0000; //don't care //G4.16
        DISP_MOON4_REG->otp_st = 0x00200020; //clk div4 //G4.31
    }
    else {
		if ( (width == 1280) && (height == 720) ) {
			//with  formula ( FVCO = (27/(M+1)) * (N+1) )
			DISP_MOON4_REG->plltv_ctl[0] = 0x80020000; //don't bypass //G4.14
			DISP_MOON4_REG->plltv_ctl[1] = 0x00800080; //G4.15
			DISP_MOON4_REG->plltv_ctl[2] = 0xFFFF0827; //en pll , clk = 60M //(FVCO= (27/(M+1)) * (N+1) ) M=8,N=39 //G4.16
			DISP_MOON4_REG->otp_st = 0x00300000; //clk no div //G4.31			
		}
		else {
			//with  formula ( FVCO = (27/(M+1)) * (N+1) )
			DISP_MOON4_REG->plltv_ctl[0] = 0x80020000; //don't bypass //G4.14
			//DISP_MOON4_REG->plltv_ctl[1] = 0x00000000; //G4.15
			DISP_MOON4_REG->plltv_ctl[2] = 0xFFFF1f27; //en pll , clk = 33.75M //(FVCO= (27/(M+1)) * (N+1) ) M=31,N=39 //G4.16
			DISP_MOON4_REG->otp_st = 0x00300000; //clk no div //G4.31
		}

    }
}

void SPI_CS_HIGH(void)
{
	DISP_GPIO6_REG->sft_cfg[10] = 0x01000100;	//G6.10 G_MX40 OE
	DISP_GPIO6_REG->sft_cfg[18] = 0x01000100;	//G6.18 G_MX40 DO = HIGH
	//DISP_GPIO6_REG->sft_cfg[18] = 0x01000000;	//G6.18 G_MX40 DO = LOW
	DISP_GPIO101_REG->sft_cfg[26] = 0x00000100;	//G101.26 G_MX40 First
	//DISP_GPIO6_REG->sft_cfg[2] = 0x01000100;	//G6.2 G_MX40 Master	
}

void SPI_CS_LOW(void)
{
	DISP_GPIO6_REG->sft_cfg[10] = 0x01000100;	//G6.10 G_MX40 OE
	//DISP_GPIO6_REG->sft_cfg[18] = 0x01000100;	//G6.18 G_MX40 DO = HIGH
	DISP_GPIO6_REG->sft_cfg[18] = 0x01000000;	//G6.18 G_MX40 DO = LOW
	DISP_GPIO101_REG->sft_cfg[26] = 0x00000100;	//G101.26 G_MX40 First
	//DISP_GPIO6_REG->sft_cfg[2] = 0x01000100;	//G6.2 G_MX40 Master	
}

void SPI_CLK_HIGH(void)
{
	DISP_GPIO6_REG->sft_cfg[10] = 0x02000200;	//G6.10 G_MX41 OE
	DISP_GPIO6_REG->sft_cfg[18] = 0x02000200;	//G6.18 G_MX41 DO = HIGH
	//DISP_GPIO6_REG->sft_cfg[18] = 0x02000000;	//G6.18 G_MX41 DO = LOW
	DISP_GPIO101_REG->sft_cfg[26] = 0x00000200;	//G101.26 G_MX41 First
	//DISP_GPIO6_REG->sft_cfg[2] = 0x02000200;	//G6.2 G_MX41 Master	
}

void SPI_CLK_LOW(void)
{
	DISP_GPIO6_REG->sft_cfg[10] = 0x02000200;	//G6.10 G_MX41 OE
	//DISP_GPIO6_REG->sft_cfg[18] = 0x02000200;	//G6.18 G_MX41 DO = HIGH
	DISP_GPIO6_REG->sft_cfg[18] = 0x02000000;	//G6.18 G_MX41 DO = LOW
	DISP_GPIO101_REG->sft_cfg[26] = 0x00000200;	//G101.26 G_MX41 First
	//DISP_GPIO6_REG->sft_cfg[2] = 0x02000200;	//G6.2 G_MX41 Master	
}

void SPI_MOSI_HIGH(void)
{
	DISP_GPIO6_REG->sft_cfg[10] = 0x04000400;	//G6.10 G_MX42 OE
	DISP_GPIO6_REG->sft_cfg[18] = 0x04000400;	//G6.18 G_MX42 DO = HIGH
	//DISP_GPIO6_REG->sft_cfg[18] = 0x04000000;	//G6.18 G_MX42 DO = LOW
	DISP_GPIO101_REG->sft_cfg[26] = 0x00000400;	//G101.26 G_MX42 First
	//DISP_GPIO6_REG->sft_cfg[2] = 0x04000400;	//G6.2 G_MX42 Master	
}

void SPI_MOSI_LOW(void)
{
	DISP_GPIO6_REG->sft_cfg[10] = 0x04000400;	//G6.10 G_MX42 OE
	//DISP_GPIO6_REG->sft_cfg[18] = 0x04000400;	//G6.18 G_MX42 DO = HIGH
	DISP_GPIO6_REG->sft_cfg[18] = 0x04000000;	//G6.18 G_MX42 DO = LOW
	DISP_GPIO101_REG->sft_cfg[26] = 0x00000400;	//G101.26 G_MX42 First
	//DISP_GPIO6_REG->sft_cfg[2] = 0x04000400;	//G6.2 G_MX42 Master	
}

void Spi_Write_16bit(int data)
{
	int i,j;
	SPI_MOSI_LOW();
	udelay(60);
	SPI_CS_LOW();
	for (i = 0; i < 16; i++)
	{
		if(data & 0x8000)
			SPI_MOSI_HIGH();
		else
			SPI_MOSI_LOW();
		data <<= 1;

		for (j = 0; j < 3; j++)
			SPI_CLK_LOW();
		//udelay(1);
		for (j = 0; j < 2; j++)
			SPI_CLK_HIGH();
		//udelay(1);
	}
	SPI_CS_HIGH();
	SPI_CLK_LOW();
	SPI_MOSI_LOW();
	udelay(60);
}

int spi_init[7] = {0x0000, 0x0110, 0x02B1, 0x0339, 0x0404, 0x0618, 0x0F60};
int spi_init_self[7] = {0x0000, 0x0110, 0xB102, 0x3903, 0x0404, 0x1806, 0x600F};

int spi_init_jig[142] = {	0x0000, 0x0110, 0x02B1, 0x0339, 0x0404, 0x0618, 0x0F60,
							0x0001, 0x0504, 0x0A10, 0x0B1A, 0x0C10, 0x0D1A, 0x0E10, 0x0F1A,
							0x0002, 0x010A, 0x0448, 0x0524, 0x06A7, 0x07A7, 0x0F08, 0x1228, 0x1308, 0x1700, 0x1809,
							0x0008, 0x0985, 0x0B00,
							0x0003, 0x0100, 0x020B, 0x0311, 0x0415, 0x0515, 0x0611, 0x0717, 0x81B, 0x0924, 0x0A25,
								0x0B25, 0x0C1A, 0x0D14, 0x0E19, 0x0F27, 0x102A, 0x110C, 0x120F, 0x132D, 0x141E,
							0x0006, 0x0100, 0x020B, 0x0311, 0x0415, 0x0515, 0x0611, 0x0717, 0x81D, 0x0924, 0x0A25,
								0x0B25, 0x0C1A, 0x0D14, 0x0E19, 0x0F27, 0x101A, 0x110C, 0x120F, 0x132D, 0x141E,
							0x000F, 0x0301, 0x1300, 0x1520, 0x1601, 0x1A42, 0x1E80,
							0x0010, 0x0610, 0x0EC0, 0x0F00, 0x1B3C, 0x1EC0,
							0x0011, 0x0901, 0x0A10, 0x0B0F, 0x0C0E, 0x0D0D, 0x0E0C, 0x0F0B, 0x100A, 0x1109, 0x1228,
								0x1425, 0x1602, 0x1704, 0x1801, 0x1903, 0x1A05,
							0x0012, 0x0901, 0x0A10, 0x0B0F, 0x0C0E, 0x0D0D, 0x0E0C, 0x0F0B, 0x100A, 0x1109, 0x1228,
								0x1425, 0x1602, 0x1704, 0x1801, 0x1903, 0x1A05,
							0x0013, 0x0901, 0x0A09, 0x0B0A, 0x0C0B, 0x0D0C, 0x0E0D, 0x0F0E, 0x100F, 0x1110, 0x1228, 0x1425,
							0x0014, 0x0901, 0x0A09, 0x0B0A, 0x0C0B, 0x0D0C, 0x0E0D, 0x0F0E, 0x100F, 0x1110, 0x1228, 0x1425,
};

void ttl_spi_init(int method)
{
	int i;
	if (method == 0) {
		//SPI_CS_LOW();
		for (i = 0; i < 142; i++)
			Spi_Write_16bit(spi_init_jig[i]);

		//for (i = 0; i < 7; i++)
		//	Spi_Write_16bit(spi_init_self[i]);

		//SPI_CS_HIGH();
	}
	else {
		DISP_MOON0_REG->clken[2] = 0x00080008; //clken SPI0 CLKEN

		DISP_GPIO6_REG->sft_cfg[10] = 0x01000100;	//G6.10 G_MX40 OE
		//DISP_GPIO6_REG->sft_cfg[18] = 0x01000100;	//G6.18 G_MX40 DO = HIGH
		DISP_GPIO6_REG->sft_cfg[18] = 0x01000000;	//G6.18 G_MX40 DO = LOW
		DISP_GPIO101_REG->sft_cfg[26] = 0x00000100;	//G101.26 G_MX40 First
		//DISP_GPIO6_REG->sft_cfg[2] = 0x01000100;	//G6.2 G_MX40 Master

		DISP_MOON2_REG->sft_cfg[22] = 0xffff2200; //pinmux SPI0 CLK
		//DISP_MOON2_REG->sft_cfg[23] = 0xffff2321; //pinmux SPI0 DO / SPI0 CS
		DISP_MOON2_REG->sft_cfg[23] = 0xffff2300; //pinmux SPI0 DO

		printf("spi init start\n");
		//DISP_GPIO6_REG->sft_cfg[18] = 0x01000000;	//G6.18 G_MX40 DO = LOW
		DISP_SPI0_REG->config = 0x0042c864;	//G91.15
		DISP_SPI0_REG->status = 0x02020210;	//G91.14
		printf("data1 \n");
		//DISP_GPIO6_REG->sft_cfg[18] = 0x01000000;	//G6.18 G_MX40 DO = LOW
		DISP_SPI0_REG->data = 0x00000000;	//G91.13 //data
		DISP_SPI0_REG->data = 0x00000000;	//G91.13 //data
		DISP_SPI0_REG->config = 0x0042c864;	//G91.15
		DISP_SPI0_REG->status = 0x02020211;	//G91.14 //start
		printf("data2 \n");
		DISP_SPI0_REG->status = 0x0000002;	//G91.14 //reset
		//DISP_SPI0_REG->data = 0x00001001;	//G91.13 //data(correct)
		//DISP_SPI0_REG->data = 0x00001001;	//G91.13 //data(correct)
		DISP_SPI0_REG->data = 0x00000110;	//G91.13 //data
		DISP_SPI0_REG->data = 0x00000110;	//G91.13 //data
		DISP_SPI0_REG->config = 0x0042c864;	//G91.15
		DISP_SPI0_REG->status = 0x02020211;	//G91.14 //start
		printf("data3 \n");
		DISP_SPI0_REG->status = 0x0000002;	//G91.14 //reset
		//DISP_SPI0_REG->data = 0x0000B102;	//G91.13 //data(correct)
		//DISP_SPI0_REG->data = 0x0000B102;	//G91.13 //data(correct)
		DISP_SPI0_REG->data = 0x000002B1;	//G91.13 //data
		DISP_SPI0_REG->data = 0x000002B1;	//G91.13 //data
		DISP_SPI0_REG->config = 0x0042c864;	//G91.15
		DISP_SPI0_REG->status = 0x02020211;	//G91.14 //start
		printf("data4 \n");
		DISP_SPI0_REG->status = 0x0000002;	//G91.14 //reset
		//DISP_SPI0_REG->data = 0x00003903;	//G91.13 //data(correct)
		//DISP_SPI0_REG->data = 0x00003903;	//G91.13 //data(correct)
		DISP_SPI0_REG->data = 0x00000339;	//G91.13 //data
		DISP_SPI0_REG->data = 0x00000339;	//G91.13 //data
		DISP_SPI0_REG->config = 0x0042c864;	//G91.15
		DISP_SPI0_REG->status = 0x02020211;	//G91.14 //start
		printf("data5 \n");
		DISP_SPI0_REG->status = 0x0000002;	//G91.14 //reset
		DISP_SPI0_REG->data = 0x00000404;	//G91.13 //data
		DISP_SPI0_REG->data = 0x00000404;	//G91.13 //data
		DISP_SPI0_REG->config = 0x0042c864;	//G91.15
		DISP_SPI0_REG->status = 0x02020211;	//G91.14 //start
		printf("data6 \n");
		DISP_SPI0_REG->status = 0x0000002;	//G91.14 //reset
		//DISP_SPI0_REG->data = 0x00001806;	//G91.13 //data(correct)
		//DISP_SPI0_REG->data = 0x00001806;	//G91.13 //data(correct)
		DISP_SPI0_REG->data = 0x00000618;	//G91.13 //data
		DISP_SPI0_REG->data = 0x00000618;	//G91.13 //data
		DISP_SPI0_REG->config = 0x0042c864;	//G91.15
		DISP_SPI0_REG->status = 0x02020211;	//G91.14 //start
		printf("data7 \n");
		DISP_SPI0_REG->status = 0x0000002;	//G91.14 //reset
		//DISP_SPI0_REG->data = 0x0000600f;	//G91.13 //data(correct)
		//DISP_SPI0_REG->data = 0x0000600f;	//G91.13 //data(correct)
		DISP_SPI0_REG->data = 0x00000f60;	//G91.13 //data
		DISP_SPI0_REG->data = 0x00000f60;	//G91.13 //data
		DISP_SPI0_REG->config = 0x0042c864;	//G91.15
		DISP_SPI0_REG->status = 0x02020211;	//G91.14 //start
		printf("spi init end\n");
		DISP_GPIO6_REG->sft_cfg[18] = 0x01000100;	//G6.18 G_MX40 DO = HIGH
		DISP_SPI0_REG->status = 0x0000002;	//G91.14 //reset
	}
}

void disp_set_output_resolution(int is_hdmi, int width, int height)
{
    int mode = 0;
    DRV_VideoFormat_e fmt;
    DRV_FrameRate_e fps;

	if(is_hdmi) { //hdmitx output
		if((width == 720)&&(height == 480)) {
			mode = 0;
		}
		else if((width == 720)&&(height == 576)) {
			mode = 1;
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
        hdmi_clk_init(mode);
		debug("hdmitx output , mode = %d \n", mode);
	}
	else { //TTL output
		mode = 7;
		debug("TTL output , mode = %d \n", mode);
	}

    DRV_DVE_SetMode(mode);

	switch (mode)
	{
		default:
		case 0:
            debug("hdmitx output , 480P 59.94Hz \n");
			fmt = DRV_FMT_480P;
			fps = DRV_FrameRate_5994Hz;
            hdmitx_set_timming(0);
			break;
		case 1:
			fmt = DRV_FMT_576P;
			fps = DRV_FrameRate_50Hz;
            debug("hdmitx output , 576P 50Hz \n");
            hdmitx_set_timming(1);
			break;
		case 2:
            debug("hdmitx output , 720P 59.94Hz \n");
			fmt = DRV_FMT_720P;
			fps = DRV_FrameRate_5994Hz;
            hdmitx_set_timming(2);
			break;
		case 3:
			fmt = DRV_FMT_720P;
			fps = DRV_FrameRate_50Hz;
			break;
		case 4:
			fmt = DRV_FMT_1080P;
			fps = DRV_FrameRate_5994Hz;
			break;
		case 5:
			fmt = DRV_FMT_1080P;
			fps = DRV_FrameRate_50Hz;
			break;
		case 6:
			fmt = DRV_FMT_1080P;
			fps = DRV_FrameRate_24Hz;
			break;
		case 7:
			fmt = DRV_FMT_USER_MODE;
			fps = DRV_FrameRate_5994Hz;
			debug("set TGEN user mode\n");
			break;
	}

    DRV_TGEN_Set(fmt, fps);

}

static int sp7021_display_probe(struct udevice *dev)
{
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	int max_bpp = 8; //default 8BPP format
	int is_hdmi = 1; //default HDMITX out
	int width, height;
	void *fb_alloc;

	#ifdef CONFIG_EN_SP7021_TTL
	is_hdmi = 0; //Switch to TTL out
	#else	
	#endif
	width = CONFIG_VIDEO_SP7021_MAX_XRES;
	height = CONFIG_VIDEO_SP7021_MAX_YRES;

	debug("Disp: probe ... \n");

	#if 0 //TBD, for DTS
	priv->regs = (void *)dev_read_addr_index(dev,0);
	if ((fdt_addr_t)priv->regs == FDT_ADDR_T_NONE) {
		printf("%s: sp7021 dt register address error\n", __func__);
		return -EINVAL;
	}
	else {
		printf("%s: sp7021 dt register address %px \n", __func__, priv->regs);
	}
	#endif

	fb_alloc = malloc((CONFIG_VIDEO_SP7021_MAX_XRES*
					CONFIG_VIDEO_SP7021_MAX_YRES*
					(CONFIG_VIDEO_SP7021_MAX_BPP >> 3)) + 64 );
	if (fb_alloc == NULL) {
		printf("Error: malloc in %s failed! \n",__func__);
		return -EINVAL;
	}

	if(((u32)fb_alloc & 0x3f) != 0)
		fb_alloc = (void *)(((uintptr_t)fb_alloc + 64 ) & ~0x3f);

    ttl_pinmux_init(is_hdmi);
    if(!is_hdmi) {
        ttl_clk_init(1, width, height);

		if ( (width == 1280) && (height == 720) )
			ttl_spi_init(0);
    }

	DRV_DMIX_Init();
	DRV_TGEN_Init(is_hdmi, width, height);
	DRV_DVE_Init(is_hdmi, width, height);
	DRV_VPP_Init();
	DRV_OSD_Init();
	DRV_hdmitx_Init(is_hdmi, width, height);

	DRV_DMIX_Layer_Init(DRV_DMIX_BG, DRV_DMIX_AlphaBlend, DRV_DMIX_PTG);
	DRV_DMIX_Layer_Init(DRV_DMIX_L1, DRV_DMIX_Transparent, DRV_DMIX_VPP0);
	DRV_DMIX_Layer_Init(DRV_DMIX_L6, DRV_DMIX_AlphaBlend, DRV_DMIX_OSD0);

	disp_set_output_resolution(is_hdmi, width, height);

	if(CONFIG_VIDEO_SP7021_MAX_BPP == 16) {
		API_OSD_UI_Init(width ,height, (u32)fb_alloc, DRV_OSD_REGION_FORMAT_RGB_565);
		max_bpp = 16;
	}
	else if(CONFIG_VIDEO_SP7021_MAX_BPP == 32) {
		API_OSD_UI_Init(width ,height, (u32)fb_alloc, DRV_OSD_REGION_FORMAT_ARGB_8888);
		max_bpp = 32;
	}
	else {
		API_OSD_UI_Init(width ,height, (u32)fb_alloc, DRV_OSD_REGION_FORMAT_8BPP);
		max_bpp = 8;
	}

	uc_plat->base = (ulong)fb_alloc;
	uc_plat->size = width * height * (max_bpp >> 3);

	uc_priv->xsize = width;
	uc_priv->ysize = height;
	uc_priv->rot = CONFIG_VIDEO_SP7021_ROTATE;

	if(CONFIG_VIDEO_SP7021_MAX_BPP == 16) {
		uc_priv->bpix = VIDEO_BPP16;
	}
	else if(CONFIG_VIDEO_SP7021_MAX_BPP == 32) {
		uc_priv->bpix = VIDEO_BPP32;
	}
	else {
		uc_priv->bpix = VIDEO_BPP8;
	}

	#ifdef CONFIG_BMP_8BPP_UPDATE_CMAP
	if(uc_priv->bpix == VIDEO_BPP8) {
		uc_priv->cmap = (u32 *)(u32)(osd0_header+32);
	}
	#endif

	video_set_flush_dcache(dev, true);

	debug("Disp: probe done \n");

	return 0;
}

static int sp7021_display_bind(struct udevice *dev)
{
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);

	uc_plat->size = 0;

	return 0;
}

static const struct video_ops sp7021_display_ops = {
};

static const struct udevice_id sp7021_display_ids[] = {
	{ .compatible = "sunplus,sp7021-display" },
	{ }
};

U_BOOT_DRIVER(sp7021_display) = {
	.name	= "sp7021_display",
	.id	= UCLASS_VIDEO,
	.ops	= &sp7021_display_ops,
	.of_match = sp7021_display_ids,
	.bind	= sp7021_display_bind,
	.probe	= sp7021_display_probe,
	.priv_auto_alloc_size	= sizeof(struct sp7021_disp_priv),
};

