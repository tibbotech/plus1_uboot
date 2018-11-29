#include <common.h>
#include <malloc.h>
//#include <asm/io.h>
#include <nand.h>

#include <linux/mtd/nand.h>
#include <dm.h>
#include <linux/io.h>
#include <linux/ioport.h>

#include "sp_bch.h"
#include "sp_spinand.h"


/**************************************************************************
 *                             M A C R O S                                *
 **************************************************************************/
#define USE_DESCRIPTOR_MODE 0
#define DEVICE_STS_AUTO_CHK 0
#define USE_SP_BCH          1   // 1:Using BCH, 0:using Device internal ECC
#define USB_SPDMA_AUTOBCH	1


/* SRAM */
/* #define CFG_BBT_USE_FLASH */
#define CFG_BUFF_MAX		(18 << 10)
#define CFG_DESC_MAX		4
#define CFG_CMD_TIMEOUT_MS	10

#define USE_DESCRIPTOR_MODE	0
/* 1:Using S+ BCH, 0:using Device internal ECC */
#define CONFIG_SRAM_BASE                0x9e800000

DECLARE_GLOBAL_DATA_PTR;
/**************************************************************************
 *                         D A T A   T Y P E S                            *
 **************************************************************************/

/**************************************************************************
 *                        G L O B A L   D A T A                           *
 **************************************************************************/
static struct sp_spinand_info *our_spinfc = NULL;


/**************************************************************************
 *                 E X T E R N A L   R E F E R E N C E S                  *
 **************************************************************************/
 
/**************************************************************************/
/* check SPI NAND Device status by ctrl_status_registers, */
void wait_spi_idle(struct sp_spinand_info *info)
{
	struct sp_spinand_regs *regs = info->regs;

	while (readl(&regs->spi_ctrl) & SPI_DEVICE_IDLE) {	/* --> ctrl bit-31, wait spi_ctrl idle */
		/* wait */;
	}
}

int spi_nand_getfeatures(struct sp_spinand_info *info,uint32_t addr)
{
	struct sp_spinand_regs *regs = info->regs;
	int value = 0;
	
	value = (SPI_NAND_CHIP_A)|(SPI_NAND_AUTO_WEL)|(SPI_NAND_CLK_32DIV)|(SPINAND_CMD_GETFEATURES<<8)|(SPI_NAND_CTRL_EN)|(SPINAND_CUSTCMD_1_DATA)|(SPINAND_CUSTCMD_1_ADDR);
	writel(value ,&regs->spi_ctrl);

	writel(addr ,&regs->spi_page_addr);

	value = SPINAND_CFG01_DEFAULT;
	writel(value ,&regs->spi_cfg[1]);

	value = (1<<1);
	writel(value, &regs->spi_timing);

	value = SPINAND_AUTOCFG_CMDEN;
	writel(value ,&regs->spi_auto_cfg);

	wait_spi_idle(info);

	return (readl(&regs->spi_data) & 0xFF);

}

void spi_nand_setfeatures(struct sp_spinand_info *info,uint32_t addr, uint32_t data)
{
	struct sp_spinand_regs *regs = info->regs;
	int value = 0;

	value = (SPI_NAND_CHIP_A)|(SPI_NAND_AUTO_WEL)|(SPI_NAND_CLK_32DIV)|(SPINAND_CMD_SETFEATURES<<8)|(SPI_NAND_CTRL_EN)|(SPINAND_CUSTCMD_1_DATA)|(SPI_NAND_WRITE_MDOE)|(SPINAND_CUSTCMD_1_ADDR);
	writel(value ,&regs->spi_ctrl);

	writel(addr ,&regs->spi_page_addr);

	writel(data ,&regs->spi_data);

	value = SPINAND_CFG01_DEFAULT1;
	writel(value ,&regs->spi_cfg[1]);

	value = SPINAND_AUTOCFG_CMDEN;
	writel(value ,&regs->spi_auto_cfg);

	wait_spi_idle(info);

}


static int sp_spinand_reset(struct sp_spinand_info *info)
{
	struct sp_spinand_regs *regs = info->regs;
	int value = 0;
	int ret = -1;


	//initial
	while (readl(&regs->spi_ctrl) & SPI_DEVICE_IDLE) {
		/* wait */ ;
	}

	/* ==== Flash reset ==== */
	value = (SPI_NAND_CHIP_A)|(SPI_NAND_CLK_32DIV)|(SPINAND_CMD_RESET<<8)|(SPI_NAND_CTRL_EN)|(SPI_NAND_WRITE_MDOE);
	writel(value, &regs->spi_ctrl);

	value = SPINAND_CFG01_DEFAULT3;
	writel(value, &regs->spi_cfg[1]);	

	value = SPINAND_AUTOCFG_CMDEN;
	writel(value, &regs->spi_auto_cfg);

	wait_spi_idle(info);

#if DEVICE_STS_AUTO_CHK
	wait_spi_idle(info);
	ret = spi_nand_getfeatures(info, DEVICE_STATUS_ADDR);
#else
	do {
		ret = spi_nand_getfeatures(info, DEVICE_STATUS_ADDR);
		printf("%s Status:%x\n", __FUNCTION__, ret);
	} while (ret & 0x01);
#endif

	return ret;
}

void spi_nand_readid(struct sp_spinand_info *info, uint32_t addr, uint32_t *data)
{
	struct sp_spinand_regs *regs = info->regs;
	int value = 0;

	writel(addr, &regs->spi_page_addr);
	/*read 3 byte cycle same to 8388 */
	value = SPI_NAND_CHIP_A|SPI_NAND_AUTO_WEL|(SPI_NAND_CLK_32DIV)|(SPINAND_CMD_READID<<8)|SPI_NAND_CTRL_EN|(SPINAND_CUSTCMD_3_DATA)|(SPINAND_CUSTCMD_1_ADDR);
	writel(value, &regs->spi_ctrl);

	value = SPINAND_CFG01_DEFAULT;
	writel(value ,&regs->spi_cfg[1]);

	value = (1<<1);
	writel(value, &regs->spi_timing);

	value = SPINAND_AUTOCFG_CMDEN;
	writel(value ,&regs->spi_auto_cfg);

	wait_spi_idle(info);

	value = readl(&regs->spi_data);

	printf("\nReadID:0x%02x,0x%02x,0x%02x,0x%02x\n",
	       (value & 0xFF),
	       ((value >> 8) & 0xFF),
	       ((value >> 16) & 0xFF),
	       ((value >> 24) & 0xFF));

	*data = value;
}

int spi_nand_blkerase(struct sp_spinand_info *info, uint32_t addr)
{	
	struct sp_spinand_regs *regs = info->regs;
	int value = 0;
	
	value = (1<<24)|(1<<20)|(1<<19)|(2<<16)|(0xd8<<8)|(1<<7)|(0<<4)|(1<<2)|(3);
	writel(value ,&regs->spi_ctrl);

	writel(addr ,&regs->spi_page_addr);

	value = 0x150095;
	writel(value ,&regs->spi_cfg[1]);

	value = (1<<21);
	writel(value ,&regs->spi_auto_cfg);

	wait_spi_idle(info);

	value = (1<<21)|(1<<19);
	writel(value ,&regs->spi_auto_cfg);

	wait_spi_idle(info);
	
	value = readl(&regs->spi_status);

	if (value & ERASE_STATUS)
		printf("\nErase Fail!\n");

	return (value & ERASE_STATUS);
}



void spi_nand_pageread2cache(struct sp_spinand_info *info, uint32_t addr)
{
	struct sp_spinand_regs *regs = info->regs;
	int value = 0;

#if USE_SP_BCH
	spi_nand_setfeatures(info, 0xB0, 0x00); /* en-able QuadIO,ECC-off */
#else
	spi_nand_setfeatures(info, 0xB0, 0x10); /* en-able QuadIO,ECC-on */
#endif

	info->row = addr;

	value = (SPI_NAND_CHIP_A)|(SPI_NAND_CLK_32DIV)|(SPINAND_CMD_PAGE2CACHE<<8)|(SPI_NAND_CTRL_EN)|(SPINAND_CUSTCMD_3_ADDR);
	writel(value, &regs->spi_ctrl);

	value = (1<<23)|(1<<19);
	writel(value ,&regs->spi_cfg[0]);

	value = SPINAND_CFG01_DEFAULT2;
	writel(value ,&regs->spi_cfg[1]);

	writel(addr, &regs->spi_page_addr);

	value = (SPINAND_CMD_PAGE2CACHE<<24)|(SPINAND_AUTOCFG_CMDEN)|(SPINAND_AUTOCFG_RDCACHE)|(SPINAND_AUTOCFG_RDSTATUS);
	writel(value ,&regs->spi_auto_cfg);

	wait_spi_idle(info);
}

void spi_nand_readcacheQuadIO_byMapping(struct sp_spinand_info *info, uint32_t addr, unsigned int size, uint32_t *pbuf)
{
	struct sp_spinand_regs *regs = info->regs;
	int value = 0;
	int i;
	
	value = SPINAND_CFG02_DEFAULT;
	writel(value, &regs->spi_cfg[2]);

	value = (SPINAND_CMD_PAGEREAD <<24)|(SPINAND_AUTOCFG_RDCACHE)|(SPINAND_AUTOCFG_RDSTATUS);
	writel(value, &regs->spi_auto_cfg);
	
	value = readl(&regs->spi_auto_cfg);

	while((value>>24)!= 0x3);

	if ((info->row & (0x40)) && (((info->id & 0xFF) == 0xC2)||((info->id & 0xFF) == 0x2C))) {
		for (i = addr ; i < (addr + size) ; i += 4)
			*(unsigned int *)pbuf++ = *(unsigned int *)(0x9dff0000 + 0x1000 + i);
	} else {
		for (i = addr ; i < (addr + size) ; i += 4)
			*(unsigned int *)pbuf++ = *(unsigned int *)(0x9dff0000 + i);
	}

	wait_spi_idle(info);
}

int spi_nanddma_pageread(struct sp_spinand_info *info, uint32_t addr, unsigned int size, uint32_t *pbuf)
{
	struct sp_spinand_regs *regs = info->regs;
	int value = 0;

	while (readl(&regs->spi_auto_cfg) & SPI_NAND_DMA_OWNER); 

	value = SPI_NAND_CHIP_A|SPI_NAND_CLK_32DIV|SPI_NAND_CTRL_EN|(2);
	writel(value, &regs->spi_ctrl);

	writel(addr, &regs->spi_page_addr);

	value = 0x08150095; // 4 bit data 8 dummy clock 1bit cmd  1bit addr
	writel(value, &regs->spi_cfg[1]);
	writel(value, &regs->spi_cfg[2]);

	value = readl(&regs->spi_cfg[0]);
	value = value|size; // 1k data len
	writel(value, &regs->spi_cfg[0]);

	value = (1<<1);
	writel(value, &regs->spi_timing);

	if ((addr & 0x40) && (((info->id & 0xFF) == 0xC2)||((info->id & 0xFF) == 0x2C)))		
		value = 0x1000;
	else
		value = 0x0;
	
	writel(value, &regs->spi_col_addr);

	value = (0x40<<4)|(0x1);
	writel(value, &regs->spi_page_size);

	writel((uint32_t)pbuf, &regs->mem_data_addr);

	//config ctrl info	
	//set auto cfg
	value = (0x3<<1);
	writel(value, &regs->spi_intr_msk);
	writel(value, &regs->spi_intr_sts);
	value = (0x03<<24)|(1<<20)|(1<18)|(1<<17);
	writel(value, &regs->spi_auto_cfg);
	while((readl(&regs->spi_intr_sts) & 0x2) == 0x0);

	return 0;
}

int spi_nanddma_pageprogram(struct sp_spinand_info *info, uint32_t addr, unsigned int size, uint32_t *pbuf)
{
	struct sp_spinand_regs *regs = info->regs;
	int value = 0;	

	/* polling DMA_OWNER == 0 */
	while(readl(&regs->spi_auto_cfg) & (1<<17));

	value = (1<<24)|(2<<16)|(1<<7)|(1<<2)|(2);
	writel(value, &regs->spi_ctrl);

	writel(addr, &regs->spi_page_addr);

	// config device info
	//set cfg[1]= cmd 1 bit addr 1 bit data 1 bit	
	value = 0x150095;
	writel(value, &regs->spi_cfg[1]);
	writel(value, &regs->spi_cfg[2]);

	//read 2k data
	value = readl(&regs->spi_cfg[0]);
	value = value|size; 
	writel(value, &regs->spi_cfg[0]);
	
	// col addr set
	if ((addr & 0x40) && (((info->id & 0xFF) == 0xC2)||((info->id & 0xFF) == 0x2C)))		
		value = 0x1000;
	else
		value = 0x0;
	writel(value, &regs->spi_col_addr);

	//set nand page size
	value = (0x0<<15)|(0x40<<4)|(0x1); 
	writel(value, &regs->spi_page_size);

	//page size 2K
	writel((uint32_t)pbuf, &regs->mem_data_addr);

	//config ctrl info	
	//set auto cfg 	
	value = (0x3<<1);	
	writel((uint32_t)value, &regs->spi_intr_msk);
	writel((uint32_t)value, &regs->spi_intr_sts);

	value = (1<18)|(1<<17)|(0x02<<8)|(1<<1)|(1);
	writel(value, &regs->spi_auto_cfg);

	//polling dma operation done bit	
	while((readl(&regs->spi_intr_sts) & 0x2) == 0x0);

	//	data check
	wait_spi_idle(info);

	value = (1<<21)|(1<<19);	

	wait_spi_idle(info);

	value = readl(&regs->spi_status);
	if(value & 0x8)
		return -1;	
	else
		return 0;
}

static void sp_spinand_select_chip(struct mtd_info *mtd, int chipnr)
{
	struct sp_spinand_info *info = our_spinfc;

	switch (chipnr) {
	case 0:
	case 1:
	case 2:
	case 3:
		info->cs = chipnr;
		break;
	default:
		break;
	}
}


static int sp_spinand_fixup(struct sp_spinand_info *info)
{
	info->cac = 2;
	return 0;
}

static int sp_spinand_desc_prep(struct sp_spinand_info *info,
				uint8_t cmd, int col, int row)
{
	switch (cmd) {
	case NAND_CMD_READOOB:	/* 0x50 */
		col += info->mtd->writesize;
		/* fall through */
	case NAND_CMD_READ0:	/* 0x00 */
		info->cmd = SPINAND_CMD_PAGE2CACHE;
		info->buff.idx = col;
		info->row = row;
		#if 0
		spi_nand_pageread2cache(info, row & 0x01FFFF);
		spi_nand_readcacheQuadIO_byMapping(info, 0,
						   info->mtd->writesize +
						   info->mtd->oobsize,
						   (unsigned int *)info->buff.
						   virt);
		#else
		spi_nanddma_pageread(info, row,info->mtd->writesize +
						   info->mtd->oobsize,
						   (unsigned int *)info->buff.phys);

		#endif
		break;
	case NAND_CMD_SEQIN:	/* 0x80 */
		info->cmd = SPINAND_CMD_PROLOADx4;
		info->buff.idx = col;
		/* spi_nand_wren(info); */
		spi_nand_setfeatures(info, DEVICE_PROTECTION_ADDR, 0x0);
		info->row = (row & 0x01FFFF);
		break;
	case NAND_CMD_ERASE1:	/* 0x60 */
		info->cmd = SPINAND_CMD_BLKERASE;
		spi_nand_setfeatures(info, DEVICE_PROTECTION_ADDR, 0x0);
		/* spi_nand_wren(info); */
		/* bit-5~0 is page address */
		spi_nand_blkerase(info, (row & (0x01FFC0)));
		break;
	case NAND_CMD_STATUS:	/* 0x70 */
		info->cmd = SPINAND_CMD_GETFEATURES;
		info->buff.idx = 0;
		if (spi_nand_getfeatures(info, DEVICE_PROTECTION_ADDR) &
		    PROTECT_STATUS) {
			*(unsigned int *)info->buff.virt = 0x0;	/* protected */
		} else {
			/* not protectd. comply w/ raw NAND */
			*(unsigned int *)info->buff.virt = 0x80;
		}
		break;
	case NAND_CMD_READID:	/* 0x90 */
	case NAND_CMD_PARAM:	/* 0xEC */
	case NAND_CMD_GET_FEATURES:	/* 0xEE */
		break;
	case NAND_CMD_SET_FEATURES:	/* 0xEF */
		break;
	case NAND_CMD_RESET:	/* 0xFF */
		break;
	default:
		break;
	}

	return 0;
}

static int sp_spinand_wait(struct sp_spinand_info *info)
{
	struct sp_spinand_regs *regs = info->regs;
	unsigned long now = get_timer(0);
	int ret = -1;

	while (get_timer(now) < CFG_CMD_TIMEOUT_MS) {
		if (!(readl(&regs->spi_ctrl) & SPI_DEVICE_IDLE)) {
			ret = 0;
			break;
		}
	}

	return ret;
}

static int sp_spinand_desc_send(struct sp_spinand_info *info, int need_wait)
{
	int ret;

	if (info->cmd == NAND_CMD_PAGEPROG) {
		info->cmd = SPINAND_CMD_PROEXECUTE;

		ret = spi_nanddma_pageprogram(info,info->row,info->mtd->writesize + info->mtd->oobsize,(unsigned int *)info->buff.
						   virt);
	}

	ret = sp_spinand_wait(info);
	if (ret) {
		pr_info("sp_spinand: timeout, cmd=0x%x\n", info->cmd);
		sp_spinand_reset(info);
	}


	return ret;
}


static void sp_spinand_cmd_ctrl(struct mtd_info *mtd, int cmd,
				unsigned int ctrl)
{
	return;
}

static void sp_spinand_cmdfunc(struct mtd_info *mtd, unsigned cmd, int col,
			       int row)
{
	struct sp_spinand_info *info = our_spinfc;

	info->cmd = cmd;
	switch (cmd) {
	case NAND_CMD_READ0:	/* 0x00 */
		sp_spinand_desc_prep(info, cmd, col, row);
		sp_spinand_desc_send(info, 1);
		break;
	case NAND_CMD_READOOB:	/* 0x50 */
		sp_spinand_desc_prep(info, cmd, col, row);
		sp_spinand_desc_send(info, 1);
		break;
	case NAND_CMD_SEQIN:	/* 0x80 */
		sp_spinand_desc_prep(info, cmd, col, row);
		break;
	case NAND_CMD_PAGEPROG:	/* 0x10 */
		sp_spinand_desc_send(info, 1);
		break;
	case NAND_CMD_ERASE1:	/* 0x60 */
		sp_spinand_desc_prep(info, cmd, col, row);
		break;
	case NAND_CMD_ERASE2:	/* 0xD0 */
		sp_spinand_desc_send(info, 1);
		break;
	case NAND_CMD_STATUS:	/* 0x70 */
		sp_spinand_desc_prep(info, cmd, -1, -1);
		sp_spinand_desc_send(info, 1);
		break;
	case NAND_CMD_RESET:	/* 0xFF */
		sp_spinand_reset(info);
		mdelay(5);
		break;
	case NAND_CMD_READID:	/* 0x90 */
		info->buff.idx = 0;
		spi_nand_readid(info, 0, info->buff.virt);
		break;
	case NAND_CMD_PARAM:	/* 0xEC */
	case NAND_CMD_GET_FEATURES:	/* 0xEE */
		sp_spinand_desc_prep(info, cmd, col, -1);
		/* sp_spinand_desc_send(info, 1); */
		break;
	case NAND_CMD_SET_FEATURES:	/* 0xEF */
		sp_spinand_desc_prep(info, cmd, col, -1);
		break;
	default:
		printf("sp_spinand: unknown command=0x%02x.\n", cmd);
		break;
	}
}

static int sp_spinand_dev_ready(struct mtd_info *mtd)
{
	struct sp_spinand_info *info = our_spinfc;

	return ((spi_nand_getfeatures(info, DEVICE_STATUS_ADDR) & 0x01) == 0);
}

static uint8_t sp_spinand_read_byte(struct mtd_info *mtd)
{
	struct sp_spinand_info *info = our_spinfc;

	uint8_t ret = 0;

	switch (info->cmd) {
	case NAND_CMD_STATUS:
		ret = readb(info->buff.virt);
		break;

	default:
		if (info->buff.idx < info->buff.size) {
			ret = readb(info->buff.virt + info->buff.idx);
			info->buff.idx += 1;
		}
		break;
	}

	return ret;
}

static void sp_spinand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	struct sp_spinand_info *info = our_spinfc;

	memcpy(buf, info->buff.virt + info->buff.idx, len);
	info->buff.idx += len;
}

static void sp_spinand_write_buf(struct mtd_info *mtd,
				 const u_char *buf, int len)
{
	struct sp_spinand_info *info = our_spinfc;

	memcpy(info->buff.virt + info->buff.idx, buf, len);
	info->buff.idx += len;
}

static int sp_spinand_read_page(struct mtd_info *mtd,
				struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
#ifdef CONFIG_USE_SP_BCH
	struct sp_spinand_info *info = our_spinfc;
	int ret;

	ret = sp_bch_decode(mtd, info->buff.virt,
			    info->buff.virt + mtd->writesize);
	if (ret)
		printf("sp_spinand: bch decode failed at page=%d\n", page);
#endif

	chip->read_buf(mtd, buf, mtd->writesize);

	if (!oob_required || page < 0)
		return 0;

	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	return 0;
}

static int sp_spinand_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				 const uint8_t *buf, int oob_required, int page)
{
#ifdef CONFIG_USE_SP_BCH
	struct sp_spinand_info *info = our_spinfc;
#endif

	chip->write_buf(mtd, buf, mtd->writesize);
	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);

#ifdef CONFIG_USE_SP_BCH
	sp_bch_encode(mtd, info->buff.virt, info->buff.virt + mtd->writesize);
#endif

	return 0;
}

static int sp_spinand_init(struct sp_spinand_info *info)
{
	unsigned int id;
	struct nand_chip *nand = &info->nand;
	struct mtd_info *mtd = &nand->mtd;
	int ret,value;

	info->mtd = mtd;
	nand->IO_ADDR_R = nand->IO_ADDR_W = info->regs;
	printf("sp_spinand: regs@0x%p\n", info->regs);

	info->buff.size = CFG_BUFF_MAX;
	info->buff.phys = CONFIG_SRAM_BASE;
	info->buff.virt = (void *)info->buff.phys;
	printk("sp_spinand: buff=0x%p@0x%08x size=%u\n", info->buff.virt,
	       info->buff.phys, info->buff.size);

	info->nand.select_chip = sp_spinand_select_chip;
	info->nand.cmd_ctrl = sp_spinand_cmd_ctrl;
	info->nand.cmdfunc = sp_spinand_cmdfunc;
	info->nand.dev_ready = sp_spinand_dev_ready;

	info->nand.read_byte = sp_spinand_read_byte;
	info->nand.read_buf = sp_spinand_read_buf;
	info->nand.write_buf = sp_spinand_write_buf;
	//info->nand.verify_buf = sp_spinand_verify_buf;

	info->nand.ecc.read_page = sp_spinand_read_page;
	info->nand.ecc.write_page = sp_spinand_write_page;
	info->nand.ecc.layout = &info->ecc_layout;
	info->nand.ecc.mode = NAND_ECC_HW;	

	if (sp_spinand_reset(info) < 0)
		return -EIO;

	/* Read ID */
	spi_nand_readid(info, 0, &id);

	if ((id & 0xFF) == WB_ID) {
		info->id = (id & WB_ID);
		printf("Winbond SPI NAND found\n");
		printf("Set WB in BUF-1..& ECC-OFF,using S+ BCH");
		spi_nand_setfeatures(info, DEVICE_FEATURE_ADDR,WB_BUF1_DIS_ECC);
	} else if (((id & 0xFF) == 0xC8) && (!((((id>>8) & 0xFF) == 0x21)||(((id>>8) & 0xFF) == 0x01)||(((id>>8) & 0xFF) == 0x0a)))) { 
		printf("GD SPI NAND found\n");
		spi_nand_setfeatures(info, DEVICE_FEATURE_ADDR,0);	
	} else {
		if ((id & 0xFF) == 0xC2) {	/* MXIC */
			info->id = 0xC2;
			printf("MXIC SPI NAND found\n");
		}
		else if ((id & 0xFF) == 0x2C) { /* MICRON */
			info->id = 0x2C;
			printf("MICRON SPI NAND found\n");
		}
		else if ((id & 0xFF) == 0xd5) {/* Etron */
			info->id = 0xd5;
			printf("Etron SPI NAND found\n");
		}
		else if ((id & 0xFF) == 0xC8) { /* ESMT */
			info->id = (id & 0xFFFF);
			printf("ESMT SPI NAND found\n");
		} else {
			printf("SPI NAND found\n");
			info->id = (id & 0xFFFF);
		}

		value=spi_nand_getfeatures(info, DEVICE_FEATURE_ADDR);
		value &= ~0x00000010;
		spi_nand_setfeatures(info, DEVICE_FEATURE_ADDR, value);
	}

	spi_nand_setfeatures(info, DEVICE_PROTECTION_ADDR, 0x0);

	ret = nand_scan_ident(mtd, 1, (struct nand_flash_dev *)sp_nand_ids);
	if (ret < 0)
		return ret;

	ret = sp_spinand_fixup(info);
	if (ret < 0)
		return ret;

#ifdef CONFIG_USE_SP_BCH
	ret = sp_bch_init(mtd);
	if (ret < 0 )
		return ret;
#endif
	ret = nand_scan_tail(mtd);
	if (ret < 0)
		return ret;

	nand_register(0,mtd);
	return 0;
}

static int sp_spinand_probe(struct udevice *dev)
{
	struct resource res;
	int ret;
	
	our_spinfc = dev_get_priv(dev);

	/* get spi-nand reg */
	ret = dev_read_resource_byname(dev, "spinand_reg", &res);
	if (ret)
		return ret;

	our_spinfc->regs = devm_ioremap(dev, res.start, resource_size(&res));	

	/* get bch reg */
#ifdef CONFIG_USE_SP_BCH	
	ret = dev_read_resource_byname(dev, "bch_reg", &res);
	if (ret)
		return ret;

	our_spinfc->bch_regs = devm_ioremap(dev, res.start, resource_size(&res));	
#endif
	return sp_spinand_init(our_spinfc);
}

struct sp_spinand_info* get_spinand_info(void)
{
	return our_spinfc;
}


static const struct udevice_id sunplus_spinand[] = {
	{
		.compatible = "sunplus,sunplus-q628-spinand",
	},
};


U_BOOT_DRIVER(pentagram_spi_nand) ={
	.name						= "pentagram_spi_nand",	
	.id							= UCLASS_MTD,	
	.of_match					= sunplus_spinand,	
	.priv_auto_alloc_size = sizeof(struct sp_spinand_info),
	.probe						= sp_spinand_probe,	
};

void board_spinand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_GET_DRIVER(pentagram_spi_nand),
					  &dev);

	if (ret && ret != -ENODEV)
		printf("Failed to initialize sunplus SPI NAND controller. (error %d)\n",
		       ret);
}


