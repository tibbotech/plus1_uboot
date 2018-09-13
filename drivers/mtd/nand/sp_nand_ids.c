/*
 * (C) Copyright 2014
 * Sunplus Technology
 * Kuo-Jung Su <dante.su@sunplus.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <linux/mtd/nand.h>
#include <linux/sizes.h>

#define LP_OPTIONS NAND_SAMSUNG_LP_OPTIONS
#define LP_OPTIONS16 (LP_OPTIONS | NAND_BUSWIDTH_16)

#define NAND_LP(name, id, ext_id, pgsz, bksz, size) \
	{ name, id, pgsz, size, bksz, LP_OPTIONS, ext_id }

/*
 *	Chip ID list
 *
 *	Name. ID code, pagesize, chipsize in MegaByte, eraseblock size,
 *	options
 *
 *	Pagesize; 0, 256, 512
 *	0	get this information from the extended chip ID
 *	256	256 Byte page size
 *	512	512 Byte page size
 */
const struct nand_flash_dev sp_nand_ids[] = {
	/* Micron */
	{.name="MT29F1G01AAADD",{.mfr_id=0x2c,.dev_id=0x12},.pagesize=SZ_2K,.chipsize=128, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},
	{NULL}
};
