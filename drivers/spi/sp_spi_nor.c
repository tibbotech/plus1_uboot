/*
 * (C) Copyright 2017
 * Sunplus Technology
 * Henry Liou<henry.liou@sunplus.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include "sp_spi_nor.h"
#include <dm.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

static UINT8 *cmd_buf;
static size_t cmd_len;
static volatile pentagram_spi_nor_regs* spi_reg;

int AV1_GetStc32(void)
{
	//TBD
	return 0;
}

static void spi_nor_io_CUST_config(UINT8 cmd_b, UINT8 addr_b, UINT8 data_b, SPI_ENHANCE enhance,UINT8 dummy)
{
	UINT32 config;
	if(enhance.enhance_en == 1)
	{
		config = spi_reg->spi_cfg0  & CLEAR_ENHANCE_DATA;
		if(enhance.enhance_bit == 4)
		{
			config &= ~(1<<18);
		}else if(enhance.enhance_bit == 8)
		{
			config |= (1<<18);
		}
		spi_reg->spi_cfg0 = config | ENHANCE_DATA(enhance.enhance_data);
	}
	config = 0;
	switch (cmd_b)
	{
		case 4:
			config |= SPI_CMD_4b | SPI_CMD_OEN_4b;
			break;
		case 2:
			config |= SPI_CMD_2b | SPI_CMD_OEN_2b;
			break;
		case 1:
			config |= SPI_CMD_1b | SPI_CMD_OEN_1b;
			break;
		case 0:
		default:
			config |= SPI_CMD_NO | SPI_CMD_OEN_NO;
			break;
	}
	switch (addr_b)
	{
		case 4:
			config |= SPI_ADDR_4b | SPI_ADDR_OEN_4b;
			break;
		case 2:
			config |= SPI_ADDR_2b | SPI_ADDR_OEN_2b;
			break;
		case 1:
			config |= SPI_ADDR_1b | SPI_ADDR_OEN_1b;
			break;
		case 0:
		default:
			config |= SPI_ADDR_NO | SPI_ADDR_OEN_NO;
			break;
	}
	switch (data_b)
	{
		case 4:
			config |= SPI_DATA_4b | SPI_DATA_OEN_4b;
			break;
		case 2:
			config |= SPI_DATA_2b | SPI_DATA_OEN_2b;
			break;
		case 1:
			config |= SPI_DATA_1b | SPI_DATA_OEN_1b | SPI_DATA_IEN_DQ1;
			break;
		case 0:
		default:
			config |= SPI_DATA_NO | SPI_DATA_OEN_NO;
			break;
	}
	switch (enhance.enhance_bit_mode)
	{
		case 4:
			config |= SPI_ENHANCE_4b;
			break;                  
		case 2:                     
			config |= SPI_ENHANCE_2b;
			break;                  
		case 1:                     
			config |= SPI_ENHANCE_1b;
			break;                  
		case 0:                     
		default:                    
			config |= SPI_ENHANCE_NO;
			break;
	}
	spi_reg->spi_cfg1 =   config | SPI_DUMMY_CYC(dummy);
	msg_printf("spi_reg->spi_cfg0 0x%x\n",spi_reg->spi_cfg0);
	msg_printf("spi_reg->spi_cfg1 0x%x\n",spi_reg->spi_cfg1);
}
static void spi_fast_read_enable(void)
{
	SPI_ENHANCE enhance;
	enhance.enhance_en = 0;
	diag_printf("%s\n",__FUNCTION__);
	while((spi_reg->spi_ctrl & SPI_CTRL_BUSY)!=0)
	{
		msg_printf("wait spi_reg->spi_ctrl 0x%x\n",spi_reg->spi_ctrl);
	};
	spi_reg->spi_ctrl = A_CHIP | SPI_CLK_D_16;
	spi_nor_io_CUST_config(CMD_1,ADDR_1,DATA_1,enhance,DUMMY_CYCLE(8));
	return ;
}
static void spi_fast_read_disable(void)
{
	SPI_ENHANCE enhance;
	enhance.enhance_en = 0;
	diag_printf("%s\n",__FUNCTION__);
	while((spi_reg->spi_ctrl & SPI_CTRL_BUSY)!=0)
	{
		msg_printf("wait spi_reg->spi_ctrl 0x%x\n",spi_reg->spi_ctrl);
	};
	spi_reg->spi_ctrl = A_CHIP | SPI_CLK_D_32;
	spi_nor_io_CUST_config(CMD_1,ADDR_1,DATA_1,enhance,DUMMY_CYCLE(0));
	return ;
}
static UINT8 spi_nor_read_status1(void)
{
	diag_printf("%s\n",__FUNCTION__);
	UINT32 ctrl;
	ctrl = spi_reg->spi_ctrl & CLEAR_CUST_CMD;
	ctrl = ctrl | READ | BYTE_0 | ADDR_0B | CUST_CMD(0x05);
	while((spi_reg->spi_ctrl & SPI_CTRL_BUSY)!=0)
	{
		msg_printf("wait spi_reg->spi_ctrl 0x%x\n",spi_reg->spi_ctrl);
	};
	spi_reg->spi_data = 0;
	spi_reg->spi_ctrl = ctrl;
	spi_reg->spi_auto_cfg |= PIO_TRIGGER;
	while((spi_reg->spi_auto_cfg & PIO_TRIGGER)!=0)
	{
		msg_printf("wait PIO_TRIGGER\n");
	};
	msg_printf("spi_reg->spi_status 0x%x\n",spi_reg->spi_status);
	return (spi_reg->spi_status&0xff);
}
static int spi_flash_xfer_read(UINT8 *cmd, size_t cmd_len, void *data, size_t data_len)
{
	diag_printf("%s\n",__FUNCTION__);
	UINT32 total_count = data_len;
	UINT32 data_count;
	UINT32 addr_offset = 0;
	UINT32 addr_temp = 0;
	UINT8 *data_in = data;
	UINT32 data_temp = 0;
	UINT8 addr_len = 0;
	UINT32 timeout = 0;
	UINT32 time = 0;
	UINT32 ctrl = 0;
	int fast_read = 0;

	msg_printf("data length %d\n",data_len);
	while (total_count > 0)
	{
		if (total_count > SPI_DATA64_MAX_LEN)
		{
			total_count = total_count - SPI_DATA64_MAX_LEN;
			data_count = SPI_DATA64_MAX_LEN;
		} else
		{
			data_count = total_count;
			total_count = 0;
		}
		while((spi_reg->spi_ctrl & SPI_CTRL_BUSY)!=0)
		{
			msg_printf("wait spi_reg->spi_ctrl 0x%x\n",spi_reg->spi_ctrl);
		};
		ctrl = spi_reg->spi_ctrl & CLEAR_CUST_CMD;
		ctrl = ctrl | READ | BYTE_0 | ADDR_0B | CUST_CMD(cmd[0]);
		spi_reg->spi_cfg0 = (spi_reg->spi_cfg0 & CLEAR_DATA64_LEN) | data_count | DATA64_EN;
		spi_reg->spi_page_addr = 0;
		spi_reg->spi_buf_addr = DATA64_READ_ADDR(0) | DATA64_WRITE_ADDR(0);
		if (cmd_len > 1)
		{
			addr_temp = (cmd[1] << 16) | (cmd[2] << 8) | cmd[3];
			addr_temp = addr_temp + addr_offset * SPI_DATA64_MAX_LEN;
			spi_reg->spi_page_addr = addr_temp;
			ctrl = ctrl | ADDR_3B ;
			msg_printf("addr 0x%x\n", spi_reg->spi_page_addr);
		}
		if (cmd[0] == CMD_FAST_READ)
		{
			spi_fast_read_enable();
			fast_read = 1;
		}
		spi_reg->spi_data = 0;
		spi_reg->spi_ctrl = ctrl;
		spi_reg->spi_auto_cfg |= PIO_TRIGGER;
		//msg_printf("spi_reg->spi_ctrl 0x%x\n", spi_reg->spi_ctrl);
		//msg_printf("spi_reg->spi_page_addr 0x%x\n", spi_reg->spi_page_addr);
		//msg_printf("spi_reg->spi_cfg0 0x%x\n", spi_reg->spi_cfg0);

		if (cmd[0] == CMD_READ_STATUS)
		{
			data_in[0] = spi_reg->spi_status& 0xff;
			data_count = 0;
		}
		while (data_count > 0)
		{
			if ((data_count / 4) > 0)
			{
				time = AV1_GetStc32();
				while((spi_reg->spi_status_2 & SPI_SRAM_ST )==SRAM_EMPTY)
				{
					timeout = AV1_GetStc32();
					if ((timeout - time) > SPI_TIMEOUT) {
						msg_printf("timeout \n");
						break;
					}
				};
				data_temp = spi_reg->spi_data64;
				//msg_printf("data_temp 0x%x\n",data_temp);
				data_in[0] = data_temp & 0xff;
				data_in[1] = ((data_temp & 0xff00) >> 8);
				data_in[2] = ((data_temp & 0xff0000) >> 16);
				data_in[3] = ((data_temp & 0xff000000) >> 24);
				data_in = data_in + 4;
				data_count = data_count - 4;
			} else {
				time = AV1_GetStc32();
				while((spi_reg->spi_status_2 & SPI_SRAM_ST )==SRAM_EMPTY)
				{
					timeout = AV1_GetStc32();
					if ((timeout - time) > SPI_TIMEOUT) {
						msg_printf("timeout \n");
						break;
					}
				};
				data_temp = spi_reg->spi_data64;
				//msg_printf("data_temp 0x%x\n",data_temp);
				if(data_count%4 == 3)
				{
					data_in[0] = data_temp & 0xff;
					data_in[1] = ((data_temp & 0xff00) >> 8);
					data_in[2] = ((data_temp & 0xff0000) >> 16);
					data_count = data_count-3; 
				}else if(data_count%4 == 2)
				{
					data_in[0] = data_temp & 0xff;
					data_in[1] = ((data_temp & 0xff00) >> 8);
					data_count = data_count-2; 
				}else if (data_count%4 == 1)
				{
					data_in[0] = data_temp & 0xff;
					data_count = data_count-1; 
				}
			}
		}
		addr_offset = addr_offset + 1;
		while ((spi_nor_read_status1() & 0x01) != 0)
		{
			msg_printf("wait DEVICE busy\n");
		};
	}
	while((spi_reg->spi_auto_cfg & PIO_TRIGGER)!=0)
	{
		msg_printf("wait PIO_TRIGGER\n");
	};
	while((spi_reg->spi_ctrl & SPI_CTRL_BUSY)!=0)
	{
		msg_printf("wait spi_reg->spi_ctrl 0x%x\n",spi_reg->spi_ctrl);
	};
	spi_reg->spi_cfg0 &= DATA64_DIS;
	if (fast_read == 1)
	{
		spi_fast_read_disable();
	}
	return 0;
}
static int spi_flash_xfer_write(UINT8 *cmd, size_t cmd_len, void *data, size_t data_len)
{
	diag_printf("%s\n",__FUNCTION__);
	UINT32 total_count = data_len;
	UINT32 data_count = 0;
	UINT32 addr_offset = 0;
	UINT32 addr_temp = 0;
	UINT8 *data_in = data;
	UINT32 data_temp = 0;
	UINT8 addr_len = 0;
	UINT32 timeout = 0;
	UINT32 time = 0;
	UINT32 ctrl = 0;

	msg_printf("data length %d\n",data_len);
	if (total_count == 0) 
	{
		while((spi_reg->spi_ctrl & SPI_CTRL_BUSY)!=0)
		{
			msg_printf("wait spi_reg->spi_ctrl 0x%x\n",spi_reg->spi_ctrl);
		};
		ctrl = spi_reg->spi_ctrl & CLEAR_CUST_CMD;
		ctrl = ctrl | WRITE | BYTE_0 | ADDR_0B | CUST_CMD(cmd[0]);
		spi_reg->spi_cfg0 = (spi_reg->spi_cfg0 & CLEAR_DATA64_LEN) | data_count | DATA64_EN;
		spi_reg->spi_buf_addr = DATA64_READ_ADDR(0) | DATA64_WRITE_ADDR(0);
		if (cmd_len > 1)
		{
			spi_reg->spi_page_addr = addr_temp;
			ctrl = ctrl | ADDR_3B ;
			msg_printf("addr 0x%x\n", spi_reg->spi_page_addr);
		}
		spi_reg->spi_data = 0;
		spi_reg->spi_ctrl = ctrl;
		spi_reg->spi_auto_cfg |= PIO_TRIGGER;
		//msg_printf("spi_reg->spi_ctrl 0x%x\n", spi_reg->spi_ctrl);
		//msg_printf("spi_reg->spi_page_addr 0x%x\n", spi_reg->spi_page_addr);
		//msg_printf("spi_reg->spi_cfg0 0x%x\n", spi_reg->spi_cfg0);
	}
	while (total_count > 0) 
	{
		if (total_count > SPI_DATA64_MAX_LEN) {
			total_count = total_count - SPI_DATA64_MAX_LEN;
			data_count = SPI_DATA64_MAX_LEN;
		} else {
			data_count = total_count;
			total_count = 0;
		}
		while((spi_reg->spi_ctrl & SPI_CTRL_BUSY)!=0)
		{
			msg_printf("wait spi_reg->spi_ctrl 0x%x\n",spi_reg->spi_ctrl);
		};
		ctrl = spi_reg->spi_ctrl & CLEAR_CUST_CMD;
		ctrl = ctrl | WRITE | BYTE_0 | ADDR_0B | CUST_CMD(cmd[0]);
		spi_reg->spi_cfg0 = (spi_reg->spi_cfg0 & CLEAR_DATA64_LEN) | data_count | DATA64_EN;
		spi_reg->spi_buf_addr = DATA64_READ_ADDR(0) | DATA64_WRITE_ADDR(0);
		if (cmd_len > 1) {
			addr_temp = (cmd[1] << 16) | (cmd[2] << 8) | cmd[3];
			addr_temp = addr_temp + addr_offset * SPI_DATA64_MAX_LEN;
			spi_reg->spi_page_addr = addr_temp;
			ctrl = ctrl | ADDR_3B ;
			msg_printf("addr 0x%x\n", spi_reg->spi_page_addr);
		}
		spi_reg->spi_data = 0;
		spi_reg->spi_ctrl = ctrl;
		spi_reg->spi_auto_cfg |= PIO_TRIGGER;
		//msg_printf("spi_reg->spi_ctrl 0x%x\n", spi_reg->spi_ctrl);
		//msg_printf("spi_reg->spi_page_addr 0x%x\n", spi_reg->spi_page_addr);
		//msg_printf("spi_reg->spi_cfg0 0x%x\n", spi_reg->spi_cfg0);

		while (data_count > 0) 
		{
			if ((data_count / 2) > 0) {
				if ((spi_reg->spi_status_2 & SPI_SRAM_ST) == SRAM_FULL) {
					time = AV1_GetStc32();
					while ((spi_reg->spi_status_2 & SPI_SRAM_ST) != SRAM_EMPTY) {
						timeout = AV1_GetStc32();
						if ((timeout - time) > SPI_TIMEOUT) {
							msg_printf("timeout \n");
							break;
						}
					};
				}
				data_temp = (data_in[3] << 24) | (data_in[2] << 16) | (data_in[1] << 8) | data_in[0];
				spi_reg->spi_data64 = data_temp;
				data_in = data_in + 4;
				data_count = data_count - 4;
			} else {
				if ((spi_reg->spi_status_2 & SPI_SRAM_ST) == SRAM_FULL) {
					time = AV1_GetStc32();
					while ((spi_reg->spi_status_2 & SPI_SRAM_ST) != SRAM_EMPTY) {
						timeout = AV1_GetStc32();
						if ((timeout - time) > SPI_TIMEOUT) {
							msg_printf("timeout \n");
							break;
						}
					};
				}
				//data_temp = data_in[0] & 0xff;
				if(data_count%4 == 3)
				{
					data_temp = (data_in[2] << 16) | (data_in[1] << 8) | data_in[0];
					data_count = data_count-3; 
				}else if(data_count%4 == 2)
				{
					data_temp =  (data_in[1] << 8) | data_in[0];
					data_count = data_count-2; 
				}else if (data_count%4 == 1)
				{
					data_temp = data_in[0];
					data_count = data_count-1; 
				}
				spi_reg->spi_data64 = data_temp;
			}
		}
		addr_offset = addr_offset + 1;
		while ((spi_nor_read_status1() & 0x01) != 0)
		{
			msg_printf("wait DEVICE busy\n");
		};
	}
	while((spi_reg->spi_auto_cfg & PIO_TRIGGER)!=0)
	{
		msg_printf("wait PIO_TRIGGER\n");
	};
	while((spi_reg->spi_ctrl & SPI_CTRL_BUSY)!=0)
	{
		msg_printf("wait spi_reg->spi_ctrl 0x%x\n",spi_reg->spi_ctrl);
	};
	spi_reg->spi_cfg0 &= DATA64_DIS;
	return 0;
}
static int pentagram_spi_nor_ofdata_to_platdata(struct udevice *bus)
{
	diag_printf("%s\n",__FUNCTION__);
	struct pentagram_spi_nor_platdata *plat = bus->platdata;
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(bus);

	plat->regs = (struct pentagram_spi_nor_regs *)fdtdec_get_addr(blob, node, "reg");

	plat->clock = fdtdec_get_int(blob, node, "spi-max-frequency",
					50000000);

	return 0;
}
static int pentagram_spi_nor_probe(struct udevice *bus)
{
	diag_printf("%s\n",__FUNCTION__);
	struct pentagram_spi_nor_platdata *plat = dev_get_platdata(bus);
	struct pentagram_spi_nor_priv *priv = dev_get_priv(bus);

	priv->regs = plat->regs;
	priv->clock = plat->clock;

	spi_reg = (pentagram_spi_nor_regs *)priv->regs;
	cmd_buf = malloc(CMD_BUF_LEN * sizeof(UINT8));

	return 0;
}

static int pentagram_spi_nor_remove(struct udevice *dev)
{
	diag_printf("%s\n",__FUNCTION__);
	free(cmd_buf);
	return 0;
}

static int pentagram_spi_nor_claim_bus(struct udevice *dev)
{
	diag_printf("%s\n",__FUNCTION__);
	//set pinmux
	UINT32* grp1_sft_cfg = (UINT32 *) 0x9c000080;
	grp1_sft_cfg[1] = RF_MASK_V(0xf, (1 << 2) | 1);
	spi_reg->spi_ctrl = A_CHIP | SPI_CLK_D_32;//SPI_CLK_D_16 = 62M
	spi_reg->spi_cfg1 = SPI_CMD_OEN_1b | SPI_ADDR_OEN_1b | SPI_DATA_OEN_1b | SPI_CMD_1b | SPI_ADDR_1b
						| SPI_DATA_1b | SPI_ENHANCE_NO | SPI_DUMMY_CYC(0) | SPI_DATA_IEN_DQ1;
	spi_reg->spi_auto_cfg &= ~(AUTO_RDSR_EN);

	return 0;
}

static int pentagram_spi_nor_release_bus(struct udevice *dev)
{
	diag_printf("%s\n",__FUNCTION__);
	return 0;
}

static int pentagram_spi_nor_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	diag_printf("%s\n", __FUNCTION__);
	struct udevice *bus = dev->parent;
	struct pentagram_spi_nor_platdata *pdata = dev_get_platdata(bus);
	struct pentagram_spi_nor_priv *priv = dev_get_priv(bus);
	unsigned int len;
	int flc = 0;

	if (bitlen == 0)
		goto out;

	if (bitlen % 8)
		goto out;

	len = bitlen / 8;

	if (flags & SPI_XFER_BEGIN) {
		if (len > 0 && dout) {
			cmd_len = len;
			memset(cmd_buf, 0, CMD_BUF_LEN);
			memcpy(cmd_buf, dout, len);
			msg_printf("cmd %x\n", cmd_buf[0]);
			msg_printf("addr 0x%x\n", cmd_buf[1] << 16 | cmd_buf[2] << 8 | cmd_buf[3]);
			msg_printf("len %d\n", len);
		}
		if (!(flags & SPI_XFER_END))
			goto out;
		else
			flc = 1;
	}

	if (!dout) {
		// read
		msg_printf("read\n");
		spi_flash_xfer_read(cmd_buf, cmd_len, din, len);
	} else if ((!din) | (flags & SPI_XFER_END)) {
		// write
		msg_printf("write\n");
		if (flc == 1)
			spi_flash_xfer_write(cmd_buf, cmd_len, NULL, 0);
		else
			spi_flash_xfer_write(cmd_buf, cmd_len, dout, len);
	}
out:
	return 0;
}

static int pentagram_spi_nor_set_speed(struct udevice *bus, uint speed)
{
	diag_printf("%s\n",__FUNCTION__);
	struct pentagram_spi_nor_platdata *plat = bus->platdata;
	struct pentagram_spi_nor_priv *priv = dev_get_priv(bus);
	int ret;

	if (speed > plat->clock)
		speed = plat->clock;
	//set spi nor clock
	priv->clock = speed;
	debug("%s: regs=%p, speed=%d\n", __func__, priv->regs, priv->clock);

	return 0;
}
static int pentagram_spi_nor_set_mode(struct udevice *bus, uint mode)
{
	diag_printf("%s\n",__FUNCTION__);
	struct pentagram_spi_nor_priv *priv = dev_get_priv(bus);
	uint32_t reg;

	//set spi nor mode
	debug("%s: regs=%p, mode=%d\n", __func__, priv->regs, priv->mode);

	return 0;
}

static const struct dm_spi_ops pentagram_spi_nor_ops = {
	.claim_bus	= pentagram_spi_nor_claim_bus,
	.release_bus= pentagram_spi_nor_release_bus,
	.xfer		= pentagram_spi_nor_xfer,
	.set_speed	= pentagram_spi_nor_set_speed,
	.set_mode	= pentagram_spi_nor_set_mode,
};

static const struct udevice_id pentagram_spi_nor_ids[] = {
	{ .compatible = "sunplus,pentagram-spi-nor" },
	{ }
};

U_BOOT_DRIVER(pentagram_spi_nor) = {
	.name	= "pentagram_spi_nor",
	.id	= UCLASS_SPI,
	.of_match = pentagram_spi_nor_ids,
	.ops	= &pentagram_spi_nor_ops,
	.ofdata_to_platdata = pentagram_spi_nor_ofdata_to_platdata,
	.probe	= pentagram_spi_nor_probe,
	.remove	= pentagram_spi_nor_remove,
	.platdata_auto_alloc_size = sizeof(struct pentagram_spi_nor_platdata),
	.priv_auto_alloc_size = sizeof(struct pentagram_spi_nor_priv),
};
/*
probe flow 
1.ofdata_to_platdata
2.set_speed
3.set_mode
4.claim bus
*/

