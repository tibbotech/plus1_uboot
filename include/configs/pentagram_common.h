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
#define CONFIG_SMP_PEN_ADDR	(0x9ea7fff4)
#define A_SYS_COUNTER		(0x9ed0a000)
/* ... */
#elif defined(CONFIG_TARGET_PENTAGRAM_B_BOOT)
/* ... */
#else
#error "No board configuration is defined"
#endif

#define CONFIG_CLOCKS

/* Disable some options which is enabled by default: */
#undef CONFIG_CMD_IMLS

#define CONFIG_SYS_SDRAM_BASE		0
#if defined(CONFIG_SYS_ENV_ZEBU)
#define CONFIG_SYS_SDRAM_SIZE           (64 << 20)
#else /* normal SP7021 evb environment can have larger DRAM size */
#define CONFIG_SYS_SDRAM_SIZE           (512 << 20)
#endif
#define CONFIG_SYS_MALLOC_LEN           (6 << 20)

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

/* Main storage selection */
#if (SPINOR == 1) || (NOR_JFFS2 == 1)
#define SP_MAIN_STORAGE			"nor"
#elif defined(CONFIG_SP_SPINAND)
#define SP_MAIN_STORAGE			"nand"
#elif defined(CONFIG_MMC_SP_EMMC)
#define SP_MAIN_STORAGE			"emmc"
#elif defined(BOOT_KERNEL_FROM_TFTP)
#define SP_MAIN_STORAGE			"tftp"
#else
#define SP_MAIN_STORAGE			"none"
#endif

/* u-boot env parameter */
#define CONFIG_SYS_MMC_ENV_DEV		0


#define CONFIG_SYS_MAXARGS		32
#define CONFIG_SYS_CBSIZE		(2 << 10)
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_ARCH_MISC_INIT
#define CONFIG_SYS_HZ			1000

#include <asm/arch/sp_bootmode_bitmap_sc7xxx.h>

#undef DBG_SCR
#ifdef DBG_SCR
#define dbg_scr(s) s
#else
#define dbg_scr(s) ""
#endif


#ifdef CONFIG_SP_SPINAND
//#define CONFIG_MTD_PARTITIONS
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_NAND_BASE		0x9c002b80
//#define CONFIG_MTD_DEVICE		/* needed for mtdparts cmd */
#define MTDIDS_DEFAULT			"nand0=sp_spinand.0"
#define MTDPARTS_DEFAULT		"mtdparts=sp_spinand.0:-(whole_nand)"
#endif

/* TFTP server IP and board MAC address settings for TFTP ISP.
 * You should modify BOARD_MAC_ADDR to the address which you are assigned to */
#if !defined(BOOT_KERNEL_FROM_TFTP)
#define TFTP_SERVER_IP			172.18.12.62
#define BOARD_MAC_ADDR			00:22:60:00:88:20
#define USER_NAME			_username
#endif

#ifdef CONFIG_BOOTARGS_WITH_MEM
#define DEFAULT_BOOTARGS		"console=ttyS0,115200 root=/dev/ram rw loglevel=8 user_debug=255 earlyprintk"
#endif

#define RASPBIAN_CMD                    // Enable Raspbian command


/*
 * In the beginning, bootcmd will check bootmode in SRAM and the flag
 * if_zebu to choose different boot flow :
 * romter_boot
 *      kernel is in SPI nor offset 6M, and dtb is in offset 128K.
 *      kernel will be loaded to 0x307FC0 and dtb will be loaded to 0x2FFFC0.
 *      Then bootm 0x307FC0 - 0x2FFFC0.
 * qk_romter_boot
 *      kernel is in SPI nor offset 6M, and dtb is in offset 128K(0x20000).
 *      kernel will be loaded to 0x307FC0 and dtb will be loaded to 0x2FFFC0.
 *      Then sp_go 0x307FC0 0x2FFFC0.
 * emmc_boot
 *      kernel is stored in emmc LBA CONFIG_SRCADDR_KERNEL and dtb is
 *      stored in emmc LBA CONFIG_SRCADDR_DTB.
 *      kernel will be loaded to 0x307FC0 and dtb will be loaded to 0x2FFFC0.
 *      Then bootm 0x307FC0 - 0x2FFFC0.
 * qk_emmc_boot
 *      kernel is stored in emmc LBA CONFIG_SRCADDR_KERNEL and dtb is
 *      stored in emmc LBA CONFIG_SRCADDR_DTB.
 *      kernel will be loaded to 0x307FC0 and dtb will be loaded to 0x2FFFC0.
 *      Then sp_go 0x307FC0 - 0x2FFFC0.
 * zmem_boot / qk_zmem_boot
 *      kernel is preloaded to 0x307FC0 and dtb is preloaded to 0x2FFFC0.
 *      Then sp_go 0x307FC0 0x2FFFC0.
 * zebu_emmc_boot
 *      kernel is stored in emmc LBA CONFIG_SRCADDR_KERNEL and dtb is
 *      stored in emmc LBA CONFIG_SRCADDR_DTB.
 *      kernel will be loaded to 0x307FC0 and dtb will be loaded to 0x2FFFC0.
 *      Then sp_go 0x307FC0 0x2FFFC0.
 *
 * About "sp_go"
 * Earlier, sp_go do not handle header so you should pass addr w/o header.
 * But now sp_go DO verify magic/hcrc/dcrc in the quick sunplus uIamge header.
 * So the address passed for sp_go must have header in it.
 */
#define CONFIG_BOOTCOMMAND \
"echo [scr] bootcmd started; " \
"md.l ${bootinfo_base} 1; " \
"if itest.l *${bootinfo_base} == " __stringify(SPI_NOR_BOOT) "; then " \
	"if itest ${if_zebu} == 1; then " \
		"if itest ${if_qkboot} == 1; then " \
			"echo [scr] qk zmem boot; " \
			"run qk_zmem_boot; " \
		"else " \
			"echo [scr] zmem boot; " \
			"run zmem_boot; " \
		"fi; " \
	"else " \
		"if itest ${if_qkboot} == 1; then " \
			"echo [scr] qk romter boot; " \
			"run qk_romter_boot; " \
		"elif itest.s ${sp_main_storage} == tftp; then " \
			"echo [scr] tftp_boot; " \
			"run tftp_boot; " \
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
"elif itest.l *${bootinfo_base} == " __stringify(SPINAND_BOOT) "; then " \
	"echo [scr] nand boot; " \
	"run nand_boot; " \
"elif itest.l *${bootinfo_base} == " __stringify(USB_ISP) "; then " \
	"echo [scr] ISP from USB storage; " \
	"run isp_usb; " \
"elif itest.l *${bootinfo_base} == " __stringify(SDCARD_ISP) "; then " \
	"echo [scr] ISP from SD Card; " \
	"run isp_sdcard; " \
"else " \
	"echo Stop; " \
"fi"

#define DSTADDR_KERNEL		0x307FC0 /* if stext is on 0x308000 */
#define DSTADDR_DTB		0x2FFFC0
#define TMPADDR_HEADER		0x800000
#define DSTADDR_ROOTFS		0x13FFFC0

#define XBOOT_SIZE		0x10000	/* for sdcard .ISPBOOOT.BIN size is equal to xboot.img size, do boot.otherwise do ISP*/

//#define SUPPROT_NFS_ROOTFS
#ifdef SUPPROT_NFS_ROOTFS
#define USE_NFS_ROOTFS		1
#define NFS_ROOTFS_DIR		"/home/rootfsdir"
#define NFS_ROOTFS_SERVER_IP	172.28.114.216
#define NFS_ROOTFS_CLINT_IP	172.28.114.7
#define NFS_ROOTFS_GATEWAY_IP	172.28.114.1
#define NFS_ROOTFS_NETMASK	255.255.255.0
#endif

#if !defined(CONFIG_MMC_SP_EMMC)
#define SDCARD_DEVICE_ID	0       // 0: SDC (No eMMC, only SD card)
#else
#define SDCARD_DEVICE_ID	1       // 0: eMMC, 1: SDC
#endif

#ifdef RASPBIAN_CMD
#define RASPBIAN_INIT		"setenv filesize 0; " \
				"fatsize $isp_if $isp_dev /cmdline.txt; " \
				"if test $filesize != 0; then " \
				"	fatload $isp_if $isp_dev $addr_dst_dtb /cmdline.txt; " \
				"	raspb init $fileaddr $filesize; " \
				"	echo new bootargs; " \
				"	echo $bootargs; " \
				"fi; "
#else
#define RASPBIAN_INIT		""
#endif


#define SDCARD_EXT_CMD \
	"scriptaddr=0x1000000; " \
	"bootscr=boot.scr; " \
	"bootenv=uEnv.txt; " \
	"if run loadbootenv; then " \
		"echo Loaded environment from ${bootenv}; " \
		"env import -t ${scriptaddr} ${filesize}; " \
	"fi; " \
	"if test -n ${uenvcmd}; then " \
		"echo Running uenvcmd ...; " \
		"run uenvcmd; " \
	"fi; " \
	"if run loadbootscr; then " \
		"echo Jumping to ${bootscr}; " \
		"source ${scriptaddr}; "\
	"fi; "

#if (NOR_JFFS2 == 1)
#define NOR_LOAD_KERNEL \
	dbg_scr("echo kernel from ${addr_src_kernel} to ${addr_dst_kernel} sz ${sz_kernel}; ") \
	"setexpr kernel_off ${addr_src_kernel} - 0x98000000; " \
	"sf probe 0 50000000; " \
	"sf read ${addr_dst_kernel} ${kernel_off} ${sz_kernel}; " \
	"setexpr sz_kernel ${sz_kernel} + 0xffff; " \
	"setexpr sz_kernel ${sz_kernel} / 0x10000; " \
	"setexpr sz_kernel ${sz_kernel} * 0x10000; " \
	"setenv bootargs ${b_c} root=/dev/mtdblock6 rw rootfstype=jffs2 user_debug=255 rootwait " \
	"mtdparts=9c000b00.spinor:64k@0(iboot)ro,64k(xboot)ro,128k(dtb),768k(uboot),1m(reserve),0x${sz_kernel}(kernel),-(rootfs); "
#else
#define NOR_LOAD_KERNEL \
	"setexpr sz_kernel ${sz_kernel} + 3; setexpr sz_kernel ${sz_kernel} / 4; " \
	dbg_scr("echo kernel from ${addr_src_kernel} to ${addr_dst_kernel} sz ${sz_kernel}; ") \
	"cp.l ${addr_src_kernel} ${addr_dst_kernel} ${sz_kernel}; "
#endif

#ifdef CONFIG_PENTAGRAM_SEPDTS
#define DTS_LOAD_ADDR "${addr_dst_dtb}"
#define DTS_LOAD_EMMC \
	"mmc read ${addr_tmp_header} ${addr_src_dtb} 0x1; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x4; run be2le; " \
	"setexpr sz_dtb ${tmpval} + 0x28; " \
	"setexpr sz_dtb ${sz_dtb} + 0x200; setexpr sz_dtb ${sz_dtb} / 0x200; " \
	"mmc read ${addr_dst_dtb} ${addr_src_dtb} ${sz_dtb}; "
#if 0
#define DTS_LOAD_NAND \
	"nand read ${addr_tmp_header} dtb 0x40; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x4; run be2le; " \
	"setexpr sz_dtb ${tmpval} + 0x28; " \
	"setexpr sz_dtb ${sz_dtb} + 0x200; setexpr sz_dtb ${sz_dtb} / 0x200; " \
	"nand read ${addr_dst_dtb} dtb ${sz_dtb}; "
#else
#define DTS_LOAD_NAND "nand read ${addr_dst_dtb} dtb; "
#endif
#else
#define DTS_LOAD_ADDR "${fdtcontroladdr}"
#define DTS_LOAD_EMMC
#define DTS_LOAD_NAND
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
"sz_sign=0x100\0" \
"b_c=console=tty1 console=ttyS0,115200 earlyprintk\0" \
"emmc_root=root=/dev/mmcblk0p8 rw rootwait\0" \
"stdin=" STDIN_CFG "\0" \
"stdout=" STDOUT_CFG "\0" \
"stderr=" STDOUT_CFG "\0" \
"bootinfo_base="		__stringify(SP_BOOTINFO_BASE) "\0" \
"addr_src_kernel="		__stringify(CONFIG_SRCADDR_KERNEL) "\0" \
"addr_src_dtb="			__stringify(CONFIG_SRCADDR_DTB) "\0" \
"addr_dst_kernel="		__stringify(DSTADDR_KERNEL) "\0" \
"addr_dst_dtb="			__stringify(DSTADDR_DTB) "\0" \
"addr_tmp_header="		__stringify(TMPADDR_HEADER) "\0" \
"nfs_gatewayip="		__stringify(NFS_ROOTFS_GATEWAY_IP) "\0" \
"nfs_netmask="			__stringify(NFS_ROOTFS_NETMASK) "\0" \
"nfs_clintip=" 			__stringify(NFS_ROOTFS_CLINT_IP) "\0" \
"nfs_serverip="			__stringify(NFS_ROOTFS_SERVER_IP) "\0" \
"nfs_rootfs_dir="		__stringify(NFS_ROOTFS_DIR) "\0" \
"if_use_nfs_rootfs="		__stringify(USE_NFS_ROOTFS) "\0" \
"if_zebu="			__stringify(CONFIG_SYS_ENV_ZEBU) "\0" \
"if_qkboot="			__stringify(CONFIG_SYS_USE_QKBOOT_HEADER) "\0" \
"sp_main_storage="		SP_MAIN_STORAGE "\0" \
"serverip=" 			__stringify(TFTP_SERVER_IP) "\0" \
"macaddr="			__stringify(BOARD_MAC_ADDR) "\0" \
"sdcard_devid="			__stringify(SDCARD_DEVICE_ID) "\0" \
"loadbootscr=fatload ${isp_if} ${isp_dev}:1 ${scriptaddr} /${bootscr} || " \
	"fatload ${isp_if} ${isp_dev}:1 ${scriptaddr} /boot/${bootscr} || " \
	"fatload ${isp_if} ${isp_dev}:1 ${scriptaddr} /sunplus/sp7021/${bootscr}; " \
	"\0" \
"loadbootenv=fatload ${isp_if} ${isp_dev}:1 ${scriptaddr} /${bootenv} || " \
	"fatload ${isp_if} ${isp_dev}:1 ${scriptaddr} /boot/${bootenv} || " \
	"fatload ${isp_if} ${isp_dev}:1 ${scriptaddr} /sunplus/sp7021/${bootenv}; " \
	"\0" \
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
"romter_boot=cp.b ${addr_src_kernel} ${addr_tmp_header} 0x40; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	dbg_scr("md ${addr_tmp_header} 0x10; printenv tmpval; ") \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + ${sz_sign}; " \
	"echo loading kernel ...; "\
	"sp_wdt_set;" \
	NOR_LOAD_KERNEL \
	dbg_scr("echo bootm ${addr_dst_kernel} - ${fdtcontroladdr}; ") \
	"run boot_kernel \0" \
"qk_romter_boot=cp.b ${addr_src_kernel} ${addr_tmp_header} 0x40; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	dbg_scr("md ${addr_tmp_header} 0x10; printenv tmpval; ") \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel  ${sz_kernel} + 4; setexpr sz_kernel ${sz_kernel} / 4; " \
	dbg_scr("echo kernel from ${addr_src_kernel} to ${addr_dst_kernel} sz ${sz_kernel}; ") \
	"cp.l ${addr_src_kernel} ${addr_dst_kernel} ${sz_kernel}; " \
	dbg_scr("echo sp_go ${addr_dst_kernel} ${fdtcontroladdr}; ") \
	"sp_go ${addr_dst_kernel} ${fdtcontroladdr}\0" \
"emmc_boot=sp_wdt_set;" \
	DTS_LOAD_EMMC \
	"mmc read ${addr_tmp_header} ${addr_src_kernel} 0x1; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + ${sz_sign}; " \
	"setexpr sz_kernel ${sz_kernel} + 0x200; setexpr sz_kernel ${sz_kernel} / 0x200; " \
	"mmc read ${addr_dst_kernel} ${addr_src_kernel} ${sz_kernel}; " \
	"setenv bootargs ${b_c} ${emmc_root} ${args_emmc} ${args_kern}; " \
	"run boot_kernel \0" \
"qk_emmc_boot=mmc read ${addr_tmp_header} ${addr_src_kernel} 0x1; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + 0x200; setexpr sz_kernel ${sz_kernel} / 0x200; " \
	"mmc read ${addr_dst_kernel} ${addr_src_kernel} ${sz_kernel}; " \
	"sp_go ${addr_dst_kernel} ${fdtcontroladdr}\0" \
"nand_boot=sp_wdt_set;" \
	DTS_LOAD_NAND \
	"nand read ${addr_tmp_header} kernel 0x40; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	dbg_scr("md ${addr_tmp_header} 0x10; printenv tmpval; ") \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + ${sz_sign}; " \
	dbg_scr("echo from kernel partition to ${addr_dst_kernel} sz ${sz_kernel}; ") \
	"nand read ${addr_dst_kernel} kernel ${sz_kernel}; " \
	"setenv bootargs ${b_c} root=ubi0:rootfs rw ubi.mtd=9,2048 rootflags=sync rootfstype=ubifs mtdparts=${mtdparts} user_debug=255 rootwait; " \
	"run boot_kernel \0" \
"boot_kernel="\
	"if itest ${if_use_nfs_rootfs} == 1; then " \
		"setenv bootargs ${b_c} root=/dev/nfs nfsroot=${nfs_serverip}:${nfs_rootfs_dir} ip=${nfs_clintip}:${nfs_serverip}:${nfs_gatewayip}:${nfs_netmask}::eth0:off rdinit=/linuxrc noinitrd rw; "\
	"fi; " \
	"verify ${addr_dst_kernel}; " \
	"bootm ${addr_dst_kernel} - " DTS_LOAD_ADDR "; " \
	"\0" \
"qk_zmem_boot=sp_go ${addr_dst_kernel} ${fdtcontroladdr}\0" \
"zmem_boot=bootm ${addr_dst_kernel} - ${fdtcontroladdr}\0" \
"zebu_emmc_boot=mmc rescan; mmc part; " \
	"mmc read ${addr_tmp_header} ${addr_src_kernel} 0x1; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + ${sz_sign}; " \
	"setexpr sz_kernel ${sz_kernel} + 0x200; setexpr sz_kernel ${sz_kernel} / 0x200; " \
	"mmc read ${addr_dst_kernel} ${addr_src_kernel} ${sz_kernel}; " \
	"sp_go ${addr_dst_kernel} ${fdtcontroladdr}\0" \
"tftp_boot=setenv ethaddr ${macaddr} && printenv ethaddr; " \
	"printenv serverip; " \
	"setenv filesize 0; " \
	"dhcp ${addr_dst_dtb} ${serverip}:dtb" __stringify(USER_NAME) " && " \
	"dhcp ${addr_dst_kernel} ${serverip}:uImage" __stringify(USER_NAME) "; " \
	"if test $? != 0; then " \
	"	echo Error occurred while getting images from tftp server!; " \
	"	exit; " \
	"fi; " \
	"verify ${addr_dst_kernel}; " \
	"bootm ${addr_dst_kernel} - ${addr_dst_dtb}; " \
	"\0" \
"isp_usb=setenv isp_if usb && setenv isp_dev 0; " \
	"$isp_if start; " \
	"run isp_common; " \
	"\0" \
"isp_sdcard=setenv isp_if mmc && setenv isp_dev $sdcard_devid; " \
	"sp_wdt_set;" \
	"mmc list; " \
	"fatsize $isp_if $isp_dev /ISPBOOOT.BIN; " \
	"echo ISPBOOOT filesize = $filesize; "\
	"if itest.l ${filesize} == " __stringify(XBOOT_SIZE) ";  then " \
		"echo run sdcard_boot; "\
		SDCARD_EXT_CMD \
	"else "\
		"echo run isp_common; "\
		"run isp_common; " \
	"fi" \
	"\0" \
"isp_common=setenv isp_ram_addr 0x1000000; " \
	RASPBIAN_INIT \
	"fatload $isp_if $isp_dev $isp_ram_addr /ISPBOOOT.BIN 0x800 0x100000; " \
	"setenv isp_main_storage ${sp_main_storage} && printenv isp_main_storage; " \
	"setexpr script_addr $isp_ram_addr + 0x20 && setenv script_addr 0x${script_addr} && source $script_addr; " \
	"\0" \
"update_usb=setenv isp_if usb && setenv isp_dev 0; " \
	"$isp_if start; " \
	"run update_common; " \
	"\0" \
"update_sdcard=setenv isp_if mmc && setenv isp_dev 1; " \
	"mmc list; " \
	"run update_common; " \
	"\0" \
"update_common=setenv isp_ram_addr 0x1000000; " \
	"setenv isp_update_file_name ISP_UPDT.BIN; " \
	"fatload $isp_if $isp_dev $isp_ram_addr /$isp_update_file_name 0x800; " \
	"setenv isp_main_storage ${sp_main_storage} && printenv isp_main_storage; " \
	"setenv isp_image_header_offset 0; " \
	"setexpr script_addr $isp_ram_addr + 0x20 && setenv script_addr 0x${script_addr} && source $script_addr; " \
	"\0" \
"update_tftp=setenv isp_ram_addr 0x1000000; " \
	"setenv ethaddr ${macaddr} && printenv ethaddr; " \
	"printenv serverip; " \
	"dhcp $isp_ram_addr $serverip:TFTP0000.BIN; " \
	"setenv isp_main_storage ${sp_main_storage} && printenv isp_main_storage; " \
	"setexpr script_addr $isp_ram_addr + 0x00 && setenv script_addr 0x${script_addr} && source $script_addr; " \
	"\0"

/* MMC related configs */
#define CONFIG_SUPPORT_EMMC_BOOT
/* #define CONFIG_MMC_TRACE */

#define CONFIG_ENV_OVERWRITE    /* Allow to overwrite ethaddr and serial */

#if !defined(CONFIG_SP_SPINAND)
#define SPEED_UP_SPI_NOR_CLK    /* Set CLK based on flash id */
#endif

#ifdef CONFIG_DM_VIDEO
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE (2<<20)
#define CONFIG_BMP_16BPP
#define CONFIG_BMP_24BPP
#define CONFIG_BMP_32BPP
#ifdef CONFIG_DM_VIDEO_SP7021_LOGO
#define STDOUT_CFG "serial"
#else
#define STDOUT_CFG "vidconsole,serial"
#endif
#else
#define STDOUT_CFG "serial"
#endif

#ifdef CONFIG_USB_OHCI_HCD
/* USB Config */
#define CONFIG_USB_OHCI_NEW			1
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#endif

#ifdef CONFIG_USB_KEYBOARD
#define STDIN_CFG "usbkbd,serial"
//#define CONFIG_PREBOOT "usb start"
#undef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY 1
#else
#define STDIN_CFG "serial"
#endif

#endif /* __CONFIG_PENTAGRAM_H */
