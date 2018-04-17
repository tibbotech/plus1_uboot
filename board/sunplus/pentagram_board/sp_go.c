#include <common.h>
#include <command.h>

__attribute__((weak))
unsigned long do_sp_go_exec(ulong (*entry)(int, char * const [], unsigned int), int argc,
			    char * const argv[], unsigned int dtb)
{
	u32 dtb_addr;

	dtb_addr = simple_strtoul(argv[1], NULL, 16);
	printf("[u-boot] dtb address %08x\n", dtb_addr);

	return entry (0, 0, dtb_addr);
}

static int do_sp_go(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong   addr, rc;
	int     rcode = 0;

	addr = simple_strtoul(argv[1], NULL, 16);

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
	"sp_go [kernel addr] [dtb addr]\n"
);

