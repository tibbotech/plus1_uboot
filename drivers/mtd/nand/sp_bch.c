/*
 * Sunplus Technology
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <linux/bitops.h>
#include <asm/io.h>
#include <nand.h>
#include <asm/arch/regmap.h>
#include "sp_bch.h"

#define CFG_CMD_TIMEOUT_MS	50

#if defined(CONFIG_SP_NFTL) || defined(CONFIG_SP_BCH_REPORT)
unsigned int *sp_bch_ftl_info;
unsigned int *Get_SP_BCH_Info(void)
{
	return sp_bch_ftl_info;
}
#endif

/* Used in uboot/common/cmd_ispsp.c */
uint32_t ispsp_chk4badblock_max_err_bit_in_page;
uint32_t ispsp_chk4badblock_err_bit_in_page;
uint32_t ispsp_chk4badblock_ecc_size;
uint32_t ispsp_chk4badblock_ecc_strength;

extern void prefetch_next_page(void);

static struct sp_bch_info our_bch = {
	.regs = (void __iomem *)CONFIG_SP_BCH_BASE,
};

static int get_setbits(uint32_t n)
{
	n = n - ((n >> 1) & 0x55555555);
	n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
	return (((n + (n >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

int sp_bch_blank(void *ecc, int len)
{
	if (!ecc)
		return -EINVAL;

	uint8_t *oob = ecc;
	int mbe = 4;		/* Max. BIT errors */
	int ret = 0;
	int i, n;

	if (oob) {
		ret = 1;
		for (i = 2, n = 0; i < len; ++i) {
			if (oob[i] != 0xff)
				n += get_setbits(oob[i] ^ 0xff);
			if (n > mbe) {
				ret = 0;
				break;
			}
		}
	}

	oob[0] = 0xff;

	return ret;
}

#if defined(CONFIG_SP_PNG_DECODER)
int sp_png_dec_run(void);
#endif

static int sp_bch_wait(struct sp_bch_info *info)
{
	struct sp_bch_regs *regs;
	unsigned long now = get_timer(0);
	int ret = -1;

	if (!info)
		BUG();
	if (!info->regs)
		BUG();
	regs = info->regs;

	while (get_timer(now) < CFG_CMD_TIMEOUT_MS) {
#if defined(CONFIG_SP_PNG_DECODER)
		sp_png_dec_run();
#endif

		if (!(readl(&regs->isr) & ISR_BUSY)) {
			ret = 0;
			break;
		}
	}

	if (ret)
		printf("sp_bch: cmd timeout\n");

	return ret;
}

static int sp_bch_reset(struct sp_bch_info *info)
{
	struct sp_bch_regs *regs;
	unsigned long now = get_timer(0);
	int ret = -1;

	if (!info)
		BUG();
	if (!info->regs)
		BUG();
	regs = info->regs;

	writel(32 << 4, &regs->cr1);

	/* reset controller */
	writel(SRR_RESET, &regs->srr);
	while (get_timer(now) < CFG_CMD_TIMEOUT_MS) {
		if (!(readl(&regs->srr) & SRR_RESET)) {
			ret = 0;
			break;
		}
	}
	if (ret) {
		printf("sp_bch: reset timeout\n");
		return ret;
	}

	/* reset interrupts */
	writel(IER_DONE | IER_FAIL, &regs->ier);
	writel(ISR_BCH, &regs->isr);

	return 0;
}

int sp_bch_init(struct mtd_info *mtd)
{
	struct sp_bch_info *info = &our_bch;
	struct nand_chip *nand;;
	struct nand_oobfree *oobfree;

	int i;
	int oobsz;
	int pgsz;
	int rsvd;		/* Reserved bytes for YAFFS2 */
	int size;		/* BCH data length per sector */
	int bits;		/* BCH strength per sector */
	int nrps;		/* BCH parity sector number */
	int pssz;		/* BCH parity sector size */
	int free;		/* BCH free bytes per sector */

	if (!mtd || !info)
		BUG();

	if (!mtd->priv)
		BUG();

	rsvd = 32;
	oobsz = mtd->oobsize;
	pgsz = mtd->writesize;
	nand = mtd->priv;
	info->mtd = mtd;

	/* 1024x60 */
	size = 1024;
	bits = 60;
	nrps = pgsz >> 10;
	info->cr0 = CR0_CMODE_1024x60 | CR0_NBLKS(nrps);
	if (oobsz >= nrps * 128) {
		info->cr0 |= CR0_DMODE(0) | CR0_BMODE(5);
		pssz = 128;
		free = 23;
	} else if (oobsz >= nrps * 112) {
		info->cr0 |= CR0_DMODE(1) | CR0_BMODE(1);
		pssz = 112;
		free = 7;
	} else {
		pssz = 0;
		free = 0;
	}
	if (free * nrps >= rsvd)
		goto ecc_detected;

	/* 1024x40 */
	size = 1024;
	bits = 40;
	nrps = pgsz >> 10;
	info->cr0 = CR0_CMODE_1024x40 | CR0_NBLKS(nrps);
	if (oobsz >= nrps * 96) {
		info->cr0 |= CR0_DMODE(0) | CR0_BMODE(6);
		pssz = 96;
		free = 26;
	} else if (oobsz >= nrps * 80) {
		info->cr0 |= CR0_DMODE(1) | CR0_BMODE(2);
		pssz = 80;
		free = 10;
	} else {
		pssz = 0;
		free = 0;
	}
	if (free * nrps >= rsvd)
		goto ecc_detected;

	/* 1024x24 */
	size = 1024;
	bits = 24;
	nrps = pgsz >> 10;
	info->cr0 = CR0_CMODE_1024x24 | CR0_NBLKS(nrps);
	if (oobsz >= nrps * 64) {
		info->cr0 |= CR0_DMODE(0) | CR0_BMODE(5);
		pssz = 64;
		free = 22;
	} else if (oobsz >= nrps * 48) {
		info->cr0 |= CR0_DMODE(1) | CR0_BMODE(1);
		pssz = 48;
		free = 6;
	} else {
		pssz = 0;
		free = 0;
	}
	if (free * nrps >= rsvd)
		goto ecc_detected;

	/* 1024x16 */
	size = 1024;
	bits = 16;
	nrps = pgsz >> 10;
	info->cr0 = CR0_CMODE_1024x16 | CR0_NBLKS(nrps);
	if (oobsz >= nrps * 64) {
		info->cr0 |= CR0_DMODE(0) | CR0_BMODE(6);
		pssz = 64;
		free = 28;
	} else if (oobsz >= nrps * 48) {
		info->cr0 |= CR0_DMODE(1) | CR0_BMODE(4);
		pssz = 48;
		free = 20;
	} else {
		pssz = 0;
		free = 0;
	}
	if (free * nrps >= rsvd)
		goto ecc_detected;

	/* 512x8 */
	size = 512;
	bits = 8;
	nrps = pgsz >> 9;
	info->cr0 = CR0_CMODE_512x8 | CR0_NBLKS(nrps);
	if (oobsz >= nrps * 32) {
		info->cr0 |= CR0_DMODE(0) | CR0_BMODE(4);
		pssz = 32;
		free = 18;
	} else if (oobsz >= nrps * 16) {
		info->cr0 |= CR0_DMODE(1) | CR0_BMODE(0);
		pssz = 16;
		free = 2;
	} else {
		pssz = 0;
		free = 0;
	}
	if (free * nrps >= rsvd)
		goto ecc_detected;

	/* 512x4 */
	size = 512;
	bits = 4;
	nrps = pgsz >> 9;
	info->cr0 = CR0_CMODE_512x4 | CR0_NBLKS(nrps);
	if (oobsz >= nrps * 32) {
		info->cr0 |= CR0_DMODE(0) | CR0_BMODE(6);
		pssz = 32;
		free = 25;
	} else {
		info->cr0 |= CR0_DMODE(1) | CR0_BMODE(2);
		pssz = 16;
		free = 9;
	}

ecc_detected:
	debug("sp_bch: ecc mode=%ux%u, cr0=0x%x\n", size, bits, info->cr0);
	nand->ecc.size = size;
	nand->ecc.strength = bits;
	nand->ecc.steps = nrps;
	ispsp_chk4badblock_ecc_strength = nand->ecc.strength;
	ispsp_chk4badblock_ecc_size = nand->ecc.size;
#if 0
	if (size == 512)
		nand->ecc.bytes = (13 * bits + 7) / 8;
	else
		nand->ecc.bytes = (14 * bits + 7) / 8;
#else
	nand->ecc.bytes = ((12 + (size >> 9)) * bits + 7) >> 3;
#endif

	/* sanity check */
	if (nand->ecc.steps > MTD_MAX_OOBFREE_ENTRIES_LARGE)
		BUG();

	if (free * nrps < rsvd)
		BUG();

	nand->ecc.layout->oobavail = 0;
	oobfree = nand->ecc.layout->oobfree;
	for (i = 0; i < nand->ecc.steps; ++i) {
		oobfree->offset = i * pssz;
		oobfree->length = free;

		/* reserved bad block + scrambler marker */
		if (i == 0) {
			oobfree->offset += 2;
			oobfree->length -= 2;
		}

		if (oobfree->length) {
			nand->ecc.layout->oobavail += oobfree->length;
			++oobfree;
		}
	}

	pr_info("sp_bch: oob avail=%u\n", nand->ecc.layout->oobavail);

	sp_bch_reset(info);

#if defined(CONFIG_SP_NFTL) || defined(CONFIG_SP_BCH_REPORT)
	sp_bch_ftl_info = (unsigned int *)info;
	struct sp_bch_regs *regs = info->regs;
	regs->cr0 = info->cr0;
#endif
	return 0;
}

/*
 * Calculate BCH ecc code
 */
int sp_bch_encode(struct mtd_info *mtd, void *buf, void *ecc)
{
	struct sp_bch_info *info = &our_bch;
	struct sp_bch_regs *regs;
	int ret;

	if (!mtd || !info || !buf || !ecc)
		BUG();
	if (!info->regs)
		BUG();
	regs = info->regs;

	if (info->mtd != mtd)
		sp_bch_init(mtd);

	flush_dcache_range((ulong) buf, (ulong) buf + mtd->writesize);
	flush_dcache_range((ulong) ecc, (ulong) ecc + mtd->oobsize);

	writel((uint32_t) buf, &regs->buf);
	writel((uint32_t) ecc, &regs->ecc);

	writel(CR0_START | CR0_ENCODE | info->cr0, &regs->cr0);
	ret = sp_bch_wait(info);

	return ret;
}

int sp_bch_encode_1024x60(void *buf, void *ecc)
{
	struct sp_bch_info *info = &our_bch;
	struct sp_bch_regs *regs;
	int ret;

	if (!ecc || !info || !buf)
		BUG();
	if (!info->regs)
		BUG();
	regs = info->regs;

	flush_dcache_range((ulong) buf, (ulong) buf + 1024);
	flush_dcache_range((ulong) ecc, (ulong) ecc + 128);

	writel((uint32_t) buf, &regs->buf);
	writel((uint32_t) ecc, &regs->ecc);

	writel(CR0_START | CR0_ENCODE | CR0_CMODE_1024x60, &regs->cr0);

	ret = sp_bch_wait(info);
	return ret;
}

int sp_bch_decode_1024x60(void *buf, void *ecc)
{
	struct sp_bch_info *info = &our_bch;
	struct sp_bch_regs *regs;
	int ret;

	if (!ecc || !info || !buf)
		BUG();
	if (!info->regs)
		BUG();
	regs = info->regs;

	flush_dcache_range((ulong) buf, (ulong) buf + 1024);
	flush_dcache_range((ulong) ecc, (ulong) ecc + 128);

	writel((uint32_t) buf, &regs->buf);
	writel((uint32_t) ecc, &regs->ecc);

	writel(CR0_START | CR0_DECODE | CR0_CMODE_1024x60, &regs->cr0);

	ret = sp_bch_wait(info);
	return ret;
}

/*
 * Detect and correct bit errors
 */
int sp_bch_decode(struct mtd_info *mtd, void *buf, void *ecc)
{
	struct sp_bch_info *info = &our_bch;
	struct sp_bch_regs *regs;
	uint32_t status;
	int ret;
#ifdef CONFIG_SP_BCH_REPORT
	uint32_t bch_tmp;
#define BCH_ECC_BITS(x)		(((x) >> 8) & 0x7)
#endif
	if (!mtd || !buf || !ecc || !info)
		BUG();
	if (!info->regs)
		BUG();
	regs = info->regs;

	if (info->mtd != mtd)
		sp_bch_init(mtd);

	flush_dcache_range((ulong) buf, (ulong) buf + mtd->writesize);
	flush_dcache_range((ulong) ecc, (ulong) ecc + mtd->oobsize);

	writel((uint32_t) buf, &regs->buf);
	writel((uint32_t) ecc, &regs->ecc);
#ifdef CONFIG_SP_BCH_REPORT
	bch_tmp = readl(&regs->cr0);
	status = BCH_ECC_BITS(bch_tmp);	/* allowed_err_bits */
	switch (status) {
	case 0:
		bch_tmp = 60;
		break;
	case 1:
		bch_tmp = 40;
		break;
	case 2:
		bch_tmp = 24;
		break;
	case 3:
		bch_tmp = 16;
		break;
	case 4:
		bch_tmp = 8;
		break;
	case 5:
		bch_tmp = 4;
		break;
	default:
		bch_tmp = 0xfffe;
		break;
	}
	info->ecc_sts = bch_tmp;
#endif

	writel(CR0_START | CR0_DECODE | info->cr0, &regs->cr0);

#ifdef CONFIG_SP_NAND_SCRAMBLER
	prefetch_next_page();
#endif
	ret = sp_bch_wait(info);
	status = readl(&regs->sr);

	if (ret) {
		printf("sp_bch: decode timeout\n");
	} else if (status & SR_FAIL) {
#if 0
		if ((status & SR_BLANK_FF) || (status & SR_BLANK_00)) {
#else
		if (sp_bch_blank(ecc, mtd->oobsize)) {
#endif
#ifdef CONFIG_SP_BCH_REPORT
			info->ecc_sts |= (SR_ERR_MAX(status) << 16);
#endif
			ret = 0;
		} else {
			printf("sp_bch: decode failed,status:0x%x\n", status);
#ifdef CONFIG_SP_BCH_REPORT
			info->ecc_sts |= ((bch_tmp + 1) << 16);
#endif
			mtd->ecc_stats.failed += SR_ERR_BITS(status);
			sp_bch_reset(info);

			ret = -1;
		}
	} else {
#ifdef CONFIG_SP_BCH_REPORT
		info->ecc_sts |= (SR_ERR_MAX(status) << 16);
#endif
		mtd->ecc_stats.corrected += SR_ERR_BITS(status);
		ispsp_chk4badblock_err_bit_in_page =
		    (uint32_t) (readl(&regs->esr));
		if (ispsp_chk4badblock_err_bit_in_page >
		    ispsp_chk4badblock_max_err_bit_in_page)
			ispsp_chk4badblock_max_err_bit_in_page =
			    ispsp_chk4badblock_err_bit_in_page;
	}

	return ret;
}
