#include <common.h>

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
	volatile unsigned int rsv_12;
	volatile unsigned int rsv_13;
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
	volatile unsigned int rsv_30;
	volatile unsigned int rsv_31;
} stc_avReg_t;

#define GEMINI_TIMER_ADDR	(0x9C003180)	/* SPHE8388's STC_AV2 */

void s_init(void)
{
	/* Init watchdog timer, ... */
	/* TODO: Setup timer used by U-Boot, required to change on I-139 */
	stc_avReg_t *pstc_avReg = (stc_avReg_t *)(GEMINI_TIMER_ADDR);

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
	stc_avReg_t *pstc_avReg = (stc_avReg_t *)(GEMINI_TIMER_ADDR);

	pstc_avReg->stcl_2 = 0; /* latch */
	value  = (unsigned long)(pstc_avReg->stcl_2);
	value  = value << 16;
	value |= (unsigned long)(pstc_avReg->stcl_1);
	value  = value << 16;
	value |= (unsigned long)(pstc_avReg->stcl_0);
	value = value >> 5; /* divided by 32 => (1000 * 32)/32 = 1000*/
	return value;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
unsigned long get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}

void reset_cpu(ulong ignored)
{
	puts("System is going to reboot ...\n");
	while (1) {
		 printf("%s, %s: TBD.\n", __FILE__, __func__);
	}
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("%s, %s: TBD.\n", __FILE__, __func__);
	return 0;
}
#endif

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	printf("%s, %s: TBD.\n", __FILE__, __func__);
	return 0;
}
#endif
