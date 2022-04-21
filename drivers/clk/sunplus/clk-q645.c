// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Sunplus Technology Co., Ltd.
 *       All rights reserved.
 */
#include <asm/arch/clk-sunplus.h>
#include <dt-bindings/clock/sp-q645.h>

#define EXT_CLK "clk@osc0"
#define CLK_SP_PLL "sp_clk_pll"

#define MASK_SET(shift, width, value) \
({ \
	u32 m = ((1 << (width)) - 1) << (shift); \
	(m << 16) | (((value) << (shift)) & m); \
})
#define MASK_GET(shift, width, value)	(((value) >> (shift)) & ((1 << (width)) - 1))

#define REG(g, i)	((void *)(0xf8000000 + ((g) * 32 + (i)) * 4))

#define PLLH_CTL	REG(4, 1)
#define PLLN_CTL	REG(4, 4)
#define PLLS_CTL	REG(4, 7)
#define PLLC_CTL	REG(4, 10)
#define PLLD_CTL	REG(4, 13)
#define PLLA_CTL	REG(4, 24)

#define IS_PLLH()	(pll->reg == PLLH_CTL)
#define IS_PLLN()	(pll->reg == PLLN_CTL)
#define IS_PLLA()	(pll->reg == PLLA_CTL)

#define PD_N	0
#define PREDIV	1
#define PRESCL	3
#define FBKDIV	4
#define PSTDIV	12

#define FBKDIV_WIDTH	8
#define FBKDIV_MIN	64
#define FBKDIV_MAX	(FBKDIV_MIN + BIT(FBKDIV_WIDTH) - 1)

struct sp_pll {
	struct clk clk;
	void	*reg;
	int	bp; // bypass bit_idx
	ulong	brate;
	int	idiv; // struct divs[] index
};
#define to_sp_pll(_clk)	container_of(_clk, struct sp_pll, clk)

static u32 mux_table[] = { 0x00, 0x01, 0x03, 0x07, 0x0f };
#define MAX_PARENTS ARRAY_SIZE(mux_table)

static struct clk *clks[CLK_MAX + AC_MAX + PLL_MAX];

#define clk_regs	REG(0, 1)	/* G0.1 ~ CLKEN */
#define mux_regs	REG(2, 0)	/* G2.0 ~ CLK_SEL */
#define pll_regs	REG(4, 0)	/* G4.0 ~ PLL */

struct sp_clk {
	const char *name;
	u32 id;		/* defined in sp-q645.h, also for gate (reg_idx<<4)|(shift) */
	u32 mux;	/* mux reg_idx: MOON2.xx */
	u32 shift;	/* mux shift */
	u32 width;	/* mux width */
	const char *parent_names[MAX_PARENTS];
};

static const char * const default_parents[] = { EXT_CLK };

#define _(id, ...)	{ #id, id, ##__VA_ARGS__ }

static struct sp_clk sp_clks[] = {
	_(SYSTEM,	0,	0,	2, {"f_600m", "f_750m", "f_500m"}),
	_(CA55CORE0,	0,	6,	1, {"PLLC", "SYSTEM"}),
	_(CA55CORE1,	0,	11,	1, {"PLLC", "SYSTEM"}),
	_(CA55CORE2,	1,	0,	1, {"PLLC", "SYSTEM"}),
	_(CA55CORE3,	1,	5,	1, {"PLLC", "SYSTEM"}),
	_(CA55CUL3,	1,	10,	1, {"f_1200m", "SYSTEM"}),
	_(CA55),
	_(IOP),
	_(PBUS0),
	_(PBUS1),
	_(PBUS2),
	_(PBUS3),
	_(BR0),
	_(CARD_CTL0,	3,	7,	1, {"f_360m", "f_800m"}),
	_(CARD_CTL1,	3,	8,	1, {"f_360m", "f_800m"}),
	_(CARD_CTL2,	3,	9,	1, {"f_360m", "f_800m"}),

	_(CBDMA0),
	_(CPIOL),
	_(CPIOR),
	_(DDR_PHY0),
	_(SDCTRL0),
	_(DUMMY_MASTER0),
	_(DUMMY_MASTER1),
	_(DUMMY_MASTER2),
	_(EVDN,		3,	4,	1, {"f_800m", "f_1000m"}),
	_(SDPROT0),
	_(UMCTL2),
	_(GPU,		2,	9,	3, {"f_800m", "f_1000m", "f_1080m", "f_400m"}),
	_(HSM,		2,	5,	1, {"f_500m", "SYSTEM"}),
	_(RBUS_TOP),
	_(SPACC),
	_(INTERRUPT),

	_(N78,		3,	2,	2, {"f_1000m", "f_1200m", "f_1080m"}),
	_(SYSTOP),
	_(OTPRX),
	_(PMC),
	_(RBUS_BLOCKA),
	_(RBUS_BLOCKB),
	_(RBUS_rsv1),
	_(RBUS_rsv2),
	_(RTC,		0,	0,	0, {"rtcclk"}),
	_(MIPZ),
	_(SPIFL,	3,	11,	1, {"f_360m", "f_216m"}),
	_(BCH),
	_(SPIND,	3,	10,	1, {"f_600m", "f_800m"}),
	_(UADMA01),
	_(UADMA23),
	_(UA0,		2,	13,	1, {EXT_CLK, "f_200m"}),

	_(UA1,		2,	14,	1, {EXT_CLK, "f_200m"}),
	_(UA2,		2,	15,	1, {EXT_CLK, "f_200m"}),
	_(UA3,		3,	0,	1, {EXT_CLK, "f_200m"}),
	_(UA4),
	_(UA5),
	_(UADBG,	3,	1,	1, {EXT_CLK, "f_200m"}),
	_(UART2AXI),
	_(GDMAUA),
	_(UPHY0),
	_(USB30C0,	2,	2,	1, {"f_125m", "f_125m"}), /* CLKPIPE0_SRC also 125m */
	_(USB30C1,	2,	3,	1, {"f_125m", "f_125m"}), /* CLKPIPE1_SRC also 125m */
	_(U3PHY0,	2,	0,	2, {"f_100m", "f_50m", EXT_CLK}),
	_(U3PHY1,	2,	0,	2, {"f_100m", "f_50m", EXT_CLK}),
	_(USBC0),
	_(VCD,		0,	0,	0, {"f_360m"}),
	_(VCE,		24,	9,	2, {"f_540m", "f_600m", "f_750m"}),

	_(CM4,		3,	5,	2, {"SYSTEM", "SYSTEM_D2", "SYSTEM_D4"}), /* SYS_CLK, SYS_CLK/2, SYS_CLK/4 */
	_(STC0),
	_(STC_AV0),
	_(STC_AV1),
	_(STC_AV2),
	_(MAILBOX),
	_(PAI),
	_(PAII),
	_(DDRPHY,	0,	0,	0, {"f_800m"}),
	_(DDRCTL),
	_(I2CM0),
	_(SPI_COMBO_0),
	_(SPI_COMBO_1),
	_(SPI_COMBO_2),
	_(SPI_COMBO_3),
	_(SPI_COMBO_4),

	_(SPI_COMBO_5),
	_(CSIIW0,	0,	0,	0, {"mipiclk"}),
	_(MIPICSI0,	0,	0,	0, {"mipiclk"}),
	_(CSIIW1,	0,	0,	0, {"mipiclk"}),
	_(MIPICSI1,	0,	0,	0, {"mipiclk"}),
	_(CSIIW2,	0,	0,	0, {"mipiclk"}),
	_(MIPICSI2,	0,	0,	0, {"mipiclk"}),
	_(CSIIW3,	0,	0,	0, {"mipiclk"}),
	_(MIPICSI3,	0,	0,	0, {"mipiclk"}),
	_(VCL),
	_(DISP_PWM,	0,	0,	0, {"f_200m"}),
	_(I2CM1),
	_(I2CM2),
	_(I2CM3),
	_(I2CM4),
	_(I2CM5),

	_(UA6),
	_(UA7),
	_(UA8),
	_(AUD),
	_(VIDEO_CODEC),

	_(VCLCORE0,	25,	0,	4, {"f_500m", "f_600m", "f_400m", "f_300m", "f_200m"}),
	_(VCLCORE1,	25,	4,	3, {"f_400m", "f_500m", "f_300m", "f_200m"}),
	_(VCLCORE2,	25,	7,	3, {"f_300m", "f_400m", "f_100m", "f_200m"}),
};

/************************************************* PLL_A *************************************************/

/* from Q645_PLLA_REG_setting.xlsx */
struct {
	u32 rate;
	u32 regs[5];
} pa[] = {
	{
		.rate = 135475200,
		.regs = {
			0x5473, // G4.24
			0x0a11, // G4.25
			0x0014, // G4.26
			0x00c2, // G4.27
			0x0bfd, // G4.28
		}
	},
	{
		.rate = 147456000,
		.regs = {
			0x5473,
			0x0a11,
			0x0028,
			0x01f5,
			0x0bfd,
		}
	},
};

static void plla_set_rate(struct sp_pll *pll)
{
	const u32 *pp = pa[pll->idiv].regs;
	int i;

	for (i = 0; i < ARRAY_SIZE(pa->regs); i++) {
		writel(0xffff0000 | pp[i], pll->reg + (i * 4));
	}
}

static ulong plla_round_rate(struct sp_pll *pll, ulong rate)
{
	int i = ARRAY_SIZE(pa);

	while (--i) {
		if (rate >= pa[i].rate)
			break;
	}
	pll->idiv = i;
	return pa[i].rate;
}

/************************************************* SP_PLL *************************************************/

struct sp_div {
	u32 div2;
	u32 bits;
};

#define BITS(prediv, prescl, pstdiv) ((prediv << PREDIV)|(prescl << PRESCL)|(pstdiv << PSTDIV))
#define DIV(prediv, prescl, pstdiv) \
{ \
	prediv * 2 / prescl * pstdiv, \
	BITS((prediv - 1), (prescl - 1), (pstdiv - 1)) \
}

#define DIV2_BITS	BITS(3, 1, 3)

static struct sp_div divs[] = {
	DIV(4, 1, 4), // 32
	DIV(3, 1, 4), // 24
	DIV(3, 1, 3), // 18
	DIV(2, 1, 4), // 16
	DIV(2, 1, 3), // 12
	DIV(3, 2, 3), // 9
	DIV(4, 1, 1), // 8
	DIV(3, 1, 1), // 6
	DIV(2, 1, 1), // 4
	DIV(3, 2, 1), // 3
	DIV(1, 1, 1), // 2
	DIV(1, 2, 1), // 1
};

static ulong sp_pll_calc_div(struct sp_pll *pll, ulong rate)
{
	ulong ret = 0, mr = 0;
	int mi = 0, md = 0x7fffffff, d;
	int i, j = IS_PLLH() ? (ARRAY_SIZE(divs) - 6) : 0;

	for (i = ARRAY_SIZE(divs) - 1; i >= j; i--) {
		long br = pll->brate * 2 / divs[i].div2;

		ret = DIV_ROUND_CLOSEST(rate, br);
		if (ret >= FBKDIV_MIN && ret <= FBKDIV_MAX) {
			br *= ret;
			if (br < rate)
				d = rate - br;
			else if (br > rate)
				d = br - rate;
			else { // br == rate
				pll->idiv = i;
				return ret;
			}
			if (d < md) {
				md = d;
				mi = i;
				mr = ret;
			}
		}
	}

	pll->idiv = mi;
	return mr;
}

static ulong sp_pll_round_rate(struct clk *clk, ulong rate)
{
	struct sp_pll *pll = to_sp_pll(clk);
	ulong ret;

	TRACE;
	if (rate == XTAL)
		ret = XTAL; /* bypass */
	else if (IS_PLLA())
		ret = plla_round_rate(pll, rate);
	else {
		ret = sp_pll_calc_div(pll, rate);
		ret = pll->brate * 2 / divs[pll->idiv].div2 * ret;
	}

	return ret;
}

static ulong sp_pll_get_rate(struct clk *clk)
{
	struct sp_pll *pll = to_sp_pll(clk);
	u32 reg = readl(pll->reg);
	u32 bp = pll->bp;
	ulong ret;

	//TRACE;
	if (readl(pll->reg + (bp / 16) * 4) & BIT(bp % 16)) // bypass ?
		ret = XTAL;
	else if (IS_PLLA())
		ret = pa[pll->idiv].rate;
	else {
		u32 fbkdiv = MASK_GET(FBKDIV, FBKDIV_WIDTH, reg) + 64;
		u32 prediv = MASK_GET(PREDIV, 2, reg) + 1;
		u32 prescl = MASK_GET(PRESCL, 1, reg) + 1;
		u32 pstdiv = MASK_GET(PSTDIV, 2, reg) + 1;

		ret = pll->brate / prediv * fbkdiv * prescl / (IS_PLLH() ? 1 : pstdiv);
	}

	return ret;
}

static ulong sp_pll_set_rate(struct clk *clk, ulong rate)
{
	struct sp_pll *pll = to_sp_pll(clk);
	u32 bp = pll->bp;
	u32 reg;

	//TRACE;
	reg = BIT((bp % 16) + 16); // BP_HIWORD_MASK

	if (rate == XTAL) {
		reg |= BIT(bp % 16);
		writel(reg, pll->reg + (bp / 16) * 4); // set bp
	} else if (IS_PLLA())
		plla_set_rate(pll);
	else {
		u32 fbkdiv = sp_pll_calc_div(pll, rate) - FBKDIV_MIN;

		if (bp > 16)
			writel(reg, pll->reg + (bp / 16) * 4); // clear bp @ another reg
		reg |= 0x3ffe0000; // BIT[13:1] HIWORD_MASK
		reg |= MASK_SET(FBKDIV, FBKDIV_WIDTH, fbkdiv) | divs[pll->idiv].bits;
		writel(reg, pll->reg);
	}

	return 0;
}

static int sp_pll_enable(struct clk *clk)
{
	struct sp_pll *pll = to_sp_pll(clk);

	writel(BIT(PD_N + 16) | BIT(PD_N), pll->reg); /* power up */

	return 0;
}

static int sp_pll_disable(struct clk *clk)
{
	struct sp_pll *pll = to_sp_pll(clk);

	writel(BIT(PD_N + 16), pll->reg); /* power down */

	return 0;
}

const struct clk_ops sp_pll_ops = {
	.enable = sp_pll_enable,
	.disable = sp_pll_disable,
	.round_rate = sp_pll_round_rate,
	.get_rate = sp_pll_get_rate,
	.set_rate = sp_pll_set_rate,
};

struct clk *sp_register_pll_struct(const char *name, const char *parent_name,
				     struct sp_pll *pll)
{
	int ret;
	struct clk *clk = &pll->clk;

	ret = clk_register(clk, CLK_SP_PLL, name, parent_name);
	if (ret)
		return ERR_PTR(ret);
	return clk;
}

struct clk *clk_register_sp_pll(const char *name, void *reg, u32 bp)
{
	struct clk *clk;
	struct sp_pll *pll;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);
	pll->reg = reg;
	pll->brate = (reg == PLLN_CTL) ? (XTAL / 2) : XTAL;
	pll->bp = bp;

	clk = sp_register_pll_struct(name, EXT_CLK, pll);
	if (IS_ERR(clk))
		kfree(pll);
	return clk;
}

U_BOOT_DRIVER(sp_pll) = {
	.name	= CLK_SP_PLL,
	.id	= UCLASS_CLK,
	.ops	= &sp_pll_ops,
};

static struct clk *clk_register_sp_clk(struct sp_clk *sp_clk)
{
	const char * const *parent_names = sp_clk->parent_names[0] ? sp_clk->parent_names : default_parents;
	struct clk_mux *mux = NULL;
	struct clk_gate *gate;
	struct clk *clk;
	int num_parents = sp_clk->width + 1;

	if (sp_clk->width) {
		mux = kzalloc(sizeof(*mux), GFP_KERNEL);
		if (!mux)
			return ERR_PTR(-ENOMEM);
		mux->reg = mux_regs + (sp_clk->mux << 2);
		mux->shift = sp_clk->shift;
		mux->mask = BIT(sp_clk->width) - 1;
		mux->table = mux_table;
		mux->flags = CLK_MUX_HIWORD_MASK | CLK_MUX_ROUND_CLOSEST;
		mux->num_parents = num_parents;
	}

	gate = kzalloc(sizeof(*gate), GFP_KERNEL);
	if (!gate) {
		kfree(mux);
		return ERR_PTR(-ENOMEM);
	}
	gate->reg = clk_regs + (sp_clk->id >> 4 << 2);
	gate->bit_idx = sp_clk->id & 0x0f;
	gate->flags = CLK_GATE_HIWORD_MASK;

	clk = clk_register_composite(NULL, sp_clk->name,
				     parent_names, num_parents,
				     mux ? &mux->clk : NULL, &clk_mux_ops,
				     NULL, NULL,
				     &gate->clk, &clk_gate_ops,
				     CLK_IGNORE_UNUSED);
	if (IS_ERR(clk)) {
		kfree(mux);
		kfree(gate);
	}

	return clk;
}

int sp_clk_is_enabled(struct clk *clk)
{
	u32 id = clk->id;

	if (id >= PLL(0)) {
		struct sp_pll *pll = to_sp_pll(clks[id]);
		return readl(pll->reg) & BIT(PD_N);
	} else {
		struct sunplus_clk *priv = dev_get_priv(clk->dev);
		return readl(priv->base + (id >> 4) * 4) & BIT(id & 0x0f);
	}
}

void sp_clk_endisable(struct clk *clk, int enable)
{
	u32 id = clk->id;

	if (id >= PLL(0)) {
		struct sp_pll *pll = to_sp_pll(clks[id]);
		writel(BIT(PD_N + 16) | (enable << PD_N), pll->reg);
	} else {
		struct sunplus_clk *priv = dev_get_priv(clk->dev);
		u32 j = id & 0x0f;
		writel((enable << j) | BIT(j + 16), priv->base + (id >> 4) * 4);
	}
}

ulong sp_clk_get_rate(struct clk *clk)
{
	return clk_get_rate(clks[clk->id]);
}

ulong sp_clk_set_rate(struct clk *clk, ulong rate)
{
	return clk_set_rate(clks[clk->id], rate);
}

void sp_clk_dump(struct clk *clk)
{
	if (clk->dev == clkc_dev)
		clk = clks[clk->id];
	CLK_PRINT(clk->dev->name, clk_get_rate(clk));
}

void sp_plls_dump(void)
{
	for (int i = PLL(0); i < ARRAY_SIZE(clks); i++)
		sp_clk_dump(clks[i]);
}

int sp_clkc_init(void)
{
	int i;

	pr_info("sp-clkc init\n");

	/* PLLs */
	clk_dm(PLLS, clks[PLLS] = clk_register_sp_pll("PLLS", PLLS_CTL, 14));
	clk_dm(PLLC, clks[PLLC] = clk_register_sp_pll("PLLC", PLLC_CTL, 14));
	clk_dm(PLLN, clks[PLLN] = clk_register_sp_pll("PLLN", PLLN_CTL, 14));
	clk_dm(PLLH, clks[PLLH] = clk_register_sp_pll("PLLH", PLLH_CTL, 24)); // BP: 16 + 8
	clk_dm(PLLD, clks[PLLD] = clk_register_sp_pll("PLLD", PLLD_CTL, 14));
	clk_dm(PLLA, clks[PLLA] = clk_register_sp_pll("PLLA", PLLA_CTL, 2));

	pr_info("sp-clkc: register fixed_rate/factor\n");
	/* fixed frequency & fixed factor */
	clk_register_fixed_factor(NULL, "f_1200m", "PLLS", 0, 1, 2);
	clk_register_fixed_factor(NULL, "f_600m",  "PLLS", 0, 1, 4);
	clk_register_fixed_factor(NULL, "f_300m",  "PLLS", 0, 1, 8);
	clk_register_fixed_factor(NULL, "f_750m",  "PLLC", 0, 1, 2);
	clk_register_fixed_factor(NULL, "f_1000m", "PLLN", 0, 1, 1);
	clk_register_fixed_factor(NULL, "f_500m",  "PLLN", 0, 1, 2);
	clk_register_fixed_factor(NULL, "f_250m",  "PLLN", 0, 1, 4);
	clk_register_fixed_factor(NULL, "f_125m",  "PLLN", 0, 1, 8);
	clk_register_fixed_factor(NULL, "f_1080m", "PLLH", 0, 1, 2);
	clk_register_fixed_factor(NULL, "f_540m",  "PLLH", 0, 1, 4);
	clk_register_fixed_factor(NULL, "f_360m",  "PLLH", 0, 1, 6);
	clk_register_fixed_factor(NULL, "f_216m",  "PLLH", 0, 1, 10);
	clk_register_fixed_factor(NULL, "f_800m",  "PLLD", 0, 1, 2);
	clk_register_fixed_factor(NULL, "f_400m",  "PLLD", 0, 1, 4);
	clk_register_fixed_factor(NULL, "f_200m",  "PLLD", 0, 1, 8);
	clk_register_fixed_factor(NULL, "f_100m", EXT_CLK, 0, 4, 1);
	clk_register_fixed_factor(NULL, "f_50m",  EXT_CLK, 0, 2, 1);

	pr_info("sp-clkc: register sp_clks\n");

	/* sp_clks */
	for (i = 0; i < ARRAY_SIZE(sp_clks); i++) {
		struct sp_clk *sp_clk = &sp_clks[i];
		int j = sp_clk->id;

		clk_dm(j, clks[j] = clk_register_sp_clk(sp_clk));
		if (IS_ERR(clks[j]))
			return PTR_ERR(clks[j]);

		sp_clk_dump(clks[j]);
		if (i == 0) {
			// SYSTEM_D2/D4's parent is clks[0]:SYSTEM
			clk_register_fixed_factor(NULL, "SYSTEM_D2", "SYSTEM", 0, 1, 2); // SYS_CLK/2
			clk_register_fixed_factor(NULL, "SYSTEM_D4", "SYSTEM", 0, 1, 4); // SYS_CLK/4
		}
	}
	pr_info("sp-clkc init done!\n");
	return 0;
}
