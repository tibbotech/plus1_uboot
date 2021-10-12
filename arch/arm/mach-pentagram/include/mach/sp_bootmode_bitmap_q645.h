#ifndef __SP_BOOTMODE_BITMAP_Q645_H
#define __SP_BOOTMODE_BITMAP_Q645_H

/*
 * This head is included by config header in include/configs/<soc>.h
 * DO NOT put struct/function definition in this file.
 */

/* copy from iboot "include/config.h" */
#define AUTO_SCAN               0x11
#define SPI_NOR_BOOT            0x17
#define SPINAND_BOOT            0x1D
#define EMMC_BOOT               0x1F
#define SDCARD_ISP              0x19
#define UART_ISP                0x15
#define USB_ISP                 0x1B
#define USB_BOOT                0xfd
#define SDCARD_BOOT             0xfe
#define NAND_LARGE_BOOT         0xff /* Q645: no PARA_NAND */

/* where to get boot info */
#define SP_SRAM_BASE        0xfa200000
#define SP_BOOTINFO_BASE    0x3efdc8

#endif /* __SP_BOOTMODE_BITMAP_Q645_H */
