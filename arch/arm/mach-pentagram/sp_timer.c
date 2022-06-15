/*
 * (C) Copyright 2014
 * Sunplus Technology
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <asm/io.h>
#include <asm/global_data.h>

#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
struct stc_regs {
	unsigned int stc_15_0;       // 12.0
	unsigned int stc_31_16;      // 12.1
	unsigned int stc_47_32;      // 12.2
	unsigned int stc_63_48;      // 12.3
	unsigned int stc_64;         // 12.4
	unsigned int stc_divisor;    // 12.5
	unsigned int stc_config;     // 12.6
	unsigned int rtc_15_0;       // 12.7
	unsigned int rtc_23_16;      // 12.8
	unsigned int rtc_divisor;    // 12.9
	unsigned int timerw_ctl;     // 12.10
	unsigned int timerw_cnt;     // 12.11
	unsigned int timer0_ctl;     // 12.12
	unsigned int timer0_cnt;     // 12.13
	unsigned int timer0_reload;  // 12.14
	unsigned int timer1_ctl;     // 12.15
	unsigned int timer1_cnt;     // 12.16
	unsigned int timer1_reload;  // 12.17
	unsigned int timer2_ctl;     // 12.18
	unsigned int timer2_cnt;     // 12.19
	unsigned int timer2_reload;  // 12.20
	unsigned int timer2_pres_val;// 12.21
	unsigned int timer3_ctl;     // 12.22
	unsigned int timer3_cnt;     // 12.23
	unsigned int timer3_reload;  // 12.24
	unsigned int timer3_pres_val;// 12.25
	unsigned int stcl_0;         // 12.26
	unsigned int stcl_1;         // 12.27
	unsigned int stcl_2;         // 12.28
	unsigned int atc_0;          // 12.29
	unsigned int atc_1;          // 12.30
	unsigned int atc_2;          // 12.31
};
#elif defined(CONFIG_TARGET_PENTAGRAM_SP7350)
struct stc_regs {
	unsigned int stc_31_0;       // 12.0
	unsigned int stc_63_32;      // 12.1
	unsigned int stc_64;         // 12.2
	unsigned int stc_divisor;    // 12.3
	unsigned int stc_config;     // 12.4
	unsigned int rtc_23_0;       // 12.5
	unsigned int rtc_divisor;    // 12.6
	unsigned int timerw_ctl;     // 12.7
	unsigned int timerw_cnt;     // 12.8
	unsigned int timer0_ctl;     // 12.9
	unsigned int timer0_cnt;     // 12.10
	unsigned int timer0_reload;  // 12.11
	unsigned int timer1_ctl;     // 12.12
	unsigned int timer1_cnt;     // 12.13
	unsigned int timer1_reload;  // 12.14
	unsigned int timer2_ctl;     // 12.15
	unsigned int timer2_cnt;     // 12.16
	unsigned int timer2_reload;  // 12.17
	unsigned int timer3_ctl;     // 12.18
	unsigned int timer3_cnt31_0; // 12.19
	unsigned int timer3_cnt63_32;// 12.20
	unsigned int timer3_reload31_0; // 12.21
	unsigned int timer3_reload63_32;// 12.22
	unsigned int stcl_31_0;      // 12.23
	unsigned int stcl_32;        // 12.24
	unsigned int atc_31_0;       // 12.25
	unsigned int atc_33_32;      // 12.26
	unsigned int timerw_rst_intrst_ctl; // 12.27
	unsigned int timerw_intrst_cnt;// 12.28
	unsigned int reserve[3];     // 12.29 --12.31
};
#else
struct stc_regs {
	unsigned int stc_15_0;       // 12.0
	unsigned int stc_31_16;      // 12.1
	unsigned int stc_64;         // 12.2
	unsigned int stc_divisor;    // 12.3
	unsigned int rtc_15_0;       // 12.4
	unsigned int rtc_23_16;      // 12.5
	unsigned int rtc_divisor;    // 12.6
	unsigned int stc_config;     // 12.7
	unsigned int timer0_ctrl;    // 12.8
	unsigned int timer0_cnt;     // 12.9
	unsigned int timer1_ctrl;    // 12.10
	unsigned int timer1_cnt;     // 12.11
	unsigned int timerw_ctrl;    // 12.12
	unsigned int timerw_cnt;     // 12.13
	unsigned int stc_47_32;      // 12.14
	unsigned int stc_63_48;      // 12.15
	unsigned int timer2_ctl;     // 12.16
	unsigned int timer2_pres_val;// 12.17
	unsigned int timer2_reload;  // 12.18
	unsigned int timer2_cnt;     // 12.19
	unsigned int timer3_ctl;     // 12.20
	unsigned int timer3_pres_val;// 12.21
	unsigned int timer3_reload;  // 12.22
	unsigned int timer3_cnt;     // 12.23
	unsigned int stcl_0;         // 12.24
	unsigned int stcl_1;         // 12.25
	unsigned int stcl_2;         // 12.26
	unsigned int atc_0;          // 12.27
	unsigned int atc_1;          // 12.28
	unsigned int atc_2;          // 12.29
};
#endif

#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
#define SPHE_DEVICE_BASE	(0xf8000000)
#elif  defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#define SPHE_DEVICE_BASE    (0xf8800000)   /*  sp7350  AO Domain base address */
#else
#define SPHE_DEVICE_BASE	(0x9C000000)
#endif

#define RF_GRP(_grp, _reg)      ((((_grp)*32 + (_reg))*4) + SPHE_DEVICE_BASE)
#ifdef CONFIG_TARGET_PENTAGRAM_SP7350

#define STC_REG     ((volatile struct stc_regs *)RF_GRP(23, 0))
#define STC_AV0_REG ((volatile struct stc_regs *)RF_GRP(24, 0))
#define STC_AV2_REG ((volatile struct stc_regs *)RF_GRP(26, 0))

#else

#define STC_REG     ((volatile struct stc_regs *)RF_GRP(12, 0))
#define STC_AV0_REG ((volatile struct stc_regs *)RF_GRP(96, 0))
#define STC_AV2_REG ((volatile struct stc_regs *)RF_GRP(99, 0))

#endif

DECLARE_GLOBAL_DATA_PTR;

static volatile struct stc_regs *g_regs = STC_AV2_REG;

#undef USE_EXT_CLK
/*
 * TRIGGER_CLOCK is timer's runtime frequency. We expect it to be 1MHz.
 * TRIGGER_CLOCK = SOURCE_CLOCK / ([13:0] of 12.3 + 1).
 */
#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
#define USE_EXT_CLK
#define SP_STC_TRIGGER_CLOCK	90000
#else
#define SP_STC_TRIGGER_CLOCK	1000000
#endif

#ifdef USE_EXT_CLK
#define SP_STC_SOURCE_CLOCK	13500000	/* Use div_ext_clk, it is 13.5 MHz. */
#else
#ifdef CONFIG_TARGET_PENTAGRAM_SP7350
#define SP_STC_SOURCE_CLOCK	25000000	/* Use sysclk, it is 25 MHz. */
#else  /*Q628*/
#define SP_STC_SOURCE_CLOCK	202000000	/* Use sysclk, it is 202 MHz. */
#endif
#endif

ulong get_timer_masked(void)
{
	ulong freq = gd->arch.timer_rate_hz;
	ulong secs = 0xffffffff / freq;
	ulong tick;

#ifdef CONFIG_TARGET_PENTAGRAM_SP7350
	writel(0x1234, &g_regs->stcl_32);
	tick = readl(&g_regs->stcl_31_0);
#else
	writel(0x1234, &g_regs->stcl_2); /* 99.26 stcl 2, write anything to latch */
	tick = (readl(&g_regs->stcl_1) << 16) | readl(&g_regs->stcl_0);
#endif
	if (tick >= secs * freq)
	{
		tick = 0;
		gd->arch.tbl += secs * CONFIG_SYS_HZ;
#ifdef CONFIG_TARGET_PENTAGRAM_SP7350
		writel(0, &g_regs->stc_31_0);
		writel(0, &g_regs->stc_63_32);
		writel(0, &g_regs->stc_64);
#else
		writel(0, &g_regs->stc_15_0);
		writel(0, &g_regs->stc_31_16);
		writel(0, &g_regs->stc_64);
#endif
	}

	return gd->arch.tbl + (tick / (freq / CONFIG_SYS_HZ));

	return 0;
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void reset_timer_masked(void)
{
	writel(0, &g_regs->stc_64);
}

void reset_timer(void)
{
	reset_timer_masked();
}

int timer_init(void)
{
	/*
	 * refer to 12.3 STC pre-scaling Register (stc divisor)
	 * trigger clock = sysClk or divExtClk / ([13:0] of 12.3 + 1)
	 * so 12.3[13:0] = (sysClk or divExtClk / trigger clcok) - 1
	 * 12.3[15] = 1 for div_ext_clk
	 */
#ifdef USE_EXT_CLK
	g_regs->stc_divisor = (1 << 15) | ((SP_STC_SOURCE_CLOCK / SP_STC_TRIGGER_CLOCK) - 1);
#else  /* Use sysclk */
	g_regs->stc_divisor = (0 << 15) | ((SP_STC_SOURCE_CLOCK / SP_STC_TRIGGER_CLOCK) - 1);
#endif
	gd->arch.timer_rate_hz = SP_STC_TRIGGER_CLOCK;

	gd->arch.tbl = 0;
	reset_timer_masked();

	return 0;
}

void udelay_masked(unsigned long usec)
{
	ulong freq = gd->arch.timer_rate_hz;
	ulong secs = 0xffffffff / freq;
	ulong wait, tick, timeout;

	/*
	 * how many cycle should be count
	 * When freq is 10K-1M, use the second rule or it will be always 0.
	 */
	wait = (freq >= 1000000) ? usec * (freq / 1000000) :
	       (usec * (freq / 10000)) / 100;

	if (!wait) {
		wait = 1;
	}

#ifdef CONFIG_TARGET_PENTAGRAM_SP7350
	writel(0x1234, &g_regs->stcl_32);
	tick = readl(&g_regs->stcl_31_0);
#else
	writel(0x1234, &g_regs->stcl_2); /* 99.26 stcl 2, write anything to latch */
	tick = (readl(&g_regs->stcl_1) << 16) | readl(&g_regs->stcl_0);
#endif

	/* restart timer if counter is going to overflow */
	if (secs * freq - tick < wait) {
		gd->arch.tbl += tick / (freq / CONFIG_SYS_HZ);
#ifdef CONFIG_TARGET_PENTAGRAM_SP7350
		writel(0, &g_regs->stc_31_0);
		writel(0, &g_regs->stc_63_32);
		writel(0, &g_regs->stc_64);
#else
		writel(0, &g_regs->stc_15_0);
		writel(0, &g_regs->stc_31_16);
		writel(0, &g_regs->stc_64);
#endif

		tick = 0;
	}

	/* now we wait ... */
	timeout = tick + wait;
	do {
#ifdef CONFIG_TARGET_PENTAGRAM_SP7350
		writel(0x1234, &g_regs->stcl_32);
		tick = readl(&g_regs->stcl_31_0);
#else
		writel(0x1234, &g_regs->stcl_2); /* 99.26 stcl 2, write anything to latch */
		tick = (readl(&g_regs->stcl_1) << 16) | readl(&g_regs->stcl_0);
#endif
	} while (timeout > tick);
}

void __udelay(unsigned long usec)
{
	udelay_masked(usec);
}

unsigned long long get_ticks(void)
{
	unsigned long long ret;

#ifdef CONFIG_TARGET_PENTAGRAM_SP7350
	writel(0x1234, &g_regs->stcl_32);
	ret = readl(&g_regs->stcl_31_0);
	if (readl(&g_regs->stcl_32) & 1)
		ret |= 0x100000000ull;
#else
	writel(0x1234, &g_regs->stcl_2); /* 99.26 stcl 2, write anything to latch */
	ret = (readl(&g_regs->stcl_1) << 16) | readl(&g_regs->stcl_0);

	if (readl(&g_regs->stcl_2) & 1)
		ret |= 0x100000000ull;
#endif


	return ret;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return gd->arch.timer_rate_hz;
}
