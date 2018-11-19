/*
 * Sunplusl 2swc ethernet driver for u-boot
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <net.h>
#include <malloc.h>
#include <asm/io.h>
#include <phy.h>
#include <miiphy.h>
#include "sunplus_l2sw.h"

//#define cpu_to_le32(x)	(x)

//#define le32_to_cpu(x)	(x)

static void  spl2sw_hw_init(struct spl2sw_dev *priv)
{
	struct spl2sw_regs *regs = (struct spl2sw_regs *)priv->dev->iobase;
	u32 reg;



	//set vlan gp
	writel((1<<4)+0,&regs->PVID_config0);
	writel((6<<8)+9,&regs->VLAN_memset_config0);

	//enable soc port0 crc padding
	reg=readl(&regs->cpu_cntl);
	writel((reg&(~(0x1<<6)))|(0x1<<8),&regs->cpu_cntl);

	//enable port0
	reg=readl(&regs->port_cntl0);
	writel(reg&(~(0x3<<24)),&regs->port_cntl0);


	reg=reg=readl(&regs->cpu_cntl);
	writel(reg&(~(0x3F<<0)),&regs->cpu_cntl);

}

static inline void desc_set_buf_len(struct spl2sw_desc *p, u32 buf_sz)
{
	p->cmd2 = cpu_to_le32(buf_sz);
}


static inline void tx_desc_send_set(struct spl2sw_desc *p,
					      void *paddr, int len)
{
    u32 cmd1;
	u32 cmd2;
	u32 force_dp=0x1;
	u32	to_vlan=0x1;

	cmd1 = (OWN_BIT | FS_BIT | LS_BIT | (force_dp<<18) | (to_vlan<<12)| (len&LEN_MASK));
	cmd2 = (len&LEN_MASK);
    p->addr1 = cpu_to_le32(paddr);
	
    p->cmd1 = cmd1;
	p->cmd2 = cmd2;
	printf("tx_desc_send_set cmd1= [%x]\n",p->cmd1);
	printf("tx_desc_send_set cmd2= [%x]\n",p->cmd2);
	printf("tx_desc_send_set addr1= [%x]\n",p->addr1);
	printf("tx_desc_send_set addr2= [%x]\n",p->addr2);
}



static inline void desc_init_rx_desc(struct spl2sw_desc *p, int ring_size,
				     int buf_sz)
{
	struct spl2sw_desc *end = p + ring_size - 1;
	
	memset(p, 0, sizeof(*p) * ring_size);

	for (; p <= end; p++)
	{
		p->cmd1 = OWN_BIT;
		desc_set_buf_len(p, buf_sz);
	}
	end->cmd2|= cpu_to_le32(EOR_BIT);
}

static inline void desc_init_tx_desc(struct spl2sw_desc *p, u32 ring_size)
{
	memset(p, '\0', sizeof(*p) * ring_size);
	printf("p = %x\n", p);
	printf("p[ring_size - 1] = %x\n", p[ring_size - 1]);
	p[ring_size - 1].cmd2|= cpu_to_le32(EOR_BIT);
}

static inline int desc_get_owner(struct spl2sw_desc *p)
{
	return le32_to_cpu(p->cmd1) & OWN_BIT;
}

static inline void desc_set_rx_owner(struct spl2sw_desc *p)
{
	/* Clear all fields and set the owner */
	p->cmd1 |= cpu_to_le32(OWN_BIT);
}



static inline int desc_get_rx_frame_len(struct spl2sw_desc *p)
{
	u32 data = le32_to_cpu(p->cmd2);
	u32 len = (data & RXDESC_FRAME_LEN_MASK);

	return len;
}

static inline void *desc_get_buf_addr(struct spl2sw_desc *p)
{
	return (void *)le32_to_cpu(p->addr1);
}

static inline void desc_set_buf_addr(struct spl2sw_desc *p,
				     void *paddr, int len)
{
	p->addr1 = cpu_to_le32(paddr);
}

static void init_rx_desc(struct spl2sw_dev *priv)
{
	struct spl2sw_desc *rxdesc;
	struct spl2sw_regs *regs = (struct spl2sw_desc *)priv->dev->iobase;
	void *rxbuffer = priv->rxbuffer;
	int i,j;

	for(i=0; i<RX_DESC_QUEUE_NUM; i++)
	{	
		
		priv->rx_desc_num[i] = RX_DESC_NUM;
		priv->rx_desc[i] = &priv->rx_desc_chain[i * RX_DESC_NUM];
		desc_init_rx_desc(priv->rx_desc[i], RX_DESC_NUM, ETH_BUF_SZ);
	
		for (j = 0; j < RX_DESC_NUM; j++) {
			rxdesc = priv->rx_desc[i] ;
			
			desc_set_buf_addr(rxdesc + j, rxbuffer + (j * ETH_BUF_SZ),
					  ETH_BUF_SZ);
			
			desc_set_rx_owner(rxdesc + j);
		}
	}	


	writel(priv->rx_desc[0], &regs->rx_lbase_addr_0);
	writel(priv->rx_desc[1], &regs->rx_hbase_addr_0);


}

static void init_tx_desc(struct spl2sw_dev *priv)
{
	struct spl2sw_regs *regs = (struct spl2sw_desc *)priv->dev->iobase;
	int i;
	
	for(i=0; i<TX_DESC_QUEUE_NUM; i++)
	{
		priv->tx_desc_num[i] = TX_DESC_NUM;
		printf("tx_desc1  = %x\n",  priv->tx_desc_chain[i * TX_DESC_NUM]);
		printf("tx_desc2  = %x\n",  &priv->tx_desc_chain[i * TX_DESC_NUM]);
		printf("tx_desc3  = %x\n",  priv->tx_desc_chain);
		priv->tx_desc[i] = &priv->tx_desc_chain[i * TX_DESC_NUM];//0x9E820000;
		desc_init_tx_desc(priv->tx_desc[i], TX_DESC_NUM);
	}	

	printf("tx_desc  = %x\n", priv->tx_desc[0]);
	//printf("&tx_desc  = %x\n", &priv->tx_desc[0]);
	writel(0, &regs->tx_lbase_addr_0);
	writel(priv->tx_desc[0], &regs->tx_hbase_addr_0);
}


static void spl2sw_hwaddr_set(struct eth_device *dev)
{

	struct spl2sw_regs *regs = (struct spl2sw_regs *)dev->iobase;
	writel(dev->enetaddr[0]+(dev->enetaddr[1]<<8),&regs->w_mac_15_0_bus);
	writel(dev->enetaddr[2]+(dev->enetaddr[3]<<8)+(dev->enetaddr[4]<<16)+(dev->enetaddr[5]<<24),&regs->w_mac_47_16);
}

int spl2sw_pinmux_set(struct eth_device *dev)
{
	u32 reg;

	struct spl2sw_regs *regs = (struct spl2sw_regs *)dev->iobase;

	MOON2_REG->sft_cfg[0] = 0x22282228;
	MOON2_REG->sft_cfg[1] = 0x17231723;
	MOON2_REG->sft_cfg[2] = 0x202C202C;
	MOON2_REG->sft_cfg[3] = 0x2B212B21;
	MOON2_REG->sft_cfg[4] = 0x2A292A29;
	MOON2_REG->sft_cfg[5] = 0x26252625;
	MOON2_REG->sft_cfg[6] = 0x24272427;
	MOON2_REG->sft_cfg[7] = 0x1D1F1D1F;
	MOON2_REG->sft_cfg[8] = 0x191E191E;
	MOON2_REG->sft_cfg[9] = 0x1B1A1B1A;
	MOON2_REG->sft_cfg[10] = 0x01180118;
	
	//set clock
	reg = MOON5_REG->sft_cfg[5];
	MOON5_REG->sft_cfg[5] = (reg|0xF<<16|0xF);
	//enable port0
	reg=readl(&regs->port_cntl0);
	writel(reg&(~(0x3<<24)),&regs->port_cntl0);

	//phy address
	reg=readl(&regs->mac_force_mode);
	writel((reg&(~(0x1f<<16)))|(PHY0_ADDR<<16), &regs->mac_force_mode);
	reg=readl(&regs->mac_force_mode);
	writel((reg&(~(0x1f<<24)))|(PHY1_ADDR<<24), &regs->mac_force_mode);


}

void spl2sw_enable_port(struct eth_device *dev)
{
    u32 reg;
	u8 port_map=0x0;
	u8 cpu_port=0x0;
	u8 age=0x0;
	u8 proxy=0x0;
	u8 mc_ingress=0x0;
	u8 vlan_id=0x0;
	u8 vlan_memset=0x9;

	struct spl2sw_regs *regs = (struct spl2sw_regs *)dev->iobase;


	//phy address
	reg=readl(&regs->mac_force_mode);
	writel((reg&(~(0x1f<<16)))|(PHY0_ADDR<<16), &regs->mac_force_mode);
	reg=readl(&regs->mac_force_mode);
	writel((reg&(~(0x1f<<24)))|(PHY1_ADDR<<24), &regs->mac_force_mode);

	//enable soc port0 crc padding
	reg=readl(&regs->cpu_cntl);
	writel((reg&(~(0x1<<6)))|(0x1<<8),&regs->cpu_cntl);

	//enable port0
	reg=readl(&regs->port_cntl0);
	writel(reg&(~(0x3<<24)),&regs->port_cntl0);

	/*set vlan gp*/
	writel((port_map<<12)+(cpu_port<<10)+(vlan_id<<7)+(age<<4)+(proxy<<3)+(mc_ingress<<2)+0x1,&regs->wt_mac_ad0);
	reg=readl(&regs->wt_mac_ad0);
	while((reg&(0x1<<1))==0x0) {
		printf("wt_mac_ad0 = [%x]\n", reg);
		reg=readl(&regs->wt_mac_ad0);
	}

}

#define DEBUG0 printf

void spl2sw_dump_regs(struct eth_device *dev)
{
    u32 reg;

	struct spl2sw_regs *regs = (struct spl2sw_regs *)dev->iobase;
	DEBUG0("moon 0.10 	= %x\r\n",MOON0_REG->clken[9]);
	DEBUG0("moon 0.20 	= %x\r\n",MOON0_REG->gclken[9]);
	DEBUG0("moon 0.30 	= %x\r\n",MOON0_REG->reset[9]);
	DEBUG0("moon 5.04 	= %x\r\n",MOON5_REG->sft_cfg[4]);
	DEBUG0("moon 5.05 	= %x\r\n",MOON5_REG->sft_cfg[5]);
	DEBUG0("moon 30.08	= %x\r\n",MOON30_REG->sft_cfg[8]);

	


    DEBUG0("sw_int_status_0     = %x\r\n",regs->sw_int_status_0);
	DEBUG0("sw_int_mask_0       = %x\r\n",regs->sw_int_mask_0);
	DEBUG0("fl_cntl_th          = %x\r\n",regs->fl_cntl_th);
	DEBUG0("cpu_fl_cntl_th      = %x\r\n",regs->cpu_fl_cntl_th);
	DEBUG0("pri_fl_cntl         = %x\r\n",regs->pri_fl_cntl);
    DEBUG0("vlan_pri_th         = %x\r\n",regs->vlan_pri_th);
	DEBUG0("En_tos_bus          = %x\r\n",regs->En_tos_bus);
	DEBUG0("Tos_map0            = %x\r\n",regs->Tos_map0);
	DEBUG0("Tos_map1            = %x\r\n",regs->Tos_map1);
	DEBUG0("Tos_map2            = %x\r\n",regs->Tos_map2);
	DEBUG0("Tos_map3            = %x\r\n",regs->Tos_map3);
	DEBUG0("Tos_map4            = %x\r\n",regs->Tos_map4);
	DEBUG0("Tos_map5            = %x\r\n",regs->Tos_map5);
	DEBUG0("Tos_map6            = %x\r\n",regs->Tos_map6);
	DEBUG0("Tos_map7            = %x\r\n",regs->Tos_map7);
	DEBUG0("global_que_status   = %x\r\n",regs->global_que_status);
	DEBUG0("addr_tbl_srch       = %x\r\n",regs->addr_tbl_srch);
	DEBUG0("addr_tbl_st         = %x\r\n",regs->addr_tbl_st);
	DEBUG0("MAC_ad_ser0         = %x\r\n",regs->MAC_ad_ser0);
	DEBUG0("MAC_ad_ser1         = %x\r\n",regs->MAC_ad_ser1);
    DEBUG0("wt_mac_ad0          = %x\r\n",regs->wt_mac_ad0);
	DEBUG0("w_mac_15_0_bus      = %x\r\n",regs->w_mac_15_0_bus);
	DEBUG0("w_mac_47_16         = %x\r\n",regs->w_mac_47_16);
	DEBUG0("PVID_config0        = %x\r\n",regs->PVID_config0);
	DEBUG0("PVID_config1        = %x\r\n",regs->PVID_config1);
	DEBUG0("VLAN_memset_config0 = %x\r\n",regs->VLAN_memset_config0);
	DEBUG0("VLAN_memset_config1 = %x\r\n",regs->VLAN_memset_config1);
	DEBUG0("port_ability        = %x\r\n",regs->port_ability);
	DEBUG0("port_st             = %x\r\n",regs->port_st);
	DEBUG0("cpu_cntl            = %x\r\n",regs->cpu_cntl);
	DEBUG0("port_cntl0          = %x\r\n",regs->port_cntl0);
	DEBUG0("port_cntl1          = %x\r\n",regs->port_cntl1);
	DEBUG0("port_cntl2          = %x\r\n",regs->port_cntl2);
	DEBUG0("sw_glb_cntl         = %x\r\n",regs->sw_glb_cntl);
	DEBUG0("l2sw_rsv1           = %x\r\n",regs->l2sw_rsv1);
	DEBUG0("led_port0           = %x\r\n",regs->led_port0);
	DEBUG0("led_port1           = %x\r\n",regs->led_port1);
	DEBUG0("led_port2           = %x\r\n",regs->led_port2);
	DEBUG0("led_port3           = %x\r\n",regs->led_port3);
	DEBUG0("led_port4           = %x\r\n",regs->led_port4);
	DEBUG0("watch_dog_trig_rst  = %x\r\n",regs->watch_dog_trig_rst);
	DEBUG0("watch_dog_stop_cpu  = %x\r\n",regs->watch_dog_stop_cpu);
	DEBUG0("phy_cntl_reg0       = %x\r\n",regs->phy_cntl_reg0);
	DEBUG0("phy_cntl_reg1       = %x\r\n",regs->phy_cntl_reg1);
	DEBUG0("mac_force_mode      = %x\r\n",regs->mac_force_mode);
	DEBUG0("VLAN_group_config0  = %x\r\n",regs->VLAN_group_config0);
	DEBUG0("VLAN_group_config1  = %x\r\n",regs->VLAN_group_config1);
	DEBUG0("flow_ctrl_th3       = %x\r\n",regs->flow_ctrl_th3);
	DEBUG0("queue_status_0      = %x\r\n",regs->queue_status_0);
	DEBUG0("debug_cntl          = %x\r\n",regs->debug_cntl);
	DEBUG0("queue_status_0      = %x\r\n",regs->queue_status_0);
	DEBUG0("debug_cntl          = %x\r\n",regs->debug_cntl);
	DEBUG0("l2sw_rsv2           = %x\r\n",regs->l2sw_rsv2);
	DEBUG0("mem_test_info       = %x\r\n",regs->mem_test_info);
	DEBUG0("sw_int_status_1     = %x\r\n",regs->sw_int_status_1);
	DEBUG0("sw_int_mask_1       = %x\r\n",regs->sw_int_mask_1);
	DEBUG0("cpu_tx_trig         = %x\r\n",regs->cpu_tx_trig);
	DEBUG0("tx_hbase_addr_0     = %x\r\n",regs->tx_hbase_addr_0);
	DEBUG0("tx_lbase_addr_0     = %x\r\n",regs->tx_lbase_addr_0);
	DEBUG0("rx_hbase_addr_0     = %x\r\n",regs->rx_hbase_addr_0);
	DEBUG0("rx_lbase_addr_0     = %x\r\n",regs->rx_lbase_addr_0);
	DEBUG0("tx_hw_addr_0        = %x\r\n",regs->tx_hw_addr_0);
	DEBUG0("tx_lw_addr_0        = %x\r\n",regs->tx_lw_addr_0);
	DEBUG0("rx_hw_addr_0        = %x\r\n",regs->rx_hw_addr_0);
	DEBUG0("rx_lw_addr_0        = %x\r\n",regs->rx_lw_addr_0);
	DEBUG0("cpu_port_cntl_reg_0 = %x\r\n",regs->cpu_port_cntl_reg_0);


}

static int mac_hw_stop(struct eth_device *dev)
{
    struct spl2sw_regs *regs = (struct spl2sw_regs *)dev->iobase;
	struct spl2sw_dev *priv = dev->priv;
	int reg;
    
    regs->sw_int_mask_0 = 0xffffffff;
    regs->sw_int_status_0 = 0xffffffff & (~MAC_INT_PSC);

	reg = regs->cpu_cntl;
	regs->cpu_cntl = DIS_PORT_TX | reg;

	reg = regs->port_cntl0;
	regs->port_cntl0 = DIS_PORT_RX | reg;	

}


static int spl2sw_init(struct eth_device *dev, bd_t * bis)
{
	struct spl2sw_regs *regs = (struct spl2sw_regs *)dev->iobase;
	struct spl2sw_dev *priv = dev->priv;
	int value;

	printf("spl2sw_init1\n");
	mac_hw_stop(dev);

	/* Initialize the descriptor chains */
	init_tx_desc(priv);
	printf("spl2sw_init5\n");
	init_rx_desc(priv);
	printf("spl2sw_init6\n");

	/* enable port */
	spl2sw_enable_port(dev);
	printf("spl2sw_init3\n");

	/* set the hardware MAC address */
	spl2sw_hwaddr_set(dev);
	printf("spl2sw_init4\n");


	/* Initialize l2sw hw */
	spl2sw_hw_init(priv);
	printf("spl2sw_init end\n");
	spl2sw_dump_regs(dev);
	regs->sw_int_mask_0 = 0x00000000;

	return 0;
}

static int spl2sw_tx(struct eth_device *dev, void *packet, int length)
{
	struct spl2sw_regs *regs = (struct spl2sw_regs *)dev->iobase;
	struct spl2sw_dev *priv = dev->priv;
	u32 currdesc = priv->tx_currdesc;
	struct spl2sw_desc *txdesc;
	int timeout;
	//priv->txbuffer =0x9E820100;

	printf("spl2sw_tx, length = %d\n",length);

	int i;
	char * temp;
	#if 0
	temp=packet;
	for (i=0; i<length; i++)
	{
		printf("tx data idx %d [%x]\n",i ,*(temp+i) );

	}
	#endif 
	
	txdesc = priv->tx_desc[TX_DESC_QUEUE_NUM - 1];
	printf("txdesc = %x\n",txdesc);

	if(length < TX_BUF_MIN_SZ)
	{
		memset(priv->txbuffer, 0, TX_BUF_MIN_SZ);
		memcpy(priv->txbuffer, packet, length);
		tx_desc_send_set(txdesc, priv->txbuffer, TX_BUF_MIN_SZ);
	}
	else
	{
		tx_desc_send_set(txdesc, packet, length);
	}

	init_rx_desc(priv);
	regs->sw_int_status_0 = 0xffffffff ;

	/* write tx trig */
	regs->cpu_tx_trig=1;
	writel(0x1,&regs->cpu_tx_trig);


	timeout = 1000000;
	while (desc_get_owner(txdesc)) {
		//printf("le32_to_cpu(txdesc->cmd1) = [%x] \n",le32_to_cpu(txdesc->cmd1));
		//printf("txdesc->cmd1 = [%x] \n",txdesc->cmd1);
		#if 0
		DEBUG0("sw_int_status_0     = %x\r\n",regs->sw_int_status_0);
		if((regs->sw_int_status_0 & 0x00000080) == 0x80)
		{
			for(i=0; i<RX_TOTAL_DESC_NUM; i++)
			{
				printf("rx desc dump i=%d [%x]\n",i, priv->rx_desc_chain[i].cmd1);	
			}
		}
		#endif
		if (timeout-- < 0) {
			
			spl2sw_dump_regs(dev);

			for(i= 0;i<12;i++)
			{	
				regs->queue_status_0 = (i<<16);
				DEBUG0("sw_int_status_0  i=%d   = %x\r\n",i,regs->queue_status_0);
			
			}
			return -ETIMEDOUT;
		}
		udelay(1);
		
	}
	
	printf("spl2sw_tx done\n");

	priv->tx_currdesc = (currdesc + 1) & (TX_DESC_NUM - 1);
	return 0;
}

static int spl2sw_rx(struct eth_device *dev)
{
	struct spl2sw_regs *regs = (struct spl2sw_regs *)dev->iobase;
	struct spl2sw_dev *priv = dev->priv;
	u32 currdesc = priv->rx_currdesc;
	struct spl2sw_desc *rxdesc = &priv->rx_desc_chain[currdesc];
	int length = 0;

	//printf("spl2sw_rx\n");
	/* check if the host has the desc */
	if (desc_get_owner(rxdesc))
		return -1; /* something bad happened */

	length = desc_get_rx_frame_len(rxdesc);

	net_process_received_packet(desc_get_buf_addr(rxdesc), length);

	/* set descriptor back to owned by XGMAC */
	desc_set_rx_owner(rxdesc);

	priv->rx_currdesc = (currdesc + 1) & (RX_TOTAL_DESC_NUM - 1);

	return length;
}

static void spl2sw_halt(struct eth_device *dev)
{
	struct spl2sw_regs *regs = (struct spl2sw_regs *)dev->iobase;
	struct spl2sw_dev *priv = dev->priv;
	int value;

	/* Disable TX/RX */
	printf("spl2sw_halt\n");

	writel(0xffffffff, &regs->sw_int_mask_0);
	

	value = readl(&regs->cpu_cntl);
	value = (DIS_PORT_TX | value);
	writel(value, &regs->cpu_cntl);

	value = readl(&regs->port_cntl0);
	value = (DIS_PORT_RX | value);
	writel(value, &regs->port_cntl0);


	/* must set to 0, or when started up will cause issues */
	priv->tx_currdesc = 0;
	priv->rx_currdesc = 0;
}


int spl2sw_initialize(u32 id, ulong base_addr)
{
	struct eth_device *dev;
	struct spl2sw_dev *priv;
	struct spl2sw_regs *regs;
	u32 macaddr[2];

	printf("spl2sw_initialize\n");

	regs = (struct spl2sw_regs *)base_addr;

	/* check hardware version */
	//if (readl(&regs->version) != 0x1012)
	//	return -1;

	dev = malloc(sizeof(*dev));
	if (!dev)
		return 0;
	memset(dev, 0, sizeof(*dev));
	printf("spl2sw_initialize debug 1\n");
	/* Structure must be aligned, because it contains the descriptors */
	priv = memalign(32, sizeof(*priv));
	if (!priv) {
		free(dev);
		return 0;
	}
	printf("spl2sw_initialize debug 2\n");

	dev->iobase = (int)base_addr;
	dev->priv = priv;
	priv->dev = dev;
	sprintf(dev->name, "spl2sw%d", id);

	/* The MAC address is already configured, so read it from registers. */
	//macaddr[1] = readl(&regs->macaddr[0].hi);
	//macaddr[0] = readl(&regs->macaddr[0].lo);
	//memcpy(dev->enetaddr, macaddr, 6);
	
	memset(dev->enetaddr, 0x88, 6);
	dev->init = spl2sw_init;
	dev->send = spl2sw_tx;
	dev->recv = spl2sw_rx;
	dev->halt = spl2sw_halt;
	printf("spl2sw_initialize debug 3\n");

	eth_register(dev);
	printf("spl2sw_initialize debug 4\n");
	/* set broad pinmux */
	spl2sw_pinmux_set(dev);

	return 1;
}


