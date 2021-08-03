#include "pinctrl_sunplus.h"


static const unsigned pins_spif[] = { 6, 7, 8, 9, 10, 11 };
static const sppctlgrp_t q645grps_spif[] = {
	EGRP("SPI_FLASH", 1, pins_spif),
};

static const unsigned pins_emmc[] = { 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };
static const sppctlgrp_t q645grps_emmc[] = {
	EGRP("CARD0_EMMC", 1, pins_emmc)
};

static const unsigned pins_snand1[] = { 16, 17, 18, 19, 20, 21 };
static const unsigned pins_snand2[] = { 6,   7,  8,  9, 10, 11 };
static const sppctlgrp_t q645grps_snand[] = {
	EGRP("SPI_NAND1", 1, pins_snand1),
	EGRP("SPI_NAND2", 2, pins_snand2)
};

static const unsigned pins_sdc30[] = { 28, 29, 30, 31, 32, 33 };
static const sppctlgrp_t q645grps_sdc30[] = {
	EGRP("SD_CARD", 1, pins_sdc30)
};

static const unsigned pins_sdio30[] = { 34, 35, 36, 37, 38, 39 };
static const sppctlgrp_t q645grps_sdio30[] = {
	EGRP("SDIO", 1, pins_sdio30)
};

static const unsigned pins_uart0[] = { 22, 23 };
static const sppctlgrp_t q645grps_uart0[] = {
	EGRP("UART0", 1, pins_uart0)
};

static const unsigned pins_uart1[] = { 24, 25, 26, 27 };
static const sppctlgrp_t q645grps_uart1[] = {
	EGRP("UART1", 1, pins_uart1)
};

static const unsigned pins_uart2[] = { 40, 41, 42, 43 };
static const sppctlgrp_t q645grps_uart2[] = {
	EGRP("UART2", 1, pins_uart2)
};

static const unsigned pins_uart3[] = { 44, 45 };
static const sppctlgrp_t q645grps_uart3[] = {
	EGRP("UART3", 1, pins_uart3)
};

static const unsigned pins_uart4[] = { 46, 47 };
static const sppctlgrp_t q645grps_uart4[] = {
	EGRP("UART4", 1, pins_uart4)
};

static const unsigned pins_uart6[] = { 48, 49 };
static const sppctlgrp_t q645grps_uart6[] = {
	EGRP("UART6", 1, pins_uart6)
};

static const unsigned pins_uart7[] = { 50, 51 };
static const sppctlgrp_t q645grps_uart7[] = {
	EGRP("UART7", 1, pins_uart7)
};

static const unsigned pins_uart8[] = { 52, 53 };
static const sppctlgrp_t q645grps_uart8[] = {
	EGRP("UART8", 1, pins_uart8)
};

static const unsigned pins_spicombo0[] = { 54, 55, 56, 57 };
static const sppctlgrp_t q645grps_spimaster0[] = {
	EGRP("SPI_MASTER0", 1, pins_spicombo0)
};
static const sppctlgrp_t q645grps_spislave0[] = {
	EGRP("SPI_SLAVE0", 1, pins_spicombo0)
};

static const unsigned pins_spicombo1[] = { 58, 59, 60, 61 };
static const sppctlgrp_t q645grps_spimaster1[] = {
	EGRP("SPI_MASTER1", 1, pins_spicombo1)
};
static const sppctlgrp_t q645grps_spislave1[] = {
	EGRP("SPI_SLAVE1", 1, pins_spicombo1)
};

static const unsigned pins_spicombo2[] = { 63, 64, 65, 66 };
static const sppctlgrp_t q645grps_spimaster2[] = {
	EGRP("SPI_MASTER2", 1, pins_spicombo2)
};
static const sppctlgrp_t q645grps_spislave2[] = {
	EGRP("SPI_SLAVE2", 1, pins_spicombo2)
};

static const unsigned pins_spicombo3[] = { 67, 68, 69, 70 };
static const sppctlgrp_t q645grps_spimaster3[] = {
	EGRP("SPI_MASTER3", 1, pins_spicombo3)
};
static const sppctlgrp_t q645grps_spislave3[] = {
	EGRP("SPI_SLAVE3", 1, pins_spicombo3)
};

static const unsigned pins_spicombo4[] = { 71, 72, 73, 74 };
static const sppctlgrp_t q645grps_spimaster4[] = {
	EGRP("SPI_MASTER4", 1, pins_spicombo4)
};
static const sppctlgrp_t q645grps_spislave4[] = {
	EGRP("SPI_SLAVE4", 1, pins_spicombo4)
};

static const unsigned pins_spicombo5[] = { 77, 78, 79, 80 };
static const sppctlgrp_t q645grps_spimaster5[] = {
	EGRP("SPI_MASTER5", 1, pins_spicombo5)
};
static const sppctlgrp_t q645grps_spislave5[] = {
	EGRP("SPI_SLAVE5", 1, pins_spicombo5)
};

static const unsigned pins_i2cm0[] = { 75, 76 };
static const sppctlgrp_t q645grps_i2cm0[] = {
	EGRP("I2C_MASTER0", 1, pins_i2cm0)
};

static const unsigned pins_i2cm1[] = { 81, 82 };
static const sppctlgrp_t q645grps_i2cm1[] = {
	EGRP("I2C_MASTER1", 1, pins_i2cm1)
};

static const unsigned pins_i2cm2[] = { 83, 84 };
static const sppctlgrp_t q645grps_i2cm2[] = {
	EGRP("I2C_MASTER2", 1, pins_i2cm2)
};

static const unsigned pins_i2cm3[] = { 85, 86 };
static const sppctlgrp_t q645grps_i2cm3[] = {
	EGRP("I2C_MASTER3", 1, pins_i2cm3)
};

static const unsigned pins_i2cm4[] = { 87, 88 };
static const sppctlgrp_t q645grps_i2cm4[] = {
	EGRP("I2C_MASTER4", 1, pins_i2cm4)
};

static const unsigned pins_i2cm5[] = { 89, 90 };
static const sppctlgrp_t q645grps_i2cm5[] = {
	EGRP("I2C_MASTER5", 1, pins_i2cm5)
};

static const unsigned pins_pwm[] = { 58, 59, 60, 61 };
static const sppctlgrp_t q645grps_pwm[] = {
	EGRP("PWM", 1, pins_pwm)
};

static const unsigned pins_int_x1[] = { 0 };
static const unsigned pins_int_x2[] = { 1 };
static const unsigned pins_int_x3[] = { 2 };
static const unsigned pins_int_x4[] = { 3 };
static const unsigned pins_int_x5[] = { 46 };
static const unsigned pins_int_x6[] = { 106 };
static const unsigned pins_int_x7[] = { 107 };
static const sppctlgrp_t q645grps_int0[] = {
	EGRP("INT0_X1", 1, pins_int_x1),
	EGRP("INT0_X2", 1, pins_int_x2),
	EGRP("INT0_X3", 1, pins_int_x3),
	EGRP("INT0_X4", 1, pins_int_x4),
	EGRP("INT0_X5", 1, pins_int_x5),
	EGRP("INT0_X6", 1, pins_int_x6),
	EGRP("INT0_X7", 1, pins_int_x7)
};
static const sppctlgrp_t q645grps_int1[] = {
	EGRP("INT1_X1", 1, pins_int_x1),
	EGRP("INT1_X2", 1, pins_int_x2),
	EGRP("INT1_X3", 1, pins_int_x3),
	EGRP("INT1_X4", 1, pins_int_x4),
	EGRP("INT1_X5", 1, pins_int_x5),
	EGRP("INT1_X6", 1, pins_int_x6),
	EGRP("INT1_X7", 1, pins_int_x7)
};
static const sppctlgrp_t q645grps_int2[] = {
	EGRP("INT2_X1", 1, pins_int_x1),
	EGRP("INT2_X2", 1, pins_int_x2),
	EGRP("INT2_X3", 1, pins_int_x3),
	EGRP("INT2_X4", 1, pins_int_x4),
	EGRP("INT2_X5", 1, pins_int_x5),
	EGRP("INT2_X6", 1, pins_int_x6),
	EGRP("INT2_X7", 1, pins_int_x7)
};
static const sppctlgrp_t q645grps_int3[] = {
	EGRP("INT3_X1", 1, pins_int_x1),
	EGRP("INT3_X2", 1, pins_int_x2),
	EGRP("INT3_X3", 1, pins_int_x3),
	EGRP("INT3_X4", 1, pins_int_x4),
	EGRP("INT3_X5", 1, pins_int_x5),
	EGRP("INT3_X6", 1, pins_int_x6),
	EGRP("INT3_X7", 1, pins_int_x7)
};
static const sppctlgrp_t q645grps_int4[] = {
	EGRP("INT4_X1", 1, pins_int_x1),
	EGRP("INT4_X2", 1, pins_int_x2),
	EGRP("INT4_X3", 1, pins_int_x3),
	EGRP("INT4_X4", 1, pins_int_x4),
	EGRP("INT4_X5", 1, pins_int_x5),
	EGRP("INT4_X6", 1, pins_int_x6),
	EGRP("INT4_X7", 1, pins_int_x7)
};
static const sppctlgrp_t q645grps_int5[] = {
	EGRP("INT5_X1", 1, pins_int_x1),
	EGRP("INT5_X2", 1, pins_int_x2),
	EGRP("INT5_X3", 1, pins_int_x3),
	EGRP("INT5_X4", 1, pins_int_x4),
	EGRP("INT5_X5", 1, pins_int_x5),
	EGRP("INT5_X6", 1, pins_int_x6),
	EGRP("INT5_X7", 1, pins_int_x7)
};
static const sppctlgrp_t q645grps_int6[] = {
	EGRP("INT6_X1", 1, pins_int_x1),
	EGRP("INT6_X2", 1, pins_int_x2),
	EGRP("INT6_X3", 1, pins_int_x3),
	EGRP("INT6_X4", 1, pins_int_x4),
	EGRP("INT6_X5", 1, pins_int_x5),
	EGRP("INT6_X6", 1, pins_int_x6),
	EGRP("INT6_X7", 1, pins_int_x7)
};
static const sppctlgrp_t q645grps_int7[] = {
	EGRP("INT7_X1", 1, pins_int_x1),
	EGRP("INT7_X2", 1, pins_int_x2),
	EGRP("INT7_X3", 1, pins_int_x3),
	EGRP("INT7_X4", 1, pins_int_x4),
	EGRP("INT7_X5", 1, pins_int_x5),
	EGRP("INT7_X6", 1, pins_int_x6),
	EGRP("INT7_X7", 1, pins_int_x7)
};

func_t list_funcs[] = {
	FNCE("SPI_FLASH",       fOFF_G, 1, 0, 2, q645grps_spif),
	FNCE("PWM",             fOFF_G, 1, 2, 1, q645grps_pwm),
	FNCE("CARD0_EMMC",      fOFF_G, 1, 3, 1, q645grps_emmc),
	FNCE("SPI_NAND",        fOFF_G, 1, 4, 2, q645grps_snand),
	FNCE("SD_CARD",         fOFF_G, 1, 6, 1, q645grps_sdc30),
	FNCE("SDIO",            fOFF_G, 1, 7, 1, q645grps_sdio30),
	FNCE("UART0",           fOFF_G, 1, 8, 1, q645grps_uart0),
	FNCE("UART1",           fOFF_G, 1, 9, 1, q645grps_uart1),
	FNCE("UART2",           fOFF_G, 1,10, 1, q645grps_uart2),
	FNCE("UART3",           fOFF_G, 1,11, 1, q645grps_uart3),
	FNCE("UART4",           fOFF_G, 1,12, 1, q645grps_uart4),
	FNCE("UART6",           fOFF_G, 1,13, 1, q645grps_uart6),
	FNCE("UART7",           fOFF_G, 1,14, 1, q645grps_uart7),
	FNCE("UART8",           fOFF_G, 1,15, 1, q645grps_uart8),
	FNCE("SPI_MASTER0",     fOFF_G, 2, 0, 1, q645grps_spimaster0),
	FNCE("SPI_SLAVE0",      fOFF_G, 2, 1, 1, q645grps_spislave0),
	FNCE("SPI_MASTER1",     fOFF_G, 2, 6, 1, q645grps_spimaster1),
	FNCE("SPI_SLAVE1",      fOFF_G, 2, 7, 1, q645grps_spislave1),
	FNCE("SPI_MASTER2",     fOFF_G, 2, 8, 1, q645grps_spimaster2),
	FNCE("SPI_SLAVE2",      fOFF_G, 2, 9, 1, q645grps_spislave2),
	FNCE("SPI_MASTER3",     fOFF_G, 2,10, 1, q645grps_spimaster3),
	FNCE("SPI_SLAVE3",      fOFF_G, 2,11, 1, q645grps_spislave3),
	FNCE("SPI_MASTER4",     fOFF_G, 2,12, 1, q645grps_spimaster4),
	FNCE("SPI_SLAVE4",      fOFF_G, 2,13, 1, q645grps_spislave4),
	FNCE("SPI_MASTER5",     fOFF_G, 2,14, 1, q645grps_spimaster5),
	FNCE("SPI_SLAVE5",      fOFF_G, 2,15, 1, q645grps_spislave5),
	FNCE("I2C_MASTER0",     fOFF_G, 3, 0, 1, q645grps_i2cm0),
	FNCE("I2C_MASTER1",     fOFF_G, 3, 1, 1, q645grps_i2cm1),
	FNCE("I2C_MASTER2",     fOFF_G, 3, 2, 1, q645grps_i2cm2),
	FNCE("I2C_MASTER3",     fOFF_G, 3, 3, 1, q645grps_i2cm3),
	FNCE("I2C_MASTER4",     fOFF_G, 3, 4, 1, q645grps_i2cm4),
	FNCE("I2C_MASTER5",     fOFF_G, 3, 5, 1, q645grps_i2cm5),

	FNCE("INT0",            fOFF_G, 4,10, 3, q645grps_int0),
	FNCE("INT1",            fOFF_G, 4,13, 3, q645grps_int1),
	FNCE("INT2",            fOFF_G, 5, 0, 3, q645grps_int2),
	FNCE("INT3",            fOFF_G, 5, 3, 3, q645grps_int3),
	FNCE("INT4",            fOFF_G, 5, 6, 3, q645grps_int4),
	FNCE("INT5",            fOFF_G, 5, 9, 3, q645grps_int5),
	FNCE("INT6",            fOFF_G, 5,12, 3, q645grps_int6),
	FNCE("INT7",            fOFF_G, 6, 0, 3, q645grps_int7),
};

const int list_funcsSZ = ARRAY_SIZE(list_funcs);
