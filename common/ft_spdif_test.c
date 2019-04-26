#include <common.h>
////////////  AUD_TEST 
/*********** audio global*********/

typedef unsigned int UINT32;
typedef unsigned char UINT8;



#if 1
typedef struct
{
    // GROUP 000 : Chip Information
    UINT32  stamp                                 ; // 00  (ADDR : 0x9C00_0000)
    UINT32  emulation                             ; // 01  (ADDR : 0x9C00_0004)
    UINT32  G000_reserved_2                       ; // 02  (ADDR : 0x9C00_0008)
    UINT32  G000_reserved_3                       ; // 03  (ADDR : 0x9C00_000C)
    UINT32  clk_sel0                              ; // 04  (ADDR : 0x9C00_0010)
    UINT32  clk_sel1                              ; // 05  (ADDR : 0x9C00_0014)
    UINT32  sspll_cfg                             ; // 06  (ADDR : 0x9C00_0018)
    UINT32  clken0                                ; // 07  (ADDR : 0x9C00_001C)
    UINT32  clken1                                ; // 08  (ADDR : 0x9C00_0020)
    UINT32  clken2                                ; // 09  (ADDR : 0x9C00_0024)
    UINT32  clken3                                ; // 10  (ADDR : 0x9C00_0028)
    UINT32  clken4                                ; // 11  (ADDR : 0x9C00_002C)
    UINT32  gclken0                               ; // 12  (ADDR : 0x9C00_0030)
    UINT32  gclken1                               ; // 13  (ADDR : 0x9C00_0034)
    UINT32  gclken2                               ; // 14  (ADDR : 0x9C00_0038)
    UINT32  gclken3                               ; // 15  (ADDR : 0x9C00_003C)
    UINT32  gclken4                               ; // 16  (ADDR : 0x9C00_0040)
    UINT32  reset0                                ; // 17  (ADDR : 0x9C00_0044)
    UINT32  reset1                                ; // 18  (ADDR : 0x9C00_0048)
    UINT32  reset2                                ; // 19  (ADDR : 0x9C00_004C)
    UINT32  reset3                                ; // 20  (ADDR : 0x9C00_0050)
    UINT32  reset4                                ; // 21  (ADDR : 0x9C00_0054)
    UINT32  pwr_iso                               ; // 22  (ADDR : 0x9C00_0058)
    UINT32  pwr_ctl                               ; // 23  (ADDR : 0x9C00_005C)
    UINT32  hw_bo0                                ; // 24  (ADDR : 0x9C00_0060)
    UINT32  hw_bo1                                ; // 25  (ADDR : 0x9C00_0064)
    UINT32  hw_bo2                                ; // 26  (ADDR : 0x9C00_0068)
    UINT32  hw_bo3                                ; // 27  (ADDR : 0x9C00_006C)
    UINT32  hw_cfg                                ; // 28  (ADDR : 0x9C00_0070)
    UINT32  hw_cfg_chg                            ; // 29  (ADDR : 0x9C00_0074)
    UINT32  G000_reserved_30                      ; // 30  (ADDR : 0x9C00_0078)
    UINT32  show_bo_stamp                         ; // 31  (ADDR : 0x9C00_007C)

    // Group 001 : MOON1
    UINT32  rf_sft_cfg0                           ; // 00  (ADDR : 0x9C00_0080)
    UINT32  rf_sft_cfg1                           ; // 01  (ADDR : 0x9C00_0084)
    UINT32  rf_sft_cfg2                           ; // 02  (ADDR : 0x9C00_0088)
    UINT32  rf_sft_cfg3                           ; // 03  (ADDR : 0x9C00_008C)
    UINT32  rf_sft_cfg4                           ; // 04  (ADDR : 0x9C00_0090)
    UINT32  rf_sft_cfg5                           ; // 05  (ADDR : 0x9C00_0094)
    UINT32  rf_sft_cfg6                           ; // 06  (ADDR : 0x9C00_0098)
    UINT32  rf_sft_cfg7                           ; // 07  (ADDR : 0x9C00_009C)
    UINT32  rf_sft_cfg8                           ; // 08  (ADDR : 0x9C00_00A0)
    UINT32  rf_sft_cfg9                           ; // 09  (ADDR : 0x9C00_00A4)
    UINT32  rf_sft_cfg10                          ; // 10  (ADDR : 0x9C00_00A8)
    UINT32  rf_sft_cfg11                          ; // 11  (ADDR : 0x9C00_00AC)
    UINT32  rf_sft_cfg12                          ; // 12  (ADDR : 0x9C00_00B0)
    UINT32  rf_sft_cfg13                          ; // 13  (ADDR : 0x9C00_00B4)
    UINT32  rf_sft_cfg14                          ; // 14  (ADDR : 0x9C00_00B8)
    UINT32  rf_sft_cfg15                          ; // 15  (ADDR : 0x9C00_00BC)
    UINT32  rf_sft_cfg16                          ; // 16  (ADDR : 0x9C00_00C0)
    UINT32  rf_sft_cfg17                          ; // 17  (ADDR : 0x9C00_00C4)
    UINT32  rf_sft_cfg18                          ; // 18  (ADDR : 0x9C00_00C8)
    UINT32  rf_sft_cfg19                          ; // 19  (ADDR : 0x9C00_00CC)
    UINT32  rf_sft_cfg20                          ; // 20  (ADDR : 0x9C00_00D0)
    UINT32  rf_sft_cfg21                          ; // 21  (ADDR : 0x9C00_00D4)
    UINT32  rf_sft_cfg22                          ; // 22  (ADDR : 0x9C00_00D8)
    UINT32  rf_sft_cfg23                          ; // 23  (ADDR : 0x9C00_00DC)
    UINT32  rf_sft_cfg24                          ; // 24  (ADDR : 0x9C00_00E0)
    UINT32  rf_sft_cfg25                          ; // 25  (ADDR : 0x9C00_00E4)
    UINT32  rf_sft_cfg26                          ; // 26  (ADDR : 0x9C00_00E8)
    UINT32  rf_sft_cfg27                          ; // 27  (ADDR : 0x9C00_00EC)
    UINT32  rf_sft_cfg28                          ; // 28  (ADDR : 0x9C00_00F0)
    UINT32  rf_sft_cfg29                          ; // 29  (ADDR : 0x9C00_00F4)
    UINT32  rf_sft_cfg30                          ; // 30  (ADDR : 0x9C00_00F8)
    UINT32  rf_sft_cfg31                          ; // 31  (ADDR : 0x9C00_00FC)

    // Group 002 : Reserved
    UINT32  rf_sft_cfg32                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg33                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg34                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg35                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg36                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg37                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg38                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg39                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg40                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg41                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg42                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg43                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg44                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  rf_sft_cfg45                          ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)
    UINT32  G002_RESERVED[18]                     ; //     (ADDR : 0x9C00_0100) ~ (ADDR : 0x9C00_017C)

    // Group 003 : Reserved
    UINT32  G003_RESERVED[32]                     ; //     (ADDR : 0x9C00_0180) ~ (ADDR : 0x9C00_01FC)

    // Group 004 : PAD_CTL
    UINT32  rf_pad_ctl0                           ; // 00  (ADDR : 0x9C00_0200)
    UINT32  rf_pad_ctl1                           ; // 01  (ADDR : 0x9C00_0204)
    UINT32  rf_pad_ctl2                           ; // 02  (ADDR : 0x9C00_0208)
    UINT32  rf_pad_ctl3                           ; // 03  (ADDR : 0x9C00_020C)
    UINT32  rf_pad_ctl4                           ; // 04  (ADDR : 0x9C00_0210)
    UINT32  rf_pad_ctl5                           ; // 05  (ADDR : 0x9C00_0214)
    UINT32  rf_pad_ctl6                           ; // 06  (ADDR : 0x9C00_0218)
    UINT32  rf_pad_ctl7                           ; // 07  (ADDR : 0x9C00_021C)
    UINT32  rf_pad_ctl8                           ; // 08  (ADDR : 0x9C00_0220)
    UINT32  rf_pad_ctl9                           ; // 09  (ADDR : 0x9C00_0224)
    UINT32  rf_pad_ctl10                          ; // 10  (ADDR : 0x9C00_0228)
    UINT32  rf_pad_ctl11                          ; // 11  (ADDR : 0x9C00_022C)
    UINT32  rf_pad_ctl12                          ; // 12  (ADDR : 0x9C00_0230)
    UINT32  rf_pad_ctl13                          ; // 13  (ADDR : 0x9C00_0234)
    UINT32  rf_pad_ctl14                          ; // 14  (ADDR : 0x9C00_0238)
    UINT32  rf_pad_ctl15                          ; // 15  (ADDR : 0x9C00_023C)
    UINT32  rf_pad_ctl16                          ; // 16  (ADDR : 0x9C00_0240)
    UINT32  rf_pad_ctl17                          ; // 17  (ADDR : 0x9C00_0244)
    UINT32  rf_pad_ctl18                          ; // 18  (ADDR : 0x9C00_0248)
    UINT32  rf_pad_ctl19                          ; // 19  (ADDR : 0x9C00_024C)
    UINT32  rf_pad_ctl20                          ; // 20  (ADDR : 0x9C00_0250)
    UINT32  rf_pad_ctl21                          ; // 21  (ADDR : 0x9C00_0254)
    UINT32  rf_pad_ctl22                          ; // 22  (ADDR : 0x9C00_0258)
    UINT32  rf_pad_ctl23                          ; // 23  (ADDR : 0x9C00_025C)
    UINT32  rf_pad_ctl24                          ; // 24  (ADDR : 0x9C00_0260)
    UINT32  gpio_first[6]			  ;
    UINT32  rf_pad_ctl31                          ; // 31  (ADDR : 0x9C00_027C)

    // Group 005 : GPIOXT
    UINT32 gpio_master[8];                        
    UINT32 gpio_oe[8];
    UINT32 gpio_out[8];
    UINT32 gpio_in[8];                             //(ADDR : 0x9C00_0280) ~ (ADDR : 0x9C00_02FC)

    // Group 006 : Reserved
    UINT32  G006_RESERVED[32]                     ;//(ADDR : 0x9C00_0300) ~ (ADDR : 0x9C00_037C)

    // Group 007 : PAD_CTRL1
    UINT32  G007_PAD_CTRL1[32]                    ;//(ADDR : 0x9C00_0380) ~ (ADDR : 0x9C00_03FC)

    // Group 008 : IOP
    UINT32  iop_control                           ; //
    UINT32  iop_status                            ; //
    UINT32  iop_bp                                ; //
    UINT32  iop_regsel                            ; //
    UINT32  iop_regout                            ; //
    UINT32  iop_memlimit                          ; //
//  UINT32  reserved[2]                           ; //
    UINT32  iop_resume_pcl                        ; //
    UINT32  iop_resume_pch                        ; //
    UINT32  iop_data[12]                          ; //
    UINT32  iop_lbus_offset_ext                   ; //
    UINT32  iop_lbus_offset                       ; //
    UINT32  iop_lbus_control                      ; //
    UINT32  iop_lbus_offset2_lsb                  ; //
    UINT32  iop_lbus_offset2                      ; //
    UINT32  iop_direct_adr                        ; //
    UINT32  g8_reserved[6]                        ; //

    // Group 009 : ARM926 interrupt
    UINT32  a926_intr_type[7]                     ;
    UINT32  a926_intr_polarity[7]                 ;
    UINT32  a926_priority[7]                      ;
    UINT32  a926_intr_mask[7]                     ;
    UINT32  G009_RESERVED[4]                      ;

    // Group 010 : ARM926 interrupt
    UINT32  a926_intr_clr[7]                      ;
    UINT32  masked_a926_fiqs[7]                   ;
    UINT32  masked_a926_irqs[7]                   ;
    UINT32  G010_RESERVED[11]                     ;

    // Group 011 : TSET
    UINT32  G011_TSET[32]                         ;

    // Group 012 : STC
    UINT32  stc_15_0                              ; // 00  384
    UINT32  stc_31_16                             ; // 01  385
    UINT32  stc_32                                ; // 02  386
    UINT32  stc_divisor                           ; // 03  387
    UINT32  rtc_15_0                              ; // 04  388
    UINT32  rtc_31_16                             ; // 05  389
    UINT32  rtc_divisor                           ; // 06  390
    UINT32  stc_config                            ; // 07  391
    UINT32  timer0_ctrl                           ; // 08  392
    UINT32  timer0_cnt                            ; // 09  393
    UINT32  timer1_ctrl                           ; // 0a  394
    UINT32  timer1_cnt                            ; // 0b  395
    UINT32  timerw_ctrl                           ; // 0c  396
    UINT32  timerw_cnt                            ; // 0d  397
    UINT32  g12_unused_1[2]                       ; // 0e  398~399
    UINT32  timer2_ctrl                           ; // 10  400
    UINT32  timer2_divisor                        ; // 11  401
    UINT32  timer2_reload                         ; // 12  402
    UINT32  timer2_cnt                            ; // 13  403
    UINT32  timer3_ctrl                           ; // 14  404
    UINT32  timer3_divisor                        ; // 15  405
    UINT32  timer3_reload                         ; // 16  406
    UINT32  timer3_cnt                            ; // 17  407
    UINT32  stcl_0                                ; // 18  408
    UINT32  stcl_1                                ; // 19  409
    UINT32  stcl_2                                ; // 1a  410
    UINT32  atc_0                                 ; // 1b  411
    UINT32  atc_1                                 ; // 1c  412
    UINT32  atc_2                                 ; // 1d  413
    UINT32  g12_reserved[2]                       ; // 1e  414

    // Group 013 : BR_WRAPPER
    UINT32  G013_BR_WRAPPER[32]                   ;

    // Group 014 : DUMMY_MASTER
    UINT32  dummy0_op_mode                        ; // 00  (ADDR : 0x9C00_0700)
    UINT32  dummy0_addr_base                      ; // 01  (ADDR : 0x9C00_0704)
    UINT32  dummy0_addr_offset                    ; // 02  (ADDR : 0x9C00_0708)
    UINT32  dummy0_control                        ; // 03  (ADDR : 0x9C00_070C)
    UINT32  dummy0_urgent                         ; // 04  (ADDR : 0x9C00_0710)
    UINT32  dummy0_request_period                 ; // 05  (ADDR : 0x9C00_0714)
    UINT32  G14_reserved_6                        ; // 06  (ADDR : 0x9C00_0718)
    UINT32  dummy0_error                          ; // 07  (ADDR : 0x9C00_071C)
    UINT32  dummy0_runtime_l                      ; // 08  (ADDR : 0x9C00_0720)
    UINT32  dummy0_runtime_h                      ; // 09  (ADDR : 0x9C00_0724)
    UINT32  dummy0_data_count_l                   ; // 10  (ADDR : 0x9C00_0728)
    UINT32  dummy0_data_count_h                   ; // 11  (ADDR : 0x9C00_072C)
    UINT32  dummy0_mdata_count_l                  ; // 12  (ADDR : 0x9C00_0730)
    UINT32  dummy0_mdata_count_h                  ; // 13  (ADDR : 0x9C00_0734)
    UINT32  dummy0_sdata_count_l                  ; // 14  (ADDR : 0x9C00_0738)
    UINT32  dummy0_sdata_count_h                  ; // 15  (ADDR : 0x9C00_073C)
    UINT32  dummy1_op_mode                        ; // 16  (ADDR : 0x9C00_0740)
    UINT32  dummy1_addr_base                      ; // 17  (ADDR : 0x9C00_0744)
    UINT32  dummy1_addr_offset                    ; // 18  (ADDR : 0x9C00_0748)
    UINT32  dummy1_control                        ; // 19  (ADDR : 0x9C00_074C)
    UINT32  dummy1_urgent                         ; // 20  (ADDR : 0x9C00_0750)
    UINT32  dummy1_request_period                 ; // 21  (ADDR : 0x9C00_0754)
    UINT32  G14_reserved_16                       ; // 22  (ADDR : 0x9C00_0758)
    UINT32  dummy1_error                          ; // 23  (ADDR : 0x9C00_075C)
    UINT32  dummy1_runtime_l                      ; // 24  (ADDR : 0x9C00_0760)
    UINT32  dummy1_runtime_h                      ; // 25  (ADDR : 0x9C00_0764)
    UINT32  dummy1_data_count_l                   ; // 26  (ADDR : 0x9C00_0768)
    UINT32  dummy1_data_count_h                   ; // 27  (ADDR : 0x9C00_076C)
    UINT32  dummy1_mdata_count_l                  ; // 28  (ADDR : 0x9C00_0770)
    UINT32  dummy1_mdata_count_h                  ; // 29  (ADDR : 0x9C00_0774)
    UINT32  dummy1_sdata_count_l                  ; // 30  (ADDR : 0x9C00_0778)
    UINT32  dummy1_sdata_count_h                  ; // 31  (ADDR : 0x9C00_077C)

    // Group 015 : ACHIP interrupt
    UINT32  achip_intr_type[7]                    ;
    UINT32  achip_intr_polarity[7]                ;
    UINT32  achip_priority[7]                     ;
    UINT32  achip_intr_mask[7]                    ;
    UINT32  G015_RESERVED[4]                      ;

    // Group 016 : UART2
    UINT32  uart2_data                            ; // 00  (ADDR : 0x9C00_0900)
    UINT32  uart2_lsr                             ; // 01  (ADDR : 0x9C00_0904)
    UINT32  uart2_msr                             ; // 02  (ADDR : 0x9C00_0908)
    UINT32  uart2_lcr                             ; // 03  (ADDR : 0x9C00_090C)
    UINT32  uart2_mcr                             ; // 04  (ADDR : 0x9C00_0910)
    UINT32  uart2_div_l                           ; // 05  (ADDR : 0x9C00_0914)
    UINT32  uart2_div_h                           ; // 06  (ADDR : 0x9C00_0918)
    UINT32  uart2_isc                             ; // 07  (ADDR : 0x9C00_091C)
    UINT32  uart2_tx_residue                      ; // 08  (ADDR : 0x9C00_0920)
    UINT32  uart2_rx_residue                      ; // 09  (ADDR : 0x9C00_0924)
    UINT32  uart2_rxfifo_thr                      ; // 10  (ADDR : 0x9C00_0928)
    UINT32  G016_reserved[21]                     ;

    // Group 017 : UART3
    UINT32  uart3_data                            ; // 00  (ADDR : 0x9C00_0880)
    UINT32  uart3_lsr                             ; // 01  (ADDR : 0x9C00_0884)
    UINT32  uart3_msr                             ; // 02  (ADDR : 0x9C00_0888)
    UINT32  uart3_lcr                             ; // 03  (ADDR : 0x9C00_088C)
    UINT32  uart3_mcr                             ; // 04  (ADDR : 0x9C00_0890)
    UINT32  uart3_div_l                           ; // 05  (ADDR : 0x9C00_0894)
    UINT32  uart3_div_h                           ; // 06  (ADDR : 0x9C00_0898)
    UINT32  uart3_isc                             ; // 07  (ADDR : 0x9C00_089C)
    UINT32  uart3_tx_residue                      ; // 08  (ADDR : 0x9C00_08A0)
    UINT32  uart3_rx_residue                      ; // 09  (ADDR : 0x9C00_08A4)
    UINT32  uart3_rxfifo_thr                      ; // 10  (ADDR : 0x9C00_08A8)
    UINT32  G017_reserved[21]                     ; //     (ADDR : 0x9C00_08AC)

    // Group 018 : UART0
    UINT32  uart0_data                            ; // 00  (ADDR : 0x9C00_0900)
    UINT32  uart0_lsr                             ; // 01  (ADDR : 0x9C00_0904)
    UINT32  uart0_msr                             ; // 02  (ADDR : 0x9C00_0908)
    UINT32  uart0_lcr                             ; // 03  (ADDR : 0x9C00_090C)
    UINT32  uart0_mcr                             ; // 04  (ADDR : 0x9C00_0910)
    UINT32  uart0_div_l                           ; // 05  (ADDR : 0x9C00_0914)
    UINT32  uart0_div_h                           ; // 06  (ADDR : 0x9C00_0918)
    UINT32  uart0_isc                             ; // 07  (ADDR : 0x9C00_091C)
    UINT32  uart0_tx_residue                      ; // 08  (ADDR : 0x9C00_0920)
    UINT32  uart0_rx_residue                      ; // 09  (ADDR : 0x9C00_0924)
    UINT32  uart0_rxfifo_thr                      ; // 10  (ADDR : 0x9C00_0928)
    UINT32  G018_reserved[21]                     ;

    // Group 019 : UART1
    UINT32  uart1_data                            ; // 00  (ADDR : 0x9C00_0900)
    UINT32  uart1_lsr                             ; // 01  (ADDR : 0x9C00_0904)
    UINT32  uart1_msr                             ; // 02  (ADDR : 0x9C00_0908)
    UINT32  uart1_lcr                             ; // 03  (ADDR : 0x9C00_090C)
    UINT32  uart1_mcr                             ; // 04  (ADDR : 0x9C00_0910)
    UINT32  uart1_div_l                           ; // 05  (ADDR : 0x9C00_0914)
    UINT32  uart1_div_h                           ; // 06  (ADDR : 0x9C00_0918)
    UINT32  uart1_isc                             ; // 07  (ADDR : 0x9C00_091C)
    UINT32  uart1_tx_residue                      ; // 08  (ADDR : 0x9C00_0920)
    UINT32  uart1_rx_residue                      ; // 09  (ADDR : 0x9C00_0924)
    UINT32  uart1_rxfifo_thr                      ; // 10  (ADDR : 0x9C00_0928)
    UINT32  G019_reserved[21]                     ;

    // Group 020 : DPLL
    UINT32  G020_DPLL[32]                         ;

    // Group 021 : ACHIP interrupt
    UINT32  achip_intr_clr[7]                     ;
    UINT32  masked_achip_fiqs[7]                  ;
    UINT32  masked_achip_irqs[7]                  ;
    UINT32  G021_RESERVED[11]                     ;

    // Group 022 : SPI_FLASH
    UINT32  spi_ctrl                              ;
    UINT32  spi_wait                              ;
    UINT32  spi_cust_cmd                          ;
    UINT32  spi_addr_low                          ;
    UINT32  spi_addr_high                         ;
    UINT32  spi_data_low                          ;
    UINT32  spi_data_high                         ;
    UINT32  spi_status                            ;
    UINT32  spi_cfg0                              ;
    UINT32  spi_cfg1                              ;
    UINT32  spi_cfg2                              ;
    UINT32  spi_cfg3                              ;
    UINT32  spi_cfg4                              ;
    UINT32  spi_cfg5                              ;
    UINT32  spi_cfg6                              ;
    UINT32  spi_cfg7                              ;
    UINT32  spi_cfg8                              ;
    UINT32  spi_cust_cmd2                         ;
    UINT32  spi_data64                            ;
    UINT32  spi_buf_addr                          ;
    UINT32  spi_status2                           ;
    UINT32  spi_status3                           ;
    UINT32  g22_reserved_0[10]                    ;

    // Group 023 : SPI_FLASH
    UINT32  G023_SPI_FLASH[32]                    ;

    // Group 024 : CB_SWITCH
    UINT32  G024_CB_SWITCH[32]                    ; //     (ADDR : 0x9C00_0C00) ~ (ADDR : 0x9C00_0C7C)

    // Group 025 : PBUS
    UINT32  G025_PBUS[32]                         ; //     (ADDR : 0x9C00_0C80) ~ (ADDR : 0x9C00_0CFC)

    // Group 026 : CB_DMA
    UINT32  cbdma_ver                             ; // 00  (ADDR : 0x9C00_0D00)
    UINT32  cbdma_config                          ; // 01  (ADDR : 0x9C00_0D04)
    UINT32  cbdma_dma_length                      ; // 02  (ADDR : 0x9C00_0D08)
    UINT32  cbdma_src_adr                         ; // 03  (ADDR : 0x9C00_0D0c)
    UINT32  cbdma_des_adr                         ; // 04  (ADDR : 0x9C00_0D10)
    UINT32  cbdma_int_status                      ; // 05  (ADDR : 0x9C00_0D14)
    UINT32  cbdma_int_en                          ; // 06  (ADDR : 0x9C00_0D18)
    UINT32  cbdma_memset_value                    ; // 07  (ADDR : 0x9C00_0D1c)
    UINT32  cbdma_sdram_size                      ; // 08  (ADDR : 0x9C00_0D20)
    UINT32  G026_reserved_09                      ; // 09  (ADDR : 0x9C00_0D24)
    UINT32  cbdma_sg_index                        ; // 10  (ADDR : 0x9C00_0D28)
    UINT32  cbdma_sg_config                       ; // 11  (ADDR : 0x9C00_0D2C)
    UINT32  cbdma_sg_dma_length                   ; // 12  (ADDR : 0x9C00_0D30)
    UINT32  cbdma_sg_src_adr                      ; // 13  (ADDR : 0x9C00_0D34)
    UINT32  cbdma_sg_des_adr                      ; // 14  (ADDR : 0x9C00_0D38)
    UINT32  cbdma_sg_memset_value                 ; // 15  (ADDR : 0x9C00_0D3C)
    UINT32  cbdma_sg_setting                      ; // 16  (ADDR : 0x9C00_0D40)
    UINT32  cbdma_sg_loop_en                      ; // 17  (ADDR : 0x9C00_0D44)
    UINT32  cbdma_sg_loop_sram_src                ; // 18  (ADDR : 0x9C00_0D48)
    UINT32  cbdma_sg_loop_sram_size               ; // 19  (ADDR : 0x9C00_0D4C)
    UINT32  G026_reserved_20                      ; // 20  (ADDR : 0x9C00_0D50)
    UINT32  G026_reserved_21                      ; // 21  (ADDR : 0x9C00_0D54)
    UINT32  G026_reserved_22                      ; // 22  (ADDR : 0x9C00_0D58)
    UINT32  G026_reserved_23                      ; // 23  (ADDR : 0x9C00_0D5C)
    UINT32  G026_reserved_24                      ; // 24  (ADDR : 0x9C00_0D60)
    UINT32  G026_reserved_25                      ; // 25  (ADDR : 0x9C00_0D64)
    UINT32  G026_reserved_26                      ; // 26  (ADDR : 0x9C00_0D68)
    UINT32  G026_reserved_27                      ; // 27  (ADDR : 0x9C00_0D6C)
    UINT32  G026_reserved_28                      ; // 28  (ADDR : 0x9C00_0D70)
    UINT32  G026_reserved_29                      ; // 29  (ADDR : 0x9C00_0D74)
    UINT32  G026_reserved_30                      ; // 30  (ADDR : 0x9C00_0D78)
    UINT32  G026_reserved_31                      ; // 31  (ADDR : 0x9C00_0D7C)

    // Group 027 : CB_DMA1
    UINT32  cbdma1_ver                            ; // 00  (ADDR : 0x9C00_0D00)
    UINT32  cbdma1_config                         ; // 01  (ADDR : 0x9C00_0D04)
    UINT32  cbdma1_dma_length                     ; // 02  (ADDR : 0x9C00_0D08)
    UINT32  cbdma1_src_adr                        ; // 03  (ADDR : 0x9C00_0D0c)
    UINT32  cbdma1_des_adr                        ; // 04  (ADDR : 0x9C00_0D10)
    UINT32  cbdma1_int_status                     ; // 05  (ADDR : 0x9C00_0D14)
    UINT32  cbdma1_int_en                         ; // 06  (ADDR : 0x9C00_0D18)
    UINT32  cbdma1_memset_value                   ; // 07  (ADDR : 0x9C00_0D1c)
    UINT32  cbdma1_sdram_size                     ; // 08  (ADDR : 0x9C00_0D20)
    UINT32  G027_reserved_09                      ; // 09  (ADDR : 0x9C00_0D24)
    UINT32  cbdma1_sg_index                       ; // 10  (ADDR : 0x9C00_0D28)
    UINT32  cbdma1_sg_config                      ; // 11  (ADDR : 0x9C00_0D2C)
    UINT32  cbdma1_sg_dma_length                  ; // 12  (ADDR : 0x9C00_0D30)
    UINT32  cbdma1_sg_src_adr                     ; // 13  (ADDR : 0x9C00_0D34)
    UINT32  cbdma1_sg_des_adr                     ; // 14  (ADDR : 0x9C00_0D38)
    UINT32  cbdma1_sg_memset_value                ; // 15  (ADDR : 0x9C00_0D3C)
    UINT32  cbdma1_sg_setting                     ; // 16  (ADDR : 0x9C00_0D40)
    UINT32  G027_reserved_17                      ; // 17  (ADDR : 0x9C00_0D44)
    UINT32  G027_reserved_18                      ; // 18  (ADDR : 0x9C00_0D48)
    UINT32  G027_reserved_19                      ; // 19  (ADDR : 0x9C00_0D4C)
    UINT32  G027_reserved_20                      ; // 20  (ADDR : 0x9C00_0D50)
    UINT32  G027_reserved_21                      ; // 21  (ADDR : 0x9C00_0D54)
    UINT32  G027_reserved_22                      ; // 22  (ADDR : 0x9C00_0D58)
    UINT32  G027_reserved_23                      ; // 23  (ADDR : 0x9C00_0D5C)
    UINT32  G027_reserved_24                      ; // 24  (ADDR : 0x9C00_0D60)
    UINT32  G027_reserved_25                      ; // 25  (ADDR : 0x9C00_0D64)
    UINT32  G027_reserved_26                      ; // 26  (ADDR : 0x9C00_0D68)
    UINT32  G027_reserved_27                      ; // 27  (ADDR : 0x9C00_0D6C)
    UINT32  G027_reserved_28                      ; // 28  (ADDR : 0x9C00_0D70)
    UINT32  G027_reserved_29                      ; // 29  (ADDR : 0x9C00_0D74)
    UINT32  G027_reserved_30                      ; // 30  (ADDR : 0x9C00_0D78)
    UINT32  G027_reserved_31                      ; // 31  (ADDR : 0x9C00_0D7C)

    // Group 028 : Reserved
    UINT32  G028_RESERVED[32]                     ;

    // Group 029 : MBUS
    UINT32  mbus_setting                          ;
    UINT32  G029_reserved[15]                     ;
    UINT32  mbus_int_en                           ;
    UINT32  mbus_int_status                       ;
    UINT32  mbus_int_addr                         ;
    UINT32  mbus_mx_ab_sel                        ;
    UINT32  mbus_mx_ab_arb_type                   ;
    UINT32  mbus_mx_ab_priority                   ;
    UINT32  G029_reserved1[10]                    ;

    // Group 030 : Reserved
    UINT32  G030_RESERVED[32]                     ;

    // Group 031 : MBUS
    UINT32  mbar0_page_setting                    ;
    UINT32  mbar0_m1_setting                      ;
    UINT32  mbar0_m2_setting                      ;
    UINT32  mbar0_m3_setting                      ;
    UINT32  mbar0_m4_setting                      ;
    UINT32  mbar0_m5_setting                      ;
    UINT32  mbar0_m6_setting                      ;
    UINT32  mbar0_m7_setting                      ;
    UINT32  mbar0_m8_setting                      ;
    UINT32  mbar0_m9_setting                      ;
    UINT32  mbar0_m10_setting                     ;
    UINT32  mbar0_m11_setting                     ;
    UINT32  mbar0_m12_setting                     ;
    UINT32  mbar0_m13_setting                     ;
    UINT32  mbar0_m14_setting                     ;
    UINT32  mbar0_m15_setting                     ;
    UINT32  mbar0_m16_setting                     ;
    UINT32  mbar0_m17_setting                     ;
    UINT32  mbar0_m18_setting                     ;
    UINT32  mbar0_m19_setting                     ;
    UINT32  mbar0_m20_setting                     ;
    UINT32  mbar0_m21_setting                     ;
    UINT32  mbar0_m22_setting                     ;
    UINT32  mbar0_m23_setting                     ;
    UINT32  mbar0_m24_setting                     ;
    UINT32  mbar0_m25_setting                     ;
    UINT32  mbar0_m26_setting                     ;
    UINT32  mbar0_m27_setting                     ;
    UINT32  mbar0_m28_setting                     ;
    UINT32  mbar0_m29_setting                     ;
    UINT32  mbar0_m30_setting                     ;
    UINT32  mbar0_m31_setting                     ;

    // Group 032 : SDCTRL0
    UINT32  G032_SDCTRL0[32]                      ;

    // Group 033 : SDCTRL0
    UINT32  G033_SDCTRL0[32]                      ;

    // Group 034 : SDCTRL0
    UINT32  G034_SDCTRL0[32]                      ;

    // Group 035 : SDCTRL0
    UINT32  G035_SDCTRL0[32]                      ;

    // Group 036 : SDCTRL0 (2DBG)
    UINT32  G036_SDCTRL0[32]                     ;

    // Group 037 : SDCTRL0 (2DBG)
    UINT32  G037_SDCTRL0[32]                     ;

    // Group 038 : SDCTRL0 (2DBG)
    UINT32  G038_SDCTRL0[32]                     ;

    // Group 039 : SDCTRL0 (2DBG)
    UINT32  G039_SDCTRL0[32]                     ;

    // Group 040 : SDCTRL0
    UINT32  G040_SDCTRL0[32]                      ;

    // Group 041 : SDCTRL0
    UINT32  G041_SDCTRL0[32]                      ;

    // Group 042 : SDCTRL0
    UINT32  G042_SDCTRL0[32]                      ;

    // Group 043 : Reserved
    UINT32  G043_RESERVED[32]                     ;
    
    // Group 044 : Reserved
    UINT32  G044_RESERVED[32]                     ;
    
    // Group 045 : Reserved
    UINT32  G045_RESERVED[32]                     ;

    //Group 046 : CEVA DSP
    UINT32  dma0_sram_addr                        ; // 00  
    UINT32  dma0_dram_addr                        ; // 01  
    UINT32  dma0_txsize                           ; // 02  
    UINT32  dma0_control                          ; // 03  
    UINT32  dma0_status                           ; // 04  
    UINT32  dma1_sram_addr                        ; // 05  
    UINT32  dma1_dram_addr                        ; // 06  
    UINT32  dma1_txsize                           ; // 07  
    UINT32  dma1_control                          ; // 08  
    UINT32  dma1_status                           ; // 09  
    UINT32  dma2_sram_addr                        ; // 10  
    UINT32  dma2_dram_addr                        ; // 11  
    UINT32  dma2_txsize                           ; // 12  
    UINT32  dma2_control                          ; // 13  
    UINT32  dma2_status                           ; // 14  
    UINT32  g56_reserved15                        ; // 15  
    UINT32  dsp_inst_page_sw                      ; // 16  
    UINT32  dsp_inst_page0                        ; // 17  
    UINT32  dsp_inst_page1                        ; // 18  
    UINT32  dsp_inst_page2                        ; // 19  
    UINT32  dsp_inst_page3                        ; // 20  
    UINT32  dsp_inst_page4                        ; // 21  
    UINT32  dsp_inst_page5                        ; // 22  
    UINT32  dsp_inst_page6                        ; // 23  
    UINT32  dsp_inst_page7                        ; // 24  
    UINT32  dsp_data_page                         ; // 25  
    UINT32  dsp_rom_page                          ; // 26  
    UINT32  g56_reserved27                        ; // 27  
    UINT32  g56_reserved28                        ; // 28  
    UINT32  g56_reserved29                        ; // 29  
    UINT32  g56_reserved30                        ; // 30  
    UINT32  g56_reserved31                        ; // 31  

    //Group 047 : CEVA DSP
    UINT32  rdif_ctrl                             ; // 00  
    UINT32  rdif_int_ctrl                         ; // 01  
    UINT32  rdif_vint_vector                      ; // 02  
    UINT32  rdif_code_key                         ; // 03  
    UINT32  dsp_status                            ; // 04  
    UINT32  rdif_cfg                              ; // 05  
    UINT32  rdif_oh_ctrl                          ; // 06  
    UINT32  g57_reserved7                         ; // 07  
    UINT32  dsp_timer_ctrl                        ; // 08  
    UINT32  dsp_tcount                            ; // 09  
    UINT32  dsp_tperiod                           ; // 10  
    UINT32  dsp_timer_status                      ; // 11  
    UINT32  g57_reserved12                        ; // 12  
    UINT32  g57_reserved13                        ; // 13  
    UINT32  g57_reserved14                        ; // 14  
    UINT32  g57_reserved15                        ; // 15  
    UINT32  g57_reserved16                        ; // 16  
    UINT32  g57_reserved17                        ; // 17  
    UINT32  g57_reserved18                        ; // 18  
    UINT32  g57_reserved19                        ; // 19  
    UINT32  g57_reserved20                        ; // 20  
    UINT32  g57_reserved21                        ; // 21  
    UINT32  g57_reserved22                        ; // 22  
    UINT32  g57_reserved23                        ; // 23  
    UINT32  g57_reserved24                        ; // 24  
    UINT32  g57_reserved25                        ; // 25  
    UINT32  g57_reserved26                        ; // 26  
    UINT32  g57_reserved27                        ; // 27  
    UINT32  g57_reserved28                        ; // 28  
    UINT32  g57_reserved29                        ; // 29  
    UINT32  g57_reserved30                        ; // 30  
    UINT32  g57_reserved31                        ; // 31  

    //Group 048 : CEVA DSP
    UINT32  dsp_monitor_trigger                   ; // 00  
    UINT32  g58_reserved1                         ; // 01  
    UINT32  dsp_cycle                             ; // 02  
    UINT32  dsp_dma_cnt                           ; // 03  
    UINT32  dsp_cache_nseq_cnt                    ; // 04  
    UINT32  dsp_dma_cycle                         ; // 05  
    UINT32  dsp_max_dma_cycle                     ; // 06  
    UINT32  dsp_min_dma_cycle                     ; // 07  
    UINT32  dsp_cache_acc_cycle                   ; // 08  
    UINT32  dsp_dma_status                        ; // 09  
    UINT32  dsp_error                             ; // 10  
    UINT32  g58_reserved11                        ; // 11  
    UINT32  g58_reserved12                        ; // 12  
    UINT32  g58_reserved13                        ; // 13  
    UINT32  g58_reserved14                        ; // 14  
    UINT32  g58_reserved15                        ; // 15  
    UINT32  ocm_inst_reg                          ; // 16  
    UINT32  ocm_cntl_reg                          ; // 17  
    UINT32  ocm_stat_reg                          ; // 18  
    UINT32  g58_reserved19                        ; // 19  
    UINT32  ocm_data_reg0                         ; // 20  
    UINT32  ocm_data_reg1                         ; // 21  
    UINT32  ocm_data_reg2                         ; // 22  
    UINT32  ocm_data_reg3                         ; // 23  
    UINT32  ocm_data_reg4                         ; // 24  
    UINT32  ocm_data_reg5                         ; // 25  
    UINT32  ocm_data_reg6                         ; // 26  
    UINT32  ocm_data_reg7                         ; // 27  
    UINT32  g58_reserved28                        ; // 28  
    UINT32  g58_reserved29                        ; // 29  
    UINT32  g58_reserved30                        ; // 30  
    UINT32  g58_reserved31                        ; // 31  

    //Group 049 : CEVA DSP
    UINT32  dsp_port0                             ; // 00  
    UINT32  dsp_port1                             ; // 01  
    UINT32  dsp_port2                             ; // 02  
    UINT32  dsp_port3                             ; // 03  
    UINT32  dsp_port4                             ; // 04  
    UINT32  dsp_port5                             ; // 05  
    UINT32  dsp_port6                             ; // 06  
    UINT32  dsp_port7                             ; // 07  
    UINT32  dsp_port8                             ; // 08  
    UINT32  dsp_port9                             ; // 09  
    UINT32  dsp_port10                            ; // 10  
    UINT32  dsp_port11                            ; // 11  
    UINT32  dsp_port12                            ; // 12  
    UINT32  dsp_port13                            ; // 13  
    UINT32  dsp_port14                            ; // 14  
    UINT32  dsp_port15                            ; // 15  
    UINT32  dsp_port16                            ; // 16  
    UINT32  dsp_port17                            ; // 17  
    UINT32  dsp_port18                            ; // 18  
    UINT32  dsp_port19                            ; // 19  
    UINT32  dsp_port20                            ; // 20  
    UINT32  dsp_port21                            ; // 21  
    UINT32  dsp_port22                            ; // 22  
    UINT32  dsp_port23                            ; // 23  
    UINT32  dsp_port24                            ; // 24  
    UINT32  dsp_port25                            ; // 25  
    UINT32  dsp_port26                            ; // 26  
    UINT32  dsp_port27                            ; // 27  
    UINT32  dsp_port28                            ; // 28  
    UINT32  dsp_port29                            ; // 29  
    UINT32  dsp_port30                            ; // 30  
    UINT32  dsp_port31                            ; // 31
    
    // Group 050 : DDR_PHY0
    UINT32  G050_DDR_PHY0[32]                     ;

    // Group 051 : DDR_PHY0
    UINT32  G051_DDR_PHY0[32]                     ;

    // Group 052 : DDR_PHY0
    UINT32  G052_DDR_PHY0[32]                     ;

    // Group 053 : DDR_PHY0
    UINT32  G053_DDR_PHY0[32]                      ;
    
    // Group 054 : Reserved
    UINT32  G054_RESERVED[32]                     ;
    
    // Group 055 : Reserved
    UINT32  G055_DDC0[32]                     ;
    
    // Group 056 : Reserved
    UINT32  G056_RESERVED[32]                     ;
    
    // Group 057 : Reserved
    UINT32  G057_I2CM0[32]                        ;

    // Group 058 : Reserved
    UINT32  G058_I2CM1[32]                        ;

    // Group 059 : Reserved
    UINT32  G059_RESERVED[32]                     ;

    // Group 060 : AUD
    UINT32  audif_ctrl                            ; // 00
    UINT32  aud_enable                            ; // 01
    UINT32  pcm_cfg                               ; // 02
    UINT32  i2s_mute_flag_ctrl                    ; // 03
    UINT32  ext_adc_cfg                           ; // 04
    UINT32  int_dac_ctrl0                         ; // 05
    UINT32  int_adc_ctrl                          ; // 06
    UINT32  adc_in_path_switch                    ; // 07
    UINT32  int_adc_dac_cfg                       ; // 08
    UINT32  G060_reserved_9                       ; // 09
    UINT32  iec_cfg                               ; // 10
    UINT32  iec0_valid_out                        ; // 11
    UINT32  iec0_par0_out                         ; // 12
    UINT32  iec0_par1_out                         ; // 13
    UINT32  iec1_valid_out                        ; // 14
    UINT32  iec1_par0_out                         ; // 15
    UINT32  iec1_par1_out                         ; // 16
    UINT32  iec0_rx_debug_info                    ; // 17
    UINT32  iec0_valid_in                         ; // 18
    UINT32  iec0_par0_in                          ; // 19
    UINT32  iec0_par1_in                          ; // 20
    UINT32  iec1_rx_debug_info                    ; // 21
    UINT32  iec1_valid_in                         ; // 22
    UINT32  iec1_par0_in                          ; // 23
    UINT32  iec1_par1_in                          ; // 24
    UINT32  iec2_rx_debug_info                    ; // 25
    UINT32  iec2_valid_in                         ; // 26
    UINT32  iec2_par0_in                          ; // 27
    UINT32  iec2_par1_in                          ; // 28
    UINT32  G060_reserved_29                      ; // 29
    UINT32  iec_tx_user_wdata                     ; // 30
    UINT32  iec_tx_user_ctrl                      ; // 31

    // Group 061 : AUD
    UINT32  adcp_ch_enable                        ; // 00, ADCPRC Configuration Group 1  
    UINT32  adcp_fubypass                         ; // 01, ADCPRC Configuration Group 2  
    UINT32  adcp_mode_ctrl                        ; // 02, ADCPRC Mode Control  
    UINT32  adcp_init_ctrl                        ; // 03, ADCP Initialization Control  
    UINT32  adcp_coeff_din                        ; // 04, Coefficient Data Input  
    UINT32  adcp_agc_cfg                          ; // 05, ADCPRC AGC Configuration of Ch0/1  
    UINT32  adcp_agc_cfg2                         ; // 06, ADCPRC AGC Configuration of Ch2/3  
    UINT32  adcp_gain_0                           ; // 07, ADCP System Gain1
    UINT32  adcp_gain_1                           ; // 08, ADCP System Gain2
    UINT32  adcp_gain_2                           ; // 09, ADCP System Gain3
    UINT32  adcp_gain_3                           ; // 10, ADCP System Gain4
    UINT32  adcp_risc_gain                        ; // 11, ADCP RISC Gain  
    UINT32  adcp_mic_l                            ; // 12, ADCPRC Microphone - in Left Channel Data  
    UINT32  adcp_mic_r                            ; // 13, ADCPRC Microphone - in Right Channel Data  
    UINT32  adcp_agc_gain                         ; // 14, ADCPRC AGC Gain  
    UINT32  G061_reserved_15                      ; // 15, Reserved  
    UINT32  aud_apt_mode                          ; // 16, Audio Playback Timer Mode  
    UINT32  aud_apt_data                          ; // 17, Audio Playback Timer  
    UINT32  aud_apt_parameter                     ; // 18, Audio Playback Timer Parameter  
    UINT32  G061_reserved_19                      ; // 19, Reserved  
    UINT32  aud_audhwya                           ; // 20, DRAM Base Address Offset  
    UINT32  aud_inc_0                             ; // 21, DMA Counter Increment/Decrement  
    UINT32  aud_delta_0                           ; // 22, Delta Value  
    UINT32  aud_fifo_enable                       ; // 23, Audio FIFO Enable  
    UINT32  aud_fifo_mode                         ; // 24, FIFO Mode Control  
    UINT32  aud_fifo_support                      ; // 25, Supported FIFOs ( Debug Function )  
    UINT32  aud_fifo_reset                        ; // 26, Host FIFO Reset  
    UINT32  aud_chk_ctrl                          ; // 27, Checksum Control ( Debug Function )  
    UINT32  aud_checksum_data                     ; // 28, Checksum Data ( Debug Function )  
    UINT32  aud_chk_tcnt                          ; // 29, Target Count of Checksum ( Debug Function )  
    UINT32  aud_embedded_input_ctrl               ; // 30, Embedded Input Control ( Debug Function )  
    UINT32  aud_misc_ctrl                         ; // 31, Miscellaneous Control  

    // Group 062 : AUD
    UINT32  aud_ext_dac_xck_cfg                   ; // 00
    UINT32  aud_ext_dac_bck_cfg                   ; // 01
    UINT32  aud_iec0_bclk_cfg                     ; // 02
    UINT32  aud_ext_adc_xck_cfg                   ; // 03
    UINT32  aud_ext_adc_bck_cfg                   ; // 04
    UINT32  aud_int_adc_xck_cfg                   ; // 05
    UINT32  G062_reserved_6                       ; // 06
    UINT32  aud_int_dac_xck_cfg                   ; // 07
    UINT32  aud_int_dac_bck_cfg                   ; // 08
    UINT32  aud_iec1_bclk_cfg                     ; // 09
    UINT32  G062_reserved_10                      ; // 10
    UINT32  aud_pcm_iec_bclk_cfg                  ; // 11
    UINT32  aud_xck_osr104_cfg                    ; // 12
    UINT32  aud_hdmi_tx_mclk_cfg                  ; // 13
    UINT32  aud_hdmi_tx_bck_cfg                   ; // 14
    UINT32  hdmi_tx_i2s_cfg                       ; // 15
    UINT32  hdmi_rx_i2s_cfg                       ; // 16
    UINT32  aud_aadc_agc01_cfg0                   ; // 17
    UINT32  aud_aadc_agc01_cfg1                   ; // 18
    UINT32  aud_aadc_agc01_cfg2                   ; // 19
    UINT32  aud_aadc_agc01_cfg3                   ; // 20
    UINT32  int_adc_ctrl3                      	  ; // 21
    UINT32  int_adc_ctrl2                         ; // 22
    UINT32  int_dac_ctrl2                         ; // 23
    UINT32  G062_reserved_24                      ; // 24
    UINT32  int_dac_ctrl1                         ; // 25
    UINT32  G062_reserved_26                      ; // 26
    UINT32  G062_reserved_27                      ; // 27
    UINT32  G062_reserved_28                      ; // 28
    UINT32  G062_reserved_29                      ; // 29
    UINT32  G062_reserved_30                      ; // 30
    UINT32  G062_reserved_31                      ; // 31

    // Group 063 : AUD
    UINT32  aud_bt_ifx_cfg                        ; // 00
    UINT32  aud_bt_i2s_cfg                        ; // 01
    UINT32  aud_bt_xck_cfg                        ; // 02
    UINT32  aud_bt_bck_cfg                        ; // 03
    UINT32  aud_bt_sync_cfg                       ; // 04
    UINT32  G063_reserved_5                       ; // 05
    UINT32  G063_reserved_6                       ; // 06
    UINT32  G063_reserved_7                       ; // 07
    UINT32  aud_pwm_xck_cfg                       ; // 08
    UINT32  aud_pwm_bck_cfg                       ; // 09
    UINT32  G063_reserved_10                      ; // 10
    UINT32  G063_reserved_11                      ; // 11
    UINT32  G063_reserved_12                      ; // 12
    UINT32  G063_reserved_13                      ; // 13
    UINT32  G063_reserved_14                      ; // 14
    UINT32  G063_reserved_15                      ; // 15
    UINT32  G063_reserved_16                      ; // 16
    UINT32  G063_reserved_17                      ; // 17
    UINT32  G063_reserved_18                      ; // 18
    UINT32  G063_reserved_19                      ; // 19
    UINT32  aud_aadc_agc2_cfg0                    ; // 20
    UINT32  aud_aadc_agc2_cfg1                    ; // 21
    UINT32  aud_aadc_agc2_cfg2                    ; // 22
    UINT32  aud_aadc_agc2_cfg3                    ; // 23
    UINT32  aud_opt_test_pat                      ; // 24
    UINT32  aud_sys_status0                       ; // 25
    UINT32  aud_sys_status1                       ; // 26
    UINT32  int_adc_ctrl1                         ; // 27
    UINT32  bt_mute_flag                          ; // 28
    UINT32  cdrpll_losd_ctrl                      ; // 29
    UINT32  G063_reserved_30                      ; // 30
    UINT32  other_config                          ; // 31

    // Group 064 : AUD
    UINT32  aud_a0_base                           ; // 
    UINT32  aud_a0_length                         ; // 
    UINT32  aud_a0_ptr                            ; // 
    UINT32  aud_a0_cnt                            ; // 
    UINT32  aud_a1_base                           ; // 
    UINT32  aud_a1_length                         ; // 
    UINT32  aud_a1_ptr                            ; // 
    UINT32  aud_a1_cnt                            ; // 
    UINT32  aud_a2_base                           ; // 
    UINT32  aud_a2_length                         ; // 
    UINT32  aud_a2_ptr                            ; // 
    UINT32  aud_a2_cnt                            ; // 
    UINT32  aud_a3_base                           ; // 
    UINT32  aud_a3_length                         ; // 
    UINT32  aud_a3_ptr                            ; // 
    UINT32  aud_a3_cnt                            ; // 
    UINT32  aud_a4_base                           ; // 
    UINT32  aud_a4_length                         ; // 
    UINT32  aud_a4_ptr                            ; // 
    UINT32  aud_a4_cnt                            ; // 
    UINT32  aud_a5_base                           ; // 
    UINT32  aud_a5_length                         ; // 
    UINT32  aud_a5_ptr                            ; // 
    UINT32  aud_a5_cnt                            ; // 
    UINT32  aud_a6_base                           ; // 
    UINT32  aud_a6_length                         ; // 
    UINT32  aud_a6_ptr                            ; // 
    UINT32  aud_a6_cnt                            ; // 
    UINT32  aud_a7_base                           ; // 
    UINT32  aud_a7_length                         ; // 
    UINT32  aud_a7_ptr                            ; // 
    UINT32  aud_a7_cnt                            ; //

    // Group 065 : AUD
    UINT32  aud_a8_base                           ; //
    UINT32  aud_a8_length                         ; //
    UINT32  aud_a8_ptr                            ; //
    UINT32  aud_a8_cnt                            ; //
    UINT32  aud_a9_base                           ; //
    UINT32  aud_a9_length                         ; //
    UINT32  aud_a9_ptr                            ; //
    UINT32  aud_a9_cnt                            ; //
    UINT32  aud_a10_base                          ; //
    UINT32  aud_a10_length                        ; //
    UINT32  aud_a10_ptr                           ; //
    UINT32  aud_a10_cnt                           ; //
    UINT32  aud_a11_base                          ; //
    UINT32  aud_a11_length                        ; //
    UINT32  aud_a11_ptr                           ; //
    UINT32  aud_a11_cnt                           ; //
    UINT32  aud_a12_base                          ; //
    UINT32  aud_a12_length                        ; //
    UINT32  aud_a12_ptr                           ; //
    UINT32  aud_a12_cnt                           ; //
    UINT32  aud_a13_base                          ; //
    UINT32  aud_a13_length                        ; //
    UINT32  aud_a13_ptr                           ; //
    UINT32  aud_a13_cnt                           ; //
    UINT32  aud_a14_base                          ; //
    UINT32  aud_a14_length                        ; //
    UINT32  aud_a14_ptr                           ; //
    UINT32  aud_a14_cnt                           ; //
    UINT32  aud_a15_base                          ; //
    UINT32  aud_a15_length                        ; //
    UINT32  aud_a15_ptr                           ; //
    UINT32  aud_a15_cnt                           ; //


    // Group 066 : AUD
    UINT32  aud_a16_base                          ; //
    UINT32  aud_a16_length                        ; //
    UINT32  aud_a16_ptr                           ; //
    UINT32  aud_a16_cnt                           ; //
    UINT32  aud_a17_base                          ; //
    UINT32  aud_a17_length                        ; //
    UINT32  aud_a17_ptr                           ; //
    UINT32  aud_a17_cnt                           ; //
    UINT32  aud_a18_base                          ; //
    UINT32  aud_a18_length                        ; //
    UINT32  aud_a18_ptr                           ; //
    UINT32  aud_a18_cnt                           ; //
    UINT32  aud_a19_base                          ; //
    UINT32  aud_a19_length                        ; //
    UINT32  aud_a19_ptr                           ; //
    UINT32  aud_a19_cnt                           ; //
    UINT32  aud_a20_base                          ; //
    UINT32  aud_a20_length                        ; //
    UINT32  aud_a20_ptr                           ; //
    UINT32  aud_a20_cnt                           ; //
    UINT32  aud_a21_base                          ; //
    UINT32  aud_a21_length                        ; //
    UINT32  aud_a21_ptr                           ; //
    UINT32  aud_a21_cnt                           ; //

    
    UINT32  G066_reserved_24                      ; //
    UINT32  G066_reserved_25                      ; //
    UINT32  G066_reserved_26                      ; //
    UINT32  G066_reserved_27                      ; //
    UINT32  G066_reserved_28                      ; //
    UINT32  G066_reserved_29                      ; //
    UINT32  G066_reserved_30                      ; //
    UINT32  G066_reserved_31                      ; //

    // Group 067 : AUD
    UINT32  aud_grm_master_gain                   ; // Gain Control  
    UINT32  aud_grm_gain_control_0                ; // Gain Control  
    UINT32  aud_grm_gain_control_1                ; // Gain Control  
    UINT32  aud_grm_gain_control_2                ; // Gain Control  
    UINT32  aud_grm_gain_control_3                ; // Gain Control  
    UINT32  aud_grm_gain_control_4                ; // Gain Control  
    UINT32  aud_grm_mix_control_0                 ; // Mixer Setting  
    UINT32  aud_grm_mix_control_1                 ; // Mixer Setting  
    UINT32  aud_grm_mix_control_2                 ; // Mixer Setting  
    UINT32  aud_grm_switch_0                      ; // Channel Switch  
    UINT32  aud_grm_switch_1                      ; // Channel Switch  
    UINT32  aud_grm_switch_int                    ; // Channel Switch  
    UINT32  aud_grm_delta_volume                  ; // Gain Update  
    UINT32  aud_grm_delta_ramp_pcm                ; // Gain Update  
    UINT32  aud_grm_delta_ramp_risc               ; // Gain Update  
    UINT32  aud_grm_delta_ramp_linein             ; // Gain Update  
    UINT32  aud_grm_other                         ; // Other Setting  
    UINT32  aud_grm_gain_control_5                ; // Gain Control  
    UINT32  aud_grm_gain_control_6                ; // Gain Control  
    UINT32  aud_grm_gain_control_7                ; // Gain Control  
    UINT32  aud_grm_gain_control_8                ; // Gain Control  
    UINT32  aud_grm_fifo_eflag                    ; // FIFO Error Flag  
    UINT32  G067_reserved_22                      ; // 
    UINT32  G067_reserved_23                      ; // 
    UINT32  aud_grm_switch_hdmi_tx                ; // AUD_GRM_SWITCH_HDMI_TX
    UINT32  G067_reserved_25                      ; // 
    UINT32  G067_reserved_26                      ; // 
    UINT32  G067_reserved_27                      ; // 
    UINT32  G067_reserved_28                      ; // 
    UINT32  G067_reserved_29                      ; // 
    UINT32  G067_reserved_30                      ; // 
    UINT32  G067_reserved_31                      ; // 

    // Group 068 : AUD
    UINT32  G068_AUD[32]                          ;

    // Group 069 : Reserved
    UINT32  G069_reserved_00                      ;
    UINT32  G069_reserved_01                      ;
    UINT32  G069_reserved_02                      ;
    UINT32  G069_reserved_03                      ;
    UINT32  G069_reserved_04                      ;
    UINT32  G069_reserved_05                      ;
    UINT32  G069_reserved_06                      ;
    UINT32  G069_reserved_07                      ;
    UINT32  G069_reserved_08                      ;
    UINT32  G069_reserved_09                      ;
    UINT32  G069_reserved_10                      ;
    UINT32  G069_reserved_11                      ;
    UINT32  G069_reserved_12                      ;
    UINT32  G069_reserved_13                      ;
    UINT32  G069_reserved_14                      ;
    UINT32  I2S_PWM_CONTROL_1                     ;
    UINT32  I2S_PWM_CONTROL_2                     ;
    UINT32  I2S_PWM_CONTROL_3                     ;
    UINT32  I2S_PWM_CONTROL_4                     ;
    UINT32  CLASSD_MOS_CONTROL                    ;
    UINT32  G069_reserved_20                      ;
    UINT32  G069_reserved_21                      ;
    UINT32  G069_reserved_22                      ;
    UINT32  G069_reserved_23                      ;
    UINT32  G069_reserved_24                      ;
    UINT32  G069_reserved_25                      ;
    UINT32  G069_reserved_26                      ;
    UINT32  G069_reserved_27                      ;
    UINT32  G069_reserved_28                      ;
    UINT32  G069_reserved_29                      ;
    UINT32  G069_reserved_30                      ;
    UINT32  G069_reserved_31                      ;
} RegisterFile0;


#define REG_BASE           0x9c000000
#define regs0   ((volatile RegisterFile0 *)(REG_BASE))


#endif

typedef enum _SampleRateSetting_e
{
	FS_32,
	FS_44,
	FS_48,
	FS_64,
	FS_88,
	FS_96,
	FS_128,
	FS_176,
	FS_192,
}SampleRateSetting_e;

#define SAMPLE_RATE FS_48 // use enum SampleRateSetting_e to change sample rate
//#define SLOW_100PPM
//#define SPDIF_IN
//#define EXT_I2S_IN

#define ENABLE_INPUT_INTERFACE
#define BYTE_UNIT_WRITE_TO_SDRAM
//#define PWM_8bit
#define PWM_DRAM 2304

struct t_datagen {
#ifdef PWM_8bit
	UINT8 buf[PWM_DRAM];
#else
   #ifdef BYTE_UNIT_WRITE_TO_SDRAM
     UINT8 buf[96*4];
   #else
     UINT32 buf[96];  //WORD_UNIT_WRITE_TO_SDRAM
   #endif
 #endif
   //UINT32 cnt;
   //UINT32 last_value;
   //UINT32 wave_mode;
   //UINT32 ramp_delta;
   //UINT32 precision;
   //UINT32 fast_mode;
};

struct t_datagen pcmgen;


#ifndef PWM_8bit
UINT32 pcmdata[96];
#else
#define PWM_DATA_4BYTE PWM_DRAM/4
UINT32 pcmdata[PWM_DATA_4BYTE];
#endif
UINT32 pcmdataVolCtrl[96];
#define PCM_SHIF 0;

struct t_datagen pwmgen;

//#define PWMGEN
#ifdef PWMGEN
UINT32 pwmdata[576];
#endif




void init_pcmdata(void)
{
   //1.5k 0db tone, 64 samples (stereo) 
   pcmdata[0]=0x00000000;
   pcmdata[1]=0x000018f9;
   pcmdata[2]=0x000c7c80;
   pcmdata[3]=0x30fb0018;
   pcmdata[4]=0x7d80471c;
   pcmdata[5]=0x00238e00;
   pcmdata[6]=0x5a82002d;
   pcmdata[7]=0x41006a6d;
   pcmdata[8]=0x00353680;
   pcmdata[9]=0x7641003b;
   pcmdata[10]=0x20807d8a;
   pcmdata[11]=0x003ec500;
   pcmdata[12]=0x7fff003f;
   pcmdata[13]=0xff807d8b;
   pcmdata[14]=0x003ec580;
   pcmdata[15]=0x7642003b;
   pcmdata[16]=0x21006a6f;
   pcmdata[17]=0x00353780;
   pcmdata[18]=0x5a84002d;
   pcmdata[19]=0x4200471f;
   pcmdata[20]=0x00238f80;
   pcmdata[21]=0x30fe0018;
   pcmdata[22]=0x7f0018fc;
   pcmdata[23]=0x000c7e00;
   pcmdata[24]=0x00030000;
   pcmdata[25]=0x0180e70a;
   pcmdata[26]=0x00f38500;
   pcmdata[27]=0xcf0700e7;
   pcmdata[28]=0x8380b8e6;
   pcmdata[29]=0x00dc7300;
   pcmdata[30]=0xa58000d2;
   pcmdata[31]=0xc0009595;
   pcmdata[32]=0x00caca80;
   pcmdata[33]=0x89c000c4;
   pcmdata[34]=0xe0008276;
   pcmdata[35]=0x00c13b00;
   pcmdata[36]=0x800000c0;
   pcmdata[37]=0x00008275;
   pcmdata[38]=0x00c13a80;
   pcmdata[39]=0x89bc00c4;
   pcmdata[40]=0xde009590;
   pcmdata[41]=0x00cac800;
   pcmdata[42]=0xa57a00d2;
   pcmdata[43]=0xbd00b8de;
   pcmdata[44]=0x00dc6f00;
   pcmdata[45]=0xceff00e7;
   pcmdata[46]=0x7f80e702;
   pcmdata[47]=0x00f38100;
   pcmdata[48]=0x00000000;
   pcmdata[49]=0x000018f9;
   pcmdata[50]=0x000c7c80;
   pcmdata[51]=0x30fb0018;
   pcmdata[52]=0x7d80471c;
   pcmdata[53]=0x00238e00;
   pcmdata[54]=0x5a82002d;
   pcmdata[55]=0x41006a6d;
   pcmdata[56]=0x00353680;
   pcmdata[57]=0x7641003b;
   pcmdata[58]=0x20807d8a;
   pcmdata[59]=0x003ec500;
   pcmdata[60]=0x7fff003f;
   pcmdata[61]=0xff807d8b;
   pcmdata[62]=0x003ec580;
   pcmdata[63]=0x7642003b;
   pcmdata[64]=0x21006a6f;
   pcmdata[65]=0x00353780;
   pcmdata[66]=0x5a84002d;
   pcmdata[67]=0x4200471f;
   pcmdata[68]=0x00238f80;
   pcmdata[69]=0x30fe0018;
   pcmdata[70]=0x7f0018fc;
   pcmdata[71]=0x000c7e00;
   pcmdata[72]=0x00030000;
   pcmdata[73]=0x0180e70a;
   pcmdata[74]=0x00f38500;
   pcmdata[75]=0xcf0700e7;
   pcmdata[76]=0x8380b8e6;
   pcmdata[77]=0x00dc7300;
   pcmdata[78]=0xa58000d2;
   pcmdata[79]=0xc0009595;
   pcmdata[80]=0x00caca80;
   pcmdata[81]=0x89c000c4;
   pcmdata[82]=0xe0008276;
   pcmdata[83]=0x00c13b00;
   pcmdata[84]=0x800000c0;
   pcmdata[85]=0x00008275;
   pcmdata[86]=0x00c13a80;
   pcmdata[87]=0x89bc00c4;
   pcmdata[88]=0xde009590;
   pcmdata[89]=0x00cac800;
   pcmdata[90]=0xa57a00d2;
   pcmdata[91]=0xbd00b8de;
   pcmdata[92]=0x00dc6f00;
   pcmdata[93]=0xceff00e7;
   pcmdata[94]=0x7f80e702;
   pcmdata[95]=0x00f38100;

}//init_pcmdata



#define UINT32 unsigned int

///// utilities of test program /////
#define ONEHOT_B00  0x00000001
#define ONEHOT_B01  0x00000002
#define ONEHOT_B02  0x00000004
#define ONEHOT_B03  0x00000008
#define ONEHOT_B04  0x00000010
#define ONEHOT_B05  0x00000020
#define ONEHOT_B06  0x00000040
#define ONEHOT_B07  0x00000080
#define ONEHOT_B08  0x00000100
#define ONEHOT_B09  0x00000200
#define ONEHOT_B10  0x00000400
#define ONEHOT_B11  0x00000800
#define ONEHOT_B12  0x00001000
#define ONEHOT_B13  0x00002000
#define ONEHOT_B14  0x00004000
#define ONEHOT_B15  0x00008000
#define ONEHOT_B16  0x00010000
#define ONEHOT_B17  0x00020000
#define ONEHOT_B18  0x00040000
#define ONEHOT_B19  0x00080000
#define ONEHOT_B20  0x00100000
#define ONEHOT_B21  0x00200000
#define ONEHOT_B22  0x00400000
#define ONEHOT_B23  0x00800000
#define ONEHOT_B24  0x01000000
#define ONEHOT_B25  0x02000000
#define ONEHOT_B26  0x04000000
#define ONEHOT_B27  0x08000000
#define ONEHOT_B28  0x10000000
#define ONEHOT_B29  0x20000000

#define ONECOLD_B00 0xFFFFFFFE
#define ONECOLD_B01 0xFFFFFFFD
#define ONECOLD_B02 0xFFFFFFFB
#define ONECOLD_B03 0xFFFFFFF7
#define ONECOLD_B04 0xFFFFFFEF
#define ONECOLD_B05 0xFFFFFFDF
#define ONECOLD_B06 0xFFFFFFBF
#define ONECOLD_B07 0xFFFFFF7F
#define ONECOLD_B20 0xFFEFFFFF

#define CTFIFO_BYTES		128
#define PCM_DMA_SAMPLES		 64
#define DRAM_PCM_BUF_CH_SAMPLES 256
#define DRAM_PCM_BUF_CH_BYTES  9*1024//(DRAM_PCM_BUF_CH_SAMPLES*6)*4

//enum
enum e_SpdifSrc { e_RAW=0, e_PCM };

UINT32 a0_dramAddr;
UINT32 a1_dramAddr;
UINT32 a2_dramAddr;
UINT32 a3_dramAddr;
UINT32 a4_dramAddr;
UINT32 a5_dramAddr;
UINT32 a6_dramAddr;
UINT32 a7_dramAddr;
UINT32 a10_dramAddr;
UINT32 a11_dramAddr;
UINT32 a12_dramAddr;
UINT32 a13_dramAddr;
UINT32 a16_dramAddr;
UINT32 a17_dramAddr;
UINT32 a18_dramAddr;
UINT32 a19_dramAddr;
UINT32 a20_dramAddr;
UINT32 a21_dramAddr;


unsigned int byteptrA0 = 0;
unsigned int byteptrA5 = 0;
unsigned int byteptrA6 = 0;

unsigned int byteptrA11 = 0;
unsigned int byteptrA13 = 0;
unsigned int byteptrA16 = 0;


UINT32 pcmout_block_cnt;
UINT32 pcm_dma_dram_bytes;
UINT32 pcm_max_blocks;
UINT32 pcmbuf_endaddr;

struct t_audcfg {
   UINT32 dac_Fs;
   UINT32 use_on_chip_dac;	//1:use acodec, 0:bypass it 
   UINT32 use_ext_adc0;		//1:use ext adc0 for line-in
   UINT32 pcm_iec_enable;
   UINT32 spdif_tx0_src;	//0:raw, 1:pcm 
   UINT32 spdif_tx0_fifo_enable;
   UINT32 spdif_tx1_src;	//0:raw, 1:pcm 
   UINT32 spdif_tx1_fifo_enable;
   UINT32 record_enable;	
   UINT32 record_src;		//0:line-in, 2:mic_src
   UINT32 fifo_line_in_enable;	
   UINT32 fifo_hdmi_i2s_in_enable;	
} audcfg;


/************sub api**************/
void F_Adcp_WaitInitOk(void)
{
   int val;
   do {
      //wait(50);
      //delay_1ms(50);

      val= regs0->adcp_init_ctrl; //read adcp_init_ctrl
   } while ( (val&ONEHOT_B12)!=0 );
   printf("F_Adcp_WaitInitOk \n");
}//F_Adcp_WaitInitOk

void F_Cfg_AdcIn(void)
{
   int val, ch_base;
   ///// Config ADCPRC /////

   regs0->adcp_ch_enable = 0x0;      //adcp_ch_enable
   regs0->adcp_fubypass = 0x7777;   //adcp_fubypass
   regs0->adcp_risc_gain = 0x1111;  //adcp_risc_gain, all gains are 1x

	regs0->G069_reserved_00 = 0x3; // adcprc A16~18
   // agc setting for both ch0 and ch1 //
   val=0x650100;              //steplen0=0, Eth_off=0x65, Eth_on=0x100, steplen0=0
   regs0->adcp_agc_cfg = val;  //adcp_agc_cfg0

   //ch0//
   ch_base = 0<<4;
   val=(1<<6)|ch_base|ONEHOT_B11; //initbuf of ch0
   regs0->adcp_init_ctrl = val;	  //adcp_init_ctrl
   F_Adcp_WaitInitOk();
   val=(1<<6)|ch_base|2|ONEHOT_B10;     //coeff_idx=2, inc=1

   regs0->adcp_init_ctrl = val;      //adcp_init_ctrl
   //F_Adcp_WriteIIRcoeff();

   val=0x800000;
   regs0->adcp_gain_0 = val;      //adcp_gain0

   val = regs0->adcp_risc_gain;  //read adcp_risc_gain
   val=val&0xfff0;
   val=val|1;
   regs0->adcp_risc_gain = val;  //adcp_risc_gain

   //end of ch0//

   //ch1//
   ch_base = 1<<4;
   val=(1<<6)|ch_base|ONEHOT_B11; //initbuf of ch0
   regs0->adcp_init_ctrl = val;	  //adcp_init_ctrl
   F_Adcp_WaitInitOk();
   val=(1<<6)|ch_base|2|ONEHOT_B10;     //coeff_idx=2, inc=1

   regs0->adcp_init_ctrl = val;      //adcp_init_ctrl
   //F_Adcp_WriteIIRcoeff();

   val=0x800000;
   regs0->adcp_gain_1 = val;      //adcp_gain1

   val = regs0->adcp_risc_gain;  //read adcp_risc_gain
   val=val&0xff0f;
   val=val|0x10;
   regs0->adcp_risc_gain = val;  //adcp_risc_gain

   //end of ch1//

}//F_Cfg_AdcIn

UINT32 F_Cfg_PCM_Buf(UINT32 buf_endaddr)
{
   
  printf("\n[aud_test] Start of configure PCM buffer (Word Unit)");

   regs0->aud_enable  = 0x0000;  //aud_enable


   a0_dramAddr = buf_endaddr;
   regs0->aud_a0_base = buf_endaddr;
   regs0->aud_a0_length = DRAM_PCM_BUF_CH_BYTES;
   regs0->aud_a0_ptr = 0;
   regs0->aud_a0_cnt = 0;
   buf_endaddr += DRAM_PCM_BUF_CH_BYTES;

   a1_dramAddr = buf_endaddr;
   regs0->aud_a1_base = buf_endaddr;
   regs0->aud_a1_length = DRAM_PCM_BUF_CH_BYTES;
   regs0->aud_a1_ptr = 0;
   regs0->aud_a1_cnt = 0;
   buf_endaddr += DRAM_PCM_BUF_CH_BYTES;


   a2_dramAddr = buf_endaddr;
   regs0->aud_a2_base = buf_endaddr;
   regs0->aud_a2_length = DRAM_PCM_BUF_CH_BYTES;
   regs0->aud_a2_ptr = 0;
   regs0->aud_a2_cnt = 0;
   buf_endaddr += DRAM_PCM_BUF_CH_BYTES;
 
    a3_dramAddr = buf_endaddr;
    regs0->aud_a3_base = buf_endaddr;
 	regs0->aud_a3_length = DRAM_PCM_BUF_CH_BYTES;
	regs0->aud_a3_ptr = 0;
	regs0->aud_a3_cnt = 0;
	buf_endaddr += DRAM_PCM_BUF_CH_BYTES;

    a4_dramAddr = buf_endaddr;
    regs0->aud_a4_base = buf_endaddr;
 	regs0->aud_a4_length = DRAM_PCM_BUF_CH_BYTES;
	regs0->aud_a4_ptr = 0;
	regs0->aud_a4_cnt = 0;
	buf_endaddr += DRAM_PCM_BUF_CH_BYTES;

    a20_dramAddr = buf_endaddr;
    regs0->aud_a20_base = buf_endaddr;
 	regs0->aud_a20_length = DRAM_PCM_BUF_CH_BYTES;
	regs0->aud_a20_ptr = 0;
	regs0->aud_a20_cnt = 0;
	buf_endaddr += DRAM_PCM_BUF_CH_BYTES;


   a5_dramAddr = buf_endaddr;
   regs0->aud_a5_base = buf_endaddr;

  // if (SAMPLE_RATE ==FS_44 || SAMPLE_RATE ==FS_88 || SAMPLE_RATE ==FS_176)
  // 	  regs0->aud_a5_length = 49*3*1024;
  // else
      regs0->aud_a5_length = DRAM_PCM_BUF_CH_BYTES;
   regs0->aud_a5_ptr = 0;
   regs0->aud_a5_cnt = 0;
 //  if (SAMPLE_RATE ==FS_44 || SAMPLE_RATE ==FS_88 || SAMPLE_RATE ==FS_176)
  //     buf_endaddr += 49*3*1024;
  // else
       buf_endaddr += DRAM_PCM_BUF_CH_BYTES;

   
   a6_dramAddr = buf_endaddr;
   regs0->aud_a6_base = buf_endaddr;
   #ifdef PWM_8bit
   regs0->aud_a6_length = PWM_DRAM*4;
   #else
   regs0->aud_a6_length = DRAM_PCM_BUF_CH_BYTES;//576*4;//DRAM_PCM_BUF_CH_BYTES;
   #endif
   regs0->aud_a6_ptr = 0;
   regs0->aud_a6_cnt = 0;
	#ifdef PWM_8bit
	buf_endaddr += PWM_DRAM*4;
	#else
   buf_endaddr += DRAM_PCM_BUF_CH_BYTES;//576*4;//DRAM_PCM_BUF_CH_BYTES;
   #endif

  // a10_dramAddr = buf_endaddr;
  // regs0->aud_a10_base = buf_endaddr;
  // regs0->aud_a10_length = DRAM_PCM_BUF_CH_BYTES;
  // regs0->aud_a10_ptr = 0;
  // regs0->aud_a10_cnt = 0;
   //buf_endaddr += DRAM_PCM_BUF_CH_BYTES;   

 //  a11_dramAddr = buf_endaddr;
 //  regs0->aud_a11_base = buf_endaddr;
 //  regs0->aud_a11_length = DRAM_PCM_BUF_CH_BYTES*4*96*2;
 //  regs0->aud_a11_ptr = 0;
//   regs0->aud_a11_cnt = 0;
//   buf_endaddr += DRAM_PCM_BUF_CH_BYTES*4*96*2;   



   // if ( audcfg.record_enable==1 ) {
  //    a12_dramAddr = buf_endaddr;
  //    regs0->aud_a12_base = buf_endaddr;
   //   regs0->aud_a12_length = DRAM_PCM_BUF_CH_BYTES;
  //    regs0->aud_a12_ptr = 0;
  //    regs0->aud_a12_cnt = 0;
   //   buf_endaddr += DRAM_PCM_BUF_CH_BYTES;   
   //}


	a13_dramAddr = buf_endaddr;
	regs0->aud_a13_base = buf_endaddr;
	regs0->aud_a13_length = DRAM_PCM_BUF_CH_BYTES*2;
	regs0->aud_a13_ptr = 0;
	regs0->aud_a13_cnt = 0;
	buf_endaddr += DRAM_PCM_BUF_CH_BYTES*2;   



   if ( audcfg.fifo_hdmi_i2s_in_enable ) {

      a16_dramAddr = buf_endaddr;
      regs0->aud_a16_base = buf_endaddr;
      regs0->aud_a16_length = DRAM_PCM_BUF_CH_BYTES*2;
      regs0->aud_a16_ptr = 0;
      regs0->aud_a16_cnt = 0;
      buf_endaddr += DRAM_PCM_BUF_CH_BYTES*2;   

      a17_dramAddr = buf_endaddr;
      regs0->aud_a17_base = buf_endaddr;
      regs0->aud_a17_length = DRAM_PCM_BUF_CH_BYTES*2;
      regs0->aud_a17_ptr = 0;
      regs0->aud_a17_cnt = 0;
      buf_endaddr += DRAM_PCM_BUF_CH_BYTES*2;   

      a18_dramAddr = buf_endaddr;
      regs0->aud_a18_base = buf_endaddr;
      regs0->aud_a18_length = DRAM_PCM_BUF_CH_BYTES*2;
      regs0->aud_a18_ptr = 0;
      regs0->aud_a18_cnt = 0;
      buf_endaddr += DRAM_PCM_BUF_CH_BYTES*2;   

	  a21_dramAddr = buf_endaddr;
	  regs0->aud_a21_base = buf_endaddr;
	  regs0->aud_a21_length = DRAM_PCM_BUF_CH_BYTES*2;
	  regs0->aud_a21_ptr = 0;
	  regs0->aud_a21_cnt = 0;
	  buf_endaddr += DRAM_PCM_BUF_CH_BYTES*2;

   }

 //  a19_dramAddr = buf_endaddr;
  // regs0->aud_a19_base = buf_endaddr;
  // regs0->aud_a19_length = DRAM_PCM_BUF_CH_BYTES;
   //regs0->aud_a19_ptr = 0;
   //regs0->aud_a19_cnt = 0;
   //buf_endaddr += DRAM_PCM_BUF_CH_BYTES;  

   printf("\n[aud_test] End of configure PCM buffer");
   return buf_endaddr;
}//F_Cfg_PCM_Buf

/*********** audio API ***********/
void  AUD_Set_PLL(void)
{
	//	 //Set PLLA
	//	 //regs0->rf_sft_cfg24 &= 0xfffff800;
/*
	regs0->rf_sft_cfg24 |= 0x1 << 16; //PDN = 1
		 regs0->rf_sft_cfg24 &= ~(0x1 << 17); //BP_A = 0
		 regs0->rf_sft_cfg24 &= ~(0x3 << 20); 
		 regs0->rf_sft_cfg24 |= (0x0 << 20); //MODE = 1
	   regs0->rf_sft_cfg25 = 0x68affd02;
	   regs0->rf_sft_cfg24 &= 0xc3770003;
	   regs0->rf_sft_cfg24 |= 0x03878800;
	   regs0->rf_sft_cfg23 &= 0xfffffffc;
	   regs0->rf_sft_cfg23 |= 0x00000004;
*/

	regs0->rf_sft_cfg24 &= 0xFFFFF7FF; // PLLA Disable 1.24[11]
#if 1
	if( (SAMPLE_RATE==FS_48) || (SAMPLE_RATE==FS_96) || (SAMPLE_RATE==FS_192))
	{
		regs0->rf_pad_ctl7 = ((regs0->rf_pad_ctl7 | 0xffff0000) & 0xFFFFF7FF); // PLLA Disable 4.7 [11]

		regs0->rf_pad_ctl7 = (regs0->rf_pad_ctl7 | 0xffff0000) &0xFFFFEFFF;	// Disable Bypass PLLA 4.7[12]

		regs0->rf_pad_ctl7 = (regs0->rf_pad_ctl7 | 0xffff0000) & 0xFFFFBFFF; // 4.7 [14]
		//regs0->rf_pad_ctl7 |= (0x1 << 14);

		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) & 0xFFFFF3FF;	// DIVM 4.9 [10:9]

		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) & 0xFFFFFFC0;	// DIVN 4.9 [5:0]
		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) | 0x5;

		regs0->rf_pad_ctl8 = (regs0->rf_pad_ctl8 | 0xffff0000) & 0xFFFF87FF;	//PH_SEL 4.8[14:11]
		regs0->rf_pad_ctl8 = (regs0->rf_pad_ctl8 | 0xffff0000) | (0x4<<11);

		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) & 0xFFFF9FFF;	// PH_STEP_SEL 4.9[14:13]
		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) | (0x1<<13);

		regs0->rf_pad_ctl10 = (regs0->rf_pad_ctl10 | 0xffff0000) & 0xFFFFF800;	// K_SDM 4.10[10:0]
		regs0->rf_pad_ctl10 = (regs0->rf_pad_ctl10 | 0xffff0000) | 0x273;	

		regs0->rf_pad_ctl11 = (regs0->rf_pad_ctl11 | 0xffff0000) & 0xFFFFF800;	// M_SDM 4.11 [10:0]
		#ifdef SLOW_100PPM
			regs0->rf_pad_ctl11 = (regs0->rf_pad_ctl11 | 0xffff0000) | 0x45b;
		#else
			regs0->rf_pad_ctl11 = (regs0->rf_pad_ctl11 | 0xffff0000) | 0x3FF;
		#endif
		

		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) & 0xFFFFFF3F; // DIVR 4.9 [12:11]
		
		regs0->rf_pad_ctl7 |= (regs0->rf_pad_ctl7 | 0xffff0000) | (1<<11); // PLLA Enable 4.7[11]
	}
	else if( (SAMPLE_RATE==FS_44) || (SAMPLE_RATE==FS_88) || (SAMPLE_RATE==FS_176))
	{
		regs0->rf_pad_ctl7 = ((regs0->rf_pad_ctl7 | 0xffff0000) & 0xFFFFF7FF); // PLLA Disable 4.7 [11]

		regs0->rf_pad_ctl7 = (regs0->rf_pad_ctl7 | 0xffff0000) &0xFFFFEFFF;	// Disable Bypass PLLA 4.7[12]

		regs0->rf_pad_ctl7 = (regs0->rf_pad_ctl7 | 0xffff0000) & 0xFFFFBFFF; // 4.7 [14]
		//regs0->rf_pad_ctl7 |= (0x1 << 14);

		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) & 0xFFFFF3FF;	// DIVM 4.9 [10:9]

		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) & 0xFFFFFFC0;	// DIVN 4.9 [5:0]
		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) | 0x5;

		regs0->rf_pad_ctl8 = (regs0->rf_pad_ctl8 | 0xffff0000) & 0xFFFF87FF;	//PH_SEL 4.8[14:11]
		regs0->rf_pad_ctl8 = (regs0->rf_pad_ctl8 | 0xffff0000) | (0x0<<11);

		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) & 0xFFFF9FFF;	// PH_STEP_SEL 4.9[14:13]
		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) | (0x1<<13);

		regs0->rf_pad_ctl10 = (regs0->rf_pad_ctl10 | 0xffff0000) & 0xFFFFF800;	// K_SDM 4.10[10:0]
		regs0->rf_pad_ctl10 = (regs0->rf_pad_ctl10 | 0xffff0000) | 0xB4;	

		regs0->rf_pad_ctl11 = (regs0->rf_pad_ctl11 | 0xffff0000) & 0xFFFFF800;	// M_SDM 4.11 [10:0]
		regs0->rf_pad_ctl11 = (regs0->rf_pad_ctl11 | 0xffff0000) | 0x3FF;

		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) & 0xFFFFFF3F; // DIVR 4.9 [12:11]
		
		regs0->rf_pad_ctl7 |= (regs0->rf_pad_ctl7 | 0xffff0000) | (1<<11); // PLLA Enable 4.7[11]	
	}
	else if( (SAMPLE_RATE==FS_32) || (SAMPLE_RATE==FS_64) || (SAMPLE_RATE==FS_128))
	{
		regs0->rf_pad_ctl7 = ((regs0->rf_pad_ctl7 | 0xffff0000) & 0xFFFFF7FF); // PLLA Disable 4.7 [11]

		regs0->rf_pad_ctl7 = (regs0->rf_pad_ctl7 | 0xffff0000) &0xFFFFEFFF;	// Disable Bypass PLLA 4.7[12]

		regs0->rf_pad_ctl7 = (regs0->rf_pad_ctl7 | 0xffff0000) & 0xFFFFBFFF; // 4.7 [14]
		//regs0->rf_pad_ctl7 |= (0x1 << 14);

		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) & 0xFFFFF3FF;	// DIVM 4.9 [10:9]

		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) & 0xFFFFFFC0;	// DIVN 4.9 [5:0]
		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) | 0x7;

		regs0->rf_pad_ctl8 = (regs0->rf_pad_ctl8 | 0xffff0000) & 0xFFFF87FF;	//PH_SEL 4.8[14:11]
		regs0->rf_pad_ctl8 = (regs0->rf_pad_ctl8 | 0xffff0000) | (0x2<<11);

		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) & 0xFFFF9FFF;	// PH_STEP_SEL 4.9[14:13]
		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) | (0x1<<13);

		regs0->rf_pad_ctl10 = (regs0->rf_pad_ctl10 | 0xffff0000) & 0xFFFFF800;	// K_SDM 4.10[10:0]
		regs0->rf_pad_ctl10 = (regs0->rf_pad_ctl10 | 0xffff0000) | 0x36A;//0x2B1;	//0x3d9;//0x345;	

		regs0->rf_pad_ctl11 = (regs0->rf_pad_ctl11 | 0xffff0000) & 0xFFFFF800;	// M_SDM 4.11 [10:0]
		#ifdef SLOW_100PPM
			regs0->rf_pad_ctl11 = (regs0->rf_pad_ctl11 | 0xffff0000) | 0x45C;
		#else
			regs0->rf_pad_ctl11 = (regs0->rf_pad_ctl11 | 0xffff0000) | 0x3FF;
		#endif

		regs0->rf_pad_ctl9 = (regs0->rf_pad_ctl9 | 0xffff0000) & 0xFFFFFF3F; // DIVR 4.9 [12:11]
		
		regs0->rf_pad_ctl7 |= (regs0->rf_pad_ctl7 | 0xffff0000) | (1<<11); // PLLA Enable 4.7[11]	
	}

#endif	

	   //Set PLLH
	   //regs0->rf_sft_cfg29 |= 0x1;	 // MO_PLLH_PDN = 1
	   //regs0->rf_sft_cfg29 |= 0x1 <<9; // MO_PLLH_CK2160MEN = 1
	
	   //diag_printf("Wait PLLA Ready !\n");
	   //wait(60000); // wait for PLLH Tready Time
	   //diag_printf("PLLA Ready !\n");
	
	   //Set DPLL 
	   // 270M/(2*(256*3*Fs)) = 270M/(2*(256*3*8K)) = 21 + 0.97265 = 21 + 63744/65535
	   //regs0->rf_sft_cfg8 &= ~(0x1<<17); // DPLL 1080M div 2 SEL = 0(default)
	   //regs0->G020_DPLL[17] = 63744;//DPLL2_remainder
	   //regs0->G020_DPLL[18] = 65535;//DPLL2_denominator
	   //regs0->G020_DPLL[19] = 21;//DPLL2_divider
	   //regs0->G020_DPLL[16] = 0x3; //DPLL2_ctrl

	return;
}

void aud_pin_mx(void){

  // initial
  //regs0->rf_sft_cfg1 &= 0x0;  
  //regs0->rf_sft_cfg2 &= 0x0;
 // regs0->rf_sft_cfg4 &= 0x0;
  //regs0->rf_sft_cfg7 &= 0x0;

   //regs0->rf_sft_cfg7 |= 0x1 << 7;	 // AUD_PCM_IEC_TX(0-1),  sft_cfg7[7]

   // if( PINMX == 1 ||  PINMX == 99) {
//    regs0->rf_sft_cfg4 |= 0x1 << 2;
//    regs0->rf_sft_cfg4 |= 0x1 << 4;
  //  regs0->rf_sft_cfg4 |= 0x1 << 7;
//    regs0->rf_sft_cfg4 |= 0x1 << 9;
 //   regs0->rf_sft_cfg4 |= 0x1 << 12;
 //   regs0->rf_sft_cfg4 |= 0x1 << 13;

	//regs0->rf_sft_cfg6 |= 0x1 << 17; // AUD_BT_IFX(0-2), sft_cfg6[18:17]
	//regs0->rf_sft_cfg6 |= 0x1 << 19; // AUD_EXT_ADC_IFX0(0-3), sft_cfg6[20:19]
	//regs0->rf_sft_cfg6 |= 0x1 << 21; // AUD_EXT_ADC_IFX1(0-3), sft_cfg6[22:21]
	//regs0->rf_sft_cfg6 |= 0x1 << 23; // AUD_EXT_ADC_IFX2(0-3), sft_cfg6[24:23]
	//regs0->rf_sft_cfg6 |= 0x1 << 25; // AUD_EXT_DAC_IFX0(0-4), sft_cfg6[27:25]
	//regs0->rf_sft_cfg6 |= 0x1 << 28; // AUD_EXT_DAC_IFX1(0-4), sft_cfg6[30:28]
	//regs0->rf_sft_cfg7 |= 0x1 ; 	 // AUD_EXT_DAC_IFX2(0-4), sft_cfg7[2:0]
	//regs0->rf_sft_cfg7 |= 0x1 << 3;	 // AUD_EXT_DAC_IFX3(0-4), sft_cfg7[5:3]
	////regs0->rf_sft_cfg7 |= 0x1 << 6; // AUD_TEST(0), sft_cfg7[6]
	//regs0->rf_sft_cfg7 |= 0x1 << 8;	 // AUD_IEC_RX0(0-1), sft_cfg7[8]
	//regs0->rf_sft_cfg7 |= 0x1 << 9;	 // AUD_IEC_RX1(0-3), sft_cfg7[10:9]
	//regs0->rf_sft_cfg7 |= 0x1 << 11; // AUD_IEC_RX2(0-3), sft_cfg7[12:11]
	//regs0->rf_sft_cfg7 |= 0x1 << 13; // AUD_IEC_TX(0-7), sft_cfg7[15:13]
   // }

	regs0->rf_sft_cfg2 = 0xffff0000| 1|(0x1<<2)|(0x1<<3);// 628 [0]I2S out [2]SPDIF IN [3]SPDIF OUT
	//regs0->rf_sft_cfg2 = 0xffff0000|(0x1<<2)|(0x1<<3);
  	regs0->rf_sft_cfg1 = regs0->rf_sft_cfg1 | 0xffff8000; // [15]I2S IN
	//regs0->rf_sft_cfg2 = 0xffff0001;
	regs0->rf_sft_cfg33 = 0xffff0023; //spdif out  pin mux  
	regs0->G003_RESERVED[10] = 0xffff0400;
} //aud_pin_mx

void aud_clk_cfg(void)
{
	regs0->cdrpll_losd_ctrl = 0x1105; //  for CDRPLL test
   // 147M Setting
	if((SAMPLE_RATE == FS_44) || (SAMPLE_RATE == FS_48))
	{
	   regs0->aud_hdmi_tx_mclk_cfg = 0x6883;  //PLLA, 256FS
	   regs0->aud_ext_adc_xck_cfg = 0x6083;   //PLLA, 256FS  //  for CDRPLL test
	   regs0->aud_ext_dac_xck_cfg = 0x6883;   //PLLA, 256FS
	   regs0->aud_int_dac_xck_cfg = 0x6887;   //PLLA, 128FS
	   regs0->aud_int_adc_xck_cfg = 0x6883;   //PLLA, 256FS
	   //regs0->aud_bt_xck_cfg      = 0x7080;   //DPLL, 256FS
	}
	else if((SAMPLE_RATE == FS_88) || (SAMPLE_RATE == FS_96))
	{
		regs0->aud_hdmi_tx_mclk_cfg = 0x6881;  //PLLA, 256FS
		regs0->aud_ext_adc_xck_cfg = 0x6881;   //PLLA, 256FS
		regs0->aud_ext_dac_xck_cfg = 0x6881;   //PLLA, 256FS
		regs0->aud_int_dac_xck_cfg = 0x6883;   //PLLA, 128FS
		regs0->aud_int_adc_xck_cfg = 0x6881;   //PLLA, 256FS
		//regs0->aud_bt_xck_cfg 	 = 0x7080;	 //DPLL, 256FS
	}
	else if((SAMPLE_RATE == FS_176) || (SAMPLE_RATE == FS_192))
	{
		regs0->aud_hdmi_tx_mclk_cfg = 0x6880;  //PLLA, 256FS
		regs0->aud_ext_adc_xck_cfg = 0x6880;   //PLLA, 256FS
		regs0->aud_ext_dac_xck_cfg = 0x6880;   //PLLA, 256FS  128FS 6881
		regs0->aud_int_dac_xck_cfg = 0x6881;   //PLLA, 128FS
		regs0->aud_int_adc_xck_cfg = 0x6880;   //PLLA, 256FS
		//regs0->aud_bt_xck_cfg 	 = 0x7080;	 //DPLL, 256FS
	}
	else if(SAMPLE_RATE == FS_32)
	{
		regs0->aud_hdmi_tx_mclk_cfg = 0x6887;  //PLLA, 256FS
		regs0->aud_ext_adc_xck_cfg = 0x6887;   //PLLA, 256FS
		regs0->aud_ext_dac_xck_cfg = 0x6887;   //PLLA, 256FS
		regs0->aud_int_dac_xck_cfg = 0x688F;   //PLLA, 128FS
		regs0->aud_int_adc_xck_cfg = 0x6887;   //PLLA, 256FS
		//regs0->aud_bt_xck_cfg 	 = 0x7080;	 //DPLL, 256FS
		
	}
	else if(SAMPLE_RATE == FS_64)
	{
		regs0->aud_hdmi_tx_mclk_cfg = 0x6883;  //PLLA, 256FS
		regs0->aud_ext_adc_xck_cfg = 0x6883;   //PLLA, 256FS
		regs0->aud_ext_dac_xck_cfg = 0x6883;   //PLLA, 256FS  128FS 6887
		regs0->aud_int_dac_xck_cfg = 0x6887;   //PLLA, 128FS
		regs0->aud_int_adc_xck_cfg = 0x6883;   //PLLA, 256FS
		//regs0->aud_bt_xck_cfg 	 = 0x7080;	 //DPLL, 256FS

	}
	else if(SAMPLE_RATE == FS_128)
	{
		regs0->aud_hdmi_tx_mclk_cfg = 0x6881;  //PLLA, 256FS
		regs0->aud_ext_adc_xck_cfg = 0x6881;   //PLLA, 256FS
		regs0->aud_ext_dac_xck_cfg = 0x6881;   //PLLA, 256FS
		regs0->aud_int_dac_xck_cfg = 0x6883;   //PLLA, 128FS
		regs0->aud_int_adc_xck_cfg = 0x6881;   //PLLA, 256FS
		//regs0->aud_bt_xck_cfg 	 = 0x7080;	 //DPLL, 256FS

	}

   // 196.6M Setting (147M = (196.6M * 3)/4)
   //regs0->aud_hdmi_tx_mclk_cfg = 0x680f;
   //regs0->aud_ext_adc_xck_cfg = 0x680f;
   //regs0->aud_ext_dac_xck_cfg = 0x680f;
   //regs0->aud_int_dac_xck_cfg = 0x6807;
   //regs0->aud_int_adc_xck_cfg = 0x680f;
   //regs0->aud_bt_xck_cfg      = 0x7003;

   regs0->aud_hdmi_tx_bck_cfg = 0x6003;   //64FS

	//if((SAMPLE_RATE == FS_176) || (SAMPLE_RATE == FS_192)||(SAMPLE_RATE == FS_64))
	//	regs0->aud_ext_dac_bck_cfg = 0x6001;   //64FS
	//else
   		regs0->aud_ext_dac_bck_cfg = 0x6003;   //64FS
   		
   regs0->aud_int_dac_bck_cfg = 0x6001;   //64FS
   regs0->aud_ext_adc_bck_cfg = 0x6003;   //64FS
   regs0->aud_bt_bck_cfg      = 0x6007;   //32FS, 16/16, 2 slot
   regs0->aud_iec0_bclk_cfg   = 0x6001;   //XCK from EXT_DAC_XCK, 128FS 
   regs0->aud_iec1_bclk_cfg   = 0x6001;   //XCK from EXT_DAC_XCK, 128FS (HDMI SPDIF)
   regs0->aud_pcm_iec_bclk_cfg= 0x6001;   //XCK from EXT_DAC_XCK, 128FS 

   // 196.6M Setting
   regs0->aud_pwm_xck_cfg     = 0x6801;
   //regs0->aud_pwm_bck_cfg     = 0x60ff;
	regs0->aud_pwm_bck_cfg = 0x460ff;
	regs0->I2S_PWM_CONTROL_1 = 0x4041;
	regs0->I2S_PWM_CONTROL_2 = 0x4041;
	regs0->I2S_PWM_CONTROL_3 = 0x4041;
	regs0->I2S_PWM_CONTROL_4 = 0x4041;
	regs0->CLASSD_MOS_CONTROL = 0x1;

    //regs0->I2S_PWM_CONTROL_1 = 0x41d1;
    //regs0->I2S_PWM_CONTROL_2 = 0x51;
    //regs0->I2S_PWM_CONTROL_3 = 0x51;
    //regs0->I2S_PWM_CONTROL_4 = 0x59;

    //regs0->aud_pwm_bck_cfg = 0x460ff;   // pwm fifo enable

}//aud_clk_cfg

   void F_Mixer_Setting(void)
   {
	  UINT32 val;
   
	  //67. 0~4
	  regs0->aud_grm_master_gain = 0x80000000; //aud_grm_master_gain
	  regs0->aud_grm_gain_control_0 = 0x80808080; //aud_grm_gain_control_0
	  regs0->aud_grm_gain_control_1 = 0x80808080; //aud_grm_gain_control_1
	  regs0->aud_grm_gain_control_2 = 0x808000;   //aud_grm_gain_control_2
	  regs0->aud_grm_gain_control_3 = 0x80808080; //aud_grm_gain_control_3
	  regs0->aud_grm_gain_control_4 = 0x0000007f; //aud_grm_gain_control_4
   
	  if ( audcfg.spdif_tx0_src==e_RAW )
		 val = 0x2040; //0=raw, mix40, mix39
	  else 
		 val = 0x204; //1=pcm, mix75, mix73
	  if ( audcfg.spdif_tx1_src==e_RAW )
		 val = val|0x20400000; //0=raw, mix57, mix56
	  else 
		 val = val|0x08100000; //1=pcm, mix79, mix77
   
	  regs0->aud_grm_mix_control_1 = val;  //aud_grm_mix_control_1
   
	  if ( audcfg.record_enable==1 ) {
		 if ( audcfg.record_src==0 )  //0:line-in, 2:mic_src
			val = 0x81;    //[0]:mix58, [7]:mix65
		 else
			val = 0x24;    //[2]:mix60, [9]:mix67
	  }
	  else
		 val = 0;
   
	  regs0->aud_grm_mix_control_2 = val;	   //aud_grm_mix_control_2
   
	  //EXT DAC I2S
	  regs0->aud_grm_switch_0 = 0x76543210;    //aud_grm_switch_0
	  regs0->aud_grm_switch_1 = 0xBA98;		   //aud_grm_switch_1
   
	  //INT DAC I2S
	  regs0->aud_grm_switch_int = 0x76543210;  //aud_grm_switch_int
	  regs0->aud_grm_delta_volume = 0x8000; 	   //aud_grm_delta_volume
	  regs0->aud_grm_delta_ramp_pcm = 0x8000;	   //aud_grm_delta_ramp_pcm
	  regs0->aud_grm_delta_ramp_risc = 0x8000;	   //aud_grm_delta_ramp_risc
	  regs0->aud_grm_delta_ramp_linein = 0x8000;   //aud_grm_delta_ramp_linein
	  regs0->aud_grm_other = 0x4;				   //aud_grm_other for A20
	  regs0->aud_grm_switch_hdmi_tx = 0x76543210;  //aud_grm_switch_hdmi_tx
   
   }//F_Mixer_Setting

   void F_int_dac_adc_Setting(void)
   {
   #if 0
	  //regs0->int_dac_ctrl0 &= 0xffffffe0;   // disable DAC power down mode,for pwdar0, pwdal0, pwdar1, pwdal1, pwdar2
	  regs0->int_dac_ctrl0 = 0xc0208010;
	  regs0->int_adc_ctrl  &= 0xf3fff3ff;	// disable ADC power down mode ADC0/ADC1, G60.6
	  regs0->int_adc_ctrl3 &= 0xfffff3ff;	// disable ADC power down mode ADC2,	  G62.21
   
	  regs0->int_dac_ctrl1 |= 0x1 << 31;	 // ADAC reset
	  regs0->int_adc_ctrl  |= 0x1 << 31;	 // AADC reset
   
	  regs0->int_adc_dac_cfg = 0x1c004d;	// INT ADC/DAC CONFIG
	  regs0->ext_adc_cfg &= 0x0;		// initial 
	  regs0->ext_adc_cfg &= ~(0x1 << 8);	// ADC_CFG_LRSEL[0] = 0, USE INT_ADC1_LRCK	= INT_ADC1_SW ? CONFIG1_LRCK_I : AADC_ADLRC1;
	  regs0->ext_adc_cfg |= 0x1c;		// same as INT_ADC, LRCYCLK(16 bclks), party = 1
	  regs0->int_dac_ctrl0 = 0xC0201010;  // enable ref voltage


	  		/* power on all of DAC and ADC */  // need to be refined !!!  137 AUD register file is wrong!!!!
#else
		regs0->int_dac_ctrl1 |= (0x1<<31);	//ADAC reset (normal mode)
		regs0->int_dac_ctrl0 = 0xC41B8F5F;							//power down DA0, DA1 & DA2, enable auto sleep
		regs0->int_dac_ctrl0 |= (0x7<<23);	//DAC op power on
		regs0->int_dac_ctrl0 &= 0xffffffe0;	//DAC power on
		regs0->int_dac_ctrl0 = 0xC0201010;  // enable ref voltage
		regs0->int_dac_ctrl1 |= 0x3f;			//demute DA0, DA1 & DA2
		regs0->int_adc_ctrl |= (1<<31);		//ACODEC RESET
		regs0->int_adc_ctrl = 0x4F064F1E;							//enable ADC0 & ADC1 VREF, ADC0L pga gain +6dB
		regs0->int_adc_ctrl3 = 0x3F244F06;							//enable ADC2 VREF and
		regs0->int_adc_ctrl &= 0xF3FFF3FF;	//ADC0 & ADC1 power on			
		regs0->int_adc_ctrl3 &= 0xFFFFF3FF;	//ADC2 power on
	#endif
	  //if(TEST_AADC2_PATH == 1) {
	  //	regs0->other_config |= (0x1 << 9);	// AADC2 to A16 path.
	  //	regs0->hdmi_rx_i2s_cfg = 0x1c;	
	  //}
   
   } //F_int_dac_adc_Setting


   void F_SystemInit(UINT32 aud_init_add, UINT32 aud_offset)
   {
	  int val;
	  regs0->aud_audhwya = aud_init_add;  //aud_hwya
								   //reset aud fifo
	  regs0->audif_ctrl  = 0x1;    //aud_ctrl=1
	  regs0->audif_ctrl  = 0x0;    //aud_ctrl=0
	  
	  //regs0->pcm_cfg = 0x4d;  //aud_pcm_cfg=0x4d, i2s, bck=64Fs
	  regs0->pcm_cfg = 0x4d;
	  regs0->hdmi_tx_i2s_cfg = 0x4d;
	  regs0->hdmi_rx_i2s_cfg = 0x4d;	// 0x14d for extenal i2s-in and CLKGENA to be master mode, 0x1c for int-adc
	  regs0->ext_adc_cfg = 0x1c;
	  regs0->int_adc_dac_cfg = 0x001c004d;	//0x001c004d

	
	  // if ( (audcfg.use_ext_adc0==1)&&(audcfg.use_on_chip_dac==1) )
	  //	regs0->ext_adc_cfg = 0x4d; //i2s, bck=64Fs, ACODEC + ext ADC, (aud_ext_adc_cfg)
	  // else
	  //	regs0->ext_adc_cfg = 0x14d; //i2s, bck=64Fs, bypass ACODEC
   
	  regs0->iec0_par0_out = 0x40009800;  //config PCM_IEC_TX, pcm_iec_par0_out
	  regs0->iec0_par1_out = 0x00000000;  //pcm_iec_par1_out
   
	  F_Cfg_AdcIn();
   
	  //regs0->aud_embedded_input_ctrl = 0x5; 		//aud_embedded_input_ctrl
	  
	  pcmbuf_endaddr = F_Cfg_PCM_Buf(aud_offset);

	  regs0->iec_cfg = 0x4002;	//iec_cfg
   
	  val = regs0->aud_misc_ctrl;	   //read aud_misc_ctrl
	  //val=val|ONEHOT_B09;    //select PCM FIFO for HDMI i2s output
	  //regs0->aud_misc_ctrl = val; 	 //aud_misc_ctrl   
   
	  // config playback timer //
	  regs0->aud_apt_mode = 1;			  // aud_apt_mode, reset mode
	  regs0->aud_apt_data = 0x00f0001e;   // aud_apt_parameter, parameter for 48khz
   
	  regs0->adcp_ch_enable  = 0x3; 		  //adcp_ch_enable, Only enable ADCP ch0&1
	   regs0->aud_apt_mode = 0; 		  //clear reset of PTimer before enable FIFO
		 
	  if ( audcfg.pcm_iec_enable==1 ) {
		 val=regs0->iec0_par1_out;				  //read pcm_iec_par1_out
		 regs0->iec0_par1_out = val&ONECOLD_B20;  //deassert PCM_IEC_TX_RST
	  }
   
	  val=1;
	  val |= 0x1f;  //enable A0~A4 output 
	  val |= 0x1<<19; // enable A19 BT_TX
	  val |= 0x1<<20;
	  val |= (audcfg.spdif_tx0_fifo_enable&1)<<5;
	  val |= (audcfg.spdif_tx1_fifo_enable&1)<<6;
	  val |= (audcfg.record_enable&1)<<12;
	  val |= (audcfg.fifo_line_in_enable&1)<<11;
	  if ( audcfg.fifo_hdmi_i2s_in_enable==1 )
		 val |= ((0xf<<16)|(0x1<<21)); 

	  val |= (1<<13); // spdif RX 0 
	  val |= (1<<10); // mic RX

	  regs0->aud_fifo_enable = val; 	  //aud_fifo_enable
	  regs0->aud_enable  = 0x5fffff;	  //aud_enable  [21]PWM
   
	  printf("\n[aud_test] AUD enabled");
   }//F_SystemInit

void Copy_pcmdata_to_pcmgen(void)
{  
		 int i;
  #ifdef BYTE_UNIT_WRITE_TO_SDRAM
		 int cnt, j;
		 UINT8 byte_data;

		 #ifdef PWM_8bit
		 for(i=0;i<PWM_DATA_4BYTE;i++){
			cnt = 0;
			for(j=3;j>=0;j--) {
		  byte_data = (pcmdata[i] >> (j<<3)) & (0xff);
					 pcmgen.buf[i*4+cnt] = (byte_data>>1);
		  cnt = cnt + 1;
			}
		 }

		 #else
		 for(i=0;i<96;i++){
			cnt = 0;
			for(j=3;j>=0;j--) {
		  byte_data = (pcmdata[i] >> (j*8)) & (0xff);
					 pcmgen.buf[i*4+cnt] = (byte_data>>0);
		  cnt = cnt + 1;
			}
		 }
		 #endif
  #else
		 for(i=0;i<96;i++)
			pcmgen.buf[i] = pcmdataVolCtrl[i];
  #endif
}//Copy_pcmdata_to_pcmgen
#ifdef PWMGEN
	  void Copy_pwmdata_to_pwmgen(void)
	  {
		 int i;
  #ifdef BYTE_UNIT_WRITE_TO_SDRAM
		 int cnt, j;
		 UINT8 byte_data;
		 for(i=0;i<576;i++){
			cnt = 0;
			for(j=3;j>=0;j--) {
		  byte_data = (pwmdata[i] >> (j<<3)) & (0xff);
					 pwmgen.buf[i*4+cnt] = byte_data;
		  cnt = cnt + 1;
			}
		 }
  #else
		 for(i=0;i<576;i++)
			pwmgen.buf[i] = pwmdata[i];
  #endif
	  
	  }
#endif

void F_Write_One_PCM_FIFO(int index, UINT32 dramAddr, struct t_datagen *dgen)
{
		  int i;
	  
  #ifdef BYTE_UNIT_WRITE_TO_SDRAM
		  UINT8 *addr;
		  //addr=(UINT8*)((regs0->aud_audhwya<<7)+dramAddr+(PWM_DRAM*index));
			#ifdef PWM_8bit
				addr=(UINT8*)((regs0->aud_audhwya<<7)+dramAddr+(PWM_DRAM*index));
				for(i=0;i<PWM_DRAM;i++) {
					 *(addr+i)=(*dgen).buf[i];
		  		}
			#else
				addr=(UINT8*)((regs0->aud_audhwya<<7)+dramAddr+(384*index));
		  		for(i=0;i<384;i++) {
					 *(addr+i)=(*dgen).buf[i];
		  		}
		  #endif
  #else
		  UINT32 *addr;
		 //addr=(UINT32*)(dramAddr+(DRAM_PCM_BUF_CH_BYTES*index/pcm_max_blocks));
		  addr=(UINT32*)((regs0->aud_audhwya<<7)+dramAddr+(DRAM_PCM_BUF_CH_BYTES*index/4));
		  for(i=0;i<96;i++) 
			 *(addr+i)=(*dgen).buf[i];
  #endif
		 
}//F_Write_OnePcmChannel
void F_Write_One_PWM_FIFO(int index, UINT32 dramAddr, struct t_datagen *dgen)
{
	  int i;
  
#ifdef BYTE_UNIT_WRITE_TO_SDRAM
	  UINT8 *addr;
	  //addr=(UINT8*)(dramAddr+(DRAM_PCM_BUF_CH_BYTES*index/4));
	  addr=(UINT8*)(dramAddr+(576*4*index));
	  for(i=0;i<576*4;i++) {
		 *(addr+i)=(*dgen).buf[i];
	  }
#else
	  UINT32 *addr;
	 //addr=(UINT32*)(dramAddr+(DRAM_PCM_BUF_CH_BYTES*index/pcm_max_blocks));
	  //addr=(UINT32*)(dramAddr+(DRAM_PCM_BUF_CH_BYTES*index/4));
	  addr=(UINT32*)(dramAddr+(576*4*index/4));
	  for(i=0;i<576;i++) 
		 *(addr+i)=(*dgen).buf[i];
#endif
		 
}//F_Write_OnePcmChannel

void F_Fill_PCM_to_Full(void)
{  
		 int i;
	  
		 for(i=0;i<(pcm_max_blocks-0);i++) {
			F_Write_One_PCM_FIFO(i,a0_dramAddr,&pcmgen);		//ext dac
			F_Write_One_PCM_FIFO(i,a1_dramAddr,&pcmgen);		//ext dac
			F_Write_One_PCM_FIFO(i,a2_dramAddr,&pcmgen);		//ext dac
			F_Write_One_PCM_FIFO(i,a3_dramAddr,&pcmgen);
			F_Write_One_PCM_FIFO(i,a4_dramAddr,&pcmgen);
			F_Write_One_PCM_FIFO(i,a20_dramAddr,&pcmgen);
			F_Write_One_PCM_FIFO(i,a5_dramAddr,&pcmgen);		//spdif tx0
			 F_Write_One_PCM_FIFO(i,a6_dramAddr,&pcmgen);		//spdif tx1
			//F_Write_One_PCM_FIFO(i,a6_dramAddr,&pcmgen);
			//F_Write_One_PCM_FIFO(i,a19_dramAddr,&pcmgen);		//bt tx
		 }
	  #ifdef PWMGEN
		 F_Write_One_PWM_FIFO(0,a6_dramAddr,&pwmgen);
	 #endif
}//F_Fill_PCM_to_Full

void F_Aud_Inc0_Wait(void)
{
		 int data;
		 do {
			//wait(50);
			data = regs0->aud_inc_0; //aud_inc
		 } while ( data!=0 );
}//F_Aud_Inc0_Wait

void F_Output_DMA(void)
{
		 int  val;
				  val = pcmout_block_cnt&0x00000003;
			  //F_Write_One_PCM_FIFO(val,a0_dramAddr,&pcmgen);		//ext_dac
			  //F_Write_One_PCM_FIFO(val,a1_dramAddr,&pcmgen);		//ext_dac
			  //F_Write_One_PCM_FIFO(val,a2_dramAddr,&pcmgen);		//ext_dac
				  //F_Write_One_PCM_FIFO(val,a3_dramAddr,&pcmgen);
			  //F_Write_One_PCM_FIFO(val,a5_dramAddr,&pcmgen);		//spdif tx0
			  //F_Write_One_PCM_FIFO(val,a6_dramAddr,&pcmgen);		//spdif tx1
				  //F_Write_One_PCM_FIFO(val,a19_dramAddr,&pcmgen); 	//bluetooth tx
	  
			  //Increase FIFO CNT
			 //val = 0x8006f;
			 //val = 0x27;
			 val =  (0x1<<5);
			 regs0->aud_delta_0 = pcm_dma_dram_bytes;	//aud_delta
			 regs0->aud_inc_0 = val;		  //aud_inc
				F_Aud_Inc0_Wait();
				 //printf("F_Output_DMA \n");
						 pcmout_block_cnt++; 
			//printf("\n[aud_test] pcmout_block_cnt=0x%x",pcmout_block_cnt);
            #ifdef SHOW_PCMOUT_CNT_STAMP
				  if ( (pcmout_block_cnt&CNT_MASK_FOR_PRINT)==0 ) {
					 diag_printf("\n[aud_test] pcmout_block_cnt=0x%x",pcmout_block_cnt);
					 regs0->stamp = pcmout_block_cnt;
				  }
            #endif
	  
}//F_Output_DMA

void F_Write_Output(void)
{
		 int i, val;
		 int pcm_valid;
	  
		 pcm_valid=1;
		 if ( pcm_valid==1 ) {
			for(i=0;i<10;i++) {
			   //val = regs0->aud_a0_cnt;
			   //val = DRAM_PCM_BUF_CH_BYTES - val;
			   val = regs0->aud_a0_cnt;	// 384K Sample rate
			   val = DRAM_PCM_BUF_CH_BYTES - val;
			   if ( val > pcm_dma_dram_bytes ) {
				  F_Output_DMA();
				  break;
			   }//if val > pcm_dma_dram_bytes
			   //wait(100);
			   
			}//for i=0 (for F_Write_PCM_Block())
		 }//if pcm_valid==1
}//F_Write_Output

int printCnt = 0;
int tuneCnt = 0;
int waterLevel = 9*512;
int aveWaterLevel = 0;
int lastAveWaterLevel = 0;
int upTh =	9*512+ 48000*0.005*6;
int downTh =  9*512 - 48000*0.005*6;
int lastA0Cnt =0,nowA0Cnt = 0;
int refFineTuneLevel = 0;
int checkCnt = 0;
int one_sample = 6*20;
int tune_scale = 0;
int fine_tune_scale = 0;


typedef enum
{
	TUNE,
	CHECK_CLK_INCREASE,
	CHECK_CLK_DECREASE,
	FINE_TUNE_CLK_INCREASE,
	FINE_TUNE_CLK_DECREASE,
	STABLE
}ePLLAState;
	
int pllaState = 	STABLE;
int lastPllaState = STABLE;
void AUDIF_Plla_Tuning(UINT8 out_interface, UINT8 in_interface)
{
	//int clkCntDiff;
	UINT32 tmp_clk;
	//UINT8 *debug_out;
	//debug_out = regs0->aud_a0_base;
	//int dn;
	int fifo_cnt,in_cnt;
	// pause count
	//regs0->G063_reserved_7 |= (0x1<<12);
	//clkCntDiff = regs0->G063_reserved_5 - regs0->G063_reserved_6;
	//regs0->G063_reserved_7 &= 0xEFFF;	

	if(out_interface)
		fifo_cnt =regs0->aud_a5_cnt;
	else
		fifo_cnt =regs0->aud_a0_cnt;

	if(in_interface)
		in_cnt = regs0->aud_a13_cnt;
	else
		in_cnt = regs0->aud_a16_cnt;

	if(in_cnt> (DRAM_PCM_BUF_CH_BYTES*2-200))
	{		
		printf("in over \n");

		//while(1){};
	}
	else if( fifo_cnt<100)
	{
		printf("out under \n");
		//for(dn=0;dn<9*1024;dn++){
		//	if((dn%16) == 0)
				printf("\n");
	//		
	//		printf("%02x ",*(debug_out+dn));

	//	}

		//while(1){};
	}
	else if( fifo_cnt>( DRAM_PCM_BUF_CH_BYTES - 384))
	{
		printf("out over \n");
		//while(1){};
	}


	printCnt= printCnt+1;
	
	if(printCnt == 100)
	{
		//printf("%d %0d  %04d %3x\n",pllaState,nowA0Cnt,regs0->aud_a0_cnt,regs0->rf_pad_ctl11);

		printCnt = 0;
		if(out_interface)
			aveWaterLevel = aveWaterLevel + regs0->aud_a5_cnt;
		else
			aveWaterLevel = aveWaterLevel + regs0->aud_a0_cnt;
	}

	tuneCnt = tuneCnt+1;


	
	if( tuneCnt == 5000)
	{
		tuneCnt = 0;
		aveWaterLevel = aveWaterLevel/50;

		if(out_interface)
			nowA0Cnt = regs0->aud_a5_cnt;
		else			
			nowA0Cnt = regs0->aud_a0_cnt;
		lastPllaState = pllaState;
		#if 0
		if(abs(clkCntDiff)>1)
		{
			if (clkCntDiff < 0) // out is slower
			{
				// increase PLLA 
				regs0->rf_pad_ctl11 = (regs0->rf_pad_ctl11|0xffff0000) - 2;

				//reset cnt
				regs0->G063_reserved_7 &= 0xEFFE;
				regs0->G063_reserved_7 |= 0x1;
			}
			else if(clkCntDiff > 0) // out is faster
			{
				// decrease PLLA
				regs0->rf_pad_ctl11 = (regs0->rf_pad_ctl11|0xffff0000) + 2;
				
				//reset cnt
				regs0->G063_reserved_7 &= 0xEFFE;
				regs0->G063_reserved_7 |= 0x1;
			}

		}
		#endif
		
		switch(pllaState)
		{
			case TUNE:
				if(aveWaterLevel > upTh)
				{
					regs0->rf_pad_ctl10 = (regs0->rf_pad_ctl10|0xffff0000) + tune_scale;
					refFineTuneLevel = aveWaterLevel;
					pllaState = CHECK_CLK_INCREASE;
				}
				else if(aveWaterLevel < downTh)
				{
					tmp_clk = (regs0->rf_pad_ctl10&0xfff);

					if(tmp_clk < tune_scale)
						regs0->rf_pad_ctl10 = 0xffff0000;
					else
						regs0->rf_pad_ctl10 = (regs0->rf_pad_ctl10|0xffff0000) - tune_scale;

					refFineTuneLevel = aveWaterLevel;
					pllaState = CHECK_CLK_DECREASE;
				}
				else
				{
					pllaState = lastPllaState;
				}
				break;
			case CHECK_CLK_INCREASE:
				if(aveWaterLevel < waterLevel )
				{
					refFineTuneLevel = aveWaterLevel;
					pllaState = FINE_TUNE_CLK_DECREASE;
				}

				if(aveWaterLevel  > (refFineTuneLevel+15))
				{
					pllaState = TUNE;		
				}

				break;
			case CHECK_CLK_DECREASE:
				if(aveWaterLevel > waterLevel)
				{
					refFineTuneLevel = aveWaterLevel;
					pllaState = FINE_TUNE_CLK_INCREASE;
				}
				
				if(aveWaterLevel  < (refFineTuneLevel-15))
				{
					pllaState = TUNE;		
				}
	
				break;
				
			case FINE_TUNE_CLK_INCREASE:
				if( aveWaterLevel  > (refFineTuneLevel+70))
				{
					regs0->rf_pad_ctl10 = (regs0->rf_pad_ctl10|0xffff0000) + fine_tune_scale;
					refFineTuneLevel = aveWaterLevel;
				}

				if((aveWaterLevel< downTh )||(aveWaterLevel>upTh ))
					pllaState = TUNE;

				break;
				
			case FINE_TUNE_CLK_DECREASE:
				if ( aveWaterLevel < (refFineTuneLevel-70) )
				{
					regs0->rf_pad_ctl10 = (regs0->rf_pad_ctl10|0xffff0000) - fine_tune_scale;
					refFineTuneLevel = aveWaterLevel;
				}
				
				if((aveWaterLevel< downTh )||(aveWaterLevel>upTh ))
					pllaState = TUNE;
			
				break;

			case STABLE:
				if((aveWaterLevel< downTh )||(aveWaterLevel>upTh ))
					pllaState = TUNE;

				break;

		}
		printf("%d  %d %d %03x\n",pllaState,aveWaterLevel,in_cnt,regs0->rf_pad_ctl10);
		lastA0Cnt = nowA0Cnt;
		lastAveWaterLevel = aveWaterLevel;
		aveWaterLevel = 0;
	}




	return;
}



void spdif_pll_out(void)
{

	printf(" ******************** \n ");
	printf(" AUD TEST START \n ");
	printf(" ******************** \n ");




	//int loop_cnt;

	//add your test code here
	printf("%s\n",__FUNCTION__);

		AUD_Set_PLL();

		// Configurations 
		audcfg.dac_Fs = 0;		 //0=48k, 1=96k, 2=192k
		audcfg.use_on_chip_dac = 0;  //1:use acodec, 0:bypass it
		audcfg.use_ext_adc0 = 0;	 //1:use acodec, 0:bypass it
		audcfg.spdif_tx0_src = e_RAW; //0:e_RAW , 1:e_PCM 
		audcfg.spdif_tx1_src = e_RAW; //0:e_RAW, 1:e_PCM
		
		audcfg.pcm_iec_enable = 0;
#ifdef ENABLE_INPUT_INTERFACE
		   audcfg.fifo_line_in_enable = 1;
		   audcfg.fifo_hdmi_i2s_in_enable = 1;
#else
		   audcfg.fifo_line_in_enable = 0;
		   audcfg.fifo_hdmi_i2s_in_enable = 0;
#endif
		if(audcfg.spdif_tx1_src == e_RAW)
		  audcfg.spdif_tx1_fifo_enable = 1;
		else audcfg.spdif_tx1_fifo_enable = 0;
		
		if(audcfg.spdif_tx0_src == e_RAW)
		  audcfg.spdif_tx0_fifo_enable = 1;
		else audcfg.spdif_tx0_fifo_enable = 0;
		
		audcfg.record_enable = 0;
		audcfg.record_src=0;	 //0:line-in, 2:mic_src

		//////////////////////////////////////////////////////////////////////
		
		 //if(PINMX == 99) // Test AADC 6ch to AUD.
		 //    TEST_AADC2_PATH =1;
		
		 //dram_ini();
		
		//regs0->stamp = 0x2222;  //load dram data
		
		aud_pin_mx();
		//F_int_dac_adc_Setting();

		aud_clk_cfg();
		
		
		//regs0->stamp = 0x3333;  //load dram data	  

		F_Mixer_Setting();
		F_int_dac_adc_Setting();
		//F_Blue_tooth_Setting();
		//F_DAGC_Setting();
		F_SystemInit(0x0,0x280000);  // aud init physical address = aud_init_add<<7 + aud_offset
		regs0->int_dac_ctrl1 &= 0x7fffffff;
		regs0->int_dac_ctrl1 |= (0x1<<31);

		printf("int dac \n");
		regs0->int_adc_ctrl= 0x80000726;
		regs0->int_adc_ctrl2 = 0x26;
		regs0->int_adc_ctrl1 = 0x20;  // 0x20 single   0x10 diff mode
		regs0->int_adc_ctrl &= 0x7fffffff;
		regs0->int_adc_ctrl |= (1<<31);



		printf("EXT I2S \n");
		regs0->aud_fifo_mode = 0x20000;
		//regs0->stamp = 0x4444;  //load dram data

		if(regs0->hdmi_rx_i2s_cfg == 0x24D)
			regs0->aud_ext_adc_xck_cfg = 0xc883; // turn off EXT ADC XCK
		if(regs0->pcm_cfg == 0x24D)
			regs0->aud_ext_dac_xck_cfg = 0xc883;
			
		printf("I2S RX  0x%x \n XCK 0x%x \n BCK 0x%x \n",regs0->hdmi_rx_i2s_cfg,regs0->aud_ext_adc_xck_cfg,regs0->aud_ext_adc_bck_cfg);
		printf("I2S TX  0x%x \n XCK 0x%x \n BCK 0x%x \n",regs0->pcm_cfg,regs0->aud_ext_dac_xck_cfg,regs0->aud_ext_dac_bck_cfg);		
		regs0->aud_embedded_input_ctrl = 0x4; 
		printf("internal %x \n",regs0->aud_embedded_input_ctrl);
		//loop_cnt=0;
		pcmout_block_cnt=0;
		pcm_dma_dram_bytes=PCM_DMA_SAMPLES*6;	 //24-bit PCM  (3byte stereo ??)

		//spsid out mux
		regs0->rf_sft_cfg33 = 0xffff0023;
		//I2S in mux
		regs0->rf_sft_cfg42 = 0xffff0018;
#ifdef PWM_8bit
			pcm_max_blocks = 4;
#else
			pcm_max_blocks=DRAM_PCM_BUF_CH_BYTES/(96*4); 
#endif

		printf("init pcm sine \n");
		init_pcmdata();
#ifdef PWMGEN
			init_pwmdata();
#endif
		Copy_pcmdata_to_pcmgen();
#ifdef PWMGEN
			//Copy_pwmdata_to_pwmgen();
#endif
	F_Fill_PCM_to_Full();

	

}

