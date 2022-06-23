#if 0//ndef __SPACC_HW_INC_H__
#define __SPACC_HW_INC_H__

/*
 * 1. DDT structure must be 8-byte aligned
 * 2. DDT structure must not straddle 2^12
 * 3. DDT fragments must not straddle 2^12
 * 4. Max message buffer lenth = 2^16 - 1
 *
 * note:
 * Q645 ELP_SPACC_CONFIG_CLUSTER_ADDR_WIDT = 12 -> 2^12 (4KB)
 * Q645 ELP_SPACC_CONFIG_MSG_ADDR_WIDTH = 16 (64KB)
 */
struct ddt_entry {
        u32     ptr;
        u32     len;
}  __attribute__((packed)) __attribute__((aligned(8)));

#endif /* __SPACC_HW_INC_H__ */
