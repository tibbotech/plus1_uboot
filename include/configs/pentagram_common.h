/*
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Note:
 *	Do NOT use "//" for comment, it will cause issue in u-boot.lds
 */

#ifndef __CONFIG_PENTAGRAM_H
#define __CONFIG_PENTAGRAM_H

/* define board-specific options/flags here, e.g. memory size, ... */
#if   defined(CONFIG_TARGET_PENTAGRAM_COMMON)
/* ... */
#elif defined(CONFIG_TARGET_PENTAGRAM_B_BOOT)
/* ... */
#else
#error "No board configuration is defined"
#endif

/* Disable some options which is enabled by default: */
#undef CONFIG_CMD_IMLS

#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0
#if defined(CONFIG_SYS_ENV_ZEBU)
#define CONFIG_SYS_SDRAM_SIZE           (64 << 20)
#else
#define CONFIG_SYS_SDRAM_SIZE		(256 << 20)
#endif
#define CONFIG_SYS_MALLOC_LEN		(6 << 20)

#ifndef CONFIG_SYS_TEXT_BASE		/* where U-Boot is loaded by xBoot */
/* It is defined in arch/arm/mach-pentagram/Kconfig */
#error "CONFIG_SYS_TEXT_BASE not defined"
#else
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(512 << 10)
#endif /* CONFIG_SYS_TEXT_BASE */

#ifdef CONFIG_SPL_BUILD
#ifndef CONFIG_SYS_UBOOT_START		/* default entry point */
#define CONFIG_SYS_UBOOT_START		CONFIG_SYS_TEXT_BASE
#endif
#endif

#define CONFIG_SYS_INIT_SP_ADDR		(1 << 20)	/* set it in DRAM area (not SRAM) because DRAM is ready before U-Boot executed */
#define CONFIG_SYS_LOAD_ADDR		(4 << 20)	/* kernel loaded address */

#ifndef CONFIG_BAUDRATE
#define CONFIG_BAUDRATE			115200		/* the value doesn't matter, it's not change in U-Boot */
#endif
#define CONFIG_SYS_BAUDRATE_TABLE	{ 57600, 115200 }
/* #define CONFIG_SUNPLUS_SERIAL */

/* u-boot env parameter */
#define CONFIG_SYS_MMC_ENV_DEV	0
#define CONFIG_ENV_SIZE		0x2000
#if defined(CONFIG_SYS_ENV_8388)
#define CONFIG_ENV_OFFSET	0x087E4400	/* LBA 0x00043f22 */
#else
#define CONFIG_ENV_OFFSET	0x00B04400	/* LBA 0x00005822 */
#endif

#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_MAXARGS		32
#define CONFIG_SYS_CBSIZE		(2 << 10)
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_ARCH_MISC_INIT
#define CONFIG_SYS_HZ			1000

#if defined(CONFIG_SYS_ENV_8388)
#include <asm/arch/sp_bootmode_bitmap_8388.h>
#else
#include <asm/arch/sp_bootmode_bitmap_sc7xxx.h>
#endif

#undef DBG_SCR
#ifdef DBG_SCR
#define dbg_scr(s) s
#else
#define dbg_scr(s) ""
#endif

#ifdef CONFIG_SP_SPINAND
#define CONFIG_SYS_MAX_NAND_DEVICE   1
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_NAND_BASE    0x9c000ae0
#endif
/*
 * In the beginning, bootcmd will check bootmode in SRAM and the flag
 * if_zebu to choose different boot flow :
 * romter_boot
 *      kernel is in SPI nor offset 6M, and dtb is in offset 128K.
 *      kernel will be loaded to 0x307FC0 and dtb will be loaded to 0x2FFFC0.
 *      Then bootm 0x307FC0 - 0x2FFFC0.
 * zmem_boot 
 *      kernel is preloaded to 0x307FC0 and dtb is preloaded to 0x2FFFC0.
 *      Then sp_go 0x308000 0x300000.
 *      sp_go do not handle header so use 0x300080 to skip header and dtb have
 *      additional header so real dtb is on 0x300000.
 * emmc_boot
 *      kernel is stored in emmc LBA CONFIG_SRCADDR_KERNEL and dtb is
 *      stored in emmc LBA CONFIG_SRCADDR_DTB. 
 *      kernel will be loaded to 0x307FC0 and dtb will be loaded to 0x2FFFC0.
 *      Then bootm 0x307FC0 - 0x2FFFC0. 
 * zebu_emmc_boot
 *      kernel is stored in emmc LBA CONFIG_SRCADDR_KERNEL and dtb is
 *      stored in emmc LBA CONFIG_SRCADDR_DTB.
 *	kernel will be loaded to 0x307FC0 and dtb will be loaded to 0x2FFFC0.
 *	Then sp_go 0x308000 0x300000.
 *	sp_go do not handle header so use 0x300080 to skip header and dtb have
 *      additional header so real dtb is on 0x300000.
 */
#define CONFIG_BOOTCOMMAND \
"echo [scr] bootcmd started; " \
"md.l ${bootinfo_base} 1; " \
"if itest.l *${bootinfo_base} == " __stringify(SPI_NOR_BOOT) "; then " \
	"if itest ${if_zebu} == 1; then " \
		"echo [scr] zmem boot; " \
		"run zmem_boot; " \
	"else " \
		"if itest ${if_qkboot} == 1; then " \
			"echo [scr] qk romter boot; " \
			"run qk_romter_boot; " \
		"else " \
			"echo [scr] romter boot; " \
			"run romter_boot; " \
		"fi; " \
	"fi; " \
"elif itest.l *${bootinfo_base} == " __stringify(EMMC_BOOT) "; then " \
	"if itest ${if_zebu} == 1; then " \
		"echo [scr] zebu emmc boot; " \
		"run zebu_emmc_boot; " \
	"else " \
		"if itest ${if_qkboot} == 1; then " \
			"echo [scr] qk emmc boot; " \
			"run qk_emmc_boot; " \
		"else " \
			"echo [scr] emmc boot; " \
			"run emmc_boot; " \
		"fi; " \
	"fi; " \
"elif itest.l *${bootinfo_base} == " __stringify(NAND_BOOT) "; then " \
	"echo [scr] nand boot; " \
	"run nand_boot; " \
"fi"

#define DSTADDR_KERNEL		0x307FC0 /* if stext is on 0x308000 */
#define DSTADDR_DTB		0x2FFFC0
#define TMPADDR_HEADER		0x800000

#define CONFIG_EXTRA_ENV_SETTINGS \
"bootinfo_base="		__stringify(SP_BOOTINFO_BASE) "\0" \
"addr_src_kernel="		__stringify(CONFIG_SRCADDR_KERNEL) "\0" \
"addr_src_dtb="			__stringify(CONFIG_SRCADDR_DTB) "\0" \
"addr_dst_kernel="		__stringify(DSTADDR_KERNEL) "\0" \
"addr_dst_dtb="			__stringify(DSTADDR_DTB) "\0" \
"addr_tmp_header="		__stringify(TMPADDR_HEADER) "\0" \
"if_zebu="			__stringify(CONFIG_SYS_ENV_ZEBU) "\0" \
"if_qkboot="			__stringify(CONFIG_SYS_USE_QKBOOT_HEADER) "\0" \
"be2le=setexpr byte *${tmpaddr} '&' 0x000000ff; " \
	"setexpr tmpval $tmpval + $byte; " \
	"setexpr tmpval $tmpval * 0x100; " \
	"setexpr byte *${tmpaddr} '&' 0x0000ff00; " \
	"setexpr byte ${byte} / 0x100; " \
	"setexpr tmpval $tmpval + $byte; " \
	"setexpr tmpval $tmpval * 0x100; " \
	"setexpr byte *${tmpaddr} '&' 0x00ff0000; " \
	"setexpr byte ${byte} / 0x10000; " \
	"setexpr tmpval $tmpval + $byte; " \
	"setexpr tmpval $tmpval * 0x100; " \
	"setexpr byte *${tmpaddr} '&' 0xff000000; " \
	"setexpr byte ${byte} / 0x1000000; " \
	"setexpr tmpval $tmpval + $byte;\0" \
"romter_boot=cp.b ${addr_src_dtb} ${addr_tmp_header} 0x28; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x4; run be2le; " \
	dbg_scr("md ${addr_tmp_header} 0x10; printenv tmpval; ") \
	"setexpr sz_dtb ${tmpval} + 0x28; " \
	"setexpr sz_dtb ${sz_dtb} + 4; setexpr sz_dtb ${sz_dtb} / 4; " \
	dbg_scr("echo dtb from ${addr_src_dtb} to ${addr_dst_dtb} sz ${sz_dtb}; ") \
	"cp.l ${addr_src_dtb} ${addr_dst_dtb} ${sz_dtb}; " \
	"cp.b ${addr_src_kernel} ${addr_tmp_header} 0x40; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	dbg_scr("md ${addr_tmp_header} 0x10; printenv tmpval; ") \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + 4; setexpr sz_kernel ${sz_kernel} / 4; " \
	dbg_scr("echo kernel from ${addr_src_kernel} to ${addr_dst_kernel} sz ${sz_kernel}; ") \
	"cp.l ${addr_src_kernel} ${addr_dst_kernel} ${sz_kernel}; " \
	dbg_scr("echo bootm ${addr_dst_kernel} - ${addr_dst_dtb}; ") \
	"bootm ${addr_dst_kernel} - ${addr_dst_dtb}\0" \
"qk_romter_boot=cp.b ${addr_src_dtb} ${addr_tmp_header} 0x40; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	dbg_scr("md ${addr_tmp_header} 0x10; printenv tmpval; ") \
	"setexpr sz_dtb ${tmpval} + 0x40; " \
	"setexpr sz_dtb ${sz_dtb} + 4; setexpr sz_dtb ${sz_dtb} / 4; " \
	dbg_scr("echo dtb from ${addr_src_dtb} to ${addr_dst_dtb} sz ${sz_dtb}; ") \
	"cp.l ${addr_src_dtb} ${addr_dst_dtb} ${sz_dtb}; " \
	"cp.b ${addr_src_kernel} ${addr_tmp_header} 0x40; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	dbg_scr("md ${addr_tmp_header} 0x10; printenv tmpval; ") \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel  ${sz_kernel} + 4; setexpr sz_kernel ${sz_kernel} / 4; " \
	dbg_scr("echo kernel from ${addr_src_kernel} to ${addr_dst_kernel} sz ${sz_kernel}; ") \
	"cp.l ${addr_src_kernel} ${addr_dst_kernel} ${sz_kernel}; " \
	"setexpr addr_dst_kernel ${addr_dst_kernel} + 0x40; " \
	"setexpr addr_dst_dtb ${addr_dst_dtb} + 0x40; " \
	dbg_scr("sp_go ${addr_dst_kernel} ${addr_dst_dtb}; ") \
	"sp_go ${addr_dst_kernel} ${addr_dst_dtb}\0" \
"emmc_boot=mmc read ${addr_tmp_header} ${addr_src_dtb} 0x1; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x4; run be2le; " \
	"setexpr sz_dtb ${tmpval} + 0x28; " \
	"setexpr sz_dtb ${sz_dtb} + 0x200; setexpr sz_dtb ${sz_dtb} / 0x200; " \
	"mmc read ${addr_dst_dtb} ${addr_src_dtb} ${sz_dtb}; " \
	"mmc read ${addr_tmp_header} ${addr_src_kernel} 0x1; " \
	"setenv tmpval 0; setexpre tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + 0x200; setexpr sz_kernel ${sz_kernel} / 0x200; " \
	"mmc read ${addr_dst_kernel} ${addr_src_kernel} ${sz_kernel}; " \
	"bootm ${addr_dst_kernel} - ${addr_dst_dtb}\0" \
"qk_emmc_boot=mmc read ${addr_tmp_header} ${addr_src_dtb} 0x1; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	"setexpr sz_dtb ${tmpval} + 0x40; " \
	"setexpr sz_dtb ${sz_dtb} + 0x200; setexpr sz_dtb ${sz_dtb} / 0x200; " \
	"mmc read ${addr_dst_dtb} ${addr_src_dtb} ${sz_dtb}; " \
	"mmc read ${addr_tmp_header} ${addr_src_kernel} 0x1; " \
	"setenv tmpval 0; setexpre tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + 0x200; setexpr sz_kernel ${sz_kernel} / 0x200; " \
	"mmc read ${addr_dst_kernel} ${addr_src_kernel} ${sz_kernel}; " \
	"setexpr addr_dst_kernel ${addr_dst_kernel} + 0x40; " \
	"setexpr addr_dst_dtb ${addr_dst_dtb} + 0x40; " \
	"sp_go ${addr_dst_kernel} ${addr_dst_dtb}\0" \
"zmem_boot=setexpr addr_dst_kernel ${addr_dst_kernel} + 0x40; " \
	"setexpr addr_dst_dtb ${addr_dst_dtb} + 0x40; " \
	"sp_go ${addr_dst_kernel} ${addr_dst_dtb}\0" \
"zebu_emmc_boot=mmc rescan; mmc part; " \
	"mmc read ${addr_tmp_header} ${addr_src_dtb} 0x1; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	"setexpr sz_dtb ${tmpval} + 0x40; " \
	"setexpr sz_dtb ${sz_dtb} + 0x200; setexpr sz_dtb ${sz_dtb} / 0x200; " \
	"mmc read ${addr_dst_dtb} ${addr_src_dtb} ${sz_dtb}; " \
	"mmc read ${addr_tmp_header} ${addr_src_kernel} 0x1; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + 0x200; setexpr sz_kernel ${sz_kernel} / 0x200; " \
	"mmc read ${addr_dst_kernel} ${addr_src_kernel} ${sz_kernel}; " \
	"setexpr addr_dst_kernel ${addr_dst_kernel} + 0x40; " \
	"setexpr addr_dst_dtb ${addr_dst_dtb} + 0x40; " \
	"sp_go ${addr_dst_kernel} ${addr_dst_dtb}\0"
#if 0
/* romter test booting command */
#define CONFIG_BOOTCOMMAND      "echo bootcmd started ; sp_preboot dump ; sp_preboot ; printenv ; \
echo [cmd] cp.l 0x98600000 0x307FC0 0x280000 ; \
cp.l 0x98600000 0x307FC0 0x280000 ; \
echo [cmd] cp.l 0x98020000 0x2FFFC0 0x400 ; \
cp.l 0x98020000 0x2FFFC0 0x400 ; \
sp_go 0x308000 0x300000"

/* zebu emmc booting test command */
#define CONFIG_BOOTCOMMAND      "echo [scr] emmc bootcmd started ; \
mmc rescan ; mmc part ; \
mmc read 0x2fffc0 0x1422 0x1 ; md 0x2fffc0 0x60 ; \
mmc read 0x307fc0 0x1822 0x1 ; md 0x307fc0 0x60 ; \
mmc read 0x2fffc0 0x1422 0xa ; mmc read 0x307fc0 0x1822 0x30f0 ; sp_go 0x308000 0x300000"

/* zebu zmem booting test command */
#define CONFIG_BOOTCOMMAND      "echo [scr] zmem bootcmd started ; sp_go 0x308000 0x300040"
#endif

/* MMC related configs */
#define CONFIG_SUPPORT_EMMC_BOOT 
/* #define CONFIG_MMC_TRACE */

#endif /* __CONFIG_PENTAGRAM_H */
