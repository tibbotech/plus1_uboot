/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * sp7350_lcd.h - SP7350 LCD Controller structures
 *
 * (C) Copyright 2022
 * 		hammer.hsieh@sunplus.com
 */

#ifndef _SP7350_LCD_H_
#define _SP7350_LCD_H_

typedef struct vidinfo {
	u_int logo_width;
	u_int logo_height;
	int logo_x_offset;
	int logo_y_offset;
	u_long logo_addr;
} vidinfo_t;

void sp7350_logo_info(vidinfo_t *info);

#endif //_SP7350_LCD_H_
