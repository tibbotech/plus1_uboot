#include <common.h>
#include <asm/io.h>
#include "sp_otp.h"

//#define SUPPORT_WRITE_OTP

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

	writel(0x1E04, &regs->otp_cmd);

	do {
		udelay(10);
		if (timeout-- == 0)
			return -1;

		status = readl(&regs->otp_cmd_status);
	} while ((status & OTP_READ_DONE) != OTP_READ_DONE);

	*value = (otp_data->hb_gpio_rgst_bus32[8+addr_data] >> (8 * byte_shift)) & 0xFF;

	return 0;
}

#ifdef SUPPORT_WRITE_OTP
int write_otp_data(volatile struct hb_gp_regs *otp_data, volatile struct otprx_regs *regs, int addr, char value)
{
	unsigned int data;
	u32 timeout = OTP_WAIT_MICRO_SECONDS;

	writel(0xFD01, &regs->otp_ctrl);
	writel(addr, &regs->otp_prog_addr);
	writel(0x03, &regs->otp_prog_ctl);

	data = value;
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

static int do_read_otp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	volatile struct hb_gp_regs *otp_data;
	unsigned int addr, data, efuse, otp_size;
	char value;
	int i, j;

	efuse = 0;
	if (argc == 3) {
#ifndef CONFIG_TARGET_PENTAGRAM_Q645
	#if (defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)) || \
		(defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C))
		return CMD_RET_USAGE;
	#endif
#endif
		efuse = simple_strtoul(argv[2], NULL, 0);
		if (efuse == 0) {
			otp_data = HB_GP_REG;
			otp_size = QAK645_EFUSE0_SIZE;
		} else if (efuse == 1) {
			otp_data = KEY_HB_GP_REG;
			otp_size = QAK645_EFUSE1_SIZE;
		} else if (efuse == 2) {
			otp_data = CUSTOMER_HB_GP_REG;
			otp_size = QAK645_EFUSE2_SIZE;
		} else
			return CMD_RET_USAGE;
#ifndef CONFIG_TARGET_PENTAGRAM_Q645
	#if (defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)) || \
		(defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C))
	} else if (argc == 2) {
		otp_data = HB_GP_REG;
		#if defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)
		otp_size = QAC628_EFUSE_SIZE;
		#elif defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C)
		otp_size = I143_EFUSE_SIZE;
		#endif
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
#ifdef CONFIG_TARGET_PENTAGRAM_Q645
			if (efuse == 0) {
				if (read_otp_data(HB_GP_REG, SP_OTPRX_REG, addr, &value) == -1)
					return CMD_RET_FAILURE;
			} else if (efuse == 1) {
				if (read_otp_data(KEY_HB_GP_REG, KEY_OTPRX_REG, addr, &value) == -1)
					return CMD_RET_FAILURE;
			} else if (efuse == 2) {
				if (read_otp_data(CUSTOMER_HB_GP_REG, CUSTOMER_OTPRX_REG, addr, &value) == -1)
					return CMD_RET_FAILURE;
			}
#elif (defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)) || \
	(defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C))
			if (read_otp_data(HB_GP_REG, SP_OTPRX_REG, addr, &value) == -1)
				return CMD_RET_FAILURE;
#endif

			for (i = 0; i < 4; i++, j++) {
				printf("  %03u~%03u : 0x%08X\n", 3+j*4, j*4, otp_data->hb_gpio_rgst_bus32[8+i]);
			}

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

#ifdef CONFIG_TARGET_PENTAGRAM_Q645
		if (efuse == 0) {
			if (read_otp_data(HB_GP_REG, SP_OTPRX_REG, addr, &value) == -1)
				return CMD_RET_FAILURE;
		} else if (efuse == 1) {
			if (read_otp_data(KEY_HB_GP_REG, KEY_OTPRX_REG, addr, &value) == -1)
				return CMD_RET_FAILURE;
		} else if (efuse == 2) {
			if (read_otp_data(CUSTOMER_HB_GP_REG, CUSTOMER_OTPRX_REG, addr, &value) == -1)
				return CMD_RET_FAILURE;
		}
#elif (defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)) || \
	(defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C))
		if (read_otp_data(HB_GP_REG, SP_OTPRX_REG, addr, &value) == -1)
			return CMD_RET_FAILURE;
#endif

		data = value;
		printf("eFuse%d DATA (byte %u) = 0x%02X\n", efuse, addr, data);
	}

	return 0;
}

#ifdef SUPPORT_WRITE_OTP
static int do_write_otp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int addr;
	unsigned int data;
	unsigned int otp_size;
	unsigned int efuse;
	char value;

	if (argc == 4) {
	#ifndef CONFIG_TARGET_PENTAGRAM_Q645
		#if (defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)) || \
			(defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C))
		return CMD_RET_USAGE;
		#endif
	#endif
		efuse = simple_strtoul(argv[3], NULL, 0);
		if (efuse == 0)
			otp_size = QAK645_EFUSE0_SIZE;
		else if (efuse == 1)
			otp_size = QAK645_EFUSE1_SIZE;
		else if (efuse == 2)
			otp_size = QAK645_EFUSE2_SIZE;
		else
			return CMD_RET_USAGE;
	#ifndef CONFIG_TARGET_PENTAGRAM_Q645
		#if (defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)) || \
			(defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C))
	} else if (argc == 3) {
			#if defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)
		otp_size = QAC628_EFUSE_SIZE;
			#elif defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C)
		otp_size = I143_EFUSE_SIZE;
			#endif
		#endif
	#endif
	} else {
		return CMD_RET_USAGE;
	}

	addr = simple_strtoul(argv[1], NULL, 0);
	data = simple_strtoul(argv[2], NULL, 0);

	if (((strcmp(argv[1], "0") != 0) && (addr == 0)) || ((strcmp(argv[2], "0") != 0) && (data == 0)))
		return CMD_RET_USAGE;

	if ((addr >= otp_size) || (data > 0xFF))
		return CMD_RET_USAGE;

	value = data & 0xFF;

#ifdef CONFIG_TARGET_PENTAGRAM_Q645
	if (efuse == 0) {
		if (write_otp_data(HB_GP_REG, SP_OTPRX_REG, addr, &value) == -1)
			return CMD_RET_FAILURE;
	} else if (efuse == 1) {
		if (write_otp_data(KEY_HB_GP_REG, KEY_OTPRX_REG, addr, &value) == -1)
			return CMD_RET_FAILURE;
	} else if (efuse == 2) {
		if (write_otp_data(CUSTOMER_HB_GP_REG, CUSTOMER_OTPRX_REG, addr, &value) == -1)
			return CMD_RET_FAILURE;
	}
#elif (defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)) || \
	(defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C))
	if (write_otp_data(HB_GP_REG, SP_OTPRX_REG, addr, &value) == -1)
		return CMD_RET_FAILURE;
#endif

	printf("OTP write complete !!\n");

	return 0;
}
#endif

/*******************************************************/

#ifdef CONFIG_TARGET_PENTAGRAM_Q645
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
#elif (defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)) || \
	(defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C))
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
