// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2023
 * LH Kuo, sunplus, lh.kuo@sunplus.com.
*/
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <malloc.h>
#include <pci.h>
#include <reset.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <dm/device_compat.h>
#include <linux/err.h>
#include <wait_bit.h>

typedef struct regs_i2cm {
	u32 control0;		/* 00 */
	u32 control1;		/* 01 */
	u32 control2;		/* 02 */
	u32 control3;		/* 03 */
	u32 control4;		/* 04 */
	u32 control5;		/* 05 */
	u32 i2cm_status0;	/* 06 */
	u32 interrupt;		/* 07 */
	u32 int_en0;		/* 08 */
	u32 i2cm_mode;		/* 09 */
	u32 i2cm_status1;	/* 10 */
	u32 i2cm_status2;	/* 11 */
	u32 control6;		/* 12 */
	u32 int_en1;		/* 13 */
	u32 i2cm_status3;	/* 14 */
	u32 i2cm_status4;	/* 15 */
	u32 int_en2;		/* 16 */
	u32 control7;		/* 17 */
	u32 control8;		/* 18 */
	u32 control9;		/* 19 */
	u32 reserved[3];	/* 20~22 */
	u32 version;		/* 23 */
	u32 data00_03;		/* 24 */
	u32 data04_07;		/* 25 */
	u32 data08_11;		/* 26 */
	u32 data12_15;		/* 27 */
	u32 data16_19;		/* 28 */
	u32 data20_23;		/* 29 */
	u32 data24_27;		/* 30 */
	u32 data28_31;		/* 31 */
	u32 hw_version;		/* dma 00 */
	u32 dma_config;		/* dma 01 */
	u32 dma_length;		/* dma 02 */
	u32 dma_addr;		/* dma 03 */
	u32 port_mux;		/* dma 04 */
	u32 int_flag;		/* dma 05 */
	u32 int_en;		/* dma 06 */
	u32 sw_reset_state;	/* dma 07 */
	u32 reserved_dma[2];	/* dma 08~09 */
	u32 sg_dma_index;	/* dma 10 */
	u32 sg_dma_config;	/* dma 11 */
	u32 sg_dma_length;	/* dma 12 */
	u32 sg_dma_addr;	/* dma 13 */
	u32 reserved2;		/* dma 14 */
	u32 sg_setting;		/* dma 15 */
	u32 threshold;		/* dma 16 */
	u32 reserved3;		/* dma 17 */
	u32 gdma_read_timeout;	/* dma 18 */
	u32 gdma_write_timeout;	/* dma 19 */
	u32 ip_read_timeout;	/* dma 20 */
	u32 ip_write_timeout;	/* dma 21 */
	u32 write_cnt_debug;	/* dma 22 */
	u32 w_byte_en_debug;	/* dma 23 */
	u32 sw_reset_write_cnt_debug;	/* dma 24 */
	u32 reserved4[7];	/* dma 25~31 */
}I2C_MAS_REG;

/****************************************
* I2C Master
****************************************/
//control0
#define I2C_CTL0_FREQ(x)                  (x<<24)  //bit[26:24]
#define I2C_CTL0_PREFETCH                 (1<<18)  //Now as read mode need to set high, otherwise don??t care
#define I2C_CTL0_RESTART_EN               (1<<17)  //0:disable 1:enable
#define I2C_CTL0_SUBADDR_EN               (1<<16)  //For restart mode need to set high
#define I2C_CTL0_SW_RESET                 (1<<15)
#define I2C_CTL0_SLAVE_ADDR(x)            (x<<1)   //bit[7:1]

//control1
#define I2C_CTL1_ALL_CLR                  (0x3FF)
#define I2C_CTL1_EMPTY_CLR                (1<<9)
#define I2C_CTL1_SCL_HOLD_TOO_LONG_CLR    (1<<8)
#define I2C_CTL1_SCL_WAIT_CLR             (1<<7)
#define I2C_CTL1_EMPTY_THRESHOLD_CLR      (1<<6)
#define I2C_CTL1_DATA_NACK_CLR            (1<<5)
#define I2C_CTL1_ADDRESS_NACK_CLR         (1<<4)
#define I2C_CTL1_BUSY_CLR                 (1<<3)
#define I2C_CTL1_CLKERR_CLR               (1<<2)
#define I2C_CTL1_DONE_CLR                 (1<<1)
#define I2C_CTL1_SIFBUSY_CLR              (1<<0)

//control2
#define I2C_CTL2_FREQ_CUSTOM(x)           (x<<0)   //bit[10:0]
#define I2C_CTL2_SCL_DELAY(x)             (x<<24)  //bit[25:24]
#define I2C_CTL2_SDA_HALF_ENABLE          (1<<31)

//control5
#define I2C_CTL5_RING_VALUE(x)           (x>>21)   //bit[23:21]
#define I2C_CTL5_STATE(x)                (x>>17)   //bit[20:17]
#define I2C_CTL5_ROBE_MODE(x)            (x>>12)   //bit[15:12]
#define I2C_CTL5_SIFBUSY                 (1<<8)

//control6
#define I2C_CTL6_BURST_RDATA_CLR          I2C_EN1_BURST_RDATA_INT

//control7
#define I2C_CTL7_RDCOUNT(x)               (x<<16)  //bit[31:16]
#define I2C_CTL7_WRCOUNT(x)               (x<<0)   //bit[15:0]

//interrupt
#define I2C_INT_RINC_INDEX(x)             (x<<18)  //bit[20:18]
#define I2C_INT_WINC_INDEX(x)             (x<<15)  //bit[17:15]
#define I2C_INT_SCL_HOLD_TOO_LONG_FLAG    (1<<11)
#define I2C_INT_WFIFO_ENABLE              (1<<10)
#define I2C_INT_FULL_FLAG                 (1<<9)
#define I2C_INT_EMPTY_FLAG                (1<<8)
#define I2C_INT_SCL_WAIT_FLAG             (1<<7)
#define I2C_INT_EMPTY_THRESHOLD_FLAG      (1<<6)
#define I2C_INT_DATA_NACK_FLAG            (1<<5)
#define I2C_INT_ADDRESS_NACK_FLAG         (1<<4)
#define I2C_INT_BUSY_FLAG                 (1<<3)
#define I2C_INT_CLKERR_FLAG               (1<<2)
#define I2C_INT_DONE_FLAG                 (1<<1)
#define I2C_INT_SIFBUSY_FLAG              (1<<0)

//interrupt enable0
#define I2C_EN0_SCL_HOLD_TOO_LONG_INT     (1<<13)
#define I2C_EN0_NACK_INT                  (1<<12)
#define I2C_EN0_CTL_EMPTY_THRESHOLD(x)    (x<<9)  //bit[11:9]
#define I2C_EN0_EMPTY_INT                 (1<<8)
#define I2C_EN0_SCL_WAIT_INT              (1<<7)
#define I2C_EN0_EMPTY_THRESHOLD_INT       (1<<6)
#define I2C_EN0_DATA_NACK_INT             (1<<5)
#define I2C_EN0_ADDRESS_NACK_INT          (1<<4)
#define I2C_EN0_BUSY_INT                  (1<<3)
#define I2C_EN0_CLKERR_INT                (1<<2)
#define I2C_EN0_DONE_INT                  (1<<1)
#define I2C_EN0_SIFBUSY_INT               (1<<0)

#define I2C_CTL0_FREQ_MASK                  (0x7)     // 3 bit
#define I2C_CTL0_SLAVE_ADDR_MASK            (0x7F)    // 7 bit
#define I2C_CTL2_FREQ_CUSTOM_MASK           (0x7FF)   // 11 bit
#define I2C_CTL2_SCL_DELAY_MASK             (0x3)     // 2 bit
#define I2C_CTL7_RW_COUNT_MASK              (0xFFFF)  // 16 bit
#define I2C_EN0_CTL_EMPTY_THRESHOLD_MASK    (0x7)     // 3 bit
#define I2C_SG_DMA_LLI_INDEX_MASK           (0x1F)    // 5 bit

//interrupt enable1
#define I2C_EN1_BURST_RDATA_INT           (0x80008000)  //must sync with GET_BYTES_EACHTIME

//interrupt enable2
#define I2C_EN2_BURST_RDATA_OVERFLOW_INT  (0xFFFFFFFF)

//i2c master mode
#define I2C_MODE_DMA_MODE                 (1<<2)
#define I2C_MODE_MANUAL_MODE              (1<<1)  //0:trigger mode 1:auto mode
#define I2C_MODE_MANUAL_TRIG              (1<<0)

//i2c master status2
#define I2C_SW_RESET_DONE                 (1<<0)

#define I2C_BURST_BYTES			4
#define I2C_BURST_RDATA_FLAG		0x80008000
#define I2C_BURST_RDATA_ALL_FLAG	0xFFFFFFFF

/****************************************
* GDMA
****************************************/
//dma config
#define I2C_DMA_CFG_DMA_GO                (1<<8)
#define I2C_DMA_CFG_NON_BUF_MODE          (1<<2)
#define I2C_DMA_CFG_SAME_SLAVE            (1<<1)
#define I2C_DMA_CFG_DMA_MODE              (1<<0)

//dma interrupt flag
#define I2C_DMA_INT_LENGTH0_FLAG	(1<<6)
#define I2C_DMA_INT_THRESHOLD_FLAG	(1<<5)
#define I2C_DMA_INT_IP_TIMEOUT_FLAG	(1<<4)
#define I2C_DMA_INT_GDMA_TIMEOUT_FLAG	(1<<3)
#define I2C_DMA_INT_WB_EN_ERROR_FLAG	(1<<2)
#define I2C_DMA_INT_WCNT_ERROR_FLAG	(1<<1)
#define I2C_DMA_INT_DMA_DONE_FLAG	(1<<0)

#define I2C_DMA_INT_FLAG_MASK		(0x7F)    // bit

//dma interrupt enable
#define I2C_DMA_EN_LENGTH0_INT            (1<<6)
#define I2C_DMA_EN_THRESHOLD_INT          (1<<5)
#define I2C_DMA_EN_IP_TIMEOUT_INT         (1<<4)
#define I2C_DMA_EN_GDMA_TIMEOUT_INT       (1<<3)
#define I2C_DMA_EN_WB_EN_ERROR_INT        (1<<2)
#define I2C_DMA_EN_WCNT_ERROR_INT         (1<<1)
#define I2C_DMA_EN_DMA_DONE_INT           (1<<0)

/****************************************
* SG GDMA
****************************************/
//sg dma index
#define I2C_SG_DMA_LLI_RUN_INDEX(x)       (x<<8)  //bit[12:8]
#define I2C_SG_DMA_LLI_ACCESS_INDEX(x)    (x<<0)  //bit[4:0]

//sg dma config
#define I2C_SG_DMA_CFG_LAST_LLI           (1<<2)
#define I2C_SG_DMA_CFG_DMA_MODE           (1<<0)

//sg dma setting
#define I2C_SG_DMA_SET_DMA_ENABLE         (1<<31)
#define I2C_SG_DMA_SET_DMA_GO             (1<<0)

#define I2C_CLK_RAT		202500000
#define I2C_INIT_FREQ		100
#define I2C_MAX_FREQ		400
#define I2C_FREQ		27000
#define I2C_SCL_DELAY		1

//burst write use
#define I2C_EMPTY_THRESHOLD_VALUE    4

enum sp_i2c_active_mode {
	I2C_PIO = 0,
	I2C_DMA = 1,
};

enum sp_i2c_mode {
	I2C_WRITE_MODE = 0,
	I2C_READ_MODE = 1,
	I2C_RESTART_MODE = 2,
};

enum sp_i2c_dma_mode {
	I2C_DMA_WRITE_MODE = 0,
	I2C_DMA_READ_MODE = 1,
};

typedef enum I2C_State_e_ {
	I2C_IDLE_STATE,   /* i2c is idle */	
	I2C_WRITE_STATE,  /* i2c is write */
	I2C_READ_STATE,   /* i2c is read */
	I2C_DMA_WRITE_STATE,/* i2c is dma write */
	I2C_DMA_READ_STATE, /* i2c is dma read */
} I2C_State_e;

struct i2c_master_ctlr {

	void __iomem *base;
	struct regs_i2cm *regs;
	struct reset_ctl_bulk resets;
#if CONFIG_IS_ENABLED(CLK)
	struct clk clk;
#endif
	unsigned int i2c_ip;
	unsigned int freq;		
	unsigned int dma_adr;	
	I2C_State_e  RWState;
	unsigned int BurstCount;
	u8 *buf;
};

static void sp_i2cm_data_get(struct regs_i2cm *sr, unsigned int index, void *rxdata)
{
	unsigned int *rdata = rxdata;

	*rdata = readl(&(sr->data00_03) + 4*index);
}

static void sp_i2c_data_set(struct regs_i2cm *sr, unsigned int cnt, unsigned int *wrdata)
{
	unsigned int i;

	for (i = 0 ; i < cnt ; i++)
		writel(wrdata[i], &(sr->data00_03) + 4*i);
}

static void sp_i2c_reset(struct regs_i2cm *sr)
{
	unsigned int ctl0;

	ctl0 = readl(&sr->control0);
	ctl0 |= I2C_CTL0_SW_RESET;
	writel(ctl0, &sr->control0);

	udelay(2);
}

static void sp_i2cm_addr_freq_set(struct regs_i2cm *sr, unsigned int addr, unsigned int freq)
{
	unsigned int div;
	u32 ctl0, ctl2;

	div = I2C_FREQ / freq;
	div -= 1;
	if (I2C_FREQ % freq != 0)
		div += 1;

	if (div > I2C_CTL2_FREQ_CUSTOM_MASK)
		div = I2C_CTL2_FREQ_CUSTOM_MASK;

	ctl0 = readl(&sr->control0);
	ctl0 &= (~I2C_CTL0_FREQ(I2C_CTL0_FREQ_MASK));
	ctl0 &= (~I2C_CTL0_SLAVE_ADDR(I2C_CTL0_SLAVE_ADDR_MASK));
	ctl0 |= I2C_CTL0_SLAVE_ADDR(addr); 
	writel(ctl0, &sr->control0);

	ctl2 = readl(&sr->control2);
	ctl2 &= (~I2C_CTL2_FREQ_CUSTOM(I2C_CTL2_FREQ_CUSTOM_MASK));
	ctl2 |= I2C_CTL2_FREQ_CUSTOM(div);
	writel(ctl2, &sr->control2);
}

static void sp_i2cm_scl_delay_set(struct regs_i2cm *sr, unsigned int delay)
{
	u32 ctl2;

	ctl2 = readl(&sr->control2);
	ctl2 &= (~I2C_CTL2_SCL_DELAY(I2C_CTL2_SCL_DELAY_MASK));
	ctl2 |= I2C_CTL2_SCL_DELAY(delay);
	ctl2 &= (~(I2C_CTL2_SDA_HALF_ENABLE));
	writel(ctl2, &sr->control2);
}

static void sp_i2cm_trans_cnt_set(struct regs_i2cm *sr, unsigned int write_cnt,
				  unsigned int read_cnt)
{
	u32 ctl7 = 0;

	ctl7 = I2C_CTL7_RDCOUNT(read_cnt) | I2C_CTL7_WRCOUNT(write_cnt);
	writel(ctl7, &sr->control7);
}

static void sp_i2cm_active_mode_set(struct regs_i2cm *sr, enum sp_i2c_active_mode mode)
{
	u32 val;
			  
	val = readl(&sr->i2cm_mode);
	val &= (~(I2C_MODE_MANUAL_MODE | I2C_MODE_MANUAL_TRIG | I2C_MODE_DMA_MODE));   // Trigger mode
	if(mode == I2C_DMA)
		val |= (I2C_MODE_MANUAL_MODE | I2C_MODE_DMA_MODE);

	writel(val, &sr->i2cm_mode);
}

static void sp_i2cm_rw_mode_set(struct regs_i2cm *sr, enum sp_i2c_mode rw_mode)
{
	u32 ctl0;

	ctl0 = readl(&sr->control0);
	switch (rw_mode) {
	default:
	case I2C_WRITE_MODE:
		ctl0 &= ~(I2C_CTL0_PREFETCH |
			  I2C_CTL0_RESTART_EN | I2C_CTL0_SUBADDR_EN);
		break;
	case I2C_READ_MODE:
		ctl0 &= (~(I2C_CTL0_RESTART_EN | I2C_CTL0_SUBADDR_EN));
		ctl0 |= I2C_CTL0_PREFETCH;
		break;
	case I2C_RESTART_MODE:
		ctl0 |= (I2C_CTL0_PREFETCH |
			 I2C_CTL0_RESTART_EN | I2C_CTL0_SUBADDR_EN);
		break;
	}
	writel(ctl0, &sr->control0);                                 // set read mode
}

static void sp_i2cm_int_set(struct regs_i2cm *sr, u32 int0, u32 int1, u32 int2)
{
	if(int0 != 0)
		writel(int0, &sr->int_en0);
	if(int1 != 0)
		writel(int1, &sr->int_en1);
	if(int2 != 0)
		writel(int2, &sr->int_en2);
}

static void sp_i2cm_manual_trigger(struct regs_i2cm *sr)
{
	u32 val;

	val = readl(&sr->i2cm_mode);
	val |= I2C_MODE_MANUAL_TRIG;
	writel(val, &sr->i2cm_mode);
}

static unsigned int sp_i2cm_get_int_flag(struct regs_i2cm *sr)
{
	return readl(&sr->interrupt);
}

static unsigned int sp_i2cm_get_ctl5_flag(struct regs_i2cm *sr)
{
	return readl(&sr->control5);
}

static void sp_i2cm_dma_addr_set(struct regs_i2cm *sr_dma, u8 *addr)
{
	uintptr_t temp_addr = (uintptr_t)addr;

	writel(temp_addr, &sr_dma->dma_addr);
}

static void sp_i2cm_dma_length_set(struct regs_i2cm *sr_dma, unsigned int length)
{
	length &= (0xFFFF);  //only support 16 bit
	writel(length, &sr_dma->dma_length);
}

static void sp_i2cm_dma_rw_mode_set(struct regs_i2cm *sr_dma, enum sp_i2c_dma_mode rw_mode)
{
	u32 val;

	val = readl(&sr_dma->dma_config);
	switch (rw_mode) {
	default:
	case I2C_DMA_WRITE_MODE:
		val |= I2C_DMA_CFG_DMA_MODE;
		break;
	case I2C_DMA_READ_MODE:
		val &= (I2C_DMA_CFG_DMA_MODE);
		break;
	}
	writel(val, &sr_dma->dma_config);
}

static void sp_i2cm_dma_int_en_set(struct regs_i2cm *sr_dma, unsigned int dma_int)
{
	writel(dma_int, &sr_dma->int_en);
}

static void sp_i2cm_dma_go_set(struct regs_i2cm *sr_dma)
{
	u32 val;

	val = readl(&sr_dma->dma_config);
	val |= I2C_DMA_CFG_DMA_GO;
	writel(val, &sr_dma->dma_config);
}

static unsigned int sp_i2cm_get_dma_int_flag(struct regs_i2cm *sr_dma)
{
	return readl(&sr_dma->int_flag);
}

static unsigned int sp_i2cm_clk_dma_int_flag(struct regs_i2cm *sr_dma)
{
	u32 val = 0;

	val |= I2C_DMA_INT_FLAG_MASK;
	writel(val, &sr_dma->int_flag);
}

static int sp_i2c_read(struct i2c_master_ctlr *i2c, u8  slave_addr, u8  *data_buf, unsigned int len)
{
	unsigned int int_flag;
	unsigned int i;		
	unsigned int int0 = 0, int1 = 0, int2 = 0;
	int ret = 0;
	u8 r_data[16];
	struct regs_i2cm *i2c_regs = i2c->regs;

	//printf("grp3_sft_cfg[10] %x, grp3_sft_cfg %x\n", grp3_sft_cfg[10], *grp3_sft_cfg);
	//printf("i2cread\n");
	//printf("i2c_no : %d, slave_addr: 0x%x , len %d\n", i2c_no, slave_addr,len);
	sp_i2c_reset(i2c_regs);  // reset 

	int0 = (I2C_EN0_SCL_HOLD_TOO_LONG_INT | I2C_EN0_EMPTY_INT | I2C_EN0_DATA_NACK_INT
			| I2C_EN0_ADDRESS_NACK_INT | I2C_EN0_DONE_INT );
	int1 = I2C_BURST_RDATA_FLAG;
	int2 = I2C_BURST_RDATA_ALL_FLAG;

	i2c->buf = data_buf ;
	i2c->BurstCount = len / I2C_BURST_BYTES;
	if(len % I2C_BURST_BYTES)
		i2c->BurstCount++;
	i2c->RWState = I2C_READ_STATE;

	sp_i2cm_addr_freq_set(i2c_regs, slave_addr, i2c->freq);	// set work freq and slave address
	sp_i2cm_scl_delay_set(i2c_regs, I2C_SCL_DELAY);
	sp_i2cm_trans_cnt_set(i2c_regs, 0, len);
	sp_i2cm_active_mode_set(i2c_regs, I2C_PIO);
	sp_i2cm_rw_mode_set(i2c_regs, I2C_READ_MODE);

	//printf("control0-3 0x%x \n",i2c_regs->control0);
	sp_i2cm_int_set(i2c_regs, int0, int1, int2);
	sp_i2cm_manual_trigger(i2c_regs);   // start Trigger 

	int_flag = sp_i2cm_get_int_flag(i2c_regs); 
	while((int_flag & I2C_INT_DONE_FLAG) != I2C_INT_DONE_FLAG){
		//printf("i2c_regs->interrupt00 0x%x\n",int_flag);
		if((int_flag & I2C_INT_ADDRESS_NACK_FLAG)== I2C_INT_ADDRESS_NACK_FLAG){
			printf("I2C slave address NACK !!\n");
			ret = 1;
			break;
		}else if((int_flag & I2C_INT_DATA_NACK_FLAG)== I2C_INT_DATA_NACK_FLAG){
			printf("I2C slave data NACK !!\n");
			ret = 1;
			break;
		}else if((int_flag & I2C_INT_SCL_HOLD_TOO_LONG_FLAG)== I2C_INT_SCL_HOLD_TOO_LONG_FLAG){
			printf("I2C SCL hold too long occur !!\n");
			ret = 1;
			break;
		}
		int_flag = sp_i2cm_get_int_flag(i2c_regs);
		if ((int_flag & I2C_INT_DONE_FLAG) == I2C_INT_DONE_FLAG) {
			//printf("I2C_INT_DONE_FLAG00 ");
			if (i2c->RWState == I2C_READ_STATE) {
				for (i = 0; i < i2c->BurstCount; i++) {
					sp_i2cm_data_get(i2c_regs, i, (unsigned int *)(&r_data[i*4]));
				}
				for (i = 0; i < len; i++) {
					data_buf[i] = r_data[i];			
				}
			//printf("I2C_data i %d ",i);
			}
		}
	}

	sp_i2c_reset(i2c_regs);  // reset 
	i2c->RWState = I2C_IDLE_STATE;
	return ret;
}


static int sp_i2c_write(struct i2c_master_ctlr *i2c, u8  slave_addr , u8  *data_buf , unsigned int len)
{
	unsigned int int_flag;
	unsigned int i;
	unsigned int int0 = 0, int1 = 0, int2 = 0;
	int ret = 0;
	unsigned char w_data[32] = {0};
	struct regs_i2cm *i2c_regs = i2c->regs;

	sp_i2c_reset(i2c_regs);  // reset 

	//printf("data_buf0 = 0x%x, data_buf1 = 0x%x\n", data_buf[0], data_buf[1]);
	//printf("data_bufW00_addr:%x\n ",data_buf);
	//printf("i2c_regs 0x%x \n",i2c_regs);
	if (len == 0) len = 1;

	for(i = 0; i < len; i++){
		w_data[i] = data_buf[i];
	}

	//printf("write_cnt = %d, burst_cnt = %d\n", write_cnt, burst_cnt);
	int0 = (I2C_EN0_SCL_HOLD_TOO_LONG_INT | I2C_EN0_EMPTY_INT | I2C_EN0_DATA_NACK_INT
			| I2C_EN0_ADDRESS_NACK_INT | I2C_EN0_DONE_INT );

	i2c->buf = data_buf ;
	i2c->BurstCount = len / I2C_BURST_BYTES;
	if((len % I2C_BURST_BYTES) || (i2c->BurstCount == 0))
		i2c->BurstCount++;
	i2c->RWState = I2C_WRITE_STATE;

	sp_i2cm_addr_freq_set(i2c_regs, slave_addr, i2c->freq);	// set work freq and slave address
	sp_i2cm_scl_delay_set(i2c_regs, I2C_SCL_DELAY);
	sp_i2cm_trans_cnt_set(i2c_regs, len, 0);	// set read writer count
	sp_i2cm_active_mode_set(i2c_regs, I2C_PIO);
	sp_i2cm_rw_mode_set(i2c_regs, I2C_WRITE_MODE);	// i2c write mode
	sp_i2c_data_set(i2c_regs, i2c->BurstCount, (unsigned int *)w_data);
	sp_i2cm_int_set(i2c_regs, int0, int1, int2);
	sp_i2cm_manual_trigger(i2c_regs);   // start Trigger 

	int_flag = sp_i2cm_get_int_flag(i2c_regs); 
	while((int_flag & I2C_INT_DONE_FLAG) != I2C_INT_DONE_FLAG){	
		//printf("i2c_regs->interrupt00 0x%x\n",int_flag);
		if((int_flag & I2C_INT_ADDRESS_NACK_FLAG)== I2C_INT_ADDRESS_NACK_FLAG){
			printf("I2C slave address NACK !!\n");
			ret = 1;
			break;
		}else if((int_flag & I2C_INT_DATA_NACK_FLAG)== I2C_INT_DATA_NACK_FLAG){
			printf("I2C slave data NACK !!\n");
			ret = 1;
			break;
		}else if((int_flag & I2C_INT_SCL_HOLD_TOO_LONG_FLAG)== I2C_INT_SCL_HOLD_TOO_LONG_FLAG){
			printf("I2C SCL hold too long occur !!\n");
			ret = 1;
			break;
		}
		int_flag = sp_i2cm_get_int_flag(i2c_regs);
	}

	sp_i2c_reset(i2c_regs);  // reset 
	i2c->RWState = I2C_IDLE_STATE;
	return ret;
}

static int sp_i2c_dma_read(struct i2c_master_ctlr *i2c, u8  slave_addr , u8  *data_buf , unsigned int len)
{
	unsigned int int_flag,temp_flag;	
	int ret = 0;
	unsigned int int0 = 0, int1 = 0, int2 = 0;
	struct regs_i2cm *i2c_regs = i2c->regs;

	sp_i2c_reset(i2c_regs);  // reset 

	int0 = (I2C_EN0_SCL_HOLD_TOO_LONG_INT | I2C_EN0_EMPTY_INT | I2C_EN0_DATA_NACK_INT
			| I2C_EN0_ADDRESS_NACK_INT | I2C_EN0_DONE_INT );	
	int1 = I2C_BURST_RDATA_FLAG;
	int2 = I2C_BURST_RDATA_ALL_FLAG;

	i2c->RWState = I2C_DMA_READ_STATE;

	sp_i2cm_addr_freq_set(i2c_regs, slave_addr, i2c->freq);	// set work freq and slave address
	sp_i2cm_scl_delay_set(i2c_regs, I2C_SCL_DELAY);
	sp_i2cm_active_mode_set(i2c_regs, I2C_DMA);	// clear mode enable DMA set AUTO mode
	sp_i2cm_rw_mode_set(i2c_regs, I2C_READ_MODE);
	sp_i2cm_int_set(i2c_regs, int0, int1, int2);

	sp_i2cm_dma_addr_set(i2c_regs, data_buf);
	sp_i2cm_dma_length_set(i2c_regs, len);
	sp_i2cm_dma_rw_mode_set(i2c_regs, I2C_DMA_WRITE_MODE);
	sp_i2cm_dma_int_en_set(i2c_regs, I2C_DMA_EN_DMA_DONE_INT);
	sp_i2cm_dma_go_set(i2c_regs);

	//printf("dma_config 0x%x \n",i2c_regs->dma_config);
	//printf("i2cm_control2 0x%x \n",i2c_regs->control2);
	//printf("i2cm_control0 0x%x \n",i2c_regs->control0);
	//printf("i2cm_mode 0x%x \n",i2c_regs->i2cm_mode);
	//printf("dma_addr 0x%x \n",i2c_regs->dma_addr);		
	//printf("dma_length 0x%x \n",i2c_regs->dma_length);
	//printf("int_flag 0x%x \n",i2c_regs->interrupt);
	//printf("dma_int_flag 0x%x \n",i2c_regs->int_flag);

	temp_flag = sp_i2cm_get_ctl5_flag(i2c_regs);
	while((temp_flag & I2C_CTL5_SIFBUSY) == I2C_CTL5_SIFBUSY)
	{
		int_flag = sp_i2cm_get_int_flag(i2c_regs);
		if((int_flag & I2C_INT_ADDRESS_NACK_FLAG)== I2C_INT_ADDRESS_NACK_FLAG){
			printf("I2C slave address NACK !!\n");
			ret = 1;
			break;
		}else if((int_flag & I2C_INT_DATA_NACK_FLAG)== I2C_INT_DATA_NACK_FLAG){
			printf("I2C slave data NACK !!\n");
			ret = 1;
			break;
		}else if((int_flag & I2C_INT_SCL_HOLD_TOO_LONG_FLAG)== I2C_INT_SCL_HOLD_TOO_LONG_FLAG){
			printf("I2C SCL hold too long occur !!\n");
			ret = 1;
			break;
		}
		temp_flag = sp_i2cm_get_ctl5_flag(i2c_regs);
	}

	temp_flag = sp_i2cm_get_dma_int_flag(i2c_regs);
	sp_i2cm_clk_dma_int_flag(i2c_regs);
	int_flag = sp_i2cm_get_int_flag(i2c_regs);
	while(((int_flag & I2C_INT_DONE_FLAG) != I2C_INT_DONE_FLAG) && ((temp_flag & I2C_DMA_INT_DMA_DONE_FLAG) != I2C_DMA_INT_DMA_DONE_FLAG))
	{
		//printf("i2c_regs->int_flag 0x%x\n",int_flag);
		if((temp_flag & I2C_DMA_INT_WCNT_ERROR_FLAG) == I2C_DMA_INT_WCNT_ERROR_FLAG){
			printf("I2C DMA WCNT ERR !!\n");
			ret = 1;
			break;
		}else if((temp_flag & I2C_DMA_INT_WB_EN_ERROR_FLAG) == I2C_DMA_INT_WB_EN_ERROR_FLAG){
			printf("I2C DMA WB EN ERR !!\n");
			ret = 1;
			break;
		}else if((temp_flag & I2C_DMA_INT_GDMA_TIMEOUT_FLAG) == I2C_DMA_INT_GDMA_TIMEOUT_FLAG){
			printf("I2C DMA timeout !!\n");
			ret = 1;
			break;
		}else if((temp_flag & I2C_DMA_INT_IP_TIMEOUT_FLAG) == I2C_DMA_INT_IP_TIMEOUT_FLAG){
			printf("I2C IP timeout !!\n");
			ret = 1;
			break;
		}else if((temp_flag & I2C_DMA_INT_THRESHOLD_FLAG) == I2C_DMA_INT_THRESHOLD_FLAG){
			printf("I2C Length is zero !!\n");
			ret = 1;
			break;
		}
		temp_flag = sp_i2cm_get_dma_int_flag(i2c_regs);
		sp_i2cm_clk_dma_int_flag(i2c_regs);
		int_flag = sp_i2cm_get_int_flag(i2c_regs);
	};

	sp_i2c_reset(i2c_regs);  // reset 
	i2c->RWState = I2C_IDLE_STATE;
	return ret;
}

static int sp_i2c_dma_write(struct i2c_master_ctlr *i2c, u8  slave_addr , u8  *data_buf , unsigned int len)
{
	unsigned int int_flag,temp_flag;
	int ret = 0;
	unsigned int int0 = 0, int1 = 0, int2 = 0;	
	struct regs_i2cm *i2c_regs = i2c->regs;

	sp_i2c_reset(i2c_regs);  // reset 
	//printf("data_buf0 = 0x%x, data_buf1 = 0x%x\n", data_buf[0], data_buf[1]);	
	//printf("data_bufW00_addr:%x\n ",data_buf);
	//printf("sp_i2c_write 0x%x 0x%x\n",i2c_regs,i2c_regs);

	int0 = (I2C_EN0_SCL_HOLD_TOO_LONG_INT | I2C_EN0_EMPTY_INT | I2C_EN0_DATA_NACK_INT
			| I2C_EN0_ADDRESS_NACK_INT | I2C_EN0_DONE_INT );

	i2c->RWState = I2C_DMA_WRITE_STATE;

	sp_i2cm_addr_freq_set(i2c_regs, slave_addr, i2c->freq);	// set work freq and slave address	
	sp_i2cm_scl_delay_set(i2c_regs, I2C_SCL_DELAY);
	sp_i2cm_active_mode_set(i2c_regs, I2C_DMA);	// clear mode enable DMA set AUTO mode
	sp_i2cm_rw_mode_set(i2c_regs, I2C_WRITE_MODE);	// i2c write mode
	sp_i2cm_int_set(i2c_regs, int0, int1, int2);

	sp_i2cm_dma_addr_set(i2c_regs, data_buf);
	sp_i2cm_dma_length_set(i2c_regs, len);
	sp_i2cm_dma_rw_mode_set(i2c_regs, I2C_DMA_READ_MODE);
	sp_i2cm_dma_int_en_set(i2c_regs, I2C_DMA_EN_DMA_DONE_INT);
	sp_i2cm_dma_go_set(i2c_regs);

	//printf("i2cm_control2 0x%x \n",i2c_regs->control2);
	//printf("i2cm_control0 0x%x \n",i2c_regs->control0);
	//printf("i2cm_mode 0x%x \n",i2c_regs->i2cm_mode);
	//printf("dma_config 0x%x \n",i2c_regs->dma_config);
	//printf("dma_addr 0x%x \n",i2c_regs->dma_addr);
	//printf("dma_length 0x%x \n",i2c_regs->dma_length);	

	temp_flag = sp_i2cm_get_dma_int_flag(i2c_regs);
	sp_i2cm_clk_dma_int_flag(i2c_regs);
	int_flag = sp_i2cm_get_int_flag(i2c_regs);
	while((temp_flag & I2C_DMA_INT_DMA_DONE_FLAG) != I2C_DMA_INT_DMA_DONE_FLAG)
	{
	    	//printf("i2c_regs->int_flag 0x%x\n",i2c_regs->int_flag);
		if((temp_flag & I2C_DMA_INT_WCNT_ERROR_FLAG) == I2C_DMA_INT_WCNT_ERROR_FLAG){
			printf("I2C DMA WCNT ERR !!\n");
			ret = 1;
			break;
		}else if((temp_flag & I2C_DMA_INT_WB_EN_ERROR_FLAG) == I2C_DMA_INT_WB_EN_ERROR_FLAG){
			printf("I2C DMA WB EN ERR !!\n");
			ret = 1;
			break;
		}else if((temp_flag & I2C_DMA_INT_GDMA_TIMEOUT_FLAG) == I2C_DMA_INT_GDMA_TIMEOUT_FLAG){
			printf("I2C DMA timeout !!\n");
			ret = 1;
			break;
		}else if((temp_flag & I2C_DMA_INT_IP_TIMEOUT_FLAG) == I2C_DMA_INT_IP_TIMEOUT_FLAG){
			printf("I2C IP timeout !!\n");
			ret = 1;
			break;
		}else if((temp_flag & I2C_DMA_INT_THRESHOLD_FLAG) == I2C_DMA_INT_THRESHOLD_FLAG){
			printf("I2C Length is zero !!\n");
			ret = 1;
			break;
		}
		temp_flag = sp_i2cm_get_dma_int_flag(i2c_regs);
		sp_i2cm_clk_dma_int_flag(i2c_regs);
	};

	temp_flag = sp_i2cm_get_ctl5_flag(i2c_regs);
	while((temp_flag & I2C_CTL5_SIFBUSY) == I2C_CTL5_SIFBUSY)
	{
		int_flag = sp_i2cm_get_int_flag(i2c_regs);
		if((int_flag & I2C_INT_ADDRESS_NACK_FLAG)== I2C_INT_ADDRESS_NACK_FLAG){
			printf("I2C slave address NACK !!\n");
			ret = 1;
			break;
		}else if((int_flag & I2C_INT_DATA_NACK_FLAG)== I2C_INT_DATA_NACK_FLAG){
			printf("I2C slave data NACK !!\n");
			ret = 1;
			break;
		}else if((int_flag & I2C_INT_SCL_HOLD_TOO_LONG_FLAG)== I2C_INT_SCL_HOLD_TOO_LONG_FLAG){
			printf("I2C SCL hold too long occur !!\n");
			ret = 1;
			break;
		}
		temp_flag = sp_i2cm_get_ctl5_flag(i2c_regs);
	}
	sp_i2c_reset(i2c_regs);  // reset 
	i2c->RWState = I2C_IDLE_STATE;
	return ret;
}

static int sunplus_i2c_xfer(struct udevice *bus, struct i2c_msg *msg,
			       int nmsgs)
{
	struct i2c_master_ctlr *i2c = dev_get_priv(bus);
	int ret;

	debug("i2c_xfer: %d messages\n", nmsgs);
	for (; nmsgs > 0; nmsgs--, msg++) {
		debug("i2c_xfer: chip=0x%x, len=0x%x\n", msg->addr, msg->len);
		if (msg->flags & I2C_M_RD) {
			if(msg->len <= 16)
				ret = sp_i2c_read(i2c, msg->addr, msg->buf, msg->len);
			else
				ret = sp_i2c_dma_read(i2c, msg->addr, msg->buf, msg->len);
		} else {
			if(msg->len <= 16)
				ret = sp_i2c_write(i2c, msg->addr, msg->buf, msg->len);
			else
				ret = sp_i2c_dma_write(i2c, msg->addr, msg->buf, msg->len);
		}
		if (ret) {
			printf("i2c_xfer: %s error\n",
			       msg->flags & I2C_M_RD ? "read" : "write");
			return -EREMOTEIO;
		}
	}
	return 0;
}

static int sunplus_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct i2c_master_ctlr *priv = dev_get_priv(bus);
			       
	if (speed >= I2C_SPEED_FAST_RATE) 
		priv->freq = I2C_MAX_FREQ;
	else
		priv->freq = I2C_INIT_FREQ;
			       
	return 0;
}
			       
static int sunplus_i2c_get_bus_speed(struct udevice *bus)
{
	struct i2c_master_ctlr *priv = dev_get_priv(bus);
			       
	return priv->freq;
}

int sunplus_i2c_of_to_plat(struct udevice *bus)
{
	struct i2c_master_ctlr *priv = dev_get_priv(bus);
	int ret;

	priv->base = dev_read_addr_ptr(bus);	
	if (!priv->regs){
		priv->regs = priv->base;
	}

	ret = reset_get_bulk(bus, &priv->resets);
	if (ret) {
		if (ret != -ENOTSUPP)
			dev_warn(bus, "Can't get reset: %d\n", ret);
	} else {
		reset_deassert_bulk(&priv->resets);
	}

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_get_by_index(bus, 0, &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret && ret != -ENOSYS && ret != -ENOTSUPP) {
		clk_free(&priv->clk);
		dev_err(bus, "failed to enable clock\n");
		return ret;
	}
#endif
	return 0;
}

int sunplus_i2c_probe(struct udevice *bus)
{
	struct i2c_master_ctlr *priv = dev_get_priv(bus);

	sp_i2c_reset(priv->regs);

	return 0;
}

int sunplus_i2c_remove(struct udevice *dev)
{
	struct i2c_master_ctlr *priv = dev_get_priv(dev);

#if CONFIG_IS_ENABLED(CLK)
	clk_disable(&priv->clk);
	clk_free(&priv->clk);
#endif
	return reset_release_bulk(&priv->resets);
}

const struct dm_i2c_ops sunplus_i2c_ops = {
	.xfer		= sunplus_i2c_xfer,
	.set_bus_speed	= sunplus_i2c_set_bus_speed,
	.get_bus_speed	= sunplus_i2c_get_bus_speed,
};

static const struct udevice_id sunplus_i2c_ids[] = {
	{ .compatible = "sunplus,q645-i2cm" },
	{ }
};

U_BOOT_DRIVER(i2c_sunplus) = {
	.name	= "i2c_sunplus",
	.id	= UCLASS_I2C,
	.of_match = sunplus_i2c_ids,
	.of_to_plat = sunplus_i2c_of_to_plat,
	.probe	= sunplus_i2c_probe,
	.priv_auto	= sizeof(struct i2c_master_ctlr),
	.remove = sunplus_i2c_remove,
	.flags	= DM_FLAG_OS_PREPARE,
	.ops	= &sunplus_i2c_ops,
};
