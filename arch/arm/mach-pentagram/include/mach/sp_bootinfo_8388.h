#ifndef _INC_SP_BOOTINFO_H
#define _INC_SP_BOOTINFO_H

struct sp_bootinfo {
	u32     bootrom_ver;         // iboot version
	u32     hw_bootmode;         // hw boot mode (latched: auto, nand, usb_isp, sd_isp, etc)
	u32     gbootRom_boot_mode;  // sw boot mode (category: nand, sd, usb)
	u32     bootdev;             // boot device (exact device: sd0, sd1, ...)
	u32     bootdev_pinx;        // boot device pinmux
	u32     app_blk_start;       // the block after xboot block(s)
	u32     mp_flag;             // mp machine flag
	// SDCard
	int     gSD_HIGHSPEED_EN_SET_val[4];
	int     gSD_RD_CLK_DELAY_TIME_val[4];
	// other fields are not used by u-boot
};

enum Device_table {
	DEVICE_NAND =0,
	DEVICE_NAND_SBLK, // depricated
	DEVICE_USB0_ISP,
	DEVICE_USB1_ISP,
	DEVICE_SD0,
	DEVICE_SD1,
	DEVICE_UART_ISP,
	DEVICE_SPI_NOR,
	DEVICE_SPI_NAND,
	DEVICE_USB_MSDC,
	DEVICE_EMMC,
	DEVICE_MAX
};

#define PLL_BYPASS              0x00
#define AUTO_SCAN               0x01
#define SDCARD_ISP              0x05 //new
#define SPI_NAND_BOOT           0x09 //new
#define UART_ISP                0x0D //new
#define SPI_NOR_X2_BOOT         0x11 //new, debug
#define SPI_NAND_X2_BOOT        0x15 //new, debug
#define USB_ISP                 0x19
#define TEST_BYPASS             0x1C
#define NAND_LARGE_BOOT         0x1D

// sw boot mode only
#define EMMC_BOOT               0xA3
#define CARD0_ISP               0xA5
#define CARD1_ISP               0xA6
#define CARD2_ISP               0xA7
#define CARD3_ISP               0xA8
#define SPI_NOR_BOOT            0x1F
#define USB_MSDC_BOOT           0x2F
#define NAND_BOOT               0x26
#define SPINAND_BOOT            0x46

// Get boot info
#define CONFIG_SRAM_BASE        0x9e800000
#define SP_GET_BOOTINFO()      ((struct sp_bootinfo *)(CONFIG_SRAM_BASE + 63*1024))

#define SP_IS_ISPBOOT()       (SP_GET_BOOTINFO()->gbootRom_boot_mode == USB_ISP || \
			       SP_GET_BOOTINFO()->gbootRom_boot_mode == SDCARD_ISP || \
			       SP_GET_BOOTINFO()->gbootRom_boot_mode == UART_ISP || \
			       SP_GET_BOOTINFO()->gbootRom_boot_mode == USB_MSDC_BOOT)
#endif
