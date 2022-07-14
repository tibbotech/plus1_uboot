#include <common.h>
#include <asm/io.h>
#include <command.h>
#include <linux/delay.h>
#include "sp_otp.h"

#define SUPPORT_WRITE_OTP

int read_otp_data(volatile struct hb_gp_regs *otp_data, volatile struct otprx_regs *regs, int addr, char *value)
{
	unsigned int addr_data;
	unsigned int byte_shift;
	unsigned int status;
	u32 timeout = OTP_READ_TIMEOUT;

	addr_data = addr % (OTP_WORD_SIZE * OTP_WORDS_PER_BANK);
	addr_data = addr_data / OTP_WORD_SIZE;

	byte_shift = addr % (OTP_WORD_SIZE * OTP_WORDS_PER_BANK);
	byte_shift = byte_shift % OTP_WORD_SIZE;

	writel(0x0, &regs->otp_cmd_status);

	addr = addr / (OTP_WORD_SIZE * OTP_WORDS_PER_BANK);
	addr = addr * OTP_BIT_ADDR_OF_BANK;
	writel(addr, &regs->otp_addr);

	writel(0x1e04, &regs->otp_cmd);

	do {
		udelay(10);
		if (timeout-- == 0)
			return -1;

		status = readl(&regs->otp_cmd_status);
	} while ((status & OTP_READ_DONE) != OTP_READ_DONE);

	*value = (otp_data->hb_gpio_rgst_bus32[8+addr_data] >> (8 * byte_shift)) & 0xff;

	return 0;
}

#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
int read_otp_key(volatile struct otp_key_regs *otp_data, volatile struct otprx_regs *regs, int addr, char *value)
{
	unsigned int addr_data;
	unsigned int byte_shift;
	unsigned int status;
	u32 timeout = OTP_READ_TIMEOUT;

	addr_data = addr % (OTP_WORD_SIZE * OTP_WORDS_PER_BANK);
	addr_data = addr_data / OTP_WORD_SIZE;

	byte_shift = addr % (OTP_WORD_SIZE * OTP_WORDS_PER_BANK);
	byte_shift = byte_shift % OTP_WORD_SIZE;

	writel(0x0, &regs->otp_cmd_status);

	addr = addr / (OTP_WORD_SIZE * OTP_WORDS_PER_BANK);
	addr = addr * OTP_BIT_ADDR_OF_BANK;
	writel(addr, &regs->otp_addr);

	writel(0x1e04, &regs->otp_cmd);

	do {
		udelay(10);
		if (timeout-- == 0)
			return -1;

		status = readl(&regs->otp_cmd_status);
	} while ((status & OTP_READ_DONE) != OTP_READ_DONE);

	*value = (otp_data->block_addr[addr_data] >> (8 * byte_shift)) & 0xff;

	return 0;
}
#endif

#ifdef SUPPORT_WRITE_OTP
int write_otp_data(volatile struct hb_gp_regs *otp_data, volatile struct otprx_regs *regs, int addr, char *value)
{
	unsigned int data;
	u32 timeout = OTP_WAIT_MICRO_SECONDS;

	#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
	writel(0x5dc1, &regs->otp_ctrl);
	#else
	writel(0xfd01, &regs->otp_ctrl);
	#endif
	writel(addr, &regs->otp_prog_addr);
	writel(0x03, &regs->otp_prog_ctl);

	data = *value;
	data = (data << 8) + 0x12;
	writel(data, &regs->otp_prog_reg25);

	writel(0x01, &regs->otp_prog_wr);
	writel(0x00, &regs->otp_prog_pgenb);

	do {
		udelay(1000);
		if (timeout-- == 0)
			return -1;

		data = readl(&regs->otp_prog_state);
	} while((data & 0x1F) != 0x13);

	writel(0x01, &regs->otp_prog_pgenb);
	writel(0x00, &regs->otp_prog_wr);
	writel(0x00, &regs->otp_prog_ctl);

	return 0;
}
#endif

static int do_read_otp(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	volatile struct hb_gp_regs *otp_data;
	volatile struct otprx_regs *otprx;
#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	volatile struct otp_key_regs *otp_key;
#endif
	unsigned int addr, data, efuse, otp_size;
	char value;
	int i, j;

	efuse = 0;
	if (argc == 3) {
#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
		efuse = simple_strtoul(argv[2], NULL, 0);
		if (efuse == 0) {
			otprx = SP_OTPRX_REG;
			otp_data = HB_GP_REG;
			otp_size = QAK645_EFUSE0_SIZE;
		} else if (efuse == 1) {
			otprx = KEY_OTPRX_REG;
			otp_data = KEY_HB_GP_REG;
			otp_size = QAK645_EFUSE1_SIZE;
		} else if (efuse == 2) {
			otprx = CUSTOMER_OTPRX_REG;
			otp_data = CUSTOMER_HB_GP_REG;
			otp_size = QAK645_EFUSE2_SIZE;
		} else
			return CMD_RET_USAGE;
#else
		return CMD_RET_USAGE;
#endif

#if !defined(CONFIG_TARGET_PENTAGRAM_Q645)
	} else if (argc == 2) {
		otprx = SP_OTPRX_REG;
		otp_data = HB_GP_REG;
		#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
		otp_key = OTP_KEY_REG;
		otp_size = QAK654_EFUSE_SIZE;
		#elif defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)
		otp_size = QAC628_EFUSE_SIZE;
		#elif defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C)
		otp_size = I143_EFUSE_SIZE;
		#endif
#endif
	} else {
		return CMD_RET_USAGE;
	}

	if (strcmp(argv[1], "a") == 0) {
		printf("         eFuse%d\n", efuse);
		printf(" (byte No.)   (data)\n");
		j = 0;

		for (addr = 0 ; addr < (otp_size - 1); addr += (OTP_WORD_SIZE * OTP_WORDS_PER_BANK)) {
#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
			if (addr < 64) {
				if (read_otp_data(otp_data, otprx, addr, &value) == -1)
					return CMD_RET_FAILURE;
			} else {
				if (read_otp_key(otp_key, otprx, addr, &value) == -1)
					return CMD_RET_FAILURE;
			}
#else
			if (read_otp_data(otp_data, otprx, addr, &value) == -1)
				return CMD_RET_FAILURE;
#endif

#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
			for (i = 0; i < 4; i++, j++) {
				if (addr < 64)
					printf("  %03u~%03u : 0x%08x\n", 3+j*4, j*4, otp_data->hb_gpio_rgst_bus32[8+i]);
				else
					printf("  %03u~%03u : 0x%08x\n", 3+j*4, j*4, otp_key->block_addr[i]);
			}
#else
			for (i = 0; i < 4; i++, j++) {
				printf("  %03u~%03u : 0x%08x\n", 3+j*4, j*4, otp_data->hb_gpio_rgst_bus32[8+i]);
			}
#endif

			printf("\n");
		}
	} else {
		addr = simple_strtoul(argv[1], NULL, 0);

		if ((strcmp(argv[1], "0") != 0) && (addr == 0))
			return CMD_RET_USAGE;

		if (addr >= otp_size) {
			printf("out of OTP size (0 ~ %d)\n", (otp_size - 1));
			return CMD_RET_USAGE;
		}

#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
		if (addr < 64) {
			if (read_otp_data(otp_data, otprx, addr, &value) == -1)
				return CMD_RET_FAILURE;
		} else {
			if (read_otp_key(otp_key, otprx, addr, &value) == -1)
				return CMD_RET_FAILURE;
		}
#else
		if (read_otp_data(otp_data, otprx, addr, &value) == -1)
			return CMD_RET_FAILURE;
#endif

		data = value;
		printf("eFuse%d DATA (byte %u) = 0x%02x\n", efuse, addr, data);
	}

	return 0;
}

#ifdef SUPPORT_WRITE_OTP
static int do_write_otp(struct cmd_tbl  *cmdtp, int flag, int argc, char * const argv[])
{
	#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
	volatile struct moon2_otp_regs *regs = MOON2_OTP_REG;
	unsigned int cfg;
	#endif
	unsigned int addr;
	unsigned int data;
	unsigned int otp_size;
	#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
	unsigned int efuse;
	#endif
	char value;

	if (argc == 4) {
	#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
		efuse = simple_strtoul(argv[3], NULL, 0);
		if (efuse == 0)
			otp_size = QAK645_EFUSE0_SIZE;
		else if (efuse == 1)
			otp_size = QAK645_EFUSE1_SIZE;
		else if (efuse == 2)
			otp_size = QAK645_EFUSE2_SIZE;
		else
			return CMD_RET_USAGE;
	#else
		return CMD_RET_USAGE;
	#endif

	#if !defined(CONFIG_TARGET_PENTAGRAM_Q645)
	} else if (argc == 3) {
		#if defined(CONFIG_TARGET_PENTAGRAM_SP7350)
		otp_size = QAK654_EFUSE_SIZE;
		#elif defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)
		otp_size = QAC628_EFUSE_SIZE;
		#elif defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C)
		otp_size = I143_EFUSE_SIZE;
		#endif
	#endif
	} else {
		return CMD_RET_USAGE;
	}

	addr = simple_strtoul(argv[1], NULL, 0);
	data = simple_strtoul(argv[2], NULL, 0);

	if (((strcmp(argv[1], "0") != 0) && (addr == 0)) || ((strcmp(argv[2], "0") != 0) && (data == 0)))
		return CMD_RET_USAGE;

	if ((addr >= otp_size) || (data > 0xff))
		return CMD_RET_USAGE;

	value = data & 0xff;

#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
	cfg = regs->sft_cfg[0];
	regs->sft_cfg[0] = 0x003c0008;

	if (efuse == 0) {
		if (write_otp_data(HB_GP_REG, SP_OTPRX_REG, addr, &value) == -1) {
			regs->sft_cfg[0] = 0xffff0000 | cfg;
			return CMD_RET_FAILURE;
		}
	} else if (efuse == 1) {
		if (write_otp_data(KEY_HB_GP_REG, KEY_OTPRX_REG, addr, &value) == -1) {
			regs->sft_cfg[0] = 0xffff0000 | cfg;
			return CMD_RET_FAILURE;
		}
	} else if (efuse == 2) {
		if (write_otp_data(CUSTOMER_HB_GP_REG, CUSTOMER_OTPRX_REG, addr, &value) == -1) {
			regs->sft_cfg[0] = 0xffff0000 | cfg;
			return CMD_RET_FAILURE;
		}
	}

	regs->sft_cfg[0] = 0xffff0000 | cfg;
#else
	if (write_otp_data(HB_GP_REG, SP_OTPRX_REG, addr, &value) == -1)
		return CMD_RET_FAILURE;
#endif

	printf("OTP write complete !!\n");

	return 0;
}
#endif


/*******************************************************/

#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
U_BOOT_CMD(
	rotp, 3, 1, do_read_otp,
	"read 1 byte data or all data of OTP",
	"[OTP address (0, 1, 2,.., n byte) | all (a)] [eFuse (0:sunplus, 1:security, 2:customer)]"
);

	#ifdef SUPPORT_WRITE_OTP
U_BOOT_CMD(
	wotp, 4, 1, do_write_otp,
	"write 1 byte data to OTP",
	"[OTP address (0, 1, 2,.., n byte)] [data (0~255)] [eFuse (0:sunplus, 1:security, 2:customer)]"
);
	#endif
#elif defined(CONFIG_ARCH_PENTAGRAM)
U_BOOT_CMD(
	rotp, 2, 1, do_read_otp,
	"read 1 byte data or all data of OTP",
	"[OTP address (0, 1,..., 127 byte) | all (a)]"
);

	#ifdef SUPPORT_WRITE_OTP
U_BOOT_CMD(
	wotp, 3, 1, do_write_otp,
	"write 1 byte data to OTP",
	"[OTP address (0, 1,..., 127 byte)] [data (0~255)]"
);
	#endif
#endif
