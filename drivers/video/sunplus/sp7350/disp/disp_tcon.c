// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Sunplus - All Rights Reserved
 *
 * Author(s): Hammer Hsieh <hammer.hsieh@sunplus.com>
 */

#include <common.h>
#include "reg_disp.h"
#include "disp_tcon.h"
/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
void DRV_TCON_Init(int width, int height)
{
	//printf("DRV_TCON_Init w %d h %d\n", width, height);

	G199_TCON0_REG0->sft_cfg[0] = 0x00008127;
	G199_TCON0_REG0->sft_cfg[1] = 0x00008011;
	G199_TCON0_REG0->sft_cfg[2] = 0x00000011;
	G199_TCON0_REG0->sft_cfg[3] = 0x00002002;

	if ((width == 720) && (height == 480)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x00000352;
		G199_TCON0_REG0->sft_cfg[13] = 0x00000356;
		G199_TCON0_REG0->sft_cfg[20] = 0x0000020c;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x00000352;
		G199_TCON0_REG0->sft_cfg[24] = 0x00000356;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x000002cf;
		//G199_TCON0_REG0->sft_cfg[27] = 0x00000028;
		//G199_TCON0_REG0->sft_cfg[28] = 0x00000b69;
	}
	else if ((width == 800) && (height == 480)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x00000398;
		G199_TCON0_REG0->sft_cfg[13] = 0x00000398;
		G199_TCON0_REG0->sft_cfg[20] = 0x0000020c;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x00000398;
		G199_TCON0_REG0->sft_cfg[24] = 0x0000039c;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x0000031f;
		//G199_TCON0_REG0->sft_cfg[27] = 0x00000028;
		//G199_TCON0_REG0->sft_cfg[28] = 0x00000b69;
	}
	else if ((width == 1024) && (height == 600)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x00000538;
		G199_TCON0_REG0->sft_cfg[13] = 0x00000538;
		G199_TCON0_REG0->sft_cfg[20] = 0x0000027a;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x00000538;
		G199_TCON0_REG0->sft_cfg[24] = 0x0000053c;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x000003ff;
		//G199_TCON0_REG0->sft_cfg[27] = 0x00000028;
		//G199_TCON0_REG0->sft_cfg[28] = 0x00000b69;
	} else if ((width == 480) && (height == 1280)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x00000264;//0x00000269;
		G199_TCON0_REG0->sft_cfg[13] = 0x00000268;//0x00000269;
		G199_TCON0_REG0->sft_cfg[20] = 0x00000521;//0x0000052d;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x00000264;//0x00000269;
		G199_TCON0_REG0->sft_cfg[24] = 0x00000268;//0x0000026d;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x000001df;
		//G199_TCON0_REG0->sft_cfg[27] = 0x00000028;
		//G199_TCON0_REG0->sft_cfg[28] = 0x00000b69;
	} else if ((width == 1280) && (height == 480)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x0000060a;
		G199_TCON0_REG0->sft_cfg[13] = 0x0000060a;
		G199_TCON0_REG0->sft_cfg[20] = 0x0000020c;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x0000060a;
		G199_TCON0_REG0->sft_cfg[24] = 0x0000060e;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x000004ff;
		//G199_TCON0_REG0->sft_cfg[27] = 0x00000028;
		//G199_TCON0_REG0->sft_cfg[28] = 0x00000b69;
	} else if ((width == 1280) && (height == 720)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x0000066A;
		G199_TCON0_REG0->sft_cfg[13] = 0x0000066A;
		G199_TCON0_REG0->sft_cfg[20] = 0x000002ED;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x0000066A;
		G199_TCON0_REG0->sft_cfg[24] = 0x0000066E;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x000004ff;
		//G199_TCON0_REG0->sft_cfg[27] = 0x00000028;
		//G199_TCON0_REG0->sft_cfg[28] = 0x00000b69;
	}
	else if ((width == 1920) && (height == 1080)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x00000890;
		G199_TCON0_REG0->sft_cfg[13] = 0x00000894;
		G199_TCON0_REG0->sft_cfg[20] = 0x00000464;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x00000890;
		G199_TCON0_REG0->sft_cfg[24] = 0x00000894;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x0000077f;
		G199_TCON0_REG0->sft_cfg[27] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[28] = 0x0000077f;
	}
	else if ((width == 3840) && (height == 2880)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x000011f8;
		G199_TCON0_REG0->sft_cfg[13] = 0x000011f8;
		G199_TCON0_REG0->sft_cfg[20] = 0x00000c7f;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x000011f8;
		G199_TCON0_REG0->sft_cfg[24] = 0x000011fc;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x00000eff;
		G199_TCON0_REG0->sft_cfg[27] = 0x00000028;
		G199_TCON0_REG0->sft_cfg[28] = 0x00000b69;
	}
	G199_TCON0_REG0->sft_cfg[31] = 0x00001ffe;

	//for TPG setting
	G200_TCON0_REG1->sft_cfg[4] = 0x00000000;
	G200_TCON0_REG1->sft_cfg[5] = 0x00000000;
	G200_TCON0_REG1->sft_cfg[6] = 0x00000000; //default setting

	if ((width == 720) && (height == 480)) {
		//TBD
	} else if ((width == 800) && (height == 480)) {
		//TBD
	} else if ((width == 1024) && (height == 600)) {
		//TBD
	} else if ((width == 480) && (height == 1280)) {
		//G200_TCON0_REG1->sft_cfg[6] = 0x00000001; //Internal , H color bar
		G200_TCON0_REG1->sft_cfg[7] = 0x0000026b; //HTT 0x26c -1
		G200_TCON0_REG1->sft_cfg[8] = 0x00004521; //HSTEP=0x4 VTT 0x522 -1
		G200_TCON0_REG1->sft_cfg[9] = 0x000001df; //V_ACT=0x1e0-1
		G200_TCON0_REG1->sft_cfg[10] = 0x000044ff; //VSTEP=0x4 V_ACT=0x500-1
		G200_TCON0_REG1->sft_cfg[21] = 0x00000011; //sync with G204.01 VBP
	} else if ((width == 1280) && (height == 480)) {
		//TBD
	} else if ((width == 1280) && (height == 720)) {
		//TBD
	} else if ((width == 1920) && (height == 1080)) {
		//TBD
	} else if ((width == 3840) && (height == 2880)) {
		//TBD
	}

	G200_TCON0_REG1->sft_cfg[15] = 0x00000000;
	G200_TCON0_REG1->sft_cfg[26] = 0x00000004;

}
