#ifndef __INC_REGMAP_Q645_H
#define __INC_REGMAP_Q645_H

struct moon0_regs {
	unsigned int stamp;            // 0.0
	unsigned int clken[5];         // 0.1 - 0.5
	unsigned int rsvd_1[5]; 	   	   // 0.6 - 0.10
	unsigned int gclken[5];        // 0.11
	unsigned int rsvd_2[5]; 	   	   // 0.16 - 0.20
	unsigned int reset[5];         // 0.21
	unsigned int rsvd_3[5];          // 0.26 - 030
	unsigned int hw_cfg;           // 0.31
};
#define MOON0_REG ((volatile struct moon0_regs *)RF_GRP(0, 0))

struct moon1_regs {
	unsigned int sft_cfg[32];
};
#define MOON1_REG ((volatile struct moon1_regs *)RF_GRP(1, 0))

struct moon2_regs {
	unsigned int sft_cfg[28];      // 2.0~2.27
//	unsigned int iv_mx_pad_ds;     // 2.29
//	unsigned int iv_mx_pad_pd;     // 2.30
//	unsigned int iv_mx_pad_pu;     // 2.31
};
#define MOON2_REG ((volatile struct moon2_regs *)RF_GRP(2, 0))

struct moon3_regs {
	unsigned int sft_cfg[32];
};
#define MOON3_REG ((volatile struct moon3_regs *)RF_GRP(3, 0))

struct moon4_regs {
	unsigned int sft_cfg[32];
};
#define MOON4_REG ((volatile struct moon4_regs *)RF_GRP(4, 0))

struct moon5_regs {
	unsigned int sft_cfg[32];
};
#define MOON5_REG ((volatile struct moon5_regs *)RF_GRP(5, 0))


struct gpio_first_regs {
	unsigned int gpio_first[7];
};
#define GPIO_FIRST_REG ((volatile struct gpio_first_regs *)RF_GRP(101, 25)) // 101.25 ~ 101.31

struct gpio_master_regs {
	unsigned int gpio_master[13];
};
#define GPIO_MASTER_REG ((volatile struct gpio_master_regs *)RF_GRP(5, 0)) // 5.0 ~ 5.12

struct gpio_oe_regs {
	unsigned int gpio_oe[13];
};
#define GPIO_OE_REG ((volatile struct gpio_oe_regs *)RF_GRP(5, 13)) // 5.13 ~ 5.25

struct gpio_out_regs {
	unsigned int gpio_out[13];
};
#define GPIO_OUT_REG ((volatile struct gpio_out_regs *)RF_GRP(5, 26)) // 5.26 ~ 6.6

struct gpio_in_regs {
	unsigned int gpio_in[7];
};
#define GPIO_IN_REG ((volatile struct gpio_in_regs *)RF_GRP(6, 7)) // 6.7 ~ 6.13

struct pad_ctl_regs {
	unsigned int spi_soft[3];   // 101.0
	unsigned int emmc_soft[4];  // 101.3
	unsigned int sdio_soft[3];  // 101.7
	unsigned int sd_soft[2];    // 101.10
	unsigned int rsvd[13];      // 101.12
	unsigned int gpio_first[7]; // 101.25
};
#define PAD_CTL_REG ((volatile struct pad_ctl_regs *)RF_GRP(101, 0))


#define UART0_REG    ((volatile struct sp_uart_regs *)RF_GRP(18, 0))
#define UART1_REG    ((volatile struct sp_uart_regs *)RF_GRP(19, 0))
#define UART2_REG    ((volatile struct sp_uart_regs *)RF_GRP(16, 0))
#define UART3_REG    ((volatile struct sp_uart_regs *)RF_GRP(135, 0))

struct stc_regs {
	unsigned int stc_15_0;       // 12.0
	unsigned int stc_31_16;      // 12.1
	unsigned int stc_64;         // 12.2
	unsigned int stc_divisor;    // 12.3
	unsigned int rtc_15_0;       // 12.4
	unsigned int rtc_23_16;      // 12.5
	unsigned int rtc_divisor;    // 12.6
	unsigned int stc_config;     // 12.7
	unsigned int timer0_ctrl;    // 12.8
	unsigned int timer0_cnt;     // 12.9
	unsigned int timer1_ctrl;    // 12.10
	unsigned int timer1_cnt;     // 12.11
	unsigned int timerw_ctrl;    // 12.12
	unsigned int timerw_cnt;     // 12.13
	unsigned int stc_47_32;      // 12.14
	unsigned int stc_63_48;      // 12.15
	unsigned int timer2_ctl;     // 12.16
	unsigned int timer2_pres_val;// 12.17
	unsigned int timer2_reload;  // 12.18
	unsigned int timer2_cnt;     // 12.19
	unsigned int timer3_ctl;     // 12.20
	unsigned int timer3_pres_val;// 12.21
	unsigned int timer3_reload;  // 12.22
	unsigned int timer3_cnt;     // 12.23
	unsigned int stcl_0;         // 12.24
	unsigned int stcl_1;         // 12.25
	unsigned int stcl_2;         // 12.26
	unsigned int atc_0;          // 12.27
	unsigned int atc_1;          // 12.28
	unsigned int atc_2;          // 12.29
};
#define STC_REG     ((volatile struct stc_regs *)RF_GRP(12, 0))
#define STC_AV0_REG ((volatile struct stc_regs *)RF_GRP(96, 0))
#define STC_AV1_REG ((volatile struct stc_regs *)RF_GRP(97, 0))
#define STC_AV2_REG ((volatile struct stc_regs *)RF_GRP(99, 0))

struct dpll_regs {
	unsigned int dpll0_ctrl;
	unsigned int dpll0_remainder;
	unsigned int dpll0_denominator;
	unsigned int dpll0_divider;
	unsigned int g20_reserved_0[4];
	unsigned int dpll1_ctrl;
	unsigned int dpll1_remainder;
	unsigned int dpll1_denominator;
	unsigned int dpll1_divider;
	unsigned int g20_reserved_1[4];
	unsigned int dpll2_ctrl;
	unsigned int dpll2_remainder;
	unsigned int dpll2_denominator;
	unsigned int dpll2_divider;
	unsigned int g20_reserved_2[4];
	unsigned int dpll3_ctrl;
	unsigned int dpll3_remainder;
	unsigned int dpll3_denominator;
	unsigned int dpll3_divider;
	unsigned int dpll3_sprd_num;
	unsigned int g20_reserved_3[3];
};
#define DPLL_REG     ((volatile struct dpll_regs *)RF_GRP(20, 0))

struct spi_ctrl_regs {
	unsigned int spi_ctrl;       // 22.0
	unsigned int spi_timing;     // 22.1
	unsigned int spi_page_addr;  // 22.2
	unsigned int spi_data;       // 22.3
	unsigned int spi_status;     // 22.4
	unsigned int spi_auto_cfg;   // 22.5
	unsigned int spi_cfg[3];     // 22.6
	unsigned int spi_data_64;    // 22.9
	unsigned int spi_buf_addr;   // 22.10
	unsigned int spi_statu_2;    // 22.11
	unsigned int spi_err_status; // 22.12
	unsigned int spi_data_addr;  // 22.13
	unsigned int mem_parity_addr;// 22.14
	unsigned int spi_col_addr;   // 22.15
	unsigned int spi_bch;        // 22.16
	unsigned int spi_intr_msk;   // 22.17
	unsigned int spi_intr_sts;   // 22.18
	unsigned int spi_page_size;  // 22.19
	unsigned int g20_reserved_0; // 22.20
};
#define SPI_CTRL_REG ((volatile struct spi_ctrl_regs *)RF_GRP(22, 0))

/* start of xhci */
struct uphy_u3_regs {
	unsigned int cfg[32];		       // 189.0
};

#define UPHY0_U3_REG ((volatile struct uphy_u3_regs *)RF_AMBA(189, 0))
#define UPHY1_U3_REG ((volatile struct uphy_u3_regs *)RF_AMBA(190, 0))
#define XHCI0_REG ((volatile struct xhci_hccr *)RF_AMBA(161, 0))
#define XHCI1_REG ((volatile struct xhci_hccr *)RF_AMBA(175, 0))
/* end of xhci */

struct uphy_rn_regs {
       unsigned int cfg[28];
       unsigned int gctrl[3];
       unsigned int gsts;
};
#define UPHY0_RN_REG ((volatile struct uphy_rn_regs *)RF_GRP(149, 0))
#define UPHY1_RN_REG ((volatile struct uphy_rn_regs *)RF_GRP(150, 0))
#define UPHY2_RN_REG ((volatile struct uphy_rn_regs *)RF_GRP(151, 0))

/* usb host */
struct ehci_regs {
	unsigned int ehci_len_rev;
	unsigned int ehci_sparams;
	unsigned int ehci_cparams;
	unsigned int ehci_portroute;
	unsigned int g143_reserved_0[4];
	unsigned int ehci_usbcmd;
	unsigned int ehci_usbsts;
	unsigned int ehci_usbintr;
	unsigned int ehci_frameidx;
	unsigned int ehci_ctrl_ds_segment;
	unsigned int ehci_prd_listbase;
	unsigned int ehci_async_listaddr;
	unsigned int g143_reserved_1[9];
	unsigned int ehci_config;
	unsigned int ehci_portsc;
	/*
	unsigned int g143_reserved_2[1];
	unsigned int ehci_version_ctrl;
	unsigned int ehci_general_ctrl;
	unsigned int ehci_usb_debug;
	unsigned int ehci_sys_debug;
	unsigned int ehci_sleep_cnt;
	*/
};
#define EHCI0_REG ((volatile struct ehci_regs *)AMBA_GRP(258, 2, 0)) // 0xf8102100
#define EHCI1_REG ((volatile struct ehci_regs *)AMBA_GRP(259, 2, 0)) // 0xf8103100
#define EHCI2_REG ((volatile struct ehci_regs *)AMBA_GRP(260, 2, 0)) // 0xf8104100

struct usbh_sys_regs {
	unsigned int uhversion;
	unsigned int reserved[3];
	unsigned int uhpowercs_port;
	unsigned int uhc_fsm_axi;
	unsigned int reserved2[22];
	unsigned int uho_fsm_st1;
	unsigned int uho_fsm_st2;
	unsigned int uhe_fsm_st1;
	unsigned int uhe_fsm_st2;
};
#define USBH0_SYS_REG ((volatile struct usbh_sys_regs *)AMBA_GRP(258, 0, 0)) // 0xf8102000
#define USBH1_SYS_REG ((volatile struct usbh_sys_regs *)AMBA_GRP(259, 0, 0)) // 0xf8103000
#define USBH2_SYS_REG ((volatile struct usbh_sys_regs *)AMBA_GRP(260, 0, 0)) // 0xf8104000

/* sd and mmc card regs */
struct emmc_ctl_regs {
	/*g0.0*/
	unsigned int mediatype:3;
	unsigned int reserved0:1;
	unsigned int dmasrc:3;
	unsigned int reserved1:1;
	unsigned int dmadst:3;
	unsigned int reserved2:21;

	/*g0.1*/
	unsigned int card_ctl_page_cnt:16;
	unsigned int reserved3:16;

	/* g0.2 */
	unsigned int sdram_sector_0_size:16;
	unsigned int reserved4:1;
	/* g0.3 */
	unsigned int dma_base_addr;
	/* g0.4 */
	union {
		struct {
			unsigned int reserved5:1;
			unsigned int hw_dma_en:1;
			unsigned int reserved6:1;
			unsigned int hw_sd_hcsd_en:1;
			unsigned int hw_sd_dma_type:2;
			unsigned int hw_sd_cmd13_en:1;
			unsigned int reserved7:1;
			unsigned int stop_dma_flag:1;
			unsigned int hw_dma_rst:1;
			unsigned int dmaidle:1;
			unsigned int dmastart:1;
			unsigned int hw_block_num:2;
			unsigned int reserved8:2;
			unsigned int hw_cmd13_rca:16;
		};
		unsigned int hw_dma_ctl;
	};
	/* g0.5 */
	union {
		struct {
			unsigned int reg_sd_ctl_free:1;				// 0
			unsigned int reg_sd_free:1;				// 1
			unsigned int reg_ms_ctl_free:1;				// 2
			unsigned int reg_ms_free:1;				// 3
			unsigned int reg_dma_fifo_free:1;			// 4
			unsigned int reg_dma_ctl_free:1;			// 5
			unsigned int reg_hwdma_page_free:1;			// 6
			unsigned int reg_hw_dma_free:1;				// 7
			unsigned int reg_sd_hwdma_free:1;			// 8
			unsigned int reg_ms_hwdma_free:1;			// 9
			unsigned int reg_dma_reg_free:1;			// 10
			unsigned int reg_card_reg_free:1;			// 11
			unsigned int reserved9:20;
		};
		unsigned int card_gclk_disable;
	};

	/* g0.6 ~ g0.19*/
	struct {
		unsigned int dram_sector_addr;
		unsigned int sdram_sector_size:16;
		unsigned int reserved10:16;
	} dma_addr_info[7];

	/* g0.20 */
	union {
		struct {
			unsigned int dram_sector_cnt:3;				// 2:00 ro
			unsigned int hw_block_cnt:2;				// 04:03 ro
			unsigned int reserved11:11;				// 15:05 ro
			unsigned int hw_page_cnt:16;				// 31:16 ro 
		};
		unsigned int sdram_sector_block_cnt;
	};
	/* g0.20 ~ g0.28 */
	unsigned int dma_hw_page_addr[4];
	unsigned int dma_hw_page_num[4];

	/* g0.29 */
	unsigned int hw_wait_num;

	/* g0.30 */
	unsigned int hw_delay_num:16;
	unsigned int reserved12:16;

	/* g0.31 */
	union {
		struct {
			unsigned int incnt:11;
			unsigned int outcnt:11;
			unsigned int dma_sm:3;
			unsigned int reserved13:7;
		};
		unsigned int dma_debug;
	};

	/* g1.0 */
	union {
		struct {
			unsigned int boot_ack_en:1;
			unsigned int boot_ack_tmr:1;
			unsigned int boot_data_tmr:1;
			unsigned int fast_boot:1;
			unsigned int boot_mode:1;
			unsigned int bootack:3;
			unsigned int resume_boot:1;
			unsigned int reserved14:7;
			unsigned int stop_page_num:16;
		};
		unsigned int boot_ctl;
	};

	/* g1.1 */
	union {
		struct {
			unsigned int vol_tmr:2;
			unsigned int sw_set_vol:1;
			unsigned int hw_set_vol:1;
			unsigned int vol_result:2;
			unsigned int reserved15:26;
		};
		unsigned int sd_vol_ctrl;
	};
	/* g1.2 */
	union {
		struct {
			unsigned int sdcmpen:1;
			unsigned int sd_cmp:1;		//1
			unsigned int sd_cmp_clr:1;	//2
			unsigned int sdio_int_en:1;	//3
			unsigned int sdio_int:1;	//4
			unsigned int sdio_int_clr:1;	//5
			unsigned int detect_int_en:1;	//6
			unsigned int detect_int:1;	//7
			unsigned int detect_int_clr:1;	//8
			unsigned int hwdmacmpen:1;	//9
			unsigned int hw_dma_cmp:1;	//10
			unsigned int hwdmacmpclr:1;	//11
			unsigned int reserved16:20;	//31:12
		};
		unsigned int sd_int;
	};

	/* g1.3 */
	unsigned int sd_page_num:16;
	unsigned int reserved17:16;
	/* g1.4 */
	union {
		struct {	
			unsigned int sdpiomode:1;
			unsigned int sdddrmode:1;
			unsigned int sd_len_mode:1;
			unsigned int first_dat_hcyc:1;
			unsigned int sd_trans_mode:2;
			unsigned int sdautorsp:1;
			unsigned int sdcmddummy:1;
			unsigned int sdrspchk_en:1;
			unsigned int sdiomode:1;
			unsigned int sdmmcmode:1;
			unsigned int sddatawd:1;
			unsigned int sdrsptmren:1;
			unsigned int sdcrctmren:1;
			unsigned int rx4_en:1;
			unsigned int sdrsptype:1;
			unsigned int detect_tmr:2;
			unsigned int mmc8_en:1;
			unsigned int selci:1;
			unsigned int sdfqsel:12;
		};
		unsigned int sd_config0;
	};

	/* g1.5 */
	union {
		struct {
			unsigned int rwc:1;
			unsigned int s4mi:1;
			unsigned int resu:1;
			unsigned int sus_req:1;
			unsigned int con_req:1;
			unsigned int sus_data_flag:1;
			unsigned int int_multi_trig:1;
			unsigned int reserved18:25; 
		};
		unsigned int sdio_ctrl;
	};

	/* g1.6 */
	union {
		struct {
			unsigned int sdrst:1;
			unsigned int sdcrcrst:1;
			unsigned int sdiorst:1;
			unsigned int reserved19:29; 
		};
		unsigned int sd_rst;
	};

	/* g1.7 */
	union {
		struct {
			unsigned int sdctrl0:1;
			unsigned int sdctrl1:1;
			unsigned int sdioctrl:1;
			unsigned int emmcctrl:1;
			unsigned int reserved20:28;
		} ;
		unsigned int sd_ctrl;
	};
	/* g1.8 */
	union {
		struct {
			unsigned int sdstatus:19;
			unsigned int reserved21:13;	 
		};
		unsigned int sd_status;
	};
	/* g1.9 */
	union {
		struct {
			unsigned int sdstate:3;
			unsigned int reserved22:1; 
			unsigned int sdcrdcrc:3; 
			unsigned int reserved23:1; 
			unsigned int sdstate_new:7; 
			unsigned int reserved24:17; 
		};
		unsigned int sd_state;
	};

	/* g1.10 */
	union {
		struct {
			unsigned int hwsd_sm:10;	
			unsigned int reserved25:22; 
		}; 
		unsigned int sd_hw_state;
	};

	/* g1.11 */
	union {
		struct {
			unsigned int sddatalen:11;	
			unsigned int reserved26:21; 
		}; 
		unsigned int sd_blocksize;
	};
	
	/* g1.12 */
	union {
		struct {
			unsigned int tx_dummy_num:9;
			unsigned int sdcrctmr:11;
			unsigned int sdrsptmr:11;
			unsigned int sd_high_speed_en:1;
		}; 
		unsigned int sd_config1;
	};

	/* g1.13 */
	union {
		struct {
			unsigned int sd_clk_dly_sel:3;
			unsigned int reserved27:1;
			unsigned int sd_wr_dat_dly_sel:3;
			unsigned int reserved28:1;
			unsigned int sd_wr_cmd_dly_sel:3;
			unsigned int reserved28_1:1;
			unsigned int sd_rd_rsp_dly_sel:3;
			unsigned int reserved28_2:1;
			unsigned int sd_rd_dat_dly_sel:3;
			unsigned int reserved28_3:1;
			unsigned int sd_rd_crc_dly_sel:3;
			unsigned int reserved29:9;
		};
		unsigned int sd_timing_config;
	};

	/* g1.14 */
	unsigned int sd_rxdattmr:29; 
	unsigned int reserved30:3;
	
	/* g1.15 */
	unsigned int sd_piodatatx:16;
	unsigned int reserved31:16;

	/* g1.16 */
	unsigned int sd_piodatarx;

	/* g1.17 */
	/* g1.18 */
	unsigned char sd_cmdbuf[5];
	unsigned char reserved32[3];
	/* g1.19 - g1.20 */
	unsigned int sd_rspbuf0_3;
	unsigned int sd_rspbuf4_5;
	/*  unused sd control regs */
	unsigned int reserved34[11];
	/* ms card related regs */
	unsigned int ms_regs[32];
};
#define CARD0_CTL_REG ((volatile int  *)RF_GRP(118, 0))
#define CARD1_CTL_REG ((volatile int  *)RF_GRP(125, 0))

/* NAND */
#define BCH_S338_BASE_ADDRESS    0xF8101000     // RG_AMBA(257, 0)

/* SPI NAND */
#define CONFIG_SP_SPINAND_BASE   ((volatile unsigned int *)RF_GRP(87, 0))
#define SPI_NAND_DIRECT_MAP      0xF4000000

/* SPACC */
struct spacc_regs {
	unsigned int irq_en;              // 0x00
	unsigned int irq_stat;            // 0x04
	unsigned int irq_ctrl;            // 0x08
	unsigned int fifo_stat;           // 0x0c
	unsigned int sdma_brst_sz;        // 0x10
	unsigned int rsvd_14[3];          // 0x14~0x1c
	unsigned int src_ptr;             // 0x20
	unsigned int dst_ptr;             // 0x24
	unsigned int offset;              // 0x28
	unsigned int pre_aad_len;         // 0x2c
	unsigned int post_aad_len;        // 0x30
	unsigned int proc_len;            // 0x34
	unsigned int icv_len;             // 0x38
	unsigned int icv_offset;          // 0x3c
	unsigned int iv_offset;           // 0x40
	unsigned int sw_ctrl;             // 0x44
	unsigned int aux_info;            // 0x48
	unsigned int ctrl;                // 0x4c
	unsigned int stat_pop;            // 0x50
	unsigned int status;              // 0x54
	unsigned int rsvd_58[10];         // 0x58~0x7c
	unsigned int stat_wd_ctrl;        // 0x80
	unsigned int rsvd_84[31];         // 0x84~0xfc
	unsigned int key_sz;              // 0x100
	unsigned int rsvd_104[31];        // 0x104~0x
	unsigned int version;             // 0x180
	unsigned int version_ext;         // 0x184
	unsigned int rsvd_88[2];          // 0x188~0x18c
	unsigned int version_ext2;        // 0x190
	unsigned int version_ext3;        // 0x194
	unsigned int rsvd_98[10];         // 0x198~0x1bc
	unsigned int secure_ctrl;         // 0x1c0
	unsigned int secure_ctx_release;  // 0x1c4
};

#define SPACC_BASE  (0xE8000000)
#define SPACC_REG   ((volatile struct spacc_regs *)SPACC_BASE)

struct hpi_regs {
	unsigned int rsvd[2];              // 0x00
	unsigned int fw_info[2];           // 0x08
	unsigned int irq_en;               // 0x10
	unsigned int irq_stat;             // 0x14
	unsigned int rsvd_18[2];           // 0x18
	unsigned int fw_addr;              // 0x20
	unsigned int rsvd_24;              // 0x24
	unsigned int data_addr_sec;        // 0x28
	unsigned int host_ctrl;            // 0x2c
	unsigned int host_mb_p[2];         // 0x30
	unsigned int host_mb_ctrl;         // 0x38
	unsigned int host_mb_own;          // 0x3c
	unsigned int cpu_mb_p[2];          // 0x40
	unsigned int cpu_mb_ctrl;          // 0x48
	unsigned int cpu_mb_own;           // 0x4c
	unsigned int rsvd_50[3];           // 0x50
	unsigned int oob_stat;             // 0x5c
	unsigned int err_stat;             // 0x60
	unsigned int user_notify;          // 0x64
};

#define HPI_BASE  (0xE7FF1000)

#ifdef FAKE_HPI
#define HPI_REG   ((volatile struct hpi_regs *)FAKE_HPI_BASE)
#else
#define HPI_REG   ((volatile struct hpi_regs *)HPI_BASE)
#endif


// See http://wiki2.sunplus.com/wiki/index.php/IP-Spec
struct ctl_hsm_regs {
	unsigned int hsm_fw_base          ; // 00 , [31:12] fw_base, [4] hpi_err_INT_clr, [0] semc_hw_valid
	unsigned int hsm_data_base        ; // 01 , [31:12] data_base
	unsigned int hsm_kprime_key_hh    ; // 02 , [127:96] KPF
	unsigned int hsm_kprime_key_hl    ; // 03 , [ 95:64] KPF
	unsigned int hsm_kprime_key_lh    ; // 04 , [ 63:32] KPF
	unsigned int hsm_kprime_key_ll    ; // 05 , [ 31: 0] KPF
	unsigned int hsm_duk_key_hh       ; // 06 , [127:96] DUK
	unsigned int hsm_duk_key_hl       ; // 07 , [ 95:64] DUK
	unsigned int hsm_duk_key_lh       ; // 08 , [ 63:32] DUK
	unsigned int hsm_duk_key_ll       ; // 09 , [ 31: 0] DUK
	unsigned int hsm_control          ; // 10 , [ 31:16] mask, [15:12] range_control, [10] did_hw_valid
	//      [1] hpi_err_wr_int_mask, [0] hpi_err_host_int_mask
	unsigned int hsm_devid            ; // 11 , [31:0] Device ID
	unsigned int hsm_monitor          ; // 12 , [9] adc_crowbar, [8] aic_crowbar, [7] sleep_r, [6:4] sleep_mode
	//      [3] dbg_tf_r, [2] hpi_err_wr_flag, [1] hpi_host_intr, [0] sys_halt_r
	unsigned int hsm_hpi_err          ; // 13 , [31:0] hpi_err_stat
	unsigned int g44_reserved[18]     ; // 14 ~ 31
};
#define CTL_HSM_REG     ((volatile struct ctl_hsm_regs *)RF_GRP(44, 0))


#endif /* __INC_REGMAP_Q645_H */
