#include <common.h>
#include <command.h>
#include <image.h>

static uint32_t sum32(uint32_t sum, uint8_t *data, uint32_t len)
{
	uint32_t val = 0, pos =0;

	for (; pos + 4 <= len; pos += 4)
		sum += *(uint32_t *)(data + pos);
	/*
	 * word0: 3 2 1 0
	 * word1: _ 6 5 4
	 */
	for (; len - pos; len--)
		val = (val << 8) | data[len - 1];

	sum += val;

	return sum;
}

/* Similar with original u-boot flow but use different crc calculation */
int sp_image_check_hcrc(const image_header_t *hdr)
{
	ulong hcrc;
	ulong len = image_get_header_size();
	image_header_t header;

	/* Copy header so we can blank CRC field for re-calculation */
	memmove(&header, (char *)hdr, image_get_header_size());
	image_set_hcrc(&header, 0);

	hcrc = sum32(0, (unsigned char *)&header, len);

	return (hcrc == image_get_hcrc(hdr));
}

/* Similar with original u-boot flow but use different crc calculation */
int sp_image_check_dcrc(const image_header_t *hdr)
{
	ulong data = image_get_data(hdr);
	ulong len = image_get_data_size(hdr);
	ulong dcrc = sum32(0, (unsigned char *)data, len);

	return (dcrc == image_get_dcrc(hdr));
}

/* 
 * Similar with original u-boot verifiction. Only data crc is different.
 * Return NULL if failed otherwise return header address.
 */
int sp_qk_uimage_verify(ulong img_addr, int verify)
{
	image_header_t *hdr = (image_header_t *)img_addr;

	/* original uImage header's magic */
	if (!image_check_magic(hdr)) {
		puts("Bad Magic Number\n");
		return NULL;
	}

	/* hcrc by quick sunplus crc */
	if (!sp_image_check_hcrc(hdr)) {
		puts("Bad Header Checksum(Simplified)\n");
		return NULL;
	}

	image_print_contents(hdr);

	/* dcrc by quick sunplys crc */
	if (verify) {
		puts("   Verifying Checksum ... ");
		if (!sp_image_check_dcrc(hdr)) {
			printf("Bad Data CRC(Simplified)\n");
			return NULL;
		}
		puts("OK\n");
	}

	return hdr;
}

/* return 0 if failed otherwise return 1 */
int sp_image_verify(u32 kernel_addr, u32 dtb_addr)
{
	if (!sp_qk_uimage_verify(kernel_addr, 1))
		return 0;

	if (!sp_qk_uimage_verify(dtb_addr, 1))
		return 0;

	return 1;
}

__attribute__((weak))
unsigned long do_sp_go_exec(ulong (*entry)(int, char * const [], unsigned int), int argc,
			    char * const argv[], unsigned int dtb)
{
	u32 kernel_addr, dtb_addr; /* these two addr will include headers. */

	kernel_addr = simple_strtoul(argv[0], NULL, 16);
	dtb_addr = simple_strtoul(argv[1], NULL, 16);

	printf("[u-boot] kernel address 0x%08x, dtb address 0x%08x\n",
		kernel_addr, dtb_addr);

	if (!sp_image_verify(kernel_addr, dtb_addr))
		return CMD_RET_FAILURE;

	cleanup_before_linux();

	return entry (0, 0, (dtb_addr + 0x40));
}

static int do_sp_go(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong   addr, rc;
	int     rcode = 0;

	addr = simple_strtoul(argv[1], NULL, 16);
	addr += 0x40; /* 0x40 for skipping quick uImage header */

	printf ("## Starting application at 0x%08lX ...\n", addr);

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	rc = do_sp_go_exec ((void *)addr, argc - 1, argv + 1, 0);
	if (rc != 0) rcode = 1;

	printf ("## Application terminated, rc = 0x%lX\n", rc);
	return rcode;
}

U_BOOT_CMD(
	sp_go, CONFIG_SYS_MAXARGS, 1, do_sp_go,
	"sunplus booting command",
	"sp_go - run kernel at address 'addr'\n"
	"\n"
	"sp_go [kernel addr] [dtb addr]\n"
	"\tkernel addr should include the 'qk_sp_header'\n"
	"\twhich is similar as uImage header but different crc method.\n"
	"\tdtb also should have 'qk_sp_header', althrough dtb originally\n"
	"\thas its own header.\n"
	"\n"
	"\tSo image would be like this :\n"
	"\t<kernel addr> : [qk uImage header][kernel]\n"
	"\t<dtb addr>    : [qk uImage header][dtb header][dtb]\n"
);
