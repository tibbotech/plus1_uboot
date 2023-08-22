// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sunplus - All Rights Reserved
 *
 * Author(s): Hammer Hsieh <hammer.hsieh@sunplus.com>
 */

#include <common.h>
#include "reg_disp.h"
#include "disp_mipitx.h"
#include "display2.h"
#include <asm/gpio.h>
#define SP_MIPITX_LP_MODE	0
#define SP_MIPITX_HS_MODE	1
/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
void DRV_mipitx_gpio_set(struct sp7350_disp_priv *sp)
{
	/* reset panel */
	#if 0 //active low
	dm_gpio_set_value(&sp->reset, false);
	mdelay(15);
	dm_gpio_set_value(&sp->reset, true);
	mdelay(15);
	dm_gpio_set_value(&sp->reset, false);
	mdelay(25);
	#else //active high
	dm_gpio_set_value(&sp->reset, false);
	mdelay(15);
	dm_gpio_set_value(&sp->reset, true);
	mdelay(25);
	#endif
}

void DRV_mipitx_pllclk_init(void)
{
	//printf("MIPITX CLK setting init\n");
	G205_MIPITX_REG1->sft_cfg[14] = 0x80000000; //init clock
	//init PLLH setting
	DISP_MOON3_REG->sft_cfg[15] = 0xffff40be; //PLLH BNKSEL = 0x2 (2000~2500MHz)
	DISP_MOON3_REG->sft_cfg[16] = 0xffff0009; //PLLH
	/*
	 * PLLH Fvco = 2150MHz (fixed)
	 *                             2150
	 * MIPITX pixel CLK = ----------------------- = 59.72MHz
	 *                     PST_DIV * MIPITX_SEL
	 */
	DISP_MOON3_REG->sft_cfg[14] = 0xffff0b50; //PLLH PST_DIV = div9 (default)
	DISP_MOON3_REG->sft_cfg[25] = 0x07800180; //PLLH MIPITX_SEL = div4

	//init TXPLL setting
	G205_MIPITX_REG1->sft_cfg[10] = 0x00000003; //TXPLL enable and reset
	/*
	 * PRESCAL = 1, FBKDIV = 48, PRE_DIV = 1, EN_DIV5 = 0, PRE_DIV = 2, POST_DIV = 1
	 *                    25 * PRESCAL * FBKDIV            25 * 48
	 * MIPITX bit CLK = ------------------------------ = ----------- = 600MHz
	 *                   PRE_DIV * POST_DIV * 5^EN_DIV5       2
	 */
	G205_MIPITX_REG1->sft_cfg[11] = 0x00003001; //TXPLL MIPITX CLK = 600MHz
	G205_MIPITX_REG1->sft_cfg[12] = 0x00000140; //TXPLL BNKSEL = 0x0 (320~640MHz)

	G205_MIPITX_REG1->sft_cfg[14] = 0x00000000; //init clock done
}

void DRV_mipitx_pllclk_set(int mode, int width, int height)
{
	if (mode == 0) { //LP mode
		//printf("MIPITX CLK setting for LP Mode\n");
		G204_MIPITX_REG0->sft_cfg[4] = 0x00000008; //(600/8/div9)=8.3MHz
	} else {
		//printf("MIPITX CLK setting for Video Mode w %d h %d\n", width, height);
		if ((width == 720) && (height == 480)) {
		#if 0//fine tune PLLH clk to fit 27.08MHz
			DISP_MOON3_REG->sft_cfg[14] = 0x00780050; //PLLH
			DISP_MOON3_REG->sft_cfg[14] = (0x7f800000 | (0xe << 7)); //PLLH
			DISP_MOON3_REG->sft_cfg[25] = 0x07800380; //PLLH MIPITX CLK = 27.08MHz (just test)
		#else
			DISP_MOON3_REG->sft_cfg[14] = 0x00780020; //PLLH
			DISP_MOON3_REG->sft_cfg[25] = 0x07800780; //PLLH MIPITX CLK = 26.77MHz
		#endif
			//G205_MIPITX_REG1->sft_cfg[11] = 0x00021A00; //TXPLL MIPITX CLK = 162.5MHz
			G205_MIPITX_REG1->sft_cfg[11] = 0x00021B00; //TXPLL MIPITX CLK = 168.75MHz

		} else if ((width == 800) && (height == 480)) {
			;//TBD
			//DISP_MOON3_REG->sft_cfg[14] = 0x00780050; //PLLH
			//DISP_MOON3_REG->sft_cfg[25] = 0x07800380; //PLLH MIPITX CLK = 30MHz
			//G205_MIPITX_REG1->sft_cfg[11] = 0x00021E00; //TXPLL MIPITX CLK = 175MHz
		} else if ((width == 1024) && (height == 600)) { // 1024x600
			;//TBD
			//DISP_MOON3_REG->sft_cfg[14] = 0x00780020; //PLLH
			//DISP_MOON3_REG->sft_cfg[25] = 0x07800380; //PLLH MIPITX CLK = 54MHz
			//G205_MIPITX_REG1->sft_cfg[11] = 0x00023100; //TXPLL MIPITX CLK = 306.25MHz
		} else if ((width == 480) && (height == 1280)) {
			DISP_MOON3_REG->sft_cfg[14] = 0x00780028; //PLLH
			DISP_MOON3_REG->sft_cfg[25] = 0x07800380; //PLLH MIPITX CLK = 49MHz	
			G205_MIPITX_REG1->sft_cfg[11] = 0x00000c00; //TXPLL MIPITX CLK = 300MHz
			G205_MIPITX_REG1->sft_cfg[12] = 0x00000140; //TXPLL BNKSEL = 300MHz -- 640MHz
		} else if ((width == 1280) && (height == 480)) {
			;//TBD
			//DISP_MOON3_REG->sft_cfg[14] = 0x00780028; //PLLH
			//DISP_MOON3_REG->sft_cfg[25] = 0x07800380; //PLLH MIPITX CLK = 49MHz
			//G205_MIPITX_REG1->sft_cfg[11] = 0x00000c00; //TXPLL MIPITX CLK = 300MHz
			//G205_MIPITX_REG1->sft_cfg[12] = 0x00000140; //TXPLL BNKSEL = 300MHz -- 640MHz
		} else if ((width == 1280) && (height == 720)) {
		#if 0 //fine tune PLLH clk to fit 74MHz
			DISP_MOON3_REG->sft_cfg[14] = 0x00780038; //PLLH
			DISP_MOON3_REG->sft_cfg[14] = (0x7f800000 | (0x13 << 7)); //PLLH
			DISP_MOON3_REG->sft_cfg[25] = 0x07800180; //PLLH MIPITX CLK = 74MHz (just test)
		#else
			DISP_MOON3_REG->sft_cfg[14] = 0x00780040; //PLLH
			DISP_MOON3_REG->sft_cfg[25] = 0x07800180; //PLLH MIPITX CLK = 72MHz
		#endif
			G205_MIPITX_REG1->sft_cfg[11] = 0x00012400; //TXPLL MIPITX CLK = 450MHz
		} else if ((width == 1920) && (height == 1080)) {
		#if 0 //fine tune PLLH clk to fit 148.5MHz
			DISP_MOON3_REG->sft_cfg[14] = 0x00780038; //PLLH
			DISP_MOON3_REG->sft_cfg[14] = (0x7f800000 | (0x13 << 7)); //PLLH
			DISP_MOON3_REG->sft_cfg[25] = 0x07800080; //PLLH MIPITX CLK = 148MHz (just test)
		#else
			DISP_MOON3_REG->sft_cfg[14] = 0x00780040; //PLLH
			DISP_MOON3_REG->sft_cfg[25] = 0x07800080; //PLLH MIPITX CLK = 143MHz (use this)
		#endif
			G205_MIPITX_REG1->sft_cfg[11] = 0x00002400; //TXPLL MIPITX CLK = 900MHz
			//G205_MIPITX_REG1->sft_cfg[11] = 0x00002500; //TXPLL MIPITX CLK = 925MHz
		} else if ((width == 3840) && (height == 2880)) { // 3840x2880
			DISP_MOON3_REG->sft_cfg[14] = 0x007800420; //PLLH
			DISP_MOON3_REG->sft_cfg[25] = 0x07800000; //PLLH MIPITX CLK = 430MHz max
			G205_MIPITX_REG1->sft_cfg[11] = 0x00003F00; //TXPLL MIPITX CLK = 1600MHz
		} else {
			printf("TBD mipitx pllclk setting\n");
		}
	}
}

void DRV_mipitx_Init(int is_mipi_dsi_tx, int width, int height)
{
	G205_MIPITX_REG1->sft_cfg[6] = 0x00101334; //PHY Reset(under reset) & falling edge

	//G204_MIPITX_REG0->sft_cfg[15] = 0x11000001; //lane num = 1 and DSI_EN and ANALOG_EN
	//G204_MIPITX_REG0->sft_cfg[15] = 0x11000011; //lane num = 2 and DSI_EN and ANALOG_EN
	G204_MIPITX_REG0->sft_cfg[15] = 0x11000031; //lane num = 4 and DSI_EN and ANALOG_EN

	//G204_MIPITX_REG0->sft_cfg[12] = 0x00000030; //vtf = sync pluse
	G204_MIPITX_REG0->sft_cfg[12] = 0x00001030; //vtf = sync event

	G205_MIPITX_REG1->sft_cfg[6] = 0x00101335; //PHY Reset(under normal mode)

	DRV_mipitx_pllclk_init();

	if (is_mipi_dsi_tx) {
		//MIPITX DSI host send command to panel at DATA LANE0 LP mode 
		DRV_mipitx_pllclk_set(SP_MIPITX_LP_MODE, width, height); //set pll clk for LP mode

		G204_MIPITX_REG0->sft_cfg[14] = 0x00100000; //enable command transfer at LP mode
		G204_MIPITX_REG0->sft_cfg[15] = 0x11000033; //command mode start
		G204_MIPITX_REG0->sft_cfg[17] = 0x00520004; //TA GET/SURE/GO
		//G204_MIPITX_REG0->sft_cfg[29] = 0x0000c350; //fix
		//G204_MIPITX_REG0->sft_cfg[29] = 0x000000af; //fix

		//transfer data from TX to RX (depends on panel manufacturer)
		DRV_mipitx_panel_Init(is_mipi_dsi_tx, width, height);
	}

	/*
	 * MIPITX Video Mode Setting
	 */
	DRV_mipitx_pllclk_set(SP_MIPITX_HS_MODE, width, height); //set pll clk for HS mode

	// MIPITX  Video Mode Horizontal/Vertial Timing
	G204_MIPITX_REG0->sft_cfg[2] |= height;
	if ((width == 720) && (height == 480)) { // 720x480
		G204_MIPITX_REG0->sft_cfg[0] = 0x04080005; //fix
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010823; //VSA=0x01 VFP=0x08 VBP=0x23
	} else if ((width == 800) && (height == 480)) { // 800x480
		G204_MIPITX_REG0->sft_cfg[0] = 0x04080005; //fix
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010823; //VSA=0x01 VFP=0x08 VBP=0x23
	} else if ((width == 480) && (height == 1280)) { // 480x1280
		G204_MIPITX_REG0->sft_cfg[0] = 0x04080004; //fix
		G204_MIPITX_REG0->sft_cfg[1] = 0x00011010; //VSA=0x01 VFP=0x10 VBP=0x10
		//G204_MIPITX_REG0->sft_cfg[0] = 0x04080005; //fix
		//G204_MIPITX_REG0->sft_cfg[1] = 0x00010823; //VSA=0x01 VFP=0x08 VBP=0x23
	} else if ((width == 1280) && (height == 480)) { // 1280x480
		G204_MIPITX_REG0->sft_cfg[0] = 0x04080005; //fix
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010823; //VSA=0x01 VFP=0x08 VBP=0x23
	} else if ((width == 1280) && (height == 720)) { // 1280x720
		G204_MIPITX_REG0->sft_cfg[0] = 0x04080005; //fix
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010418; //VSA=0x01 VFP=0x04 VBP=0x18
	} else if ((width == 1920) && (height == 1080)) { // 1920x1080
		#if 1
		G204_MIPITX_REG0->sft_cfg[0] = 0x04080005; //fix
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010328; //VSA=0x01 VFP=0x03 VBP=0x28
		#else
		G204_MIPITX_REG0->sft_cfg[0] = 0x04110004; //fix
		G204_MIPITX_REG0->sft_cfg[1] = 0x00011615; //VSA=0x01 VFP=0x16 VBP=0x15
		#endif
	} else if ((width == 3840) && (height == 2880)) { // 3840x2880
		G204_MIPITX_REG0->sft_cfg[0] = 0x04080005; //fix
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010628; //VSA=0x01 VFP=0x06 VBP=0x28
	} else {
		printf("TBD mipitx common setting\n");
	}
	// MIPITX  Video Mode WordCount Setting
	G204_MIPITX_REG0->sft_cfg[19] |= (width << 16 | (width * 24 / 8)) ;

	// MIPITX  LANE CLOCK DATA Timing 
	G204_MIPITX_REG0->sft_cfg[5] = 0x00100008; //fix
	G204_MIPITX_REG0->sft_cfg[6] = 0x00100010; //fix
	G204_MIPITX_REG0->sft_cfg[7] = 0x0a120020; //fix
	G204_MIPITX_REG0->sft_cfg[8] = 0x0a050010; //fix

	// MIPITX Blanking Mode 
	G204_MIPITX_REG0->sft_cfg[13] = 0x00001100; //fix

	// MIPITX CLOCK CONTROL 
	G204_MIPITX_REG0->sft_cfg[30] = 0x00000001; //fix

	// MIPITX SWITCH to Video Mode 
	G204_MIPITX_REG0->sft_cfg[15] = 0x11000031; //video mode
}

void DRV_mipitx_Init_1(int is_mipi_dsi_tx, int width, int height)
{
	G205_MIPITX_REG1->sft_cfg[6] = 0x00101330; //PHY Reset(under reset)

	//G204_MIPITX_REG0->sft_cfg[15] = 0x11000001; //lane num = 1 and DSI_EN and ANALOG_EN
	//G204_MIPITX_REG0->sft_cfg[15] = 0x11000011; //lane num = 2 and DSI_EN and ANALOG_EN
	G204_MIPITX_REG0->sft_cfg[15] = 0x11000031; //lane num = 4 and DSI_EN and ANALOG_EN

	//G204_MIPITX_REG0->sft_cfg[12] = 0x00000030; //vtf = sync pluse
	G204_MIPITX_REG0->sft_cfg[12] = 0x00001030; //vtf = sync event

	G205_MIPITX_REG1->sft_cfg[6] = 0x00101331; //PHY Reset(under normal mode)

	DRV_mipitx_pllclk_init();

	/*
	 * MIPITX Video Mode Setting
	 */
	DRV_mipitx_pllclk_set(SP_MIPITX_HS_MODE, width, height); //set pll clk for HS mode

	// MIPITX  Video Mode Horizontal/Vertial Timing
	G204_MIPITX_REG0->sft_cfg[2] |= height;
	if ((width == 720) && (height == 480)) { // 720x480
		G204_MIPITX_REG0->sft_cfg[0] = 0x04080005; //fix
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010823; //VSA=0x01 VFP=0x08 VBP=0x23
	} else if ((width == 1280) && (height == 720)) { // 1280x720
		G204_MIPITX_REG0->sft_cfg[0] = 0x04080005; //fix
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010418; //VSA=0x01 VFP=0x04 VBP=0x18
	} else if ((width == 1920) && (height == 1080)) { // 1920x1080
		G204_MIPITX_REG0->sft_cfg[0] = 0x04080005; //fix
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010328; //VSA=0x01 VFP=0x03 VBP=0x28
	} else {
		printf("TBD mipitx common setting\n");
	}
	// MIPITX  Video Mode WordCount Setting
	G204_MIPITX_REG0->sft_cfg[19] |= (width << 16 | (width * 24 / 8)) ;

	// MIPITX  LANE CLOCK DATA Timing 
	G204_MIPITX_REG0->sft_cfg[5] = 0x00100008; //fix
	G204_MIPITX_REG0->sft_cfg[6] = 0x00100010; //fix
	G204_MIPITX_REG0->sft_cfg[7] = 0x0a120020; //fix
	G204_MIPITX_REG0->sft_cfg[8] = 0x0a050010; //fix

	// MIPITX Blanking Mode 
	G204_MIPITX_REG0->sft_cfg[13] = 0x00001100; //fix

	// MIPITX CLOCK CONTROL 
	G204_MIPITX_REG0->sft_cfg[30] = 0x00000001; //fix

	// MIPITX SWITCH to Video Mode 
	G204_MIPITX_REG0->sft_cfg[15] = 0x11000031; //video mode
}

#define MIPITX_CMD_FIFO_FULL 0x00000001
#define MIPITX_CMD_FIFO_EMPTY 0x00000010
#define MIPITX_DATA_FIFO_FULL 0x00000100
#define MIPITX_DATA_FIFO_EMPTY 0x00001000

void check_cmd_fifo_full(void)
{
	u32 value = 0;
	int mipitx_fifo_timeout = 0;

	value = G204_MIPITX_REG0->sft_cfg[16];
	//printf("fifo_status 0x%08x\n", value);
	while((value & MIPITX_CMD_FIFO_FULL) == MIPITX_CMD_FIFO_FULL) {
		if(mipitx_fifo_timeout > 10000) //over 1 second
		{
			printf("cmd fifo full timeout\n");
			break;
		}
		value = G204_MIPITX_REG0->sft_cfg[16];
		++mipitx_fifo_timeout;
		udelay(100);
	}
}

void check_cmd_fifo_empty(void)
{
	u32 value = 0;
	int mipitx_fifo_timeout = 0;

	value = G204_MIPITX_REG0->sft_cfg[16];
	//printf("fifo_status 0x%08x\n", value);
	while((value & MIPITX_CMD_FIFO_EMPTY) != MIPITX_CMD_FIFO_EMPTY) {
		if(mipitx_fifo_timeout > 10000) //over 1 second
		{
			printf("cmd fifo empty timeout\n");
			break;
		}
		value = G204_MIPITX_REG0->sft_cfg[16];
		++mipitx_fifo_timeout;
		udelay(100);
	}
}

void check_data_fifo_full(void)
{
	u32 value = 0;
	int mipitx_fifo_timeout = 0;

	value = G204_MIPITX_REG0->sft_cfg[16];
	//printf("fifo_status 0x%08x\n", value);
	while((value & MIPITX_DATA_FIFO_FULL) == MIPITX_DATA_FIFO_FULL) {
		if(mipitx_fifo_timeout > 10000) //over 1 second
		{
			printf("data fifo full timeout\n");
			break;
		}
		value = G204_MIPITX_REG0->sft_cfg[16];
		++mipitx_fifo_timeout;
		udelay(100);
	}
}

void check_data_fifo_empty(void)
{
	u32 value = 0;
	int mipitx_fifo_timeout = 0;

	value = G204_MIPITX_REG0->sft_cfg[16];
	//printf("fifo_status 0x%08x\n", value);
	while((value & MIPITX_DATA_FIFO_EMPTY) != MIPITX_DATA_FIFO_EMPTY) {
		if(mipitx_fifo_timeout > 10000) //over 1 second
		{
			printf("data fifo empty timeout\n");
			break;
		}
		value = G204_MIPITX_REG0->sft_cfg[16];
		++mipitx_fifo_timeout;
		udelay(100);
	}
}

#if 0
/*
 * MIPI DSI (Display Command Set) for SP7350
 */
static void sp7350_dcs_write_buf(const void *data, size_t len)
{
	int i;
	u8 *data1;
	u32 value, data_cnt;

	data1 = (u8 *)data;

	if (len == 0) {
		check_cmd_fifo_full();
		value = 0x00000003;
		G204_MIPITX_REG0->sft_cfg[9] = value; //G204.09
	} else if (len == 1) {
		check_cmd_fifo_full();
		value = 0x00000013;
		value |= (data1[0] << 8);
		G204_MIPITX_REG0->sft_cfg[9] = value; //G204.09
	} else if (len == 2) {
		check_cmd_fifo_full();
		value = 0x00000023;
		value |= (data1[0] << 8) | (data1[1] << 16);
		G204_MIPITX_REG0->sft_cfg[9] = value; //G204.09
	//} else if ((len >= 3) && (len <= 64)) {
	} else if (len >= 3) {
		check_cmd_fifo_full();
		value = 0x00000029;
		value |= ((u32)len << 8);
		G204_MIPITX_REG0->sft_cfg[10] = value; //G204.10
		if (len % 4) data_cnt = ((u32)len / 4) + 1;
		else data_cnt = ((u32)len / 4);

		for (i = 0; i < data_cnt; i++) {
			check_data_fifo_full();
			value = 0x00000000;
			if (i * 4 + 0 >= len) data1[i * 4 + 0] = 0x00;
			if (i * 4 + 1 >= len) data1[i * 4 + 1] = 0x00;
			if (i * 4 + 2 >= len) data1[i * 4 + 2] = 0x00;
			if (i * 4 + 3 >= len) data1[i * 4 + 3] = 0x00;
			value |= ((data1[i * 4 + 3] << 24) | (data1[i * 4 + 2] << 16) |
				 (data1[i * 4 + 1] << 8) | (data1[i * 4 + 0] << 0));

			G204_MIPITX_REG0->sft_cfg[11] = value; //G204.11
		}

	} else {
		printf("data length over %ld\n", len);
	}
}

#define sp7350_dcs_write_seq(seq...)			\
({							\
	static const u8 d[] = { seq };			\
	sp7350_dcs_write_buf(d, ARRAY_SIZE(d));	\
})
#endif

/*
 * G204_MIPITX_REG0->sft_cfg[9] = 0x00000003; //command mode short pkt with 0 para
 * G204_MIPITX_REG0->sft_cfg[9] = 0x0000aa13; //command mode short pkt with 1 para
 * G204_MIPITX_REG0->sft_cfg[9] = 0x00bbaa23; //command mode short pkt with 2 para
 * 
 * G204_MIPITX_REG0->sft_cfg[10] = 0x00000029; //command mode long pkt
 * G204_MIPITX_REG0->sft_cfg[11] = 0xddccbbaa; //command mode payload
 */
void DRV_mipitx_panel_Init(int is_mipi_dsi_tx, int width, int height)
{
	if (is_mipi_dsi_tx) {
		#if 0
		printf("MIPITX DSI Panel : HXM0686TFT-001(%dx%d)\n", width, height);
		//Panel HXM0686TFT-001 IPS
		DRV_mipitx_gpio_set(( struct sp7350_disp_priv *)sp_gpio);

		sp7350_dcs_write_seq(0xB9, 0xF1, 0x12, 0x87);
		sp7350_dcs_write_seq(0xB2, 0x40, 0x05, 0x78);
		sp7350_dcs_write_seq(0xB3, 0x10, 0x10, 0x28, 0x28, 0x03, 0xFF, 0x00,
							0x00, 0x00, 0x00);
		sp7350_dcs_write_seq(0xB4, 0x80);
		sp7350_dcs_write_seq(0xB5, 0x09, 0x09);
		sp7350_dcs_write_seq(0xB6, 0x8D, 0x8D);
		sp7350_dcs_write_seq(0xB8, 0x26, 0x22, 0xF0, 0x63);
		sp7350_dcs_write_seq(0xBA, 0x33, 0x81, 0x05, 0xF9, 0x0E, 0x0E, 0x20,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44,
							0x25, 0x00, 0x91, 0x0A, 0x00, 0x00, 0x01, 0x4F,
							0x01, 0x00, 0x00, 0x37);
		sp7350_dcs_write_seq(0xBC, 0x47);
		sp7350_dcs_write_seq(0xBF, 0x02, 0x10, 0x00, 0x80, 0x04);
		sp7350_dcs_write_seq(0xC0, 0x73, 0x73, 0x50, 0x50, 0x00, 0x00, 0x12,
							0x70, 0x00);
		sp7350_dcs_write_seq(0xC1, 0x65, 0xC0, 0x32, 0x32, 0x77, 0xF4, 0x77,
							0x77, 0xCC, 0xCC, 0xFF, 0xFF, 0x11, 0x11, 0x00,
							0x00, 0x32);
		sp7350_dcs_write_seq(0xC7, 0x10, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00,
							0x00, 0xED, 0xC7, 0x00, 0xA5);
		sp7350_dcs_write_seq(0xC8, 0x10, 0x40, 0x1E, 0x03);
		sp7350_dcs_write_seq(0xCC, 0x0B);
		sp7350_dcs_write_seq(0xE0, 0x00, 0x04, 0x06, 0x32, 0x3F, 0x3F, 0x36,
							0x34, 0x06, 0x0B, 0x0E, 0x11, 0x11, 0x10, 0x12,
							0x10, 0x13, 0x00, 0x04, 0x06, 0x32, 0x3F, 0x3F,
							0x36, 0x34, 0x06, 0x0B, 0x0E, 0x11, 0x11, 0x10,
							0x12, 0x10, 0x13);
		sp7350_dcs_write_seq(0xE1, 0x11, 0x11, 0x91, 0x00, 0x00, 0x00, 0x00);
		sp7350_dcs_write_seq(0xE3, 0x03, 0x03, 0x0B, 0x0B, 0x00, 0x03, 0x00,
							0x00, 0x00, 0x00, 0xFF, 0x84, 0xC0, 0x10);
		sp7350_dcs_write_seq(0xE9, 0xC8, 0x10, 0x06, 0x05, 0x18, 0xD2, 0x81,
							0x12, 0x31, 0x23, 0x47, 0x82, 0xB0, 0x81, 0x23,
							0x04, 0x00, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00,
							0x04, 0x04, 0x00, 0x00, 0x00, 0x88, 0x0B, 0xA8,
							0x10, 0x32, 0x4F, 0x88, 0x88, 0x88, 0x88, 0x88,
							0x88, 0x0B, 0xA8, 0x10, 0x32, 0x4F, 0x88, 0x88,
							0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
		sp7350_dcs_write_seq(0xEA, 0x96, 0x0C, 0x01, 0x01, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x4B, 0xA8,
							0x23, 0x01, 0x08, 0xF8, 0x88, 0x88, 0x88, 0x88,
							0x88, 0x4B, 0xA8, 0x23, 0x01, 0x08, 0xF8, 0x88,
							0x88, 0x88, 0x88, 0x23, 0x10, 0x00, 0x00, 0x92,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
							0x80, 0x81, 0x00, 0x00, 0x00, 0x00);
		sp7350_dcs_write_seq(0xEF, 0xFF, 0xFF, 0x01);

		sp7350_dcs_write_seq(0x36, 0x14);
		sp7350_dcs_write_seq(0x35, 0x00);
		sp7350_dcs_write_seq(0x11);
		mdelay(120);
		sp7350_dcs_write_seq(0x29);
		mdelay(20);
		#else
		printf("MIPITX DSI Panel : HXM0686TFT-001(%dx%d)\n", width, height);
		//Panel HXM0686TFT-001 IPS
		DRV_mipitx_gpio_set(( struct sp7350_disp_priv *)sp_gpio);

		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000429; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x8712F1B9; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000429; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x780540B2; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000b29; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x281010B3; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00ff0328; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[9] = 0x0080B423; //G204.09
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000329; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x000909B5; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000329; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x008D8DB6; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000529; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0xF02226B8; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000063; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00001c29; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x058133BA; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x200E0EF9; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x44000000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x0A910025; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x4F010000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x37000001; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[9] = 0x0047BC23; //G204.09
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000629; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x001002BF; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000480; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000a29; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x507373C0; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x12000050; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000070; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00001229; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x32C065C1; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x77F47732; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0xFFCCCC77; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x001111FF; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00003200; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000d29; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x0A0010C7; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00C7ED00; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x000000A5; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000529; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x1E4010C8; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000003; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[9] = 0x000BCC23; //G204.09
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00002329; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x060400E0; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x363F3F32; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x0E0B0634; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x12101111; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x04001310; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x3F3F3206; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x0B063436; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x1011110E; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00131012; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000829; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x911111E1; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000f29; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x0B0303E3; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x0003000B; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0xFF000000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x0010C084; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00004029; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x0610C8E9; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x81D21805; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x47233112; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x2381B082; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x04040004; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000404; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0xA80B8800; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x884F3210; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x88888888; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x10A80B88; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x88884F32; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00888888; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00003E29; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x010C96EA; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000001; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0xA84B8800; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0xF8080123; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x88888888; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x23A84B88; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x88F80801; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x23888888; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x92000010; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x20000000; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00008180; //G204.11
		G204_MIPITX_REG0->sft_cfg[11] = 0x00000000; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[10] = 0x00000429; //G204.10
		G204_MIPITX_REG0->sft_cfg[11] = 0x01FFFFEF; //G204.11
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[9] = 0x00143623; //G204.09
		check_cmd_fifo_empty();
		check_data_fifo_empty();

		G204_MIPITX_REG0->sft_cfg[9] = 0x00003523; //G204.09
		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[9] = 0x00001113; //G204.09

		mdelay(120);

		check_cmd_fifo_empty();
		check_data_fifo_empty();
		G204_MIPITX_REG0->sft_cfg[9] = 0x00002913; //G204.09

		mdelay(20);
		#endif
	} else {
		printf("DRV_mipitx_panel_Init for CSI(none)\n");
	}
}
