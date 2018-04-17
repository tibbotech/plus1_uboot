/*
 * Copyright (C) 2017 Sunplus, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <command.h>
#ifdef CONFIG_SYS_ENV_8388
#include <asm/arch/sp_bootinfo_8388.h>
#else
#include <asm/arch/sp_bootinfo_sc7xxx.h>
#endif

#include "sp_board_preboot_env.h"

// need to use config
#define CONFIG_ZMEM_BOOTADDR	0x280000

enum {
	MEDIA_NOR = 0,
	MEDIA_NAND = 1,
	MEDIA_USB    = 2,
	MEDIA_SDCARD = 3,
	MEDIA_USB_SLAVE = 4,
};

static int do_sp_dump_bootmode(cmd_tbl_t *cmdtp, int flag, int argc,
			       char *const argv[])
{
	struct sp_bootinfo *bootinfo = SP_GET_BOOTINFO();

	printf("boot action list :\n"
	       "isp_from_sdcard [%08x]\n"
	       "isp_from_usb    [%08x]\n"
	       "boot_from_nand  [%08x]\n"
	       "boot_from_emmc  [%08x]\n"
	       "boot_from_nor   [%08x]\n"
	       "\n"
	       "boot media list :\n"
	       "sdcard, usb, nand, emmc, nor\n"
	       "\n",
	       SDCARD_ISP, USB_ISP, SPINAND_BOOT, EMMC_BOOT, SPI_NOR_BOOT);
	printf("current bootmode is %08x\n", bootinfo->gbootRom_boot_mode);

	return CMD_RET_SUCCESS; 
}

static cmd_tbl_t cmd_sp_preboot_sub[] = {
	U_BOOT_CMD_MKENT(dump_bootmode, 1, 1, do_sp_dump_bootmode,
			 "dump all available bootmode", ""),
};

/* 
 * According to boot mode from sram, set env variable for booting script.
 * Then load & run booting script.
 */
static int do_sp_preboot(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	cmd_tbl_t *c;
	struct sp_bootinfo *bootinfo;

	if (argc > 2)
		return CMD_RET_USAGE;

	/* sub function flow */
	if (argc == 2) {
		/* Strip off leading 'sp_preboot' command argument */
		argc--;
		argv++;
		c = find_cmd_tbl(argv[0], &cmd_sp_preboot_sub[0], ARRAY_SIZE(cmd_sp_preboot_sub));

		if (c)
	            	return c->cmd(cmdtp, flag, argc, argv);
		else
			return CMD_RET_USAGE;
	}

	bootinfo = SP_GET_BOOTINFO();
	/* main function */
	if (bootinfo->gbootRom_boot_mode == SPINAND_BOOT) {
		env_set(SP_ENVNAME_BOOTACT, SP_BOOTACT_FROM_NAND);
		env_set(SP_ENVNAME_BOOTMED, SP_BOOTMED_NAND);
	} else if (bootinfo->gbootRom_boot_mode == EMMC_BOOT) {
		env_set(SP_ENVNAME_BOOTACT, SP_BOOTACT_FROM_EMMC);
		env_set(SP_ENVNAME_BOOTMED, SP_BOOTMED_EMMC);
	} else if (bootinfo->gbootRom_boot_mode == SPI_NOR_BOOT) {
#if 0
		/* load & run script here */
		char cmd[32];
		char *addr = env_get(SP_ENVNAME_BOOT_RAMADDR);

		env_set(SP_ENVNAME_BOOTACT, SP_BOOTACT_FROM_NOR);
		env_set(SP_ENVNAME_BOOTMED, SP_BOOTMED_NOR);

		/* zmem use this flow */
		if (addr) {
			sprintf(cmd, "source %s", addr);
			run_command(cmd, 0);
		} else {
			sprintf(cmd, "source %x", CONFIG_ZMEM_BOOTADDR);
			run_command(cmd, 0);
		}
#endif
	} else if (bootinfo->gbootRom_boot_mode == USB_ISP) {
		env_set(SP_ENVNAME_BOOTACT, SP_BOOTACT_ISP_USB);
		env_set(SP_ENVNAME_BOOTMED, SP_BOOTMED_USB);
	} else if (bootinfo->gbootRom_boot_mode == SDCARD_ISP) {
		env_set(SP_ENVNAME_BOOTACT, SP_BOOTACT_ISP_SDCARD);
		env_set(SP_ENVNAME_BOOTMED, SP_BOOTMED_SDCARD);
	} else {
		//env_set(SP_ENVNAME_BOOTACT, SP_BOOTACT_OTHER);
		//env_set(SP_ENVNAME_BOOTMED, SP_BOOTMED_OTHER);
		printf("other boot mode [%08x]\n", bootinfo->gbootRom_boot_mode);

		return CMD_RET_FAILURE;
	}
		
	return CMD_RET_SUCCESS;
}

/*
 * JTAG pinmux is conflict with spi nor, so add these setting after spi-nor
 * loading.
 */
static int do_sp_jtag(cmd_tbl_t *cmdtp, int flag, int argc,
		      char *const argv[])
{
	volatile unsigned int* gpio_fst_ctl_1_grp_4_26  = (volatile unsigned int*)0x9C000268;
	volatile unsigned int* mo1_pinmx_ctl_0 = (volatile unsigned int*)0x9C000084;
	volatile unsigned int* mo1_pinmx_ctl_4 = (volatile unsigned int*)0x9C000094;
	volatile unsigned int* mo1_pinmx_ctl_5 = (volatile unsigned int*)0x9C000098;
	volatile unsigned int* mo1_pinmx_ctl_6 = (volatile unsigned int*)0x9c00009c;
	volatile unsigned int* cpu_debug_ctrl_grp_83_20 = (volatile unsigned int*)0x9c0029d0;
	//unsigned value = 0x00000001;

	/* disable gpio55, conflict */
	*gpio_fst_ctl_1_grp_4_26 &= ~(1 << 23);
	/* disable default pin mux */
	*mo1_pinmx_ctl_0 &= ~(1 << 29 | 1 << 28 | 1 << 27 | 1 << 26 | 1 << 25 |
			      1 << 24 | 1 << 23 | 1 << 22 | 1 << 21 | 1 << 20 |
			      1 << 19 | 1 << 18); /* bit 29-18 */
	/* disable other pin mux */
	*mo1_pinmx_ctl_6 &= ~(1 << 22 | 1 << 21 | 1 << 2 | 1 << 1 | 1 << 0);
	*mo1_pinmx_ctl_5 &= ~(1 << 30 | 1 << 29 | 1 << 28 | 1 << 27 | 1 << 26 | 1 << 25);
	*mo1_pinmx_ctl_4 &= ~(1 << 6 | 1 << 5 | 1 << 4);
	*mo1_pinmx_ctl_0 &= ~(1 << 1 | 1 << 0);
	/* enable JTAG */
	*mo1_pinmx_ctl_6 |= (1 << 31);
	/* enable debug */
	*cpu_debug_ctrl_grp_83_20 = 0x1f;

	printf("----- JTAG is ready\n");

	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char sp_preboot_help_text[] =
	"sp_preboot [option]\n"
	"\tRun this command before booting script.\n"
	"\tIt will necessary sunplus boot steps.\n"
	"\n"
	"\tSo far, it will do the following steps :\n"
	"\t1. read SRAM for boot mode and write it to env variable\n"
	"\t2. according to bootmode write it to env - \"sp_bootmode\"\n"
	"\t   available bootmode can be queried by \"sp_bootmode dump\"\n"
	"\n"
	"option :\n"
	"dump\n"
	"\tdump all available boot action and current action.\n"
	"\n";
#endif

U_BOOT_CMD(sp_preboot, 2, 1, do_sp_preboot, "Prepare for booting flow",
	   sp_preboot_help_text);
U_BOOT_CMD(sp_jtag, 1, 1, do_sp_jtag, "Setup JTAG setting", NULL);
