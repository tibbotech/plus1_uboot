/*
 * SPDX-License-Identifier: GPL-2.0+
 */

/*
 * Note:
 *	Do NOT use "//" for comment, it will cause issue in u-boot.lds
 */

#ifndef __CONFIG_PENTAGRAM_H
#define __CONFIG_PENTAGRAM_H

#define CONFIG_CLOCKS

/* Disable some options which is enabled by default: */
#undef CONFIG_CMD_IMLS

#define CONFIG_SYS_SDRAM_BASE		0
#if defined(CONFIG_SYS_ENV_ZEBU)
#define CONFIG_SYS_SDRAM_SIZE           (1024 << 20)
#elif defined(CONFIG_SYS_ENV_Q645_EVB)
#define CONFIG_SYS_SDRAM_SIZE           (1024 << 20)
#else /* normal SP7021 evb environment can have larger DRAM size */
#define CONFIG_SYS_SDRAM_SIZE           (1024 << 20)
#endif
#define CONFIG_SYS_MALLOC_LEN           (6 << 20)
#define CONFIG_SYS_BOOTM_LEN            (64 << 20)

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
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200, 921600 }
/* #define CONFIG_SUNPLUS_SERIAL */

/* Main storage selection */
#if (SPINOR == 1) || (NOR_JFFS2 == 1)
#define SP_MAIN_STORAGE			"nor"
#elif defined(CONFIG_SP_SPINAND_Q645)
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

#define COUNTER_FREQUENCY		(25*1000*1000)

#define CONFIG_SYS_MAXARGS		32
#define CONFIG_SYS_CBSIZE		(2 << 10)
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_ARCH_MISC_INIT
#define CONFIG_SYS_HZ			1000

#include <asm/arch/sp_bootmode_bitmap_q645.h>

#undef DBG_SCR
#ifdef DBG_SCR
#define dbg_scr(s) s
#else
#define dbg_scr(s) ""
#endif


#if (COMPILE_WITH_SECURE == 1)
#ifdef	CONFIG_SYS_ZMEM_SKIP_RELOC
#undef CONFIG_SYS_ZMEM_SKIP_RELOC
#endif
#endif

#ifdef CONFIG_SP_SPINAND_Q645
//#define CONFIG_MTD_PARTITIONS
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_NAND_BASE		0xf8002b80
//#define CONFIG_MTD_DEVICE		/* needed for mtdparts cmd */
#define MTDIDS_DEFAULT			"nand0=sp_spinand.0"
#if 0 // Set default mtdparts for zebu sim
#define MTDPARTS_DEFAULT		"sp_spinand.0:128k(nand_header),384k(xboot1),1408k(uboot1),2176k(uboot2),512k(env),512k(env_redund),1m(reserve),256k(dtb),25m(kernel),33536k(rootfs)"
#else
#define MTDPARTS_DEFAULT		"sp_spinand.0:128k(nand_header),384k(xboot1),1408k(uboot1),2176k(uboot2),512k(env),512k(env_redund),1m(reserve),256k(dtb),25m(kernel),230144k(rootfs)"
#endif
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
 *      kernel is in SPI nor offset 2M+128K, and dtb is in offset 256K.
 *      kernel will be loaded to 0x7FFFC0 and dtb will be loaded to 0x77FFC0.
 *      Then booti 0x7FFFC0 - 0x77FFC0.
 * qk_romter_boot
 *      kernel is in SPI nor offset 2M+128K, and dtb is in offset 256K.
 *      kernel will be loaded to 0x7FFFC0 and dtb will be loaded to 0x77FFC0.
 *      Then sp_go 0x7FFFC0 0x77FFC0.
 * emmc_boot
 *      kernel is stored in emmc LBA CONFIG_SRCADDR_KERNEL and dtb is
 *      stored in emmc LBA CONFIG_SRCADDR_DTB.
 *      kernel will be loaded to 0x7FFFC0 and dtb will be loaded to 0x77FFC0.
 *      Then booti 0x7FFFC0 - 0x77FFC0.
 * qk_emmc_boot
 *      kernel is stored in emmc LBA CONFIG_SRCADDR_KERNEL and dtb is
 *      stored in emmc LBA CONFIG_SRCADDR_DTB.
 *      kernel will be loaded to 0x7FFFC0 and dtb will be loaded to 0x77FFC0.
 *      Then sp_go 0x7FFFC0 - 0x77FFC0.
 * zmem_boot / qk_zmem_boot
 *      kernel is preloaded to 0x7FFFC0 and dtb is preloaded to 0x77FFC0.
 *      Then sp_go 0x7FFFC0 0x77FFC0.
 * zebu_emmc_boot
 *      kernel is stored in emmc LBA CONFIG_SRCADDR_KERNEL and dtb is
 *      stored in emmc LBA CONFIG_SRCADDR_DTB.
 *      kernel will be loaded to 0x7FFFC0 and dtb will be loaded to 0x77FFC0.
 *      Then sp_go 0x7FFFC0 0x77FFC0.
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
"elif itest.l *${bootinfo_base} == " __stringify(USB_BOOT) "; then " \
	"echo [scr] Boot from USB storage; " \
	"run usb_boot; " \
"elif itest.l *${bootinfo_base} == " __stringify(SDCARD_ISP) "; then " \
	"echo [scr] ISP from SD Card; " \
	"run isp_sdcard; " \
"elif itest.l *${bootinfo_base} == " __stringify(SDCARD_BOOT) "; then " \
	"echo [scr] Boot from SD Card; " \
	"run sdcard_boot; " \
"else " \
	"echo Stop; " \
"fi"

#define TMPADDR_KERNEL		0x5FFFFC0
#define TMPADDR_HEADER		0x600000
#define DSTADDR_KERNEL		0x1FFFFC0 /* if stext is on 0x2000000 */
#define DSTADDR_DTB		0x1F7FFC0

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
	"scriptaddr=0x800000; " \
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
	dbg_scr("echo kernel from ${addr_src_kernel} to ${addr_temp_kernel} sz ${sz_kernel}; ") \
	"setexpr kernel_off ${addr_src_kernel} - 0xf0000000; " \
	"echo sf probe 0; " \
	"sf probe 0; " \
	"echo sf read ${addr_temp_kernel} ${kernel_off} ${sz_kernel}; " \
	"sf read ${addr_temp_kernel} ${kernel_off} ${sz_kernel}; " \
	"setexpr sz_kernel ${sz_kernel} + 0xffff; " \
	"setexpr sz_kernel ${sz_kernel} / 0x10000; " \
	"setexpr sz_kernel ${sz_kernel} * 0x10000; " \
	"setenv bootargs ${b_c} root=/dev/mtdblock6 rw rootfstype=jffs2 user_debug=255 rootwait " \
	"mtdparts=f8000b00.spinor:96k@0(iboot)ro,160k(xboot)ro,128k(dtb),1m(uboot),640k(reserve),0x${sz_kernel}(kernel),-(rootfs); "
#else
#define NOR_LOAD_KERNEL \
	"setexpr sz_kernel ${sz_kernel} + 3; setexpr sz_kernel ${sz_kernel} / 4; " \
	dbg_scr("echo kernel from ${addr_src_kernel} to ${addr_temp_kernel} sz ${sz_kernel}; ") \
	"echo cp.l ${addr_src_kernel} ${addr_temp_kernel} ${sz_kernel}; " \
	"cp.l ${addr_src_kernel} ${addr_temp_kernel} ${sz_kernel}; "
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
"b_c=console=ttyS0,115200 earlycon\0" \
"emmc_root=root=/dev/mmcblk0p8 rw rootwait uio_pdrv_genirq.of_id=generic-uio-ev swiotlb=32768\0" \
"stdin=" STDIN_CFG "\0" \
"stdout=" STDOUT_CFG "\0" \
"stderr=" STDOUT_CFG "\0" \
"bootinfo_base="                __stringify(SP_BOOTINFO_BASE) "\0" \
"addr_src_kernel="              __stringify(CONFIG_SRCADDR_KERNEL) "\0" \
"addr_src_dtb="                 __stringify(CONFIG_SRCADDR_DTB) "\0" \
"addr_dst_kernel="              __stringify(DSTADDR_KERNEL) "\0" \
"addr_dst_dtb="                 __stringify(DSTADDR_DTB) "\0" \
"addr_tmp_header="              __stringify(TMPADDR_HEADER) "\0" \
"addr_temp_kernel="             __stringify(TMPADDR_KERNEL) "\0" \
"nfs_gatewayip="                __stringify(NFS_ROOTFS_GATEWAY_IP) "\0" \
"nfs_netmask="                  __stringify(NFS_ROOTFS_NETMASK) "\0" \
"nfs_clintip="                  __stringify(NFS_ROOTFS_CLINT_IP) "\0" \
"nfs_serverip="                 __stringify(NFS_ROOTFS_SERVER_IP) "\0" \
"nfs_rootfs_dir="               __stringify(NFS_ROOTFS_DIR) "\0" \
"if_use_nfs_rootfs="            __stringify(USE_NFS_ROOTFS) "\0" \
"if_zebu="                      __stringify(CONFIG_SYS_ENV_ZEBU) "\0" \
"if_qkboot="                    __stringify(CONFIG_SYS_USE_QKBOOT_HEADER) "\0" \
"sp_main_storage="              SP_MAIN_STORAGE "\0" \
"serverip="                     __stringify(TFTP_SERVER_IP) "\0" \
"macaddr="                      __stringify(BOARD_MAC_ADDR) "\0" \
"sdcard_devid="                 __stringify(SDCARD_DEVICE_ID) "\0" \
"do_secure="                    __stringify(COMPILE_WITH_SECURE) "\0" \
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
	"setexpr sz_kernel ${sz_kernel} + 0x48; " \
	NOR_LOAD_KERNEL \
	dbg_scr("echo booti ${addr_dst_kernel} - ${fdtcontroladdr}; ") \
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
"emmc_boot=mmc read ${addr_tmp_header} ${addr_src_kernel} 0x1; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + 0x48; " \
	"setexpr sz_kernel ${sz_kernel} + 0x200; setexpr sz_kernel ${sz_kernel} / 0x200; " \
	"mmc read ${addr_temp_kernel} ${addr_src_kernel} ${sz_kernel}; " \
	"setenv bootargs ${b_c} ${emmc_root} ${args_emmc} ${args_kern}; " \
	"run boot_kernel \0" \
"qk_emmc_boot=mmc read ${addr_tmp_header} ${addr_src_kernel} 0x1; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + 0x200; setexpr sz_kernel ${sz_kernel} / 0x200; " \
	"mmc read ${addr_dst_kernel} ${addr_src_kernel} ${sz_kernel}; " \
	"sp_go ${addr_dst_kernel} ${fdtcontroladdr}\0" \
"nand_boot=nand read ${addr_tmp_header} kernel 0x40; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	dbg_scr("md ${addr_tmp_header} 0x10; printenv tmpval; ") \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + 0x48; " \
	dbg_scr("echo from kernel partition to ${addr_dst_kernel} sz ${sz_kernel}; ") \
	"nand read ${addr_temp_kernel} kernel ${sz_kernel}; " \
	"setenv bootargs ${b_c} root=ubi0:rootfs rw ubi.mtd=9,2048 rootflags=sync rootfstype=ubifs mtdparts=${mtdparts} user_debug=255 rootwait; " \
	"run boot_kernel \0" \
"boot_kernel="\
	"if itest ${if_use_nfs_rootfs} == 1; then " \
		"setenv bootargs ${b_c} root=/dev/nfs nfsroot=${nfs_serverip}:${nfs_rootfs_dir} ip=${nfs_clintip}:${nfs_serverip}:${nfs_gatewayip}:${nfs_netmask}::eth0:off rdinit=/linuxrc noinitrd rw; "\
	"fi; " \
	"verify ${addr_temp_kernel} ${do_secure}; "\
	"setexpr addr_temp_kernel ${addr_temp_kernel} + 0x40; " \
	"setexpr addr_dst_kernel ${addr_dst_kernel} + 0x40; " \
	"echo unzip ${addr_temp_kernel} ${addr_dst_kernel}; " \
	"unzip ${addr_temp_kernel} ${addr_dst_kernel}; " \
	dbg_scr("echo booti ${addr_dst_kernel} - ${fdtcontroladdr}; ") \
	"echo booti ${addr_dst_kernel} - ${fdtcontroladdr}; " \
	"booti ${addr_dst_kernel} - ${fdtcontroladdr}\0" \
"qk_zmem_boot=sp_go ${addr_dst_kernel} ${fdtcontroladdr}\0" \
"zmem_boot=setenv verify 0; " \
	"verify ${addr_dst_kernel} ${do_secure}; " \
	"bootm ${addr_dst_kernel} - ${fdtcontroladdr}\0" \
"zebu_emmc_boot=mmc rescan; mmc part; " \
	"mmc read ${addr_tmp_header} ${addr_src_kernel} 0x1; " \
	"setenv tmpval 0; setexpr tmpaddr ${addr_tmp_header} + 0x0c; run be2le; " \
	"setexpr sz_kernel ${tmpval} + 0x40; " \
	"setexpr sz_kernel ${sz_kernel} + 0x48; " \
	"setexpr sz_kernel ${sz_kernel} + 0x200; setexpr sz_kernel ${sz_kernel} / 0x200; " \
	"mmc read ${addr_dst_kernel} ${addr_src_kernel} ${sz_kernel}; " \
	"sp_go ${addr_dst_kernel} ${fdtcontroladdr}\0" \
"tftp_boot=setenv ethaddr ${macaddr} && printenv ethaddr; " \
	"printenv serverip; " \
	"setenv filesize 0; " \
	"dhcp ${addr_dst_dtb} ${serverip}:dtb" __stringify(USER_NAME) " && " \
	"dhcp ${addr_dst_kernel} ${serverip}:uImage" __stringify(USER_NAME) "; " \
	"if test $? != 0; then " \
		"echo Error occurred while getting images from tftp server!; " \
		"exit; " \
	"fi; " \
	"booti ${addr_dst_kernel} - ${addr_dst_dtb}; " \
	"\0" \
"isp_usb=setenv isp_if usb && setenv isp_dev 0; " \
	"$isp_if start; " \
	"run isp_common; " \
	"\0" \
"usb_boot=setenv isp_if usb && setenv isp_dev 0; " \
	"$isp_if start; " \
	"run isp_common; " \
	"\0" \
"isp_sdcard=setenv isp_if mmc && setenv isp_dev $sdcard_devid; " \
	"mmc list; " \
	"echo run isp_common; "\
	"run isp_common; " \
	"\0" \
"sdcard_boot=setenv isp_if mmc && setenv isp_dev $sdcard_devid; " \
	"mmc list; " \
	"echo run sdcard_boot; "\
	SDCARD_EXT_CMD \
	"\0" \
"isp_common=setenv isp_ram_addr 0x800000; " \
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
"update_common=setenv isp_ram_addr 0x800000; " \
	"setenv isp_update_file_name ISP_UPDT.BIN; " \
	"fatload $isp_if $isp_dev $isp_ram_addr /$isp_update_file_name 0x800; " \
	"setenv isp_main_storage ${sp_main_storage} && printenv isp_main_storage; " \
	"setenv isp_image_header_offset 0; " \
	"setexpr script_addr $isp_ram_addr + 0x20 && setenv script_addr 0x${script_addr} && source $script_addr; " \
	"\0" \
"update_tftp=setenv isp_ram_addr 0x800000; " \
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

#if !defined(CONFIG_SP_SPINAND_Q645)
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
#define CONFIG_PREBOOT "usb start"
#undef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY 1
#else
#define STDIN_CFG "serial"
#endif

#endif /* __CONFIG_PENTAGRAM_H */
