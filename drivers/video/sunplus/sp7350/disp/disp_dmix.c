// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sunplus - All Rights Reserved
 *
 * Author(s): Hammer Hsieh <hammer.hsieh@sunplus.com>
 */

#include <common.h>
#include "reg_disp.h"
#include "disp_dmix.h"
#include "disp_tgen.h"
#include "display2.h"

char * const LayerNameStr[] = {"BG", "L1", "L2", "L3", "L4", "L5", "L6"};
char * const LayerModeStr[] = {"AlphaBlend", "Transparent", "Opacity"};
char * const SelStr[] = {"VPP0", "VPP1", "VPP2", "OSD0", "OSD1", "OSD2", "OSD3", "PTG"};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
void DRV_DMIX_Init(void)
{
	printf("DRV_DMIX_Init\n");
	G198_DMIX_REG->sft_cfg[0] = 0x34561070; //default setting
	//G198_DMIX_REG->sft_cfg[1] = 0x00000000;
	//G198_DMIX_REG->sft_cfg[1] = 0x00000556;
	//G198_DMIX_REG->sft_cfg[1] = 0x00000954;
	//G198_DMIX_REG->sft_cfg[1] = 0x00000155;
	G198_DMIX_REG->sft_cfg[1] = 0x00000955;
	G198_DMIX_REG->sft_cfg[9] = 0x00002001;
	//G198_DMIX_REG->sft_cfg[10] = 0x000000ff; //Htype=0xf, Vtype=0xf
	//G198_DMIX_REG->sft_cfg[11] = 0x0029f06e; //blue for BackGround layer
	G198_DMIX_REG->sft_cfg[11] = 0x00108080; //black for BackGround layer (default setting)
	G198_DMIX_REG->sft_cfg[20] = 0x00000002;
}

DRV_Status_e DRV_DMIX_PTG_ColorBar(DRV_DMIX_TPG_e tpg_sel, int bg_color_yuv, int border_len)
{
	int dmix_mode = G198_DMIX_REG->sft_cfg[9] & (~0xf000);

	if (tpg_sel >= DRV_DMIX_TPG_MAX)
		return DRV_ERR_INVALID_PARAM;

	switch (tpg_sel) {
	default:
	case DRV_DMIX_TPG_MAX:
	case DRV_DMIX_TPG_BGC:
		G198_DMIX_REG->sft_cfg[9] = dmix_mode | (2 << 12);
		DRV_DMIX_PTG_Color_Set_YCbCr(1, (bg_color_yuv>>16) & 0xff, (bg_color_yuv>>8) & 0xff, (bg_color_yuv>>0) & 0xff);
		break;
	case DRV_DMIX_TPG_V_COLORBAR:
		G198_DMIX_REG->sft_cfg[9] = dmix_mode | (0 << 15) | (0 << 14) | (0 << 12);
		break;
	case DRV_DMIX_TPG_H_COLORBAR:
		G198_DMIX_REG->sft_cfg[9] = dmix_mode | (1 << 15) | (0 << 14) | (0 << 12);
		break;
	case DRV_DMIX_TPG_BORDER:
		G198_DMIX_REG->sft_cfg[9] = dmix_mode | (0 << 15) | (0 << 14) | (1 << 12);
		DRV_DMIX_PTG_Color_Set_YCbCr(0, 0, 0, 0);
		break;
	case DRV_DMIX_TPG_REGION:
		G198_DMIX_REG->sft_cfg[9] = dmix_mode | (0 << 15) | (0 << 14) | (2 << 12);
		//DRV_DMIX_PTG_Color_Set_YCbCr(0, 0, 0, 0);
		break;
	case DRV_DMIX_TPG_SNOW:
		G198_DMIX_REG->sft_cfg[9] = dmix_mode | (0 << 15) | (1 << 14) | (0 << 12);
		//DRV_DMIX_PTG_Color_Set_YCbCr(0, 0, 0, 0);
		break;
	}

	return DRV_SUCCESS;
}

void DRV_DMIX_PTG_Color_Set(UINT32 color)
{
	UINT16 Y, Cb, Cr;
	UINT16 R, G, B;

	R = (color >> 16) & 0xff;
	G = (color >> 8) & 0xff;
	B = color & 0xff;

	Y = (R * 76 / 255) + (G * 150 / 255) + (B * 29 / 255);
	Cb = -(R * 43 / 255) - (G * 85 / 255) + (B * 128 / 255) + 128;
	Cr = (R * 128 / 255) - (G * 107 / 255) - (B * 21 / 255) + 128;

	if (Cb > 255)
		Cb = 255;
	if (Cr > 255)
		Cr = 255;

	DRV_DMIX_PTG_Color_Set_YCbCr(1, (Y & 0xff), (Cb & 0xff), (Cr & 0xff));
}

void DRV_DMIX_PTG_Color_Set_YCbCr(UINT8 enable, UINT8 Y, UINT8 Cb, UINT8 Cr)
{
	if (enable) {
		G198_DMIX_REG->sft_cfg[11] = (1 << 26) | (1 << 25) | (1 << 24) | \
							((Y & 0xff)<<16) | ((Cb & 0xff)<<8) | ((Cr & 0xff)<<0);
	} else {
		G198_DMIX_REG->sft_cfg[11] = (0 << 26) | (0 << 25) | (0 << 24) | \
							((Y & 0xff)<<16) | ((Cb & 0xff)<<8) | ((Cr & 0xff)<<0);
	}
}

DRV_Status_e DRV_DMIX_Layer_Init(DRV_DMIX_LayerId_e Layer, DRV_DMIX_LayerMode_e LayerMode, DRV_DMIX_InputSel_e FG_Sel)
{
	UINT32 tmp, tmp1;
	DRV_TGEN_Input_e input;

	if ((((int)Layer == DRV_DMIX_L2)) ||
			((int)LayerMode < DRV_DMIX_AlphaBlend) || ((int)LayerMode > DRV_DMIX_Opacity) ||
			(((int)FG_Sel != DRV_DMIX_VPP0) && ((int)FG_Sel != DRV_DMIX_OSD0) && ((int)FG_Sel != DRV_DMIX_OSD1)
			&& ((int)FG_Sel != DRV_DMIX_OSD2) && ((int)FG_Sel != DRV_DMIX_OSD3) && ((int)FG_Sel != DRV_DMIX_PTG))) {
		printf("Layer %d, LayerMode %d, InSel %d\n", Layer, LayerMode, FG_Sel);
		return DRV_ERR_INVALID_PARAM;
	}

	printf("Layer %s, LayerMode %s, InSel %s\n", LayerNameStr[Layer], LayerModeStr[LayerMode], SelStr[FG_Sel]);

	tmp = G198_DMIX_REG->sft_cfg[0];
	tmp1 = G198_DMIX_REG->sft_cfg[1];

	//Set layer mode
	if (Layer != DRV_DMIX_BG) {
		//Clear layer mode bit
		tmp1 &= ~(0x3 << ((Layer - 1) << 1));
		tmp1 |= (LayerMode << ((Layer - 1) << 1));
	}

	tmp = (tmp & ~(0X7 << ((Layer * 4) + 4))) | (FG_Sel << ((Layer * 4) + 4));

	//Finish set amix layer information
	G198_DMIX_REG->sft_cfg[0] = tmp;
	G198_DMIX_REG->sft_cfg[1] = tmp1;

	switch (FG_Sel) {
	case DRV_DMIX_VPP0:
		input = DRV_TGEN_VPP0;
		break;
	case DRV_DMIX_OSD0:
		input = DRV_TGEN_OSD0;
		break;
	case DRV_DMIX_OSD1:
		input = DRV_TGEN_OSD1;
		break;
	case DRV_DMIX_OSD2:
		input = DRV_TGEN_OSD2;
		break;
	case DRV_DMIX_OSD3:
		input = DRV_TGEN_OSD3;
		break;		
	case DRV_DMIX_PTG:
		input = DRV_TGEN_PTG;
		break;
	default:
		input = DRV_TGEN_ALL;
		break;
	}
	#if 1
	if (input != DRV_TGEN_ALL) {
		if (input == DRV_TGEN_PTG)
			DRV_TGEN_Adjust(input, 0x10);
		else
			DRV_TGEN_Adjust(input, 0x10 - ((Layer - DRV_DMIX_L1) << 1));
	}
	#endif

	return DRV_SUCCESS;
}


DRV_Status_e DRV_DMIX_Layer_Set(DRV_DMIX_LayerMode_e LayerMode, DRV_DMIX_InputSel_e FG_Sel)
{
	UINT32 tmp;
	DRV_DMIX_LayerId_e Layer = 0;

	if (((int)LayerMode < DRV_DMIX_AlphaBlend) || ((int)LayerMode > DRV_DMIX_Opacity) ||
			(((int)FG_Sel != DRV_DMIX_VPP0) && ((int)FG_Sel != DRV_DMIX_OSD0) && ((int)FG_Sel != DRV_DMIX_OSD1)
			&& ((int)FG_Sel != DRV_DMIX_OSD2) && ((int)FG_Sel != DRV_DMIX_OSD3) && ((int)FG_Sel != DRV_DMIX_PTG))) {
		printf("Layer %d, LayerMode %d, InSel %d\n", Layer, LayerMode, FG_Sel);
		return DRV_ERR_INVALID_PARAM;
	}

	switch (FG_Sel) {
	case DRV_DMIX_VPP0:
		Layer = DRV_DMIX_L1;
		break;
	case DRV_DMIX_OSD3:
		Layer = DRV_DMIX_L3;
		break;
	case DRV_DMIX_OSD2:
		Layer = DRV_DMIX_L4;
		break;
	case DRV_DMIX_OSD1:
		Layer = DRV_DMIX_L5;
		break;
	case DRV_DMIX_OSD0:
		Layer = DRV_DMIX_L6;
		break;
	case DRV_DMIX_PTG:
		Layer = DRV_DMIX_BG;
		break;
	default:
		goto ERROR;
	}
	printf("Layer %s, LayerMode %s, InSel %s\n", LayerNameStr[Layer], LayerModeStr[LayerMode], SelStr[FG_Sel]);

	tmp = G198_DMIX_REG->sft_cfg[1];

	//Set layer mode
	if (Layer != DRV_DMIX_BG) {
		tmp &= ~(0x3 << ((Layer - 1) << 1));
		tmp |= (LayerMode << ((Layer - 1) << 1));
	}

	//Finish set dmix layer information
	G198_DMIX_REG->sft_cfg[1] = tmp;

ERROR:
	return DRV_SUCCESS;
}

