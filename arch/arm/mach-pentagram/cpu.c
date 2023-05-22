#include <common.h>
#include <cpu_func.h>
#include <fdtdec.h>
#if defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#include <asm/armv8/mmu.h>
#endif
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#define PENTAGRAM_BASE_ADDR     (0xf8000000)
#else
#define PENTAGRAM_BASE_ADDR     (0x9C000000)
#endif

#ifdef CONFIG_TARGET_PENTAGRAM_SP7350
#define PENTAGRAM_AO_BASE_ADDR  (0xf8800000)   /*  sp7350  AO Domain base address */
#define PENTAGRAM_MOON0         (PENTAGRAM_AO_BASE_ADDR + (0 << 7))
#define PENTAGRAM_RTC_ADDR      (PENTAGRAM_AO_BASE_ADDR + (35 << 7))
#else
#define PENTAGRAM_MOON0         (PENTAGRAM_BASE_ADDR + (0 << 7))
#define PENTAGRAM_RTC_ADDR      (PENTAGRAM_BASE_ADDR + (116 << 7))
#endif

#define PENTAGRAM_MOON4         (PENTAGRAM_BASE_ADDR + (4 << 7))
#define PENTAGRAM_OTP_ADDR      (PENTAGRAM_BASE_ADDR + (350<<7))

void mem_map_fill(void);

void s_init(void)
{
	/* Init watchdog timer, ... */
	/* TODO: Setup timer used by U-Boot, required to change on I-139 */

#if (CONFIG_SYS_HZ != 1000)
#error "CONFIG_SYS_HZ != 1000"
#else
	/*
	 * Clock @ 27 MHz, but stc_divisor has only 14 bits
	 * Min STC clock: 270000000 / (1 << 14) = 16479.4921875
	 */
	//pstc_avReg->stc_divisor = ((270000000 + (1000 * 32) / 2) / (1000 * 32)) - 1 ; /* = 0x20F5, less then 14 bits */
#endif

}

void reset_cpu(ulong ignored)
{
	volatile unsigned int *ptr;

	puts("System is going to reboot ...\n");

#if !defined(CONFIG_TARGET_PENTAGRAM_Q645) && !defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	/*
	 * Enable all methods (in Grp(4, 29)) to cause chip reset:
	 * Bit [4:1]
	 */
	ptr = (volatile unsigned int *)(PENTAGRAM_MOON4 + (29 << 2));
	*ptr = (0x001E << 16) | 0x001E;
#endif

	/* System reset */
#ifdef CONFIG_TARGET_PENTAGRAM_SP7350
	ptr = (volatile unsigned int *)(PENTAGRAM_MOON0 + (1 << 2));
#else
	ptr = (volatile unsigned int *)(PENTAGRAM_MOON0 + (21 << 2));
#endif
	*ptr = (0x0001 << 16) | 0x0001;

	while (1) {
		/* wait for reset */
	}

	/*
	 * Note: When using Zebu's zmem, chip can't be reset correctly because part of the loaded memory is destroyed.
	 * 	 => Use eMMC for this test.
	 */
}
#ifdef CONFIG_BOOTARGS_WITH_MEM
// get dram size by otp
int dram_get_size(void)
{
	volatile unsigned int *ptr;
	ptr = (volatile unsigned int *)(PENTAGRAM_OTP_ADDR + (7 << 2));//G[350.7]
	int dramsize_Flag = ((*ptr)>>16)&0x03;
	int dramsize;
	switch(dramsize_Flag)
	{
		case 0x00:
			dramsize = 64<<20;
			break;
		case 0x01:
			dramsize = 128<<20;
			break;
		case 0x02:
			dramsize = 256<<20;
			break;
		case 0x03:
			dramsize = 512<<20;
			break;
		default:
			dramsize = 512<<20;
	}
	printf("dram size is %dM\n",dramsize>>20);
	return dramsize;
}
#endif
int dram_init(void)
{

#ifdef CONFIG_BOOTARGS_WITH_MEM
	gd->ram_size = dram_get_size();
#elif defined(CONFIG_SYS_ENV_ZEBU)
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
#else
	if(fdtdec_setup_mem_size_base() != 0)
	{
		gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	}
#endif
	mem_map_fill();
	return 0;
}
int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CONFIG_SYS_CACHELINE_SIZE: %d\n", CONFIG_SYS_CACHELINE_SIZE);
	return 0;
}
#endif

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	volatile unsigned int *ptr;

#if defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	return 0;
#endif

#ifdef CONFIG_VIDEO_SP7021
#ifdef CONFIG_DM_VIDEO_SP7021_LOGO
#else
#ifdef CONFIG_OF_CONTROL
	const char *model;
#endif
	unsigned long long size;
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	display_options_get_banner(true, buf, sizeof(buf));
	printf("%s",buf);

#ifdef CONFIG_OF_CONTROL
	model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);

	if (model)
		printf("Model: %s\n", model);
#endif
	size = gd->ram_size;
	printf("DRAM: ");
	print_size(size,"");
	printf("\n");
#endif
#endif
	ptr = (volatile unsigned int *)(PENTAGRAM_RTC_ADDR + (22 << 2));
	printf("\nReason(s) of reset: REG(116, 22): 0x%04x\n", *ptr);
	*ptr = 0xFFFF0000;
	printf("\nAfter cleaning  REG(116, 22): 0x%04x\n\n", *ptr);

	printf("%s, %s: TBD.\n", __FILE__, __func__);
	return 0;
}
#endif

#if defined(CONFIG_HAS_THUMB2) && !defined(CONFIG_SYS_DCACHE_OFF)
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

#ifdef CONFIG_ARMV7_NONSEC
#ifndef CONFIG_ARMV7_PSCI
//void smp_kick_all_cpus(void) {}
void smp_set_core_boot_addr(unsigned long addr, int corenr)
{
	volatile u32 *cpu_boot_regs = (void *)(CONFIG_SMP_PEN_ADDR - 12);

	/* wakeup core 1~3 */
	cpu_boot_regs[0] = addr;
	cpu_boot_regs[1] = addr;
	cpu_boot_regs[2] = addr;

	__asm__ __volatile__ ("dsb ishst; sev");
}
#endif
#endif

#if defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
/* 1 for register */
#define SP_MEM_MAP_USED 1
/* add 1 for dram, 1 for end  */
#define SP_MEM_MAP_MAX (SP_MEM_MAP_USED + 1 + 1)

static struct mm_region sp_mem_map[SP_MEM_MAP_MAX] = {
	{
		/* RGST */
		.virt = 0xE0000000UL,
		.phys = 0xE0000000UL,
		.size = 0x20000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	#if 0
	{
		/* RGST */
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0xE0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},
	{0}
	#endif
};
struct mm_region *mem_map = sp_mem_map;


void mem_map_fill(void)
{
	int bank = SP_MEM_MAP_USED;
	for (int i = 0; i < 1; i++) {
		sp_mem_map[bank].virt = 0x00000000UL;
		sp_mem_map[bank].phys = 0x00000000UL;
		sp_mem_map[bank].size = gd->ram_size;
		sp_mem_map[bank].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
                                 PTE_BLOCK_INNER_SHARE;
		bank += 1;
	}

	sp_mem_map[bank].size = 0; /*  end  */
}

#endif
