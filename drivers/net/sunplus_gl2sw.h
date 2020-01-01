/*
 * Sunplus l2sw ethernet driver for u-boot
 */

#ifndef __SUNPLUS_L2SW_H__
#define __SUNPLUS_L2SW_H__

#include <net.h>
#include <linux/compiler.h>


// debug macros
#define eth_err(fmt, arg...)            printf(fmt, ##arg)
#if 1
#define eth_info(fmt, arg...)           printf(fmt, ##arg)
#else
#define eth_info(fmt, arg...)
#endif


// tx/rx descriptor cmd1 bits
#define OWN_BIT                         (1<<31)
#define FS_BIT                          (1<<25)
#define LS_BIT                          (1<<24)
#define LEN_MASK                        0x000007FF

// tx/rx descriptor cmd2 bits
#define EOR_BIT                         (1<<31)

// Address table search
#define MAC_ADDR_LOOKUP_IDLE            (1<<2)
#define MAC_SEARCH_NEXT_ADDR            (1<<1)
#define MAC_BEGIN_SEARCH_ADDR           (1<<0)

// Address table status
#define MAC_HASK_LOOKUP_ADDR_MASK       (0x3ff<<22)
#define MAC_AT_TABLE_END                (1<<1)
#define MAC_AT_DATA_READY               (1<<0)

// Register write & read
#define HWREG_W(M, N)                   writel(N, (void*)&l2sw_reg_base->M)
#define HWREG_R(M)                      readl((void*)&l2sw_reg_base->M)
#define MOON5REG_W(M, N)                writel(N, (void*)&moon5_reg_base->M)
#define MOON5REG_R(M)                   readl((void*)&moon5_reg_base->M)

// Queue defines
#define CONFIG_TX_DESCR_NUM             4
#define CONFIG_TX_QUEUE_NUM             2
#define CONFIG_RX_DESCR_NUM             4
#define CONFIG_RX_QUEUE_NUM             2
#define CONFIG_ETH_BUFSIZE              2048            /* must be dma aligned */
#define TX_BUF_MIN_SZ                   60

#define CONFIG_ETH_RXSIZE               2046            /* must fit in ETH_BUFSIZE */

#define TX_TOTAL_BUFSIZE                (CONFIG_ETH_BUFSIZE * CONFIG_TX_DESCR_NUM * CONFIG_TX_QUEUE_NUM)
#define RX_TOTAL_BUFSIZE                (CONFIG_ETH_BUFSIZE * CONFIG_RX_DESCR_NUM * CONFIG_RX_QUEUE_NUM)

#define CONFIG_MDIO_TIMEOUT             (3 * CONFIG_SYS_HZ)


#define CONFIG_SYS_CACHELINE_SIZE       64
#define DCACHE_ROUNDDN(x)               (((u64)(x))&(~(CONFIG_SYS_CACHELINE_SIZE-1)))
#define DCACHE_ROUNDUP(x)               (DCACHE_ROUNDDN(x-1)+CONFIG_SYS_CACHELINE_SIZE)


DECLARE_GLOBAL_DATA_PTR;

struct spl2sw_desc {
	volatile u32 cmd1;
	volatile u32 cmd2;
	volatile u32 addr1;
	volatile u32 addr2;
};

struct emac_eth_dev {
	volatile struct spl2sw_desc rx_desc[CONFIG_RX_DESCR_NUM*CONFIG_RX_QUEUE_NUM] __aligned(ARCH_DMA_MINALIGN);
	volatile struct spl2sw_desc tx_desc[CONFIG_TX_DESCR_NUM*CONFIG_TX_QUEUE_NUM] __aligned(ARCH_DMA_MINALIGN);

	volatile char rxbuffer[RX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);
	volatile char txbuffer[TX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);

	volatile u32 tx_pos;
	volatile u32 rx_pos;

	struct mii_dev *bus;

	struct phy_device *phy_dev0;
	struct phy_device *phy_dev1;

	u32 interface;
	u32 phy_addr0;
	u32 phy_addr1;
	u32 phy_configured;

	u8 mac_addr[8];
	u32 otp_mac_addr;
};


//=================================================================================================
/*
 * TYPE: RegisterFile_L2SW
 */
struct l2sw_reg {
	u32 sw_int_status;
	u32 sw_int_mask;
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
	u32 w_mac_15_0;
	u32 w_mac_47_16;
	u32 PVID_config0;
	u32 reserved0;
	u32 VLAN_memset_config0;
	u32 VLAN_memset_config1;
	u32 port_ability;
	u32 port_st;
	u32 cpu_cntl;
	u32 port_cntl0;
	u32 port_cntl1;
	u32 port_cntl2;
	u32 sw_glb_cntl;
	u32 sw_reset;
	u32 led_port0;
	u32 led_port1;
	u32 reserved1[3];
	u32 watch_dog_trig_rst;
	u32 watch_dog_stop_cpu;
	u32 phy_cntl_reg0;
	u32 phy_cntl_reg1;
	u32 mac_force_mode0;
	u32 VLAN_group_config0;
	u32 reserved2;
	u32 flow_ctrl_th3;
	u32 queue_status_0;
	u32 debug_cntl;
	u32 debug_info;
	u32 mem_test_info;
	u32 sw_global_signal;
	u32 pause_uc_sa_sw_15_0;
	u32 pause_uc_sa_sw_47_16;
	u32 reserved3[2];
	u32 mac_force_model;
	u32 p0_softpad_config;
	u32 p1_softpad_config;
	u32 reserved4[70];
	u32 cpu_tx_trig;
	u32 tx_hbase_addr;
	u32 tx_lbase_addr;
	u32 rx_hbase_addr;
	u32 rx_lbase_addr;
	u32 tx_hw_addr;
	u32 tx_lw_addr;
	u32 rx_hw_addr;
	u32 rx_lw_addr;
	u32 cpu_port_cntl_reg;
	u32 desc_addr_cntl;
};

//=================================================================================================
/*
 * TYPE: RegisterFile_MOON5
 */
struct moon5_reg {
	u32 mo5_thermal_ctl_0;
	u32 mo5_thermal_ctl_1;
	u32 mo4_thermal_ctl_2;
	u32 mo4_thermal_ctl_3;
	u32 mo4_tmds_l2sw_ctl;
	u32 mo4_l2sw_clksw_ctl;
};

#endif /* __SUNPLUS_L2SW_H__ */