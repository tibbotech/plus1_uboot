/*
 * Sunplusl 2swc ethernet driver for u-boot
 */

#ifndef _SUNPLUS_L2SW_
#define _SUNPLUS_L2SW_

#include <net.h>

#include <asm/types.h>
#include <asm/byteorder.h>

#include <linux/compiler.h>

#define PHY0_ADDR			0x2
#define PHY1_ADDR			0x3
/* define DESC NUM*/
#define	TX_DESC_NUM			8
#define	RX_DESC_NUM			8
#define RX_QUEUE0_DESC_NUM	8
#define RX_QUEUE1_DESC_NUM	8
#define RX_DESC_NUM_MAX		(RX_QUEUE0_DESC_NUM > RX_QUEUE1_DESC_NUM ? RX_QUEUE0_DESC_NUM : RX_QUEUE1_DESC_NUM)
#define RX_DESC_QUEUE_NUM	2
#define TX_DESC_QUEUE_NUM	1

#define RX_TOTAL_DESC_NUM	(RX_DESC_NUM * RX_DESC_QUEUE_NUM)
#define TX_TOTAL_DESC_NUM	(TX_DESC_NUM * TX_DESC_QUEUE_NUM)

#define ETH_BUF_SZ			2046
#define TX_BUF_SZ			(ETH_BUF_SZ * TX_DESC_NUM * TX_DESC_QUEUE_NUM)
#define RX_BUF_SZ			(ETH_BUF_SZ * RX_DESC_NUM * RX_DESC_QUEUE_NUM)
#define TX_BUF_MIN_SZ		70

/*define tx/rx descriptor bit*/
#define OWN_BIT						(1<<31)	
#define EOR_BIT						(1<<31)	
#define FS_BIT						(1<<25)	
#define LS_BIT						(1<<24)
#define RXDESC_FRAME_LEN_MASK		0x000007FF
#define RXDESC_DA_FILTER_FAIL		0x0C000000

#define DIS_PORT_TX             0x00000040
#define DIS_PORT_RX             (1<<24)


#define LEN_MASK			0x000007FF
#define DIS_PORT_TX             0x00000040
#define DIS_PORT_RX             (1<<24)
#define MAC_INT_PSC				(1<<19)	





/* desc struct  */
struct spl2sw_desc {
	volatile u32 cmd1;
	volatile u32 cmd2;
	volatile u32 addr1;
	volatile u32 addr2;
};

/* priv struct  */

struct spl2sw_dev {
	struct spl2sw_desc rx_desc_chain[RX_TOTAL_DESC_NUM];
	struct spl2sw_desc tx_desc_chain[TX_TOTAL_DESC_NUM];
	struct spl2sw_desc *rx_desc[RX_DESC_QUEUE_NUM];
	struct spl2sw_desc *tx_desc[TX_DESC_QUEUE_NUM];
	u32    rx_desc_num[RX_DESC_QUEUE_NUM];
	u32    tx_desc_num[TX_DESC_QUEUE_NUM];
	char   rxbuffer[RX_BUF_SZ];
	char   *txbuffer;//[TX_BUF_MIN_SZ];  = 0x9E820100
	u32    tx_currdesc;
	u32    rx_currdesc;
	struct eth_device *dev;
} __aligned(32);


/* regs define */
#define REG_BASE           0x9c000000
#define RF_GRP(_grp, _reg) ((((_grp) * 32 + (_reg)) * 4) + REG_BASE)

struct moon0_regs {
	unsigned int stamp;            // 0.0
	unsigned int clken[10];        // 0.1
	unsigned int gclken[10];       // 0.11
	unsigned int reset[10];        // 0.21
	unsigned int hw_cfg;           // 0.31
};
#define MOON0_REG ((volatile struct moon0_regs *)RF_GRP(0, 0))


struct moon1_regs {
	unsigned int sft_cfg[32];
};
#define MOON1_REG ((volatile struct moon1_regs *)RF_GRP(1, 0))

struct moon2_regs {
	unsigned int sft_cfg[32];
};
#define MOON2_REG ((volatile struct moon2_regs *)RF_GRP(2, 0))
 

struct moon5_regs {
	unsigned int sft_cfg[32];
};
#define MOON5_REG ((volatile struct moon5_regs *)RF_GRP(5, 0))

struct moon30_regs {
	unsigned int sft_cfg[32];
};
#define MOON30_REG ((volatile struct moon5_regs *)RF_GRP(30, 0))


struct spl2sw_regs {
	u32 sw_int_status_0;
	u32 sw_int_mask_0;
	u32 fl_cntl_th;
	u32 cpu_fl_cntl_th;
	u32 pri_fl_cntl;
	u32 vlan_pri_th;
	u32 En_tos_bus;
	u32 Tos_map0;
	u32 Tos_map1;
	u32 Tos_map2;
	u32 Tos_map3;
	u32 Tos_map4;
	u32 Tos_map5;
	u32 Tos_map6;
	u32 Tos_map7;
	u32 global_que_status;
	u32 addr_tbl_srch;
	u32 addr_tbl_st;
	u32 MAC_ad_ser0;
	u32 MAC_ad_ser1;
	u32 wt_mac_ad0;
	u32 w_mac_15_0_bus;
	u32 w_mac_47_16;
	u32 PVID_config0;
	u32 PVID_config1;
	u32 VLAN_memset_config0;
	u32 VLAN_memset_config1;
	u32 port_ability;
	u32 port_st;
	u32 cpu_cntl;
	u32 port_cntl0;
	u32 port_cntl1;
	u32 port_cntl2;
	u32 sw_glb_cntl;
	u32 l2sw_rsv1;
	u32 led_port0;
	u32 led_port1;
	u32 led_port2;
	u32 led_port3;
	u32 led_port4;
	u32 watch_dog_trig_rst;
	u32 watch_dog_stop_cpu;
	u32 phy_cntl_reg0;
	u32 phy_cntl_reg1;
	u32 mac_force_mode;
	u32 VLAN_group_config0;
	u32 VLAN_group_config1;
	u32 flow_ctrl_th3;
	u32 queue_status_0;
	u32 debug_cntl;
	u32 l2sw_rsv2;
	u32 mem_test_info;
	u32 sw_int_status_1;
	u32 sw_int_mask_1;
	u32 l2sw_rsv3[76];
	u32 cpu_tx_trig;
	u32 tx_hbase_addr_0;
	u32 tx_lbase_addr_0;
	u32 rx_hbase_addr_0;
	u32 rx_lbase_addr_0;
	u32 tx_hw_addr_0;
	u32 tx_lw_addr_0;
	u32 rx_hw_addr_0;
	u32 rx_lw_addr_0;
	u32 cpu_port_cntl_reg_0;
	u32 tx_hbase_addr_1;
	u32 tx_lbase_addr_1;
	u32 rx_hbase_addr_1;
	u32 rx_lbase_addr_1;
	u32 tx_hw_addr_1;
	u32 tx_lw_addr_1;
	u32 rx_hw_addr_1;
	u32 rx_lw_addr_1;
	u32 cpu_port_cntl_reg_1;
};

#endif /* _SUNPLUS_L2SW_ */
