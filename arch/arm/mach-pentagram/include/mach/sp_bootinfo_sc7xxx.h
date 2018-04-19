#ifndef _INC_SP_BOOTINFO_SC7XXX_H
#define _INC_SP_BOOTINFO_SC7XXX_H

/* copy from iboot "include/common.h" */
struct sp_bootinfo {
	u32     bootrom_ver;         // iboot version
	u32     hw_bootmode;         // hw boot mode (latched: auto, nand, usb_isp, sd_isp, etc)
	u32     gbootRom_boot_mode;  // sw boot mode (category: nand, sd, usb)
	u32     bootdev;             // boot device (exact device: sd0, sd1, ...)
	u32     bootdev_pinx;        // boot device pinmux
	u32     bootdev_port;        // usb0~1, sd0~1
	u32     app_blk_start;       // the block after xboot block(s)
	u32     mp_flag;             // mp machine flag
	u32     bootcpu;             // 0: B, 1: A
};

/* copy from iboot "include/common.h" */
enum Device_table {
	DEVICE_PARA_NAND = 0,
	DEVICE_USB_ISP,
	DEVICE_EMMC,
	DEVICE_SDCARD,
	DEVICE_UART_ISP,
	DEVICE_SPI_NOR,
	DEVICE_SPI_NAND,
	DEVICE_MAX
};

/* copy from iboot "include/config.h" */
#define AUTO_SCAN               0x01
#define AUTO_SCAN_ACHIP         0x15
#define SPI_NOR_BOOT            0x11
#define SPINAND_BOOT            0x09
#define EMMC_BOOT               0x1D
#define SDCARD_ISP              0x05
#define UART_ISP                0x0D
#define USB_ISP                 0x19
#define NAND_LARGE_BOOT         0xff // Q628: no PARA_NAND

/* where to get boot info */
#define CONFIG_SRAM_BASE        0x9e800000
#define SP_GET_BOOTINFO()       ((struct sp_bootinfo *)(CONFIG_SRAM_BASE + 0x9400))

#endif /* _INC_SP_BOOTINFO_SC7XXX_H */
