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
	printf("tcon_setting w %d h %d\n", width, height);
	G199_TCON0_REG0->sft_cfg[0] = 0x00008127;
	G199_TCON0_REG0->sft_cfg[1] = 0x00008011;
	G199_TCON0_REG0->sft_cfg[2] = 0x00000011;
	G199_TCON0_REG0->sft_cfg[3] = 0x00002002;

	if ((width == 64) && (height == 64)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x00000161;
		G199_TCON0_REG0->sft_cfg[13] = 0x00000161;
		G199_TCON0_REG0->sft_cfg[20] = 0x00000063;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x00000161;
		G199_TCON0_REG0->sft_cfg[24] = 0x00000164;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x0000003f;
		G199_TCON0_REG0->sft_cfg[27] = 0x0000001c;
		G199_TCON0_REG0->sft_cfg[28] = 0x0000005c;
	}
	else if ((width == 64) && (height == 2880)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x00000160;
		G199_TCON0_REG0->sft_cfg[13] = 0x00000160;
		G199_TCON0_REG0->sft_cfg[20] = 0x00000c7f;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x00000160;
		G199_TCON0_REG0->sft_cfg[24] = 0x00000164;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x0000003f;
		G199_TCON0_REG0->sft_cfg[27] = 0x00000028;
		G199_TCON0_REG0->sft_cfg[28] = 0x00000b69;
	}
	else if ((width == 128) && (height == 128)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x00000160;
		G199_TCON0_REG0->sft_cfg[13] = 0x00000160;
		G199_TCON0_REG0->sft_cfg[20] = 0x00000095;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x00000160;
		G199_TCON0_REG0->sft_cfg[24] = 0x00000164;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x0000007f;
		//G199_TCON0_REG0->sft_cfg[27] = 0x00000028;
		//G199_TCON0_REG0->sft_cfg[28] = 0x00000b69;
	}
	else if ((width == 720) && (height == 480)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x00000352;
		G199_TCON0_REG0->sft_cfg[13] = 0x00000352;
		G199_TCON0_REG0->sft_cfg[20] = 0x0000020c;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x00000352;
		G199_TCON0_REG0->sft_cfg[24] = 0x00000356;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x000002cf;
		//G199_TCON0_REG0->sft_cfg[27] = 0x00000028;
		//G199_TCON0_REG0->sft_cfg[28] = 0x00000b69;
	}
	else if ((width == 720) && (height == 576)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x00000358;
		G199_TCON0_REG0->sft_cfg[13] = 0x00000358;
		G199_TCON0_REG0->sft_cfg[20] = 0x00000270;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x00000358;
		G199_TCON0_REG0->sft_cfg[24] = 0x0000035c;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x000002cf;
		//G199_TCON0_REG0->sft_cfg[27] = 0x00000028;
		//G199_TCON0_REG0->sft_cfg[28] = 0x00000b69;
	}
	else if ((width == 1280) && (height == 720)) {
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
		G199_TCON0_REG0->sft_cfg[13] = 0x00000890;
		G199_TCON0_REG0->sft_cfg[20] = 0x00000464;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x00000890;
		G199_TCON0_REG0->sft_cfg[24] = 0x00000894;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x0000077f;
		G199_TCON0_REG0->sft_cfg[27] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[28] = 0x0000077f;
	}
	else if ((width == 3840) && (height == 64)) {
		G199_TCON0_REG0->sft_cfg[12] = 0x000011f8;
		G199_TCON0_REG0->sft_cfg[13] = 0x000011f8;
		G199_TCON0_REG0->sft_cfg[20] = 0x00000063;
		G199_TCON0_REG0->sft_cfg[21] = 0x00000000;

		G199_TCON0_REG0->sft_cfg[23] = 0x000011f8;
		G199_TCON0_REG0->sft_cfg[24] = 0x000011fc;

		G199_TCON0_REG0->sft_cfg[25] = 0x00000000;
		G199_TCON0_REG0->sft_cfg[26] = 0x00000eff;
		G199_TCON0_REG0->sft_cfg[27] = 0x0000001c;
		G199_TCON0_REG0->sft_cfg[28] = 0x0000005d;
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
	//G200_TCON0_REG1->sft_cfg[6] = 0x00000001; //Internal , H color bar
	//G200_TCON0_REG1->sft_cfg[6] = 0x00000005; //Internal , H RAMP
	//G200_TCON0_REG1->sft_cfg[6] = 0x00000009; //Internal , H ODDEVEN
	//G200_TCON0_REG1->sft_cfg[6] = 0x0000000d; //Internal , V color bar
	//G200_TCON0_REG1->sft_cfg[6] = 0x00000011; //Internal , V RAMP
	//G200_TCON0_REG1->sft_cfg[6] = 0x00000015; //Internal , V ODDEVEN
	//G200_TCON0_REG1->sft_cfg[6] = 0x00000019; //Internal , HV CHECK
	//G200_TCON0_REG1->sft_cfg[6] = 0x0000001d; //Internal , HV FRAME
	//G200_TCON0_REG1->sft_cfg[6] = 0x00000021; //Internal , HV MOIRE_A
	//G200_TCON0_REG1->sft_cfg[6] = 0x00000025; //Internal , HV MOIRE_B
	//G200_TCON0_REG1->sft_cfg[6] = 0x00000029; //Internal , HV CONTRAST
	//G200_TCON0_REG1->sft_cfg[6] = 0x0000002d; //Internal , H color bar

	G200_TCON0_REG1->sft_cfg[7] = 0x00000167;
	G200_TCON0_REG1->sft_cfg[8] = 0x00004063; //HSTEP=0x4 H_ACT=0x063
	G200_TCON0_REG1->sft_cfg[9] = 0x0000003f;
	G200_TCON0_REG1->sft_cfg[10] = 0x0000403f; //VSTEP=0x4 V_ACT=0x03F
	G200_TCON0_REG1->sft_cfg[15] = 0x00000000;
	G200_TCON0_REG1->sft_cfg[26] = 0x00000004;

}
