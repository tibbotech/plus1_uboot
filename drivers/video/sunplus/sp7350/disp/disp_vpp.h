// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sunplus - All Rights Reserved
 *
 * Author(s): Hammer Hsieh <hammer.hsieh@sunplus.com>
 */

#ifndef __DISP_VPP_H__
#define __DISP_VPP_H__

void DRV_VPP_Init(int width, int height);
void vpost_setting(int x, int y, int input_w, int input_h, int output_w, int output_h);
void vscl_setting(int x, int y, int input_w, int input_h, int output_w, int output_h);
void imgread_setting(int luma_addr, int chroma_addr, int w, int h, int imgread_fmt);

#endif	//__DISP_VPP_H__

