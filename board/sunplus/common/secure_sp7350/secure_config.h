#if 1//ndef __HPI_CMD_INC_H__
#define __HPI_CMD_INC_H__

#include <common.h>
#include "regmap_sp7350.h"


#define CONFIG_HAVE_SPACC
#define CONFIG_HAVE_HSM

#define CB_SRAM0_BASE       0xFA200000
#define CB_SRAM_SIZE        (256 * 1024)
#define CB_SRAM0_END        (CB_SRAM0_BASE + CB_SRAM_SIZE)
#define DRAM_MAX_SIZE		0xE0000000

#define SB_INFO_SIZE		200
#define SB_MAGIC          0x55434553     // SECU (S=55h)
#define TIMER_KHZ			90

#define __ALIGN4       __attribute__((aligned(4)))
#define __ALIGN8       __attribute__((aligned(8)))

struct ddt_entry {
        u32     ptr;
        u32     len;
}  __attribute__((packed)) __attribute__((aligned(8)));

/* 512 bytes */
struct boot_ddt {
	int idx;
	int len;
#define SC_DDT_MAX 62
	struct ddt_entry ddt[SC_DDT_MAX + 1]; /* ended in a null entry */
};

struct spacc_ram {
	__ALIGN8
	struct boot_ddt src;
	__ALIGN8
	struct boot_ddt dst;
};


#endif /* __HPI_CMD_INC_H__ */
