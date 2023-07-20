#include <common.h>
#include <malloc.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <asm/io.h>
#include <nand.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <mach/gpio_drv.h>
#include <cpu_func.h>

#include <linux/dma-mapping.h>
#include <linux/mtd/bbm.h>

#include "sp_paranand.h"

void sp_pnand_regdump(struct nand_chip *nand)
{
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	volatile u32 val;
	u32 i;
	printk("===================================================\n");
	printk("0x0000: ");
	for(i = 0; i< 0x50; i=i+4){
		if(i != 0 && (i%0x10)==0){
			printk("\n");
			printk("0x%04x: ", i);
		}
		val = readl(info->io_base + i);
		printk("0x%08x ", val);
	}
	for(i = 0x100; i< 0x1B0; i=i+4){
		if(i != 0 && (i%0x10)==0){
			printk("\n");
			printk("0x%04x: ", i);
		}
		val = readl(info->io_base + i);
		printk("0x%08x ", val);
	}
	for(i = 0x200; i< 0x530; i=i+4){
		if(i != 0 && (i%0x10)==0){
			printk("\n");
			printk("0x%04x: ", i);
		}
		val = readl(info->io_base + i);
		printk("0x%08x ", val);
	}
	printk("\n===================================================\n");
}

static inline void sp_pnand_set_row_col_addr(
		struct sp_pnand_info *info, int row, int col)
{
	int val;

	val = readl(info->io_base + MEM_ATTR_SET);
	val &= ~(0x7 << 12);
	val |= (ATTR_ROW_CYCLE(row) | ATTR_COL_CYCLE(col));

	writel(val, info->io_base + MEM_ATTR_SET);
}

static int sp_pnand_check_cmdq(struct nand_chip *nand)
{
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	unsigned long timeo = get_timer(0);
	u32 status;
	int ret;

	ret = -EIO;
	timeo += CONFIG_SYS_HZ;
	while (get_timer(0) < timeo) {
		status = readl(info->io_base + CMDQUEUE_STATUS);
		if ((status & CMDQUEUE_STATUS_FULL(info->cur_chan)) == 0) {
			ret = 0;
			break;
		}
		cond_resched();
	}
	if(ret != 0)
		printk("check cmdq timeout");
	return ret;
}

static void sp_pnand_soft_reset(struct nand_chip *nand)
{
	struct sp_pnand_info *info = nand_get_controller_data(nand);

	writel((1 << info->cur_chan), info->io_base + NANDC_SW_RESET);
	// Wait for the NANDC024 software reset is complete
	while(readl(info->io_base + NANDC_SW_RESET) & (1 << info->cur_chan)) ;
}

void sp_pnand_abort(struct nand_chip *nand)
{
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct cmd_feature cmd_f;

	// Abort the operation

	// Step1. Flush Command queue & Poll whether Command queue is ready
	writel((1 << info->cur_chan), info->io_base + CMDQUEUE_FLUSH);
	// Wait until flash is ready!!
	while(!(readl(info->io_base + DEV_BUSY) & (1 << info->cur_chan))) ;

	// Step2. Reset Nandc & Poll whether the "Reset of NANDC" returns to 0
	sp_pnand_soft_reset(nand);

	// Step3. Reset the BMC region
	writel(1 << (info->cur_chan), info->io_base + BMC_REGION_SW_RESET);

	// Step4. Reset the AHB data slave port 0
	if (readl(info->io_base + FEATURE_1) & AHB_SLAVE_MODE_ASYNC(0)) {
		writel(1 << 0, info->io_base + AHB_SLAVE_RESET);
		while(readl(info->io_base + AHB_SLAVE_RESET) & (1 << 0)) ;
	}

	// Step5. Issue the Reset cmd to flash
	cmd_f.cq1 = 0;
	cmd_f.cq2 = 0;
	cmd_f.cq3 = 0;
	cmd_f.cq4 = CMD_COMPLETE_EN | CMD_FLASH_TYPE(info->flash_type) |\
	            CMD_START_CE(info->sel_chip);
	if (info->flash_type == ONFI2 || info->flash_type == ONFI3)
		cmd_f.cq4 |= CMD_INDEX(ONFI_FIXFLOW_SYNCRESET);
	else
		cmd_f.cq4 |= CMD_INDEX(FIXFLOW_RESET);

	sp_pnand_issue_cmd(nand, &cmd_f);

	sp_pnand_wait(mtd, nand);
}

int sp_pnand_issue_cmd(struct nand_chip *nand, struct cmd_feature *cmd_f)
{
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	int status;

	status = sp_pnand_check_cmdq(nand);
	if (status == 0) {
		sp_pnand_set_row_col_addr(info, cmd_f->row_cycle, cmd_f->col_cycle);

		writel(cmd_f->cq1, info->io_base + CMDQUEUE1(info->cur_chan));
		writel(cmd_f->cq2, info->io_base + CMDQUEUE2(info->cur_chan));
		writel(cmd_f->cq3, info->io_base + CMDQUEUE3(info->cur_chan));
		writel(cmd_f->cq4, info->io_base + CMDQUEUE4(info->cur_chan)); // Issue cmd
	}
	return status;
}

void sp_pnand_set_default_timing(struct nand_chip *nand)
{
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	int i;
	u32 timing[4];
#if 1
	timing[0] = 0x0f1f0f1f;
	timing[1] = 0x00007f7f;
	timing[2] = 0x7f7f7f7f;
	timing[3] = 0xff1f001f;
#else
	timing[0] = 0x02020204;
	timing[1] = 0x00001401;
	timing[2] = 0x0c140414;
	timing[3] = 0x00040014;
#endif
	for (i = 0;i < MAX_CHANNEL;i++) {
		writel(timing[0], info->io_base + FL_AC_TIMING0(i));
		writel(timing[1], info->io_base + FL_AC_TIMING1(i));
		writel(timing[2], info->io_base + FL_AC_TIMING2(i));
		writel(timing[3], info->io_base + FL_AC_TIMING3(i));
	}
}

int BMC_region_status_full(struct sp_pnand_info *info)
{
	if (((readl(info->io_base + BMC_REGION_STATUS) >> (info->cur_chan + 8)) & 0x1) == 1) {
		return 1;	// It's full
	} else {
		return 0;	// It's not full
	}
}

int BMC_region_status_empty(struct sp_pnand_info *info)
{
	if (((readl(info->io_base + BMC_REGION_STATUS) >> info->cur_chan) & 0x1) == 1) {
		return 1;	// It's empty
	} else {
		return 0;	// It's not empty
	}
}


int sp_pnand_wait(struct mtd_info *mtd, struct nand_chip *nand)
{
	struct sp_pnand_info *info = get_pnand_info();

	unsigned long timeo;
	int ret;
	volatile u32 intr_sts, ecc_intr_sts;
	volatile u8 cmd_comp_sts, sts_fail_sts;
	volatile u8 ecc_sts_for_data;
	volatile u8 ecc_sts_for_spare;

	info->cmd_status = CMD_SUCCESS;
	ret = NAND_STATUS_FAIL;
	timeo = get_timer(0);
	timeo += 5 * CONFIG_SYS_HZ;

	//No command
	if (readl(info->io_base + CMDQUEUE_STATUS) & CMDQUEUE_STATUS_EMPTY(info->cur_chan)) {
		ret = NAND_STATUS_READY;
		goto out;
	}

	do {
		intr_sts = readl(info->io_base + INTR_STATUS);
		cmd_comp_sts = ((intr_sts & 0xFF0000) >> 16);

		if (likely(cmd_comp_sts & (1 << (info->cur_chan)))) {
			// Clear the intr status when the cmd complete occurs.
			writel(intr_sts, info->io_base + INTR_STATUS);

			ret = NAND_STATUS_READY;
			sts_fail_sts = (intr_sts & 0xFF);

			if (sts_fail_sts & (1 << (info->cur_chan))) {
				printk(KERN_ERR "STATUS FAIL@(pg_addr:0x%x)\n", info->page_addr);
				info->cmd_status |= CMD_STATUS_FAIL;
				//printk(">>>>>>%s(%d)   cmd_status    %d\n",__FUNCTION__,__LINE__, info->cmd_status);
				ret = CMD_STATUS_FAIL;
				sp_pnand_abort(nand);
			}

			if (info->read_state) {
				ecc_intr_sts = readl(info->io_base + ECC_INTR_STATUS);
				// Clear the ECC intr status
				writel(ecc_intr_sts, info->io_base + ECC_INTR_STATUS);
				// ECC failed on data
				ecc_sts_for_data = (ecc_intr_sts & 0xFF);
				if (ecc_sts_for_data & (1 << info->cur_chan)) {
					info->cmd_status |= CMD_ECC_FAIL_ON_DATA;
					ret = NAND_STATUS_FAIL;
					//printk(">>>>>>%s(%d)   cmd_status    %d\n",__FUNCTION__,__LINE__, info->cmd_status);
				}

				ecc_sts_for_spare = ((ecc_intr_sts & 0xFF0000) >> 16);
				// ECC failed on spare
				if (ecc_sts_for_spare & (1 << info->cur_chan)) {
					info->cmd_status |= CMD_ECC_FAIL_ON_SPARE;
					ret = NAND_STATUS_FAIL;
					//printk(">>>>>>%s(%d)   cmd_status    %d\n",__FUNCTION__,__LINE__, info->cmd_status);
				}
			}
			goto out;
		}
		cond_resched();
	} while (get_timer(0) < timeo);

	DBGLEVEL1(sp_pnand_dbg("nand wait time out\n"));
	sp_pnand_regdump(nand);
out:
	return ret;
}

void sp_pnand_fill_prog_code(struct nand_chip *nand, int location,
		int cmd_index) {
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	writeb((cmd_index & 0xff), info->io_base + PROGRAMMABLE_OPCODE+location);
}

void sp_pnand_fill_prog_flow(struct nand_chip *nand, int *program_flow_buf,
		int buf_len) {
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	u8 *p = (u8 *)program_flow_buf;

	int i;
	for(i = 0; i < buf_len; i++) {
		writeb( *(p+i), info->io_base + PROGRAMMABLE_FLOW_CONTROL + i);
	}
}

int byte_rd(struct nand_chip *nand, int real_pg, int col, int len,
				u_char *spare_buf)
{
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct cmd_feature cmd_f = {0};
	int status, i, j, tmp_col, tmp_len, cmd_len, ret;
	u_char *buf;

	ret = 0;
	tmp_col = col;
	tmp_len = len;

	if(info->flash_type == TOGGLE1 || info->flash_type == TOGGLE2 ||
		info->flash_type == ONFI2 || info->flash_type == ONFI3) {
		if(col & 0x1) {
			tmp_col --;

			if (tmp_len & 0x1)
				tmp_len ++;
			else
				tmp_len += 2;
		}
		else if(tmp_len & 0x1) {
			tmp_len ++;
		}
	}

	buf = (u_char *)vmalloc(tmp_len);

	for(i = 0; i < tmp_len; i += info->max_spare) {
		if(tmp_len - i >= info->max_spare)
			cmd_len = info->max_spare;
		else
			cmd_len = tmp_len - i;

		cmd_f.row_cycle = ROW_ADDR_3CYCLE;
		cmd_f.col_cycle = COL_ADDR_2CYCLE;
		cmd_f.cq1 = real_pg | SCR_SEED_VAL1(info->seed_val);
		cmd_f.cq2 = CMD_EX_SPARE_NUM(cmd_len) | SCR_SEED_VAL2(info->seed_val);
		cmd_f.cq3 = CMD_COUNT(1) | tmp_col;
		cmd_f.cq4 = CMD_COMPLETE_EN | CMD_BYTE_MODE |\
				CMD_FLASH_TYPE(info->flash_type) |\
				CMD_START_CE(info->sel_chip) | CMD_SPARE_NUM(cmd_len) |\
				CMD_INDEX(LARGE_FIXFLOW_BYTEREAD);

		status = sp_pnand_issue_cmd(nand, &cmd_f);
		if(status < 0) {
			ret = 1;
			break;
		}
		sp_pnand_wait(mtd, nand);
		for(j = 0; j < (cmd_len + 3) / 4; j++) {
			memcpy(buf + i + 4 * j, info->io_base + SPARE_SRAM + 4 * j, 4);
			if (j  == ((cmd_len + 3) / 4 - 1))
				memcpy(buf + i + 4* j, info->io_base + SPARE_SRAM + 4 * j, cmd_len % 4);
		}
		tmp_col += cmd_len;
	}

	if(info->flash_type == TOGGLE1 || info->flash_type == TOGGLE2 ||
		info->flash_type == ONFI2 || info->flash_type == ONFI3) {
		if(col & 0x1)
			memcpy(spare_buf, buf + 1, len);
		else
			memcpy(spare_buf, buf, len);
	}
	else
		memcpy(spare_buf, buf, len);

	vfree(buf);
	return ret;
}

int rd_pg_w_oob(struct nand_chip *nand, int real_pg,
			uint8_t *data_buf, u8 *spare_buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	struct cmd_feature cmd_f;
	int status;
	int i;
	u32 *lbuf;
	u32 data_size;
	//unsigned long timeo;

	//DBGLEVEL2(sp_pnand_dbg("%s %d\n", __FUNCTION__, __LINE__));
	cmd_f.row_cycle = ROW_ADDR_3CYCLE;
	cmd_f.col_cycle = COL_ADDR_2CYCLE;
	cmd_f.cq1 = real_pg | SCR_SEED_VAL1(info->seed_val);
	cmd_f.cq2 = CMD_EX_SPARE_NUM(info->spare) | SCR_SEED_VAL2(info->seed_val);
	cmd_f.cq3 = CMD_COUNT(info->sector_per_page) |\
			(info->column & 0xFF);
	cmd_f.cq4 = CMD_COMPLETE_EN | CMD_FLASH_TYPE(info->flash_type) |\
			CMD_START_CE(info->sel_chip) | CMD_SPARE_NUM(info->spare) |\
			CMD_INDEX(LARGE_FIXFLOW_PAGEREAD_W_SPARE);

	status = sp_pnand_issue_cmd(nand, &cmd_f);
	if(status < 0)
		return 1;

	sp_pnand_wait(mtd, nand);
#if 0
	timeo = get_timer(0);
	timeo += CONFIG_SYS_HZ / 10;//100ms delay

	// Read the sector sized data from BMC when every time the bmc region isn't empty
	do {
	} while(BMC_region_status_empty(info) && (get_timer(0) < timeo));
#endif
	data_size = info->sector_per_page << info->eccbasft;
	if(!BMC_region_status_empty(info)) {
		lbuf = (u32 *)data_buf;
		for (i = 0; i < data_size; i += 4)
			*lbuf++ = *(volatile unsigned *)(nand->IO_ADDR_R);
	} else {
		printk(KERN_ERR "Transfer timeout!");
	}

	for(i = 0; i < mtd->oobsize / 4; i++) {
		memcpy(spare_buf + 4 * i, info->io_base + SPARE_SRAM + 4 * i, 4);
	}

	return 0;
}

int rd_pg_w_oob_sp(struct nand_chip *nand, int real_pg,
			uint8_t *data_buf, u8 *spare_buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	struct cmd_feature cmd_f;
	int status, i;
	u32 *lbuf;

	cmd_f.row_cycle = ROW_ADDR_2CYCLE;
	cmd_f.col_cycle = COL_ADDR_1CYCLE;
	cmd_f.cq1 = real_pg | SCR_SEED_VAL1(info->seed_val);
	cmd_f.cq2 = SCR_SEED_VAL2(info->seed_val);
	cmd_f.cq3 = CMD_COUNT(1) | (info->column & 0xFF);
	cmd_f.cq4 = CMD_COMPLETE_EN | CMD_FLASH_TYPE(info->flash_type) |\
			CMD_START_CE(info->sel_chip) | CMD_SPARE_NUM(info->spare) |\
			CMD_INDEX(SMALL_FIXFLOW_PAGEREAD);

	status = sp_pnand_issue_cmd(nand, &cmd_f);
	if (status < 0)
		return 1;

	sp_pnand_wait(mtd, nand);

	if(!BMC_region_status_empty(info)) {
		lbuf = (u32 *)data_buf;
		for (i = 0; i < mtd->writesize; i += 4)
			*lbuf++ = *(volatile unsigned *)(nand->IO_ADDR_R);
	} else {
		printk(KERN_ERR "Transfer timeout!");
	}

	for(i = 0; i < mtd->oobsize / 4; i++) {
		memcpy(spare_buf + 4 * i, info->io_base + SPARE_SRAM + 4 * i, 4);
	}

	return 0;
}

int rd_oob(struct nand_chip *nand, int real_pg, u8 *spare_buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	struct cmd_feature cmd_f;
	int status, i;

	cmd_f.row_cycle = ROW_ADDR_3CYCLE;
	cmd_f.col_cycle = COL_ADDR_2CYCLE;
	cmd_f.cq1 = real_pg | SCR_SEED_VAL1(info->seed_val);
	cmd_f.cq2 = CMD_EX_SPARE_NUM(info->spare) | SCR_SEED_VAL2(info->seed_val);
	cmd_f.cq3 = CMD_COUNT(1);
	cmd_f.cq4 = CMD_COMPLETE_EN | CMD_FLASH_TYPE(info->flash_type) |\
			CMD_START_CE(info->sel_chip) | CMD_SPARE_NUM(info->spare) |\
			CMD_INDEX(LARGE_FIXFLOW_READOOB);

	status = sp_pnand_issue_cmd(nand, &cmd_f);
	if (status < 0)
		return 1;

	sp_pnand_wait(mtd, nand);

	for(i = 0; i < mtd->oobsize / 4; i++) {
		memcpy(spare_buf + 4 * i, info->io_base + SPARE_SRAM + 4 * i, 4);
	}

	return 0;
}

static int rd_oob_sp(struct nand_chip *nand, int real_pg, u8 *spare_buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	struct cmd_feature cmd_f;
	int status, i;

	cmd_f.row_cycle = ROW_ADDR_2CYCLE;
	cmd_f.col_cycle = COL_ADDR_1CYCLE;
	cmd_f.cq1 = real_pg | SCR_SEED_VAL1(info->seed_val);
	cmd_f.cq2 = SCR_SEED_VAL2(info->seed_val);
	cmd_f.cq3 = CMD_COUNT(1);
	cmd_f.cq4 = CMD_COMPLETE_EN | CMD_FLASH_TYPE(info->flash_type) |\
			CMD_START_CE(info->sel_chip) | CMD_SPARE_NUM(info->spare) |\
			CMD_INDEX(SMALL_FIXFLOW_READOOB);

	status = sp_pnand_issue_cmd(nand, &cmd_f);
	if (status < 0)
		return 1;

	sp_pnand_wait(mtd, nand);

	for(i = 0; i < mtd->oobsize / 4; i++) {
		memcpy(spare_buf + 4 * i, info->io_base + SPARE_SRAM + 4 * i, 4);
	}

	return 0;
}

int rd_pg(struct nand_chip *nand, int real_pg, uint8_t *data_buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	struct cmd_feature cmd_f;
	int status;
	u32 *lbuf;
	u32 data_size;
	int i;

	cmd_f.row_cycle = ROW_ADDR_3CYCLE;
	cmd_f.col_cycle = COL_ADDR_2CYCLE;
	cmd_f.cq1 = real_pg | SCR_SEED_VAL1(info->seed_val);
	cmd_f.cq2 = CMD_EX_SPARE_NUM(info->spare) | SCR_SEED_VAL2(info->seed_val);
	cmd_f.cq3 = CMD_COUNT(info->sector_per_page) | (info->column & 0xFF);
	cmd_f.cq4 = CMD_COMPLETE_EN | CMD_FLASH_TYPE(info->flash_type) |\
			CMD_START_CE(info->sel_chip) | CMD_SPARE_NUM(info->spare) |\
			CMD_INDEX(LARGE_FIXFLOW_PAGEREAD);

	status = sp_pnand_issue_cmd(nand, &cmd_f);
	if(status < 0)
		return 1;

	sp_pnand_wait(mtd, nand);

	data_size = info->sector_per_page << info->eccbasft;
	if(!BMC_region_status_empty(info)) {
		lbuf = (u32 *)data_buf;
		for (i = 0; i < data_size; i += 4)
			*lbuf++ = *(volatile unsigned *)(nand->IO_ADDR_R);
	} else {
		printk(KERN_ERR "Transfer timeout!");
	}

	return 0;
}

int rd_pg_sp(struct nand_chip *nand, int real_pg, uint8_t *data_buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	struct cmd_feature cmd_f;
	int progflow_buf[3];
	int status;
	int i;
	u32 *lbuf;
	//unsigned long timeo;

	cmd_f.row_cycle = ROW_ADDR_2CYCLE;
	cmd_f.col_cycle = COL_ADDR_1CYCLE;
	cmd_f.cq1 = real_pg | SCR_SEED_VAL1(info->seed_val);
	cmd_f.cq2 = SCR_SEED_VAL2(info->seed_val);
	cmd_f.cq3 = CMD_COUNT(1) | (info->column & 0xFF);
	cmd_f.cq4 = CMD_COMPLETE_EN | CMD_FLASH_TYPE(info->flash_type) |\
			CMD_START_CE(info->sel_chip);

	progflow_buf[0] = 0x66414200;
	progflow_buf[1] = 0x66626561;
	progflow_buf[2] = 0x000067C8;
	sp_pnand_fill_prog_flow(nand, progflow_buf, 10);
	cmd_f.cq4 |= CMD_PROM_FLOW | CMD_INDEX(0x0);

	status = sp_pnand_issue_cmd(nand, &cmd_f);
	if (status < 0)
		return 1;

	sp_pnand_wait(mtd, nand);
#if 0
	timeo = get_timer(0);
	timeo += CONFIG_SYS_HZ / 10;//100ms delay

	do {
	} while(BMC_region_status_empty(info) && (get_timer(0) < timeo));
#endif
	if(!BMC_region_status_empty(info)) {
		lbuf = (u32 *)data_buf;
		for (i = 0; i < mtd->writesize; i += 4)
			*lbuf++ = *(volatile unsigned *)(nand->IO_ADDR_R);
	} else {
		printk(KERN_ERR "Transfer timeout!");
	}

	return 0;
}

int sp_pnand_check_bad_spare(struct nand_chip *nand, int pg)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	int spare_phy_start, spare_phy_len, eccbyte;
	int errbit_num, i, j, ret;
	int sec_num = (mtd->writesize >> info->eccbasft);
	u_char *spare_buf;
	int ecc_corr_bit_spare, chan;

	eccbyte = (info->useecc * 14) / 8;
	if (((info->useecc * 14) % 8) != 0)
		eccbyte++;

	// The amount of data-payload in each (sector+sector_parity) or
	// (spare + spare_parity) on Toggle/ONFI mode must be even.
	if(info->flash_type == TOGGLE1 || info->flash_type == TOGGLE2 ||
		info->flash_type == ONFI2 || info->flash_type == ONFI3) {
		if (eccbyte & 0x1)
			eccbyte ++;
	}

	if(info->sector_per_page < (mtd->writesize >> info->eccbasft))
		spare_phy_start = mtd->writesize;
	else
		spare_phy_start = mtd->writesize + (eccbyte * sec_num) + CONFIG_BI_BYTE;

	eccbyte = (info->useecc_spare * 14) / 8;
	if (((info->useecc_spare * 14) % 8) != 0)
		eccbyte++;

	if(info->flash_type == TOGGLE1 || info->flash_type == TOGGLE2 ||
		info->flash_type == ONFI2 || info->flash_type == ONFI3) {
		if (eccbyte & 0x1)
			eccbyte ++;
	}
	spare_phy_len = info->spare + eccbyte;
	spare_buf = vmalloc(spare_phy_len);

	ret = 0;
	errbit_num = 0;

	//*(volatile unsigned int *)0xf8000000 = (unsigned int)(0xabcd1234);

	if(!byte_rd(nand, pg, spare_phy_start, spare_phy_len, spare_buf)) {

		for(i = 0; i < spare_phy_len; i++) {
			if(*(spare_buf + i) != 0xFF) {
				for(j = 0; j < 8; j ++) {
					if((*(spare_buf + i) & (0x1 << j)) == 0)
						errbit_num ++;
				}
				printk("xt_debug i= %d", i);///
				printk("data= 0x%x\n", *(spare_buf + i));///
			}
		}
		if (errbit_num != 0) {
			if (info->cur_chan < 4) {
				chan = (info->cur_chan << 3);
				ecc_corr_bit_spare = (readl(info->io_base + ECC_CORRECT_BIT_FOR_SPARE_REG1) >> chan) & 0x7F;
			}
			else {
				chan = (info->cur_chan - 4) << 3;
				ecc_corr_bit_spare = (readl(info->io_base + ECC_CORRECT_BIT_FOR_SPARE_REG2) >> chan) & 0x7F;
			}
			printk("spare_phy_len = %d, errbit_num = %d\n", spare_phy_len, errbit_num);

			if(errbit_num > ecc_corr_bit_spare + 1)
				ret = 1;
		}
	}
	else
		ret = 1;

	vfree(spare_buf);
	return ret;

}


int sp_pnand_read_page(struct mtd_info *mtd, struct nand_chip *nand,
			      uint8_t *buf, int oob_required, int page)
{
	struct sp_pnand_info *info = nand_get_controller_data(nand);

	info->page_addr = page;

	return info->read_page(nand, buf);
}

int sp_pnand_write_page_lowlevel(struct mtd_info *mtd,
				 struct nand_chip *nand, const uint8_t *buf,
				 int oob_required, int page)
{
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	int status = 0;

//	DBGLEVEL2(sp_pnand_dbg ("w_2: ch = %d, ce = %d, page = 0x%x, size = %d, info->column = %d\n",
//				info->cur_chan, info->sel_chip,  page, mtd->writesize, info->column));
	info->page_addr = page;
	status = info->write_page(nand, buf);
	if (status < 0)
		return status;

	// Returning the any value isn't allowed, except 0, -EBADMSG, or -EUCLEAN
	return 0;
}

int sp_pnand_read_oob_std(struct mtd_info *mtd, struct nand_chip *nand, int page)
{
	struct sp_pnand_info *info = nand_get_controller_data(nand);

	info->page_addr = page;

	return info->read_oob(nand, nand->oob_poi);
}

int sp_pnand_write_oob_std(struct mtd_info *mtd, struct nand_chip *nand, int page)
{
	struct sp_pnand_info *info = nand_get_controller_data(nand);

	DBGLEVEL2(sp_pnand_dbg("write oob only to page = 0x%x\n", page));
	info->page_addr = page;

	return info->write_oob(nand, nand->oob_poi, mtd->oobsize);
}

int sp_pnand_read_page_lp(struct nand_chip *nand, uint8_t *buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	int status = 0, chk_data_0xFF, chk_spare_0xFF;
	int i, ecc_original_setting, generic_original_setting, val;
	int real_pg;
	u8  data_empty, spare_empty;
	u32 *lbuf;
	u32 data_size;

	real_pg = info->page_addr;
#if 0
	DBGLEVEL2(sp_pnand_dbg
		("r: ch = %d, ce = %d, page = 0x%x, real = 0x%x, size = %d, info->column = %d\n",
		info->cur_chan, info->sel_chip, info->page_addr, real_pg, mtd->writesize, info->column));
#endif
	info->read_state = 1;
retry:
	chk_data_0xFF = chk_spare_0xFF = 0;
	data_empty = spare_empty = 0;

	if(!rd_pg_w_oob(nand, real_pg, buf, nand->oob_poi)) {
		if (info->cmd_status &
			(CMD_ECC_FAIL_ON_DATA | CMD_ECC_FAIL_ON_SPARE)) {
			// Store the original setting
			ecc_original_setting = readl(info->io_base + ECC_CONTROL);
			generic_original_setting = readl(info->io_base + GENERAL_SETTING);
			// Disable the ECC engine & HW-Scramble, temporarily.
			val = readl(info->io_base + ECC_CONTROL);
			val = val & ~(ECC_EN(0xFF));
			writel(val, info->io_base + ECC_CONTROL);
			val = readl(info->io_base + GENERAL_SETTING);
			val &= ~DATA_SCRAMBLER;
			writel(val, info->io_base + GENERAL_SETTING);

			if(info->cmd_status==(CMD_ECC_FAIL_ON_DATA|CMD_ECC_FAIL_ON_SPARE))
			{
				if(!rd_pg_w_oob(nand, real_pg, buf, nand->oob_poi)) {
					chk_data_0xFF = chk_spare_0xFF = 1;
					data_empty = spare_empty = 1;
				}
			}
			else if(info->cmd_status == CMD_ECC_FAIL_ON_DATA) {
				if(!rd_pg(nand, real_pg, buf)) {
					chk_data_0xFF = 1;
					data_empty = 1;
				}
			}
			else if(info->cmd_status == CMD_ECC_FAIL_ON_SPARE) {
				if(!rd_oob(nand, real_pg, nand->oob_poi)) {
					chk_spare_0xFF = 1;
					spare_empty = 1;
				}
			}

			// Restore the ecc original setting & generic original setting.
			writel(ecc_original_setting, info->io_base + ECC_CONTROL);
			writel(generic_original_setting, info->io_base + GENERAL_SETTING);

			if(chk_data_0xFF == 1) {

				lbuf = (u32 *)buf;
#if 0//debug
				if(real_pg == 0) {
					printk("Dump Page 0:\n");
					data_size = info->sector_per_page << info->eccbasft;
					for (i = 0; i < (data_size >> 4); i++) {
						printk("%04xh:", 16*i);
						for(int j = 0; j < 16; j++) {
							printk("%02x ", *((u8 *)lbuf+j+16*i));
						}
						printk("\n");
					}
				}
#endif
				data_size = info->sector_per_page << info->eccbasft;
				for (i = 0; i < (data_size >> 2); i++) {
					if (*(lbuf + i) != 0xFFFFFFFF) {
						printk(KERN_ERR "22ECC err @ page0x%x real:0x%x\n",
							info->page_addr, real_pg);
						data_empty = 0;
						break;
					}
				}
				if(data_empty == 1)
					DBGLEVEL2(sp_pnand_dbg("Data Real 0xFF\n"));
			}
			if(chk_spare_0xFF == 1) {
				//lichun@add, If BI_byte test
				if (readl(info->io_base + MEM_ATTR_SET) & BI_BYTE_MASK) {
					for (i = 0; i < mtd->oobsize; i++) {
						if (*(nand->oob_poi + i) != 0xFF) {
							printk(KERN_ERR "1ECC err for spare(Read page) @");
							printk(KERN_ERR	"ch:%d ce:%d page0x%x real:0x%x\n",
								info->cur_chan, info->sel_chip, info->page_addr, real_pg);
							spare_empty = 0;
							break;
						}
					}
				}
				else {
				//~lichun
					if(sp_pnand_check_bad_spare(nand, info->page_addr)) {
						printk(KERN_ERR "2ECC err for spare(Read page) @");
						printk(KERN_ERR	"ch:%d ce:%d page0x%x real:0x%x\n",
							info->cur_chan, info->sel_chip, info->page_addr, real_pg);
						spare_empty = 0;
					}
				}

				if(spare_empty == 1)
					DBGLEVEL2(sp_pnand_dbg("Spare Real 0xFF\n"));
			}
			if( (chk_data_0xFF == 1 && data_empty == 0) ||
				(chk_spare_0xFF == 1 && spare_empty == 0) ) {

				if(info->set_param != NULL) {
					if(info->set_param(nand) == 1)
						goto retry;
				}

				mtd->ecc_stats.failed++;
				status = -1;

			}
		}
	}
	info->read_state = 0;

	if(info->terminate != NULL)
		info->terminate(nand);
	// Returning the any value isn't allowed, except 0, -EBADMSG, or -EUCLEAN
	return 0;
}


int sp_pnand_write_page_lp(struct nand_chip *nand, const uint8_t *buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	struct cmd_feature cmd_f;
	u8 *p, w_wo_spare = 1;
	u32 *lbuf;
	int real_pg;
	int i, status = 0;
	u32 data_size;

	real_pg = info->page_addr;

	DBGLEVEL2(sp_pnand_dbg (
		"w: ch = %d, ce = %d, page = 0x%x, real page:0x%x size = %d, info->column = %d\n",
		info->cur_chan, info->sel_chip,  info->page_addr, real_pg, mtd->writesize, info->column));

	p = nand->oob_poi;
	for(i = 0; i < mtd->oobsize; i++) {
		if( *( p + i) != 0xFF) {
			w_wo_spare = 0;
			break;
		}
	}

	cmd_f.row_cycle = ROW_ADDR_3CYCLE;
	cmd_f.col_cycle = COL_ADDR_2CYCLE;
	cmd_f.cq1 = real_pg | SCR_SEED_VAL1(info->seed_val);
	cmd_f.cq2 = CMD_EX_SPARE_NUM(info->spare) | SCR_SEED_VAL2(info->seed_val);
	cmd_f.cq3 = CMD_COUNT(info->sector_per_page) |
			(info->column & 0xFF);
	cmd_f.cq4 = CMD_COMPLETE_EN | CMD_FLASH_TYPE(info->flash_type) | \
			CMD_START_CE(info->sel_chip) | CMD_SPARE_NUM(info->spare);

	if(w_wo_spare == 0) {
		for(i = 0; i < mtd->oobsize / 4; i++) {
			memcpy(info->io_base + SPARE_SRAM + 4 * i, p + 4 * i, 4);
		}
		cmd_f.cq4 |= CMD_INDEX(LARGE_PAGEWRITE_W_SPARE);
	}
	else {
		cmd_f.cq4 |= CMD_INDEX(LARGE_PAGEWRITE);
	}

	status = sp_pnand_issue_cmd(nand, &cmd_f);
	if (status < 0)
		goto out;

	data_size = info->sector_per_page << info->eccbasft;
	if(!BMC_region_status_full(info)) {
		lbuf = (u32 *)buf;
		for (i = 0; i < data_size; i += 4)
			*(volatile unsigned *)(nand->IO_ADDR_R) = *lbuf++;
	} else {
		printk(KERN_ERR "Transfer timeout!");
	}

	if (sp_pnand_wait(mtd, nand) == NAND_STATUS_FAIL) {
		status = -EIO;
		printk("FAILED\n");
	}
out:
	return status;
}

int sp_pnand_read_oob_lp(struct nand_chip *nand, u8 *buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	int status = 0, i, ecc_original_setting, generic_original_setting, val;
	int real_pg, empty;

	real_pg = info->page_addr;

	DBGLEVEL2(sp_pnand_dbg(
		"read_oob: ch = %d, ce = %d, page = 0x%x, real: 0x%x, size = %d\n",
		info->cur_chan, info->sel_chip, info->page_addr, real_pg, mtd->writesize));

	//*(volatile unsigned int *)0xf8000000 = (unsigned int)(0xabcd1234);

	info->read_state = 1;
	if(!rd_oob(nand, real_pg, buf)) { //first read oob
		if(info->cmd_status & CMD_ECC_FAIL_ON_SPARE) { // spare ecc error
			//xt.hu@20220627
			// Store the original setting
			ecc_original_setting = readl(info->io_base + ECC_CONTROL);
			generic_original_setting = readl(info->io_base + GENERAL_SETTING);
			// Disable the ECC engine & HW-Scramble, temporarily.
			val = readl(info->io_base + ECC_CONTROL);
			val = val & ~(ECC_EN(0xFF));
			writel(val, info->io_base + ECC_CONTROL);
			val = readl(info->io_base + GENERAL_SETTING);
			val &= ~DATA_SCRAMBLER;
			writel(val, info->io_base + GENERAL_SETTING);
			//xt.hu@20220627:read oob again
			if(!rd_oob(nand, real_pg, buf)) {
				empty = 1;
				for (i = 0; i < mtd->oobsize; i++) {
					if (*(buf + i) != 0xFF) { //reading data is non-FF could confirm real ecc error
						printk(KERN_ERR "ECC err for spare(Read oob) @");
						printk(KERN_ERR "oob i: %d, data: 0x%x", i, *(buf + i));
						printk(KERN_ERR	"ch:%d ce:%d page0x%x real:0x%x\n",
							info->cur_chan, info->sel_chip, info->page_addr, real_pg);
						mtd->ecc_stats.failed++;
						status = -1;
						empty = 0;
						break;
					}
				}
				if (empty == 1) // otherwise oob area no data
					DBGLEVEL2(sp_pnand_dbg("Spare real 0xFF"));
			}
			// Restore the ecc original setting & generic original setting.
			writel(ecc_original_setting, info->io_base + ECC_CONTROL);
			writel(generic_original_setting, info->io_base + GENERAL_SETTING);
		}
	}
	info->read_state = 0;

	// Returning the any value isn't allowed, except 0, -EBADMSG, or -EUCLEAN
	return 0;
}

int sp_pnand_write_oob_lp(struct nand_chip *nand, u8 *buf, int len)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	struct cmd_feature cmd_f;
	int status = 0, real_pg, i;

	real_pg = info->page_addr;

	DBGLEVEL2(sp_pnand_dbg(
		"write_oob: ch = %d, ce = %d, page = 0x%x, real page:0x%x, sz = %d, oobsz = %d\n",
		info->cur_chan, info->sel_chip, info->page_addr, real_pg, mtd->writesize, mtd->oobsize));

	//memcpy in SP7350 is 64-bit
	for(i = 0; i < mtd->oobsize / 4; i++) {
		memcpy(info->io_base + SPARE_SRAM + 4 * i, buf + 4 * i, 4);
	}

	cmd_f.row_cycle = ROW_ADDR_3CYCLE;
	cmd_f.col_cycle = COL_ADDR_2CYCLE;
	cmd_f.cq1 = real_pg | SCR_SEED_VAL1(info->seed_val);
	cmd_f.cq2 = CMD_EX_SPARE_NUM(info->spare) | SCR_SEED_VAL2(info->seed_val);
	cmd_f.cq3 = CMD_COUNT(1);
	cmd_f.cq4 = CMD_COMPLETE_EN | CMD_FLASH_TYPE(info->flash_type) |\
			CMD_START_CE(info->sel_chip) | CMD_SPARE_NUM(info->spare) |\
			CMD_INDEX(LARGE_FIXFLOW_WRITEOOB);

	status = sp_pnand_issue_cmd(nand, &cmd_f);
	if (status < 0)
		goto out;

	if (sp_pnand_wait(mtd, nand) == NAND_STATUS_FAIL) {
		status = -EIO;
	}
out:
	// Returning the any value isn't allowed, except 0, -EBADMSG, or -EUCLEAN
	return status;
}

int sp_pnand_read_page_sp(struct nand_chip *nand, uint8_t *buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	int status = 0;
	int i, ecc_original_setting, val;
	int chk_data_0xFF, chk_spare_0xFF, empty;
	u8 *p;
	u32 *lbuf;

	DBGLEVEL2(sp_pnand_dbg("smallr: ch = %d, ce = %d, page = 0x%x, size = %d, info->column = %d\n",
			info->cur_chan, info->sel_chip, info->page_addr, mtd->writesize, info->column));

	info->read_state = 1;
	chk_data_0xFF = chk_spare_0xFF = 0;

	if(!rd_pg_w_oob_sp(nand, info->page_addr, buf, nand->oob_poi)) {
		if(info->cmd_status & (CMD_ECC_FAIL_ON_DATA | CMD_ECC_FAIL_ON_SPARE)) {
			// Store the original setting
			ecc_original_setting = readl(info->io_base + ECC_CONTROL);
			// Disable the ECC engine & HW-Scramble, temporarily.
			val = readl(info->io_base + ECC_CONTROL);
			val = val & ~(ECC_EN(0xFF));
			writel(val, info->io_base + ECC_CONTROL);

			if(info->cmd_status == (CMD_ECC_FAIL_ON_DATA | CMD_ECC_FAIL_ON_SPARE)) {
				if(!rd_pg_w_oob_sp(nand, info->page_addr, buf, nand->oob_poi)) {
					chk_data_0xFF = chk_spare_0xFF = 1;
				}
			}
			else if(info->cmd_status == CMD_ECC_FAIL_ON_DATA) {
				if(!rd_pg_sp(nand, info->page_addr, buf)) {
					chk_data_0xFF = 1;
				}
			}
			else if(info->cmd_status == CMD_ECC_FAIL_ON_SPARE) {
				if(!rd_oob_sp(nand, info->page_addr, nand->oob_poi)) {
					chk_spare_0xFF = 1;
				}
			}
			// Restore the ecc original setting & generic original setting.
			writel(ecc_original_setting, info->io_base + ECC_CONTROL);

			if(chk_data_0xFF == 1) {
				lbuf = (u32 *)buf;
				empty = 1;
				for (i = 0; i < (mtd->writesize >> 2); i++) {
					if (*(lbuf + i) != 0xFFFFFFFF) {//xt:?
						printk(KERN_ERR "ECC err @ page0x%x\n", info->page_addr);
						sp_pnand_regdump(nand);
						mtd->ecc_stats.failed++;
						status = -1;
						empty = 0;
						break;
					}
				}
				if (empty == 1)
					DBGLEVEL2(sp_pnand_dbg("Data Real 0xFF\n"));
			}
			if(chk_spare_0xFF == 1) {
				p = nand->oob_poi;
				empty = 1;
				for (i = 0; i < mtd->oobsize; i++) {
					if (*(p + i) != 0xFF) {
						printk(KERN_ERR"ECC err for spare(Read page) @ page0x%x\n", info->page_addr);
						mtd->ecc_stats.failed++;
						status = -1;
						empty = 0;
						break;
					}
				}
				if (empty == 1)
					DBGLEVEL2(sp_pnand_dbg("Spare Real 0xFF\n"));
			}
		}
	}
	info->read_state = 0;

	// Returning the any value isn't allowed, except 0, -EBADMSG, or -EUCLEAN
	return 0;
}

int sp_pnand_write_page_sp(struct nand_chip *nand, const uint8_t *buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	struct cmd_feature cmd_f;
	int i;
	int status = 0;
	u8 *p, w_wo_spare = 1;
	int progflow_buf[3];
	u32 *lbuf;
	unsigned long timeo;

	DBGLEVEL2(sp_pnand_dbg ("smallw: ch = %d, ce = %d, page = 0x%x, size = %d, info->column = %d\n",
				info->cur_chan, info->sel_chip,  info->page_addr, mtd->writesize, info->column));

	p = nand->oob_poi;
	for(i = 0; i < mtd->oobsize; i++) {
		if( *( p + i) != 0xFF) {
			w_wo_spare = 0;
			break;
		}
	}

	cmd_f.row_cycle = ROW_ADDR_2CYCLE;
	cmd_f.col_cycle = COL_ADDR_1CYCLE;
	cmd_f.cq1 = info->page_addr | SCR_SEED_VAL1(info->seed_val);
	cmd_f.cq2 = SCR_SEED_VAL2(info->seed_val);
	cmd_f.cq3 = CMD_COUNT(1) | (info->column / mtd->writesize);
	cmd_f.cq4 = CMD_COMPLETE_EN | CMD_FLASH_TYPE(info->flash_type) |\
			CMD_START_CE(info->sel_chip) | CMD_SPARE_NUM(info->spare);

	if(w_wo_spare == 0) {
		for(i = 0; i < mtd->oobsize / 4; i++) {
			memcpy(info->io_base + SPARE_SRAM + 4 * i, nand->oob_poi + 4 * i, 4);
		}
		cmd_f.cq4 |= CMD_INDEX(SMALL_FIXFLOW_PAGEWRITE);
	}
	else {
		progflow_buf[0] = 0x41421D00;
		progflow_buf[1] = 0x66086460;
		progflow_buf[2] = 0x000067C7;
		sp_pnand_fill_prog_flow(nand, progflow_buf, 10);
		cmd_f.cq4 |= CMD_PROM_FLOW | CMD_INDEX(0x0);
	}

	status = sp_pnand_issue_cmd(nand, &cmd_f);
	if (status < 0)
		goto out;

	timeo = get_timer(0);
	timeo += CONFIG_SYS_HZ / 10;//100ms delay

	do {
	} while(BMC_region_status_full(info) && (get_timer(0) < timeo));

	if(!BMC_region_status_full(info)) {
		lbuf = (u32 *)buf;
		for (i = 0; i < mtd->writesize; i += 4)
			*(volatile unsigned *)(nand->IO_ADDR_R) = *lbuf++;
	} else {
		printk(KERN_ERR "Transfer timeout!");
	}

	if (sp_pnand_wait(mtd, nand) == NAND_STATUS_FAIL) {
		status = -EIO;
		printk("FAILED\n");
	}
out:
	// Returning the any value isn't allowed, except 0, -EBADMSG, or -EUCLEAN
	return status;
}

int sp_pnand_read_oob_sp(struct nand_chip *nand, u8 *buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	int i, status = 0;
	int val, ecc_original_setting, empty;

	DBGLEVEL2(sp_pnand_dbg("smallread_oob: ch = %d, ce = %d, page = 0x%x, size = %d\n",
				info->cur_chan, info->sel_chip, info->page_addr, mtd->writesize));

	info->read_state = 1;
	if(!rd_oob_sp(nand, info->page_addr, buf)) {
		if(info->cmd_status & CMD_ECC_FAIL_ON_SPARE) {
			// Store the original setting
			ecc_original_setting = readl(info->io_base + ECC_CONTROL);
			// Disable the ECC engine & HW-Scramble, temporarily.
			val = readl(info->io_base + ECC_CONTROL);
			val = val & ~(ECC_EN(0xFF));
			writel(val, info->io_base + ECC_CONTROL);

			if(!rd_oob_sp(nand, info->page_addr, buf)) {
				empty = 1;
				for (i = 0; i < mtd->oobsize; i++) {
					if (*(buf + i) != 0xFF) {
						printk(KERN_ERR "ECC err for spare(Read oob) @ page0x%x\n",
								info->page_addr);
						mtd->ecc_stats.failed++;
						status = -1;
						empty = 0;
						break;
					}
				}
				if(empty == 1)
					DBGLEVEL2(sp_pnand_dbg("Spare real 0xFF"));
			}
			// Restore the ecc original setting & generic original setting.
			writel(ecc_original_setting, info->io_base + ECC_CONTROL);
		}
	}
	info->read_state = 0;

	// Returning the any value isn't allowed, except 0, -EBADMSG, or -EUCLEAN
	return 0;
}

int sp_pnand_write_oob_sp(struct nand_chip *nand, u8 *buf, int len)
{
	struct sp_pnand_info *info = nand_get_controller_data(nand);
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct cmd_feature cmd_f;
	int status = 0, i;

	DBGLEVEL2(sp_pnand_dbg("smallwrite_oob: ch = %d, ce = %d, page = 0x%x, size = %d\n",
				info->cur_chan, info->sel_chip, info->page_addr, mtd->writesize));

	for(i = 0; i < (len + 3) / 4; i++) {
		memcpy(info->io_base + SPARE_SRAM + 4 * i, buf + 4 * i, 4);
		if(i == ((len + 3) / 4 - 1))
			memcpy(info->io_base + SPARE_SRAM + 4 * i, buf + 4 * i, len % 4);
	}
	cmd_f.row_cycle = ROW_ADDR_2CYCLE;
	cmd_f.col_cycle = COL_ADDR_1CYCLE;
	cmd_f.cq1 = info->page_addr | SCR_SEED_VAL1(info->seed_val);
	cmd_f.cq2 = SCR_SEED_VAL2(info->seed_val);
	cmd_f.cq3 = CMD_COUNT(1);
	cmd_f.cq4 = CMD_COMPLETE_EN | CMD_FLASH_TYPE(info->flash_type) |\
			CMD_START_CE(info->sel_chip) | CMD_SPARE_NUM(info->spare) |\
			CMD_INDEX(SMALL_FIXFLOW_WRITEOOB);

	status = sp_pnand_issue_cmd(nand, &cmd_f);
	if (status < 0)
		goto out;

	if (sp_pnand_wait(mtd, nand) == NAND_STATUS_FAIL) {
		status = -EIO;
	}
out:
	// Returning the any value isn't allowed, except 0, -EBADMSG, or -EUCLEAN
	return status;
}

