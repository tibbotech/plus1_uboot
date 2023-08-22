// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sunplus - All Rights Reserved
 *
 * Author(s): Hammer Hsieh <hammer.hsieh@sunplus.com>
 */

#include <common.h>
#include "reg_disp.h"
#include "display2.h"

//static const char * const ImgreaddmaFmt[] = {"RGB888", "RGB565", "YUY2", "NV16", "rsv", "rsv", "NV24", "NV12"};
/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
void DRV_VPP_Init(int width, int height)
{
	//printf("DRV_VPP_Init w %d h %d\n", width, height);

	//G185_IMGREAD_REG->sft_cfg[1] = 0x80000f28; //turn on vpp

	G185_IMGREAD_REG->sft_cfg[2] = 0x00020002; //BIST_OFF, YUY2 progressive
	//G185_IMGREAD_REG->sft_cfg[2] = 0x00020022; //BIST_EN -- color bar
	//G185_IMGREAD_REG->sft_cfg[2] = 0x00020032; //BIST_EN -- border
	G185_IMGREAD_REG->sft_cfg[3] = 0x00000000 | (height << 16) | (width << 0); // HEIGHT = 64 , WIDTH = 64
	/* ADDR2 LineStride, ADDR1 LineStride */
	G185_IMGREAD_REG->sft_cfg[5] = 0x00000000 | (width*2); //YUY2/UYVY addr1 0x80 , addr2 --

	G186_VSCL_REG0->sft_cfg[0] = 0x00000000;
	//G186_VSCL_REG0->sft_cfg[1] = 0x00000200; //BIST_OFF (default setting)
	G186_VSCL_REG0->sft_cfg[1] = 0x0000021f; //BIST_OFF
	//G186_VSCL_REG0->sft_cfg[1] = 0x0000029f; //BIST_EN -- color bar
	//G186_VSCL_REG0->sft_cfg[1] = 0x0000039f; //BIST_EN -- border
	G186_VSCL_REG0->sft_cfg[3] = (0x00000000 | width); //input_width
	G186_VSCL_REG0->sft_cfg[4] = (0x00000000 | height); //input_height
	G186_VSCL_REG0->sft_cfg[5] = 0x00000000;
	G186_VSCL_REG0->sft_cfg[6] = 0x00000000;
	G186_VSCL_REG0->sft_cfg[7] = (0x00000000 | width); //input_width
	G186_VSCL_REG0->sft_cfg[8] = (0x00000000 | height); //input_height
	G186_VSCL_REG0->sft_cfg[9] = (0x00000000 | width); //output_width
	G186_VSCL_REG0->sft_cfg[10] = (0x00000000 | height); //output_height
	G186_VSCL_REG0->sft_cfg[11] = 0x00000000;
	G186_VSCL_REG0->sft_cfg[12] = 0x00000000;
	G186_VSCL_REG0->sft_cfg[13] = (0x00000000 | width); //output_width
	G186_VSCL_REG0->sft_cfg[14] = (0x00000000 | height); //output_height

	G186_VSCL_REG0->sft_cfg[18] = 0x00000002;
	G186_VSCL_REG0->sft_cfg[19] = 0x00000000;
	G186_VSCL_REG0->sft_cfg[20] = 0x00000040; //input_w*64/output_w

	G187_VSCL_REG1->sft_cfg[0] = 0x00000002;
	G187_VSCL_REG1->sft_cfg[1] = 0x00000000;
	G187_VSCL_REG1->sft_cfg[2] = 0x00000040; //input_h*64/output_h

}

void vpost_setting(int x, int y, int input_w, int input_h, int output_w, int output_h)
{
	;//TBD

}

void vscl_setting(int x, int y, int input_w, int input_h, int output_w, int output_h)
{
	#if 0
	printf("vscl position (x, y) =(%d, %d)\n", x, y);
	printf("IN (%d, %d), OUT (%d, %d)\n", input_w, input_h, output_w, output_h);

	if ((input_w == output_w) && (input_h == output_h))
		printf("scaling ratio (x, y) = (1000,1000) \n");
	else {
		printf("scaling ratio (x, y) = (%04d,%04d) \n", output_w * 1000 / input_w, output_h * 1000 / input_h);
	}

	G187_VSCL_REG->vscl0_actrl_i_xlen 	= input_w;	//G187.03
	G187_VSCL_REG->vscl0_actrl_i_ylen 	= input_h;	//G187.04
	G187_VSCL_REG->vscl0_actrl_s_xstart	= x;	//G187.05
	G187_VSCL_REG->vscl0_actrl_s_ystart	= y;	//G187.06
	G187_VSCL_REG->vscl0_actrl_s_xlen	= input_w;	//G187.07
	G187_VSCL_REG->vscl0_actrl_s_ylen	= input_h;	//G187.08
	G187_VSCL_REG->vscl0_dctrl_o_xlen	= output_w;	//G187.09
	G187_VSCL_REG->vscl0_dctrl_o_ylen	= output_h;	//G187.10	
	G187_VSCL_REG->vscl0_dctrl_d_xstart	= x;	//G187.11
	G187_VSCL_REG->vscl0_dctrl_d_ystart	= y;	//G187.12
	G187_VSCL_REG->vscl0_dctrl_d_xlen	= output_w;	//G187.13
	G187_VSCL_REG->vscl0_dctrl_d_ylen	= output_h;	//G187.14

	G187_VSCL_REG->vscl0_hint_ctrl			= 0x00000002;	//G187.18
	G187_VSCL_REG->vscl0_hint_hfactor_low	= 0x00000000;	//G187.19
	//G187_VSCL_REG->vscl0_hint_hfactor_high	= 0x00000040;	//G187.20
	G187_VSCL_REG->vscl0_hint_hfactor_high	= (input_w*64)/output_w;	//G187.20
	G187_VSCL_REG->vscl0_hint_initf_low		= 0x00000000;	//G187.21
	G187_VSCL_REG->vscl0_hint_initf_high		= 0x00000000;	//G187.22	
	
	G188_VSCL_REG->vscl0_vint_ctrl			= 0x00000002;	//G188.00
	G188_VSCL_REG->vscl0_vint_vfactor_low	= 0x00000000;	//G188.01
	//G188_VSCL_REG->vscl0_vint_vfactor_high	= 0x00000040;	//G188.02
	G188_VSCL_REG->vscl0_vint_vfactor_high	= (input_h*64)/output_h;	//G188.02
	G188_VSCL_REG->vscl0_vint_initf_low		= 0x00000000;	//G188.03
	G188_VSCL_REG->vscl0_vint_initf_high		= 0x00000000;	//G188.04

	printf("scaling reg (x, y) = (0x%08x,0x%08x) \n", G187_VSCL_REG->vscl0_hint_hfactor_high, G188_VSCL_REG->vscl0_vint_vfactor_high);
	#endif
}

void imgread_setting(int luma_addr, int chroma_addr, int w, int h, int imgread_fmt)
{
	#if 0
	int vdma_cfg_tmp, vdma_frame_size_tmp;

	printf("vppdma luma=0x%x, chroma=0x%x\n", luma_addr, chroma_addr);
	printf("vppdma w=%d, h=%d, fmt= %s\n", w, h, ImgreaddmaFmt[imgread_fmt]);

	G186_VPPDMA_REG->vdma_gc 		= 0x80000028; //G186.01 , vppdma en , urgent th = 0x28

	vdma_cfg_tmp = G186_VPPDMA_REG->vdma_cfg;

	vdma_cfg_tmp &= ~((1 << 12) | (1 << 11) | (7 << 8) | (1 << 7) | (1 << 6) | (3 << 0));

	if (vppdma_fmt == 0)
		vdma_cfg_tmp 			= (0 << 12) | (0 << 11) | (0 << 8) | (0 << 7) | (0 << 6) | (0 << 0); //source RGB888 (default)
	else if(vppdma_fmt == 1)
		vdma_cfg_tmp 			= (0 << 12) | (0 << 11) | (0 << 8) | (0 << 7) | (0 << 6) | (1 << 0); //source RGB565
	else if(vppdma_fmt == 2)
		vdma_cfg_tmp 			= (0 << 12) | (0 << 11) | (0 << 8) | (0 << 7) | (0 << 6) | (2 << 0); //source YUV422 UYVY
	else if(vppdma_fmt == 3)
		vdma_cfg_tmp 			= (0 << 12) | (0 << 11) | (0 << 8) | (0 << 7) | (0 << 6) | (3 << 0); //source YUV422 NV16
	else if(vppdma_fmt == 4)
		vdma_cfg_tmp 			= (1 << 12) | (0 << 11) | (0 << 8) | (0 << 7) | (0 << 6) | (2 << 0); //source YUV422 YUY2		

	G186_VPPDMA_REG->vdma_cfg = vdma_cfg_tmp;

	vdma_frame_size_tmp = G186_VPPDMA_REG->vdma_frame_size;
	vdma_frame_size_tmp &= ~( (0xfff << 16) | (0x1fff << 0) );
	vdma_frame_size_tmp = ((h << 16) | w);
	G186_VPPDMA_REG->vdma_frame_size = vdma_frame_size_tmp;

	//G186_VPPDMA_REG->vdma_frame_size 			= ((h<<16) | w);; //G186.03 , frame_h , frame_w
	G186_VPPDMA_REG->vdma_crop_st 				= 0x00000000; //G186.04 , start_h , Start_w
	
	if (vppdma_fmt == 0)
		G186_VPPDMA_REG->vdma_lstd_size 		= w*3; //G186.05 , stride size , source RGB888 (default)
	else if (vppdma_fmt == 1)
		G186_VPPDMA_REG->vdma_lstd_size 		= w*2; //G186.05 , stride size , source RGB565
	else if (vppdma_fmt == 2)
		G186_VPPDMA_REG->vdma_lstd_size 		= w*2; //G186.05 , stride size , source YUV422 UYVY
	else if (vppdma_fmt == 3)
		G186_VPPDMA_REG->vdma_lstd_size 		= w; //G186.05 , stride size , source YUV422 NV16
	else if (vppdma_fmt == 4)
		G186_VPPDMA_REG->vdma_lstd_size 		= w*2; //G186.05 , stride size , source YUV422 YUY2

	if (luma_addr != 0)
		G186_VPPDMA_REG->vdma_data_addr1 		= luma_addr; //G186.06 , 1st planner addr (luma)
	if (chroma_addr != 0)
		G186_VPPDMA_REG->vdma_data_addr2 		= chroma_addr; //G186.07 , 2nd planner addr (crma)
	#endif
}