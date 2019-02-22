#ifndef _SP_GPIO_H_
#define _SP_GPIO_H_

#define REG_BASE           0x9c000000
#define RF_GRP(_grp, _reg) ((((_grp) * 32 + (_reg)) * 4) + REG_BASE)
#define RF_MASK_V(_mask, _val)       (((_mask) << 16) | (_val))
#define RF_MASK_V_SET(_mask)         (((_mask) << 16) | (_mask))
#define RF_MASK_V_CLR(_mask)         (((_mask) << 16) | 0)

struct moon2_regs {
	unsigned int sft_cfg[32];
};

struct moon3_regs {
	unsigned int sft_cfg[32];
};

#define MOON2_REG ((volatile struct moon2_regs *)RF_GRP(2, 0))
#define MOON3_REG ((volatile struct moon3_regs *)RF_GRP(3, 0))

typedef enum
{
	
	//G2
	PMX_L2SW_CLK_OUT    	= 0x02004000,			// (0-64, 0)
	PMX_L2SW_MAC_SMI_MDC       = 0x02004008,			// (0-64, 8)	
	PMX_L2SW_LED_FLASH0        = 0x02014000,			// (0-64, 0)	
	PMX_L2SW_LED_FLASH1        = 0x02014008,			// (0-64, 8)
	PMX_L2SW_LED_ON0           = 0x02024000,			// (0-64, 0)
	PMX_L2SW_LED_ON1           = 0x02024008,			// (0-64, 8)
	PMX_L2SW_MAC_SMI_MDIO      = 0x02034000,			// (0-64, 0)
	PMX_L2SW_P0_MAC_RMII_TXEN  = 0x02034008,			// (0-64, 8)
	PMX_L2SW_P0_MAC_RMII_TXD0  = 0x02044000,			// (0-64, 0)	
	PMX_L2SW_P0_MAC_RMII_TXD1  = 0x02044008,			// (0-64, 8)
	PMX_L2SW_P0_MAC_RMII_CRSDV = 0x02054000,			// (0-64, 0)	
	PMX_L2SW_P0_MAC_RMII_RXD0  = 0x02054008,			// (0-64, 8)
	PMX_L2SW_P0_MAC_RMII_RXD1  = 0x02064000,			// (0-64, 0)	
	PMX_L2SW_P0_MAC_RMII_RXER  = 0x02064008,			// (0-64, 8)
	PMX_L2SW_P1_MAC_RMII_TXEN  = 0x02074000,			// (0-64, 0)	
	PMX_L2SW_P1_MAC_RMII_TXD0  = 0x02074008,			// (0-64, 8)
	PMX_L2SW_P1_MAC_RMII_TXD1  = 0x02084000,			// (0-64, 0)	
	PMX_L2SW_P1_MAC_RMII_CRSDV = 0x02084008,			// (0-64, 8)
	PMX_L2SW_P1_MAC_RMII_RXD0  = 0x02094000,			// (0-64, 0)	
	PMX_L2SW_P1_MAC_RMII_RXD1  = 0x02094008,			// (0-64, 8)
	PMX_L2SW_P1_MAC_RMII_RXER  = 0x020a4000,			// (0-64, 0)
	PMX_DAISY_MODE = 0x020a4008,			// (0-64, 8)
	PMX_SDIO_CLK   = 0x020b4000,			// (0-64, 0)
	PMX_SDIO_CMD   = 0x020b4008,			// (0-64, 8)
	PMX_SDIO_D0    = 0x020c4000,			// (0-64, 0)
	PMX_SDIO_D1    = 0x020c4008,			// (0-64, 8)
	PMX_SDIO_D2    = 0x020d4000,			// (0-64, 0)
	PMX_SDIO_D3    = 0x020d4008,			// (0-64, 8)
	PMX_PWM0       = 0x020e4000,			// (0-64, 0)
	PMX_PWM1       = 0x020e4008,			// (0-64, 8)
	PMX_PWM2       = 0x020f4000,			// (0-64, 0)
	PMX_PWM3       = 0x020f4008,			// (0-64, 8)
	PMX_PWM4       = 0x02104000,			// (0-64, 0)
	PMX_PWM5       = 0x02104008,			// (0-64, 8)
	PMX_PWM6       = 0x02114000,			// (0-64, 0)
	PMX_PWM7       = 0x02114008,			// (0-64, 8)
	PMX_ICM0_D     = 0x02124000,			// (0-64, 0)
	PMX_ICM1_D     = 0x02124008,			// (0-64, 8)
	PMX_ICM2_D     = 0x02134000,			// (0-64, 0)
	PMX_ICM3_D     = 0x02134008,			// (0-64, 8)
	PMX_ICM0_CLK   = 0x02144000,			// (0-64, 0)
	PMX_ICM1_CLK   = 0x02144008,			// (0-64, 8)
	PMX_ICM2_CLK   = 0x02154000,			// (0-64, 0)
	PMX_ICM3_CLK   = 0x02154008,			// (0-64, 8)
	PMX_SPIM0_INT  = 0x02164000,			// (0-64, 0)
	PMX_SPIM0_CLK  = 0x02164008,			// (0-64, 8)
	PMX_SPIM0_CEN  = 0x02174000,			// (0-64, 0)
	PMX_SPIM0_DO   = 0x02174008,			// (0-64, 8)
	PMX_SPIM0_DI   = 0x02184000,			// (0-64, 0)
	PMX_SPIM1_INT  = 0x02184008,			// (0-64, 8)
	PMX_SPIM1_CLK  = 0x02194000,			// (0-64, 0)
	PMX_SPIM1_CEN  = 0x02194008,			// (0-64, 8)
	PMX_SPIM1_DO   = 0x021a4000,			// (0-64, 0)
	PMX_SPIM1_DI   = 0x021a4008,			// (0-64, 8)
	PMX_SPIM2_INT  = 0x021b4000,			// (0-64, 0)
	PMX_SPIM2_CLK  = 0x021b4008,			// (0-64, 8)
	PMX_SPIM2_CEN  = 0x021c4000,			// (0-64, 0)
	PMX_SPIM2_DO   = 0x021c4008,			// (0-64, 8)
	PMX_SPIM2_DI   = 0x021d4000,			// (0-64, 0)
	PMX_SPIM3_INT  = 0x021d4008,			// (0-64, 8)
	PMX_SPIM3_CLK  = 0x021e4000,			// (0-64, 0)
	PMX_SPIM3_CEN  = 0x021e4008,			// (0-64, 8)
	PMX_SPIM3_DO   = 0x021f4000,			// (0-64, 0)
	PMX_SPIM3_DI   = 0x021f4008,			// (0-64, 8)
	//G3
	PMX_SPI0S_INT   = 0x03004000,			// (0-64, 0)
	PMX_SPI0S_CLK   = 0x03004008,			// (0-64, 8)
	PMX_SPI0S_EN    = 0x03014000,			// (0-64, 0)
	PMX_SPI0S_DO    = 0x03014008,			// (0-64, 8)
	PMX_SPI0S_DI    = 0x03024000,			// (0-64, 0)
	PMX_SPI1S_INT   = 0x03024008,			// (0-64, 8)
	PMX_SPI1S_CLK   = 0x03034000,			// (0-64, 0)
	PMX_SPI1S_EN    = 0x03034008,			// (0-64, 8)
	PMX_SPI1S_DO    = 0x03044000,			// (0-64, 0)
	PMX_SPI1S_DI    = 0x03044008,			// (0-64, 8)
	PMX_SPI2S_INT   = 0x03054000,			// (0-64, 0)
	PMX_SPI2S_CLK   = 0x03054008,			// (0-64, 8)
	PMX_SPI2S_EN    = 0x03064000,			// (0-64, 0)
	PMX_SPI2S_DO    = 0x03064008,			// (0-64, 8)
	PMX_SPI2S_DI    = 0x03074000,			// (0-64, 0)	
	PMX_SPI3S_INT   = 0x03074008,			// (0-64, 8)
	PMX_SPI3S_CLK   = 0x03084000,			// (0-64, 0)
	PMX_SPI3S_EN    = 0x03084008,			// (0-64, 8)
	PMX_SPI3S_DO    = 0x03094000,			// (0-64, 0)
	PMX_SPI3S_DI    = 0x03094008,			// (0-64, 8)
	PMX_I2CM0_CK    = 0x030a4000,			// (0-64, 0)
	PMX_I2CM0_DAT   = 0x030a4008,			// (0-64, 8)
	PMX_I2CM1_CK    = 0x030b4000,			// (0-64, 0)	
	PMX_I2CM1_DAT   = 0x030b4008,			// (0-64, 8)
	PMX_I2CM2_CK    = 0x030c4000,			// (0-64, 0)		
	PMX_I2CM2_D     = 0x030c4008,			// (0-64, 8)
	PMX_I2CM3_CK    = 0x030d4000,			// (0-64, 0)		
	PMX_I2CM3_D     = 0x030d4008,			// (0-64, 8)
	PMX_UA1_TX      = 0x030e4000,			// (0-64, 0)		
	PMX_UA1_RX      = 0x030e4008,			// (0-64, 8)
	PMX_UA1_CTS     = 0x030f4000,			// (0-64, 0)	
	PMX_UA1_RTS     = 0x030f4008,			// (0-64, 8)
	PMX_UA2_TX      = 0x03104000,			// (0-64, 0)	
	PMX_UA2_RX      = 0x03104008,			// (0-64, 8)
	PMX_UA2_CTS     = 0x03114000,			// (0-64, 0)	
	PMX_UA2_RTS     = 0x03114008,			// (0-64, 8)
	PMX_UA3_TX      = 0x03124000,			// (0-64, 0)	
	PMX_UA3_RX      = 0x03124008,			// (0-64, 8)
	PMX_UA3_CTS     = 0x03134000,			// (0-64, 0)	
	PMX_UA3_RTS     = 0x03134008,			// (0-64, 8)
	PMX_UA4_TX      = 0x03144000,			// (0-64, 0)	
	PMX_UA4_RX      = 0x03144008,			// (0-64, 8)
	PMX_UA4_CTS     = 0x03154000,			// (0-64, 0)	
	PMX_UA4_RTS     = 0x03154008,			// (0-64, 8)
	PMX_TIMER0_INT  = 0x03164000,			// (0-64, 0)		
	PMX_TIMER1_INT  = 0x03164008,			// (0-64, 8)
	PMX_TIMER2_INT  = 0x03174000,			// (0-64, 0)	
	PMX_TIMER3_INT  = 0x03174008,			// (0-64, 8)
	PMX_GPIO_INT0   = 0x03184000,			// (0-64, 0)	
	PMX_GPIO_INT1   = 0x03184008,			// (0-64, 8)	
	PMX_GPIO_INT2   = 0x03194000,			// (0-64, 0)	
	PMX_GPIO_INT3   = 0x03194008,			// (0-64, 8)	
	PMX_GPIO_INT4   = 0x031a4000,			// (0-64, 0)	
	PMX_GPIO_INT5   = 0x031a4008,			// (0-64, 8)	
	PMX_GPIO_INT6   = 0x031b4000,			// (0-64, 0)	
	PMX_GPIO_INT7   = 0x031b4008,			// (0-64, 8)

}PMXSEL_ID;


typedef struct PMXSEL_S {
	uint id;
	uint val;
}PMXSEL_T;

int gpio_pin_mux_sel(PMXSEL_ID id, u32 sel)
{
	u32 grp ,idx, max_value, reg_val, mask, bit_num;
	
	grp = (id >> 24) & 0xff;
	if (grp > 0x03) {
		return -EINVAL;
	}	
	
	idx = (id >> 16) & 0xff;
	if (idx > 0x1f) {
		return -EINVAL;
	}
	
	max_value = (id >> 8) & 0xff;
	if (sel > max_value) {
		return -EINVAL;
	}
	
	bit_num = id & 0xff;
	
	if (max_value == 1) {
		mask = 0x01 << bit_num;
	}
	else if ((max_value == 2) || (max_value == 3)) {
		mask = 0x03 << bit_num;
	}
	else {
		mask = 0x7f << bit_num;
	}	

	reg_val = *((volatile unsigned int *)(RF_GRP(grp,idx)));
	reg_val |= mask << 0x10 ;
	reg_val &= (~mask);	
	reg_val = ((sel << bit_num) | (mask << 0x10));		
	*((volatile unsigned int *) (RF_GRP(grp,idx))) = reg_val;
	
	return 0;
}

int gpio_pin_mux_get(PMXSEL_ID id, u32 *sel)
{
	u32 grp , idx, max_value, reg_val, mask, bit_num;
	
	grp = (id >> 24) & 0xff;
	
	idx = (id >> 16) & 0xff;
	if (idx > 0x11) {
		return -EINVAL;
	}

	max_value = (id >> 8) & 0xff;
	if (*sel > max_value) {
		return -EINVAL;
	}
	
	bit_num = id & 0xff;

	if (max_value == 1) {
		mask = 0x01 << bit_num;
	}
	else if ((max_value == 2) || (max_value == 3)) {
		mask = 0x03 << bit_num;
	}
	else {
		mask = 0x7f << bit_num;
	}

	reg_val = *((volatile unsigned int *)(RF_GRP(grp,idx)));
	reg_val &= mask;
	*sel = (reg_val >> bit_num);
	return 0;
}

#define GPIO_PIN_MUX_SEL(a,d) gpio_pin_mux_sel(a, d)

#endif /* _SP_GPIO_H_ */

