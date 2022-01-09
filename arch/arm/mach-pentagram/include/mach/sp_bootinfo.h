#ifndef __SP_BOOTINFO_H
#define __SP_BOOTINFO_H

#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
#include <asm/arch/sp_bootinfo_q645.h>
#elif defined(CONFIG_TARGET_PENTAGRAM_SP7350)
#include <asm/arch/sp_bootinfo_sp7350.h>
#else
#include <asm/arch/sp_bootinfo_sc7xxx.h>
#endif

#endif /* __SP_BOOTINFO_H */
