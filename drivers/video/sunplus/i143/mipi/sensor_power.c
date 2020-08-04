#include <common.h>

#include "isp_api.h"


void powerSensorOn_RAM(void)
{
	//u8 i;

	ISPAPB_LOGI("%s start\n", __FUNCTION__);

	/* enable sensor mclk and i2c sck */
	ISPAPB0_REG8(0x2781) = 0x00;
	ISPAPB0_REG8(0x2785) = 0x08;

	ISPAPB0_REG8(0x2042) |= 0x03;       /* xgpio[8,9] output enable */
	ISPAPB0_REG8(0x2044) |= 0x03;       /* xgpio[8,9] output high - power up */

	//for (i = 0; i < 100; i++);          /* delay us */
	//udelay(1000);                        /* delay ms */
	mdelay(1);

	//ISPAPB0_REG8(0x2042) &= 0xFD;
	//ISPAPB0_REG8(0x2044) &= 0xFD;
	ISPAPB0_REG8(0x2044) &= (~0x1);     /* xgpio[8] output low - reset */

	mdelay(1);

	ISPAPB0_REG8(0x2044) |= 0x03;		/* xgpio[8,9] output high - power up */
}

void powerSensorDown_RAM(void)
{
	//u8 i;

	ISPAPB_LOGI("%s start\n", __FUNCTION__);

	/* disable sensor mclk and i2c sck */
	//ISPAPB0_REG8(0x2781) = 0x48;
	ISPAPB0_REG8(0x2785) = 0x00;
	//for (i = 0; i < 6; i++);            /* delay 128 extclk = 6 us */
	udelay(6);                          /* delay 128 extclk = 6 us */

	/* xgpio[8) - 0: reset, 1: normal */
	ISPAPB0_REG8(0x2042) |= 0x01;       /* xgpio[8] output enable */
	ISPAPB0_REG8(0x2044) &= (~0x1);     /* xgpio[8] output low - reset */

	/* xgpio[9) - 0: power down, 1: power up */
	ISPAPB0_REG8(0x2042) |= 0x02;       /* xgpio[9] output enable */
	ISPAPB0_REG8(0x2044) &= 0xFD;       /* xgpio[9] output low - power down */
}
