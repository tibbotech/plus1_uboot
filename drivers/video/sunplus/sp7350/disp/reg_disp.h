/********************************************************
 * Copyright (c) 2022 by Sunplus Technology Co., Ltd.
 *    ____               __
 *   / __/_ _____  ___  / /_ _____
 *  _\ \/ // / _ \/ _ \/ / // (_-<
 * /___/\_,_/_//_/ .__/_/\_,_/___/
 *              /_/
 * Sunplus Technology Co., Ltd.19, Innovation First Road,
 * Science-Based Industrial Park, Hsin-Chu, Taiwan, R.O.C.
 * ------------------------------------------------------
 *
 * Description :  dispplay register header for sp7350
 * ------------------------------------------------------
 * Rev  Date          Author(s)      Status & Comments
 * ======================================================
 * 0.1  2022/05/27    hammer.hsieh   initial version
 * 0.2  2022/10/17    hammer.hsieh   initial version
 */
#ifndef __DISP_REG_H__
#define __DISP_REG_H__

#include <common.h>
#include "display2.h"

#define DISP_REG_BASE		0xf8000000
#define DISP_REG_AO_BASE	0xf8800000
#define DISP_RF_GRP(_grp, _reg) ((((_grp) * 32 + (_reg)) * 4) + DISP_REG_BASE)
#define DISP_RF_GRP_AO(_grp, _reg) ((((_grp) * 32 + (_reg)) * 4) + DISP_REG_AO_BASE)
#define DISP_RF_MASK_V(_mask, _val)       (((_mask) << 16) | (_val))
#define DISP_RF_MASK_V_SET(_mask)         (((_mask) << 16) | (_mask))
#define DISP_RF_MASK_V_CLR(_mask)         (((_mask) << 16) | 0)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                 struct define                                                            //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct DISP_moon0_regs{
	unsigned int sft_cfg[32];
};
#define DISP_MOON0_REG ((volatile struct DISP_moon0_regs *)DISP_RF_GRP_AO(0,0))

struct DISP_moon1_regs{
	unsigned int sft_cfg[32];
};
#define DISP_MOON1_REG ((volatile struct DISP_moon1_regs *)DISP_RF_GRP_AO(1,0))

struct DISP_moon2_regs{
	unsigned int sft_cfg[32];
};
#define DISP_MOON2_REG ((volatile struct DISP_moon2_regs *)DISP_RF_GRP_AO(2,0))

struct DISP_moon3_regs{
	unsigned int sft_cfg[32];
};
#define DISP_MOON3_REG ((volatile struct DISP_moon3_regs *)DISP_RF_GRP_AO(3,0))

typedef volatile struct _DISP_IMGREAD_REG_ {
	unsigned int sft_cfg[32];
} DISP_IMGREAD_REG_t;
typedef volatile struct _DISP_VSCL_0_REG_ {
	unsigned int sft_cfg[32];
} DISP_VSCL_0_REG_t;
typedef volatile struct _DISP_VSCL_1_REG_ {
	unsigned int sft_cfg[32];
} DISP_VSCL_1_REG_t;
typedef volatile struct _DISP_VPOST_REG_ {
	unsigned int sft_cfg[32];
} DISP_VPOST_REG_t;
typedef volatile struct _DISP_OSD_REG_ {
	u32 osd_ctrl						; // 00
	u32 osd_en							; // 01
	u32 osd_base_addr					; // 02
	u32 osd_reserved0[3]				; // 03-05
	u32 osd_bus_monitor_l				; // 06
	u32 osd_bus_monitor_h				; // 07
	u32 osd_req_ctrl					; // 08
	u32 osd_debug_cmd_lock				; // 09
	u32 osd_debug_burst_lock			; // 10
	u32 osd_debug_xlen_lock				; // 11
	u32 osd_debug_ylen_lock				; // 12
	u32 osd_debug_queue_lock			; // 13
	u32 osd_crc_chksum					; // 14
	u32 osd_reserved1					; // 15
	u32 osd_hvld_offset					; // 16
	u32 osd_hvld_width					; // 17
	u32 osd_vvld_offset					; // 18
	u32 osd_vvld_height					; // 19
	u32 osd_data_fetch_ctrl				; // 20
	u32 osd_bist_ctrl					; // 21
	u32 osd_non_fetch_0					; // 22
	u32 osd_non_fetch_1					; // 23
	u32 osd_non_fetch_2					; // 24
	u32 osd_non_fetch_3					; // 25
	u32 osd_bus_status					; // 26
	u32 osd_3d_h_offset					; // 27
	u32 osd_reserved3					; // 28
	u32 osd_src_decimation_sel			; // 29
	u32 osd_bus_time_0					; // 30
	u32 osd_bus_time_1					; // 31
} DISP_OSD_REG_t;
typedef volatile struct _DISP_GPOST_REG_ {
	u32 gpost0_config					; // 00
	u32 gpost0_mskl						; // 01
	u32 gpost0_mskr						; // 02
	u32 gpost0_mskt						; // 03
	u32 gpost0_mskb						; // 04
	u32 gpost0_bg1						; // 05
	u32 gpost0_bg2						; // 06
	u32 gpost0_contrast_config			; // 07
	u32 gpost0_adj_src					; // 08
	u32 gpost0_adj_des					; // 09
	u32 gpost0_adj_slope0				; // 10
	u32 gpost0_adj_slope1				; // 11
	u32 gpost0_adj_slope2				; // 12
	u32 gpost0_adj_bound				; // 13
	u32 gpost0_bri_value				; // 14
	u32 gpost0_hue_sat_en				; // 15
	u32 gpost0_chroma_satsin			; // 16
	u32 gpost0_chroma_satcos			; // 17
	u32 gpost0_master_en				; // 18
	u32 gpost0_master_horizontal		; // 19
	u32 gpost0_master_vertical			; // 20
	u32 gpost0_reserved0[11]			; // 21-31
} DISP_GPOST_REG_t;
typedef volatile struct _DISP_TGEN_REG_ {
	unsigned int sft_cfg[32];
} DISP_TGEN_REG_t;
typedef volatile struct _DISP_DMIX_REG_ {
	unsigned int sft_cfg[32];
} DISP_DMIX_REG_t;
typedef volatile struct _DISP_TCON0_0_REG_ {
	unsigned int sft_cfg[32];
} DISP_TCON0_0_REG_t;
typedef volatile struct _DISP_TCON0_1_REG_ {
	unsigned int sft_cfg[32];
} DISP_TCON0_1_REG_t;
typedef volatile struct _DISP_TCON0_2_REG_ {
	unsigned int sft_cfg[32];
} DISP_TCON0_2_REG_t;
typedef volatile struct _DISP_TCON0_3_REG_ {
	unsigned int sft_cfg[32];
} DISP_TCON0_3_REG_t;
typedef volatile struct _DISP_TCON0_4_REG_ {
	unsigned int sft_cfg[32];
} DISP_TCON0_4_REG_t;
typedef volatile struct _DISP_MIPITX_0_REG_ {
	unsigned int sft_cfg[32];
} DISP_MIPITX_0_REG_t;
typedef volatile struct _DISP_MIPITX_1_REG_ {
	unsigned int sft_cfg[32];
} DISP_MIPITX_1_REG_t;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                 display engine                                                           //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define G185_IMGREAD_REG		((volatile DISP_IMGREAD_REG_t *)(DISP_RF_GRP(185, 0)))
#define G186_VSCL_REG0			((volatile DISP_VSCL_0_REG_t *)(DISP_RF_GRP(186, 0)))
#define G187_VSCL_REG1			((volatile DISP_VSCL_1_REG_t *)(DISP_RF_GRP(187, 0)))
#define G188_VPOST_REG			((volatile DISP_VPOST_REG_t *)(DISP_RF_GRP(188, 0)))
#define G189_OSD0_REG			((volatile DISP_OSD_REG_t *)(DISP_RF_GRP(189, 0)))
#define G190_OSD1_REG			((volatile DISP_OSD_REG_t *)(DISP_RF_GRP(190, 0)))
#define G191_OSD2_REG			((volatile DISP_OSD_REG_t *)(DISP_RF_GRP(191, 0)))
#define G192_OSD3_REG			((volatile DISP_OSD_REG_t *)(DISP_RF_GRP(192, 0)))
#define G193_GPOST0_REG			((volatile DISP_GPOST_REG_t *)(DISP_RF_GRP(193, 0)))
#define G194_GPOST1_REG			((volatile DISP_GPOST_REG_t *)(DISP_RF_GRP(194, 0)))
#define G195_GPOST2_REG			((volatile DISP_GPOST_REG_t *)(DISP_RF_GRP(195, 0)))
#define G196_GPOST3_REG			((volatile DISP_GPOST_REG_t *)(DISP_RF_GRP(196, 0)))
#define G197_TGEN_REG			((volatile DISP_TGEN_REG_t *)(DISP_RF_GRP(197, 0)))
#define G198_DMIX_REG			((volatile DISP_DMIX_REG_t *)(DISP_RF_GRP(198, 0)))
#define G199_TCON0_REG0			((volatile DISP_TCON0_0_REG_t *)(DISP_RF_GRP(199, 0)))
#define G200_TCON0_REG1			((volatile DISP_TCON0_1_REG_t *)(DISP_RF_GRP(200, 0)))
#define G201_TCON0_REG2			((volatile DISP_TCON0_2_REG_t *)(DISP_RF_GRP(201, 0)))
#define G202_TCON0_REG3			((volatile DISP_TCON0_3_REG_t *)(DISP_RF_GRP(202, 0)))
#define G203_TCON0_REG4			((volatile DISP_TCON0_4_REG_t *)(DISP_RF_GRP(203, 0)))
#define G204_MIPITX_REG0		((volatile DISP_MIPITX_0_REG_t *)(DISP_RF_GRP(204, 0)))
#define G205_MIPITX_REG1		((volatile DISP_MIPITX_1_REG_t *)(DISP_RF_GRP(205, 0)))
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                 display address base enumerate                                           //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum DRV_Status_t {
	/* common status values */
	DRV_SUCCESS,				/*!< successful outcome					*/
	DRV_ERR_FAILURE,			/*!< operation failed					*/
	DRV_ERR_INVALID_HANDLE,		/*!< invalid handle						*/
	DRV_ERR_INVALID_ID,			/*!< invalid identifier					*/
	DRV_ERR_INVALID_PARAM,		/*!< invalid parameter					*/
	DRV_ERR_INVALID_OP,			/*!< requested operation is invalid		*/
	DRV_ERR_MEMORY_ALLOC,		/*!< problem allocating memory			*/
	DRV_ERR_MEMORY_SIZE,		/*!< problem with the size of memory	*/
	/* < supplied							*/
	DRV_ERR_RESOURCE_UNAVAILABLE,
	DRV_ERR_TIMEOUT,			/*!< timeout							*/
	DRV_WARN_NO_ACTION,			/* < the function completed successfully,*/
	/* < but no action was taken            */
	DRV_WARN_PARAM_CLIPPED,		/*!< the function has completed			*/
	/*!< successfully, though a parameter was	*/
	/*!< clipped to within a valid range.		*/
	DRV_WARN_BUFFER_EMPTY,
	DRV_WARN_BUFFER_FULL,
	DRV_WARN_UNINITED,			/*!< driver has not been initialized yet */
	DRV_WARN_INITED,			/*!< driver has been initialized already */
	DRV_ERR_MODE_MISMATCH,		/*!< deinterlacer off*/
	DRV_ERR_MAX					/*!< Max error number*/
} DRV_Status_e;

#endif // #ifndef __DISP_REG_H__
