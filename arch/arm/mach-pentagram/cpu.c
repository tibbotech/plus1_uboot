#include <common.h>
#include <fdtdec.h>
#ifdef CONFIG_TARGET_PENTAGRAM_Q645
#include <asm/armv8/mmu.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* Group 96, 97, 99: STC_AV0 - STC_AV2 */
typedef struct {
	volatile unsigned int stc_15_0;
	volatile unsigned int stc_31_16;
	volatile unsigned int stc_64;
	volatile unsigned int stc_divisor;
	volatile unsigned int rtc_15_0;
	volatile unsigned int rtc_23_16;
	volatile unsigned int rtc_divisor;
	volatile unsigned int stc_config;
	volatile unsigned int timer0_ctrl;
	volatile unsigned int timer0_cnt;
	volatile unsigned int timer1_ctrl;
	volatile unsigned int timer1_cnt;
	volatile unsigned int timerw_ctrl;	/* Only STCs @ 0x9C000600 and 0x9C003000 */
	volatile unsigned int timerw_cnt;	/* Only STCs @ 0x9C000600 and 0x9C003000 */
	volatile unsigned int stc_47_32;
	volatile unsigned int stc_63_48;
	volatile unsigned int timer2_ctrl;
	volatile unsigned int timer2_divisor;
	volatile unsigned int timer2_reload;
	volatile unsigned int timer2_cnt;
	volatile unsigned int timer3_ctrl;
	volatile unsigned int timer3_divisor;
	volatile unsigned int timer3_reload;
	volatile unsigned int timer3_cnt;
	volatile unsigned int stcl_0;
	volatile unsigned int stcl_1;
	volatile unsigned int stcl_2;
	volatile unsigned int atc_0;
	volatile unsigned int atc_1;
	volatile unsigned int atc_2;
	volatile unsigned int timer0_reload;
	volatile unsigned int timer1_reload;
} stc_avReg_t;

#ifdef CONFIG_TARGET_PENTAGRAM_Q645
#define PENTAGRAM_BASE_ADDR	(0xf8000000)
#else
#define PENTAGRAM_BASE_ADDR	(0x9C000000)
#endif

#define PENTAGRAM_MOON0		(PENTAGRAM_BASE_ADDR + (0 << 7))
#define PENTAGRAM_MOON4		(PENTAGRAM_BASE_ADDR + (4 << 7))
#define PENTAGRAM_WDTMR_ADDR	(PENTAGRAM_BASE_ADDR + (12 << 7))	/* Either Group 12 or 96 */
#define PENTAGRAM_TIMER_ADDR	(PENTAGRAM_BASE_ADDR + (99 << 7))
#define PENTAGRAM_RTC_ADDR	(PENTAGRAM_BASE_ADDR + (116 << 7))
#define PENTAGRAM_OTP_ADDR	(PENTAGRAM_BASE_ADDR + (350<<7))

#define WATCHDOG_CMD_CNT_WR_UNLOCK	0xAB00
#define WATCHDOG_CMD_CNT_WR_LOCK	0xAB01
#define WATCHDOG_CMD_CNT_WR_MAX		0xDEAF
#define WATCHDOG_CMD_PAUSE		0x3877
#define WATCHDOG_CMD_RESUME		0x4A4B
#define WATCHDOG_CMD_INTR_CLR		0x7482


void s_init(void)
{
	/* Init watchdog timer, ... */
	/* TODO: Setup timer used by U-Boot, required to change on I-139 */
	stc_avReg_t *pstc_avReg = (stc_avReg_t *)(PENTAGRAM_TIMER_ADDR);

#if (CONFIG_SYS_HZ != 1000)
#error "CONFIG_SYS_HZ != 1000"
#else
	/*
	 * Clock @ 27 MHz, but stc_divisor has only 14 bits
	 * Min STC clock: 270000000 / (1 << 14) = 16479.4921875
	 */
	pstc_avReg->stc_divisor = ((270000000 + (1000 * 32) / 2) / (1000 * 32)) - 1 ; /* = 0x20F5, less then 14 bits */
#endif
}

unsigned long notrace timer_read_counter(void)
{
	unsigned long value;
	stc_avReg_t *pstc_avReg = (stc_avReg_t *)(PENTAGRAM_TIMER_ADDR);

	pstc_avReg->stcl_2 = 0; /* latch */
	value  = (unsigned long)(pstc_avReg->stcl_2);
	value  = value << 16;
	value |= (unsigned long)(pstc_avReg->stcl_1);
	value  = value << 16;
	value |= (unsigned long)(pstc_avReg->stcl_0);
	value = value >> 5; /* divided by 32 => (1000 * 32)/32 = 1000*/
	return value;
}

void reset_cpu(ulong ignored)
{
	volatile unsigned int *ptr;
#if 0
	stc_avReg_t *pstc_avReg;
#endif

	puts("System is going to reboot ...\n");

#ifndef CONFIG_TARGET_PENTAGRAM_Q645
	/*
	 * Enable all methods (in Grp(4, 29)) to cause chip reset:
	 * Bit [4:1]
	 */
	ptr = (volatile unsigned int *)(PENTAGRAM_MOON4 + (29 << 2));
	*ptr = (0x001E << 16) | 0x001E;
#endif

#if 0
	/* Watchdogs used by RISC */
	pstc_avReg = (stc_avReg_t *)(PENTAGRAM_WDTMR_ADDR);
	pstc_avReg->stc_divisor = (0x0100 - 1);
	pstc_avReg->stc_64 = 0;		/* reset STC */
	pstc_avReg->timerw_ctrl = WATCHDOG_CMD_CNT_WR_UNLOCK;
	pstc_avReg->timerw_ctrl = WATCHDOG_CMD_PAUSE;
	pstc_avReg->timerw_cnt = 0x10 - 1;
	pstc_avReg->timerw_ctrl = WATCHDOG_CMD_RESUME;
	pstc_avReg->timerw_ctrl = WATCHDOG_CMD_CNT_WR_LOCK;
#else
	/* System reset */
	ptr = (volatile unsigned int *)(PENTAGRAM_MOON0 + (21 << 2));
	*ptr = (0x0001 << 16) | 0x0001;
#endif

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

#ifdef CONFIG_TARGET_PENTAGRAM_Q645
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

#ifdef CONFIG_HAS_THUMB2
#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
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

#ifdef CONFIG_TARGET_PENTAGRAM_Q645
static struct mm_region sp_mem_map[] = {
	{
		/* RGST */
		.virt = 0xE0000000UL,
		.phys = 0xE0000000UL,
		.size = 0x20000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		/* DRAM */
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},
	{
		0,
	}
};
struct mm_region *mem_map = sp_mem_map;

#endif
