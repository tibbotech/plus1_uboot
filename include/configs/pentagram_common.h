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

/*
 * TODO: for code development only, should be changed to NAND/eMMC.
 */
/*
#define CONFIG_ENV_IS_NOWHERE		1
#define CONFIG_ENV_SIZE			(64 << 10)
*/
#define CONFIG_ENV_IS_IN_NVRAM
#define CONFIG_ENV_ADDR			CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_ENV_SIZE			(64 << 10)

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

#if defined(CONFIG_SYS_ENV_8388)

#if 0 /* if open this, need to mark VMLINUX in ipack/update_all.sh */

#define CONFIG_BOOTCOMMAND \
"echo [scr] bootcmd started; " \
"md.l ${bootinfo_base} 1; " \
"if itest.l *${bootinfo_base} == " __stringify(SPI_NOR_BOOT) "; then " \
	"echo [scr] romter boot; " \
	"run romter_boot; " \
"elif itest.l *${bootinfo_base} == " __stringify(EMMC_BOOT) "; then " \
	"echo [scr] emmc boot; " \
	"run emmc_boot; " \
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
"be2le=md ${tmpaddr} 1; " \
	"setexpr byte *${tmpaddr} '&' 0x000000ff; " \
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
	"setexpr sz_dtb ${tmpval} + 0x28; setexpr sz_dtb ${sz_dtb} / 4; " \
	"cp.l ${addr_src_dtb} ${addr_dst_dtb} ${sz_dtb}; " \
	"cp.b ${addr_src_kernel} ${addr_tmp_header} 0x40; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	"setexpr sz_kernel ${tmpval} + 0x40; setexpr sz_kernel ${sz_kernel} / 4; " \
	"cp.l ${addr_src_kernel} ${addr_dst_kernel} ${sz_kernel}; " \
	"bootm ${addr_dst_kernel} - ${addr_dst_dtb}\0" \
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
	"bootm ${addr_dst_kernel} - ${addr_dst_dtb}\0"
#else
#define CONFIG_BOOTCOMMAND      "echo bootcmd started ; sp_preboot dump ; sp_preboot ; printenv ; \
echo [cmd] cp.l 0x98600000 0x307FC0 0x280000 ; \
cp.l 0x98600000 0x307FC0 0x280000 ; \
echo [cmd] cp.l 0x98020000 0x2FFFC0 0x400 ; \
cp.l 0x98020000 0x2FFFC0 0x400 ; \
sp_go 0x308000 0x300000"
#endif /* if 0 */

#elif defined(CONFIG_SYS_ENV_ZEBU)
#if defined (CONFIG_SD_BOOT)
#define CONFIG_BOOTCOMMAND      "echo [scr] emmc bootcmd started ; \
mmc rescan ; mmc part ; \
mmc read 0x2fffc0 0x1422 0x1 ; md 0x2fffc0 0x60 ; \
mmc read 0x307fc0 0x1822 0x1 ; md 0x307fc0 0x60 ; \
mmc read 0x2fffc0 0x1422 0xa ; mmc read 0x307fc0 0x1822 0x30f0 ; sp_go 0x308000 0x300000"
#elif defined (CONFIG_NAND_BOOT)
#else /* zmem */
#define CONFIG_BOOTCOMMAND      "echo [scr] zmem bootcmd started ; sp_go 0x308000 0x300040"
#endif

#endif /* CONFIG_SYS_ENV_ */

/* MMC related configs */
#define CONFIG_SUPPORT_EMMC_BOOT 
/* #define CONFIG_MMC_TRACE */

#endif /* __CONFIG_PENTAGRAM_H */
