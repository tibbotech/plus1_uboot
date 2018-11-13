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
	{.name="MT29F1G01ABADD",{.mfr_id=0x2c,.dev_id=0x14},.pagesize=SZ_2K,.chipsize=SZ_1K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},
	{.name="MT29F2G01ABADD",{.mfr_id=0x2c,.dev_id=0x24},.pagesize=SZ_2K,.chipsize=SZ_2K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},
	/* MXIC */
	{.name="MX35LF1GE4AB",{.mfr_id=0xc2,.dev_id=0x12},.pagesize=SZ_2K,.chipsize=SZ_1K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},
	{.name="MX35LF2GE4AB",{.mfr_id=0xc2,.dev_id=0x22},.pagesize=SZ_2K,.chipsize=SZ_2K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},
	/* Etron */
	{.name="EM73C044VCC-H",{.mfr_id=0xd5,.dev_id=0x22},.pagesize=SZ_2K,.chipsize=SZ_1K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},
	{.name="EM73D044VCE-H",{.mfr_id=0xd5,.dev_id=0x20},.pagesize=SZ_2K,.chipsize=SZ_2K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},

	/* Winbond */
	{.name="25N01GVxx1G",{.mfr_id=0xef,.dev_id=0xaa},.pagesize=SZ_2K,.chipsize=SZ_1K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},
	{.name="25N01GVxx2G",{.mfr_id=0xef,.dev_id=0xab},.pagesize=SZ_2K,.chipsize=SZ_2K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},

	/* GiGA */
	{.name="GD5F1GQ4UBYIG",{.mfr_id=0xc8,.dev_id=0xd1},.pagesize=SZ_2K,.chipsize=SZ_1K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},
	{.name="GD5F2GQ4UBYIG",{.mfr_id=0xc8,.dev_id=0xd2},.pagesize=SZ_2K,.chipsize=SZ_2K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},

	/* ESMT */
	{.name="F50L1G41LB",{.mfr_id=0xc8,.dev_id=0x01},.pagesize=SZ_2K,.chipsize=SZ_1K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},
	{.name="F50L2G41LB",{.mfr_id=0xc8,.dev_id=0x0a},.pagesize=SZ_2K,.chipsize=SZ_2K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},

	/* ISSI */
	{.name="IS38SML01G1-LLA1",{.mfr_id=0xc8,.dev_id=0x21},.pagesize=SZ_2K,.chipsize=SZ_1K, 
	 .erasesize=SZ_128K, .options=LP_OPTIONS,.id_len=2,.oobsize=64},	
	
	{NULL}
};
