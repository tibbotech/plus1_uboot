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

#if   defined(CONFIG_SYS_ENV_8388)
#define CONFIG_BOOTCOMMAND      "echo bootcmd started ; sp_preboot dump ; sp_preboot ; printenv ; \
echo [cmd] cp.b 0x98600000 0x307FC0 0xa00000 ; \
cp.b 0x98600000 0x307FC0 0xa00000 ; \
echo [cmd] cp.b 0x98020000 0x2FFFC0 0x1000 ; \
cp.b 0x98020000 0x2FFFC0 0x1000"

#else
#endif

#endif /* __CONFIG_PENTAGRAM_H */
