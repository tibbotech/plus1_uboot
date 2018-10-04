/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>

#define SIZE_SKIP_TEST		(16 << 20)
#define SIZE_TEST		(CONFIG_SYS_SDRAM_SIZE - SIZE_SKIP_TEST)

static int do_dram_tst(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int addr, data;
	unsigned int *ptr;
	unsigned int loop_cnt;

	run_command("bdinfo && meminfo", 0);
	printf("Test area: 0x00000000 - 0x%x\n", (SIZE_TEST - 1));

	for (loop_cnt = 0; ; loop_cnt++) {
		printf("\n---------- loop_cnt: %u ----------\n", loop_cnt);
		ptr = (unsigned int *)(0x00000000);
		for (addr = 0; addr < SIZE_TEST;) {
			data = (loop_cnt & 0x00000001) ? (~addr) : addr;
			*ptr = data;
			ptr++;
			addr += 4;

			if ((addr & (SIZE_SKIP_TEST - 1)) == 0) {
				printf("%u MB written.\n", (addr >> 20));
			}
		}
		ptr = (unsigned int *)(0x00000000);
		for (addr = 0; addr < SIZE_TEST;) {
			data = (loop_cnt & 0x00000001) ? (~addr) : addr;
			if (*ptr != data) {
				printf("Error @ 0x%x\n", addr);
				return -1;
			}
			ptr++;
			addr += 4;

			if ((addr & (SIZE_SKIP_TEST - 1)) == 0) {
				printf("%u MB Verified.\n", (addr >> 20));
			}
		}
	}

	return 0;
}

U_BOOT_CMD(
	dram_tst,	CONFIG_SYS_MAXARGS,	1,	do_dram_tst,
	"Memory r/w test.",
	"Memory r/w test.\n"
	"dram_tst command ...\n"
);

