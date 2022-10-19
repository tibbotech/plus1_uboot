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
/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
void DRV_mipitx_Init(int is_mipitx, int width, int height)
//void DRV_hdmitx_Init(int is_hdmi, int width, int height)
{
    printf("DRV_mipitx_Init w %d h %d\n", width, height);

	G205_MIPITX_REG1->sft_cfg[6] = 0x00000000; //PHY Reset

	G204_MIPITX_REG0->sft_cfg[0] = 0x04080005; //fix

	if ((width == 64) && (height == 64)) {
		G204_MIPITX_REG0->sft_cfg[1] = 0x0001061c; //VSA=0x01 VFP=0x06 VBP=0x1C
		//G204_MIPITX_REG0->sft_cfg[1] = 0x00010611; //VSA=0x01 VFP=0x06 VBP=0x11
		G204_MIPITX_REG0->sft_cfg[2] = 0x00000040;
	}
	else if ((width == 64) && (height == 2880)) {
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010628; //VSA=0x01 VFP=0x06 VBP=0x28
		G204_MIPITX_REG0->sft_cfg[2] = 0x00000B40;
	}
	else if ((width == 128) && (height == 128)) {
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010212; //VSA=0x01 VFP=0x02 VBP=0x12
		G204_MIPITX_REG0->sft_cfg[2] = 0x00000080;
	}
	else if ((width == 720) && (height == 480)) {
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010823; //VSA=0x01 VFP=0x08 VBP=0x23
		G204_MIPITX_REG0->sft_cfg[2] = 0x000001E0;
	}
	else if ((width == 720) && (height == 576)) {
		G204_MIPITX_REG0->sft_cfg[1] = 0x0001042B; //VSA=0x01 VFP=0x04 VBP=0x2B
		G204_MIPITX_REG0->sft_cfg[2] = 0x00000240;
	}
	else if ((width == 1280) && (height == 720)) {
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010418; //VSA=0x01 VFP=0x04 VBP=0x18
		G204_MIPITX_REG0->sft_cfg[2] = 0x000002D0;
	}
	else if ((width == 1920) && (height == 1080)) {
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010328; //VSA=0x01 VFP=0x03 VBP=0x28
		G204_MIPITX_REG0->sft_cfg[2] = 0x00000438;
	}
	else if ((width == 3840) && (height == 64)) {
		G204_MIPITX_REG0->sft_cfg[1] = 0x0001061c; //VSA=0x01 VFP=0x06 VBP=0x1c
		G204_MIPITX_REG0->sft_cfg[2] = 0x00000040;
	}
	else if ((width == 3840) && (height == 2880)) {
		G204_MIPITX_REG0->sft_cfg[1] = 0x00010628; //VSA=0x01 VFP=0x06 VBP=0x28
		G204_MIPITX_REG0->sft_cfg[2] = 0x00000B40;
	}

	G204_MIPITX_REG0->sft_cfg[5] = 0x00100008; //fix
	G204_MIPITX_REG0->sft_cfg[6] = 0x00100010; //fix
	G204_MIPITX_REG0->sft_cfg[7] = 0x0a120020; //fix
	G204_MIPITX_REG0->sft_cfg[8] = 0x0a050010; //fix
	G204_MIPITX_REG0->sft_cfg[29] = 0x000000af; //fix
	G204_MIPITX_REG0->sft_cfg[12] = 0x00001030; //fix
	G204_MIPITX_REG0->sft_cfg[13] = 0x00001100; //fix
	G204_MIPITX_REG0->sft_cfg[14] = 0x00100000; //fix
	G204_MIPITX_REG0->sft_cfg[15] = 0x11000031; //fix

	//MIPITX_REG0->sft_cfg[19] = 0x000000c0;
	if ((width == 64) && (height == 64)) {
		G204_MIPITX_REG0->sft_cfg[19] = 0x004000c0; //for new version RTL
	}
	else if ((width == 64) && (height == 2880)) {
		G204_MIPITX_REG0->sft_cfg[19] = 0x004000c0; //for new version RTL
	}
	else if ((width == 128) && (height == 128)) {
		G204_MIPITX_REG0->sft_cfg[19] = 0x00800180; //for new version RTL
	}
	else if ((width == 720) && (height == 480)) {
		G204_MIPITX_REG0->sft_cfg[19] = 0x02D00870; //for new version RTL
	}
	else if ((width == 720) && (height == 576)) {
		G204_MIPITX_REG0->sft_cfg[19] = 0x02D00870; //for new version RTL
	}
	else if ((width == 1280) && (height == 720)) {
		G204_MIPITX_REG0->sft_cfg[19] = 0x05000F00; //for new version RTL
	}
	else if ((width == 1920) && (height == 1080)) {
		G204_MIPITX_REG0->sft_cfg[19] = 0x07801680; //for new version RTL
	}
	else if ((width == 3840) && (height == 64)) {
		G204_MIPITX_REG0->sft_cfg[19] = 0x0F002D00; //for new version RTL
	}
	else if ((width == 3840) && (height == 2880)) {
		G204_MIPITX_REG0->sft_cfg[19] = 0x0F002D00; //for new version RTL
	}
	
	G204_MIPITX_REG0->sft_cfg[30] = 0x00000001; //fix

	G205_MIPITX_REG1->sft_cfg[6] = 0x00000001; //PHY Reset

}

