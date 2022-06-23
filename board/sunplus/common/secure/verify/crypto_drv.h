#ifndef __CRYPTO_DRV_H
#define __CRYPTO_DRV_H


//#include "crypto_utils.h"
//#include "sp_crypto.h"
//#include <types.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <stdbool.h>
#include <common.h>

/*
 *  platform related
 */
//#define SP_CRYPTO_REG_BASE      (0xf8000000+123*32*4)  //for q654
#define SP_CRYPTO_REG_BASE      (0x9c000000+84*32*4)  //for sp7021
//#define SP_CRYPTO_IRQ           (INTR_INDEX_SEC_INT)
#define SP_CRYPTO_IRQ           (148)

/*
 *   function related
 */
#define HASH_RING_SIZE          128
#define AES_RING_SIZE           128
#define MAX_TRB_PAYLOAD         ((64<<10)-1)
#define MAX_RSA_NBYTES          (2048>>3)

#ifndef SDK_RELEASE
/*
 *  debug related
 */
#define CRYPTO_DEBUG_ON
#endif

#define TAG "[CRYPTO] "

#ifdef CRYPTO_DEBUG_ON
//#define CRYPTO_LOGE(fmt, ...) printf("<1>"TAG fmt,##__VA_ARGS__)
//#define CRYPTO_LOGW(fmt, ...) printf("<2>"TAG fmt,##__VA_ARGS__)
//#define CRYPTO_LOGI(fmt, ...) printf("<3>"TAG fmt,##__VA_ARGS__)
//#define CRYPTO_LOGD(fmt, ...) printf("<4>"TAG fmt,##__VA_ARGS__)
#define CRYPTO_LOGE(fmt, ...) do{}while(0)
#define CRYPTO_LOGW(fmt, ...) do{}while(0)
#define CRYPTO_LOGI(fmt, ...) do{}while(0)
#define CRYPTO_LOGD(fmt, ...) do{}while(0)
#else
#define CRYPTO_LOGE(fmt, ...)  printf("<1>"TAG fmt,##__VA_ARGS__)
#define CRYPTO_LOGW(fmt, ...)  do{}while(0)
#define CRYPTO_LOGI(fmt, ...)  do{}while(0)
#define CRYPTO_LOGD(fmt, ...)  do{}while(0)
#endif

/*
//#define ERR_OUT(err, label, info, ...) \
//do { \
//	if (err) { \
//		CRYPTO_LOGE("[err@%s(%d)]"info"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
//		goto label; \
//	} \
//} while (0)
*/
#define ERR_OUT(err, label, info, ...) \
do{}while(0) \

/*
 *   linux types
 */
typedef u32 dma_addr_t;
//typedef u64 dma_addr_t;
#define SPCRYPTO_RET_SUCCESS             0
#define SPCRYPTO_RET_INVALID_ARG         1
#define SPCRYPTO_RET_DEV_ERROR           2
#define HZ          1000





/*
 *  macros for AESDMA_CRCR or HASHDMA_CRCR
 */
#define AUTODMA_CRCR_CP         (1 << 4)
#define AUTODMA_CRCR_CRR        (1 << 3)
#define AUTODMA_CRCR_CS         (1 << 1)
#define AUTODMA_CRCR_RCS        (1 << 0)
#define AUTODMA_CRCR_FLAGS      (AUTODMA_CRCR_RCS | AUTODMA_CRCR_CP)

/*
 *  macros for AESDMA_RCSR or HASHDMA_RCSR
 */
#define AUTODMA_RCSR_EN         (1 << 31)
#define AUTODMA_RCSR_ERF        (1 << 30)
#define AUTODMA_RCSR_SIZE_MASK  (0xffff)

/*
 * macros for AESDMA_RTR or HASHDMA_RTR
 */
#define AUTODMA_TRIGGER         (1)

/*
 *  macros for SECIF register
 */
#define AES_CMD_RD_IF           (1 << 8)
#define HASH_CMD_RD_IF          (1 << 7)
#define AES_TRB_IF              (1 << 6)
#define HASH_TRB_IF             (1 << 5)
#define AES_ERF_IF              (1 << 4)
#define HASH_ERF_IF             (1 << 3)
#define AES_DMA_IF              (1 << 2)
#define HASH_DMA_IF             (1 << 1)
#define RSA_DMA_IF              (1 << 0)

/*
 *  macros for SECIE register
 */
#define AES_CMD_RD_IE           (1 << 8)
#define HASH_CMD_RD_IE          (1 << 7)
#define AES_TRB_IE              (1 << 6)
#define HASH_TRB_IE             (1 << 5)
#define AES_ERF_IE              (1 << 4)
#define HASH_ERF_IE             (1 << 3)
#define AES_DMA_IE              (1 << 2)
#define HASH_DMA_IE             (1 << 1)
#define RSA_DMA_IE              (1 << 0)

struct sp_crypto_reg {
	/*  Field Name  Bit     Access Description  */
	/*  123.0 AES DMA Control Status register (AESDMACS)
	 *  SIZE        31:16   RW DMA Transfer Length
	 *  Reserved    15:4    RO Reserved
	 *  ED          3       RO Endian 0:little endian 1:big endian
	 *  Reserved    2:1     RO Reserved
	 *  EN          0       RW DMA enable, it will be auto-clear to 0
	 *  when DMA finishes
	 */
	unsigned int AESDMACS;

	/* 123.1 AES Source Data pointer (AESSPTR) */
	/*  SPTR        31:0    RW Source address must 32B alignment */
	unsigned int AESSPTR;

	/* 123.2 AES Destination Data pointer (AESDPTR)
	 *  DPTR        31:0    RW Destination address must 32B alignment
	 */
	unsigned int AESDPTR;

	/* 123.3 0.4 0.5: AES Parameters */
	unsigned int AESPAR0; // mode
	unsigned int AESPAR1; // iptr
	unsigned int AESPAR2; // kptr

	/* 123.6 HASH DMA Control Status register (HASHDMACS)
	 *   SIZE        31:16   RW DMA Transfer Length
	 *   Reserved    15:4    RO Reserved
	 *   ED          3       RO Endian 0:little endian 1:big endian
	 *   Reserved    2:1     RO Reserved
	 *   EN          0       RW DMA enable, it will be auto-clear to 0
	 *   when DMA finishes
	 */
	unsigned int HASHDMACS;

	/* 123.7 HASH Source Data pointer (HASHSPTR)
	 *   SPTR 31:0 RW Source address must 32B alignment
	 */
	unsigned int HASHSPTR;

	/* 123.8 HASH Destination Data pointer (HASHDPTR)
	 *   DPTR        31:0    RW Source address must 32B alignment
	 */
	unsigned int HASHDPTR;

	/* 123.9 0.10 0.11: HASH Parameters */
	unsigned int HASHPAR0; /* mode */
	unsigned int HASHPAR1; /* iptr */
	unsigned int HASHPAR2; /* kptr(hptr) */

	/* 123.12 RSA DMA Control Status register (RSADMACS)
	 *   SIZE        31:16   RW DMA Transfer Length
	 *   Reserved    15:4    RO Reserved
	 *   ED          3       RO Endian 0:little endian 1:big endian
	 *   Reserved    2:1     RO Reserved
	 *   EN          0       RW DMA enable, it will be auto-clear to 0
	 *   when DMA finishes
	 */
	unsigned int RSADMACS;
#define RSA_DMA_ENABLE  (1 << 0)
#define RSA_DATA_BE     (1 << 3)
#define RSA_DATA_LE     (0 << 3)
#define RSA_DMA_SIZE(x) (x << 16)

	/*123.13 RSA Source Data pointer (RSASPTR)
	 *   SPTR        31:0    RW Source(X) address Z=X**Y (mod N),
	 *  must 32B alignment
	 */
	unsigned int RSASPTR;

	/* 123.14 RSA Destination Data pointer (RSADPTR)
	 *   DPTR        31:0    RW Destination(Z) address Z=X**Y (mod N),
	 *   must 32B alignment
	 */
	unsigned int RSADPTR;

	/* 123.15 RSA Dma Parameter 0 (RSAPAR0)
	 *   D           31:16   RW N length, Only support 64*n(3<=n<=32)
	 *   Reserved    15:8    RO Reserved
	 *   PRECALC     7       RW Precalculate P2
	 *                       0: Precalculate and write back to pointer from P2PTR
	 *                       1: Fetch from P2PTR
	 *   Reserved    6:0     RO Reserved
	 */
	unsigned int RSAPAR0;
#define RSA_SET_PARA_D(x)   ((x) << 16)
#define RSA_PARA_PRECAL_P2  (0 << 7)
#define RSA_PARA_FETCH_P2   (1 << 7)

	/* 123.16 RSA Dma Parameter 1 (RSAPAR1)
	 *   YPTR 31:0 RW Y pointer Z=X**Y (mod N)
	 */
	unsigned int RSAYPTR;

	/* 123.17 RSA Dma Parameter 2 (RSAPAR2)
	 *   NPTR 31:0 RW N pointer Z=X**Y (mod N)
	 */
	unsigned int RSANPTR;

	/* 123.18 RSA Dma Parameter 3 (RSAPAR3)
	 * P2PTR 31:0 RW P2 pointer P2 = P**2(mod N)
	 */
	unsigned int RSAP2PTR;

	/* 123.19 0.20 RSA Dma Parameter 4 (RSAPAR4)
	 *   WPTR 31:0 RW W pointer W= -N**-1(mod N)
	 */
	unsigned int RSAWPTRL;
	unsigned int RSAWPTRH;

	/* 123.21 AES DMA Command Ring Control Register (AESDMA_CRCR)
	 *   CRPTR       31:5    RW Command Ring Pointer The command ring
	 *   should be 32bytes aligned
	 *   CP          4       RW Cycle bit Position 0:Word 0[0] 1:Word7[31]
	 *   CRR         3       RO Command Ring Running Indicates the command
	 *   ring is running, SW can only change the pointer
	 *   when this bit is cleared
	 *   Reserved    2       RO Reserved
	 *   CS          1       RW Command Ring Stop
	 *   Write 1 to stop the command ring
	 *   RCS         0       RW Ring Cycle State
	 *   Indicates the initial state of ring cycle bit
	 */
	unsigned int AESDMA_CRCR;

	/* 123.22 AES DMA Event Ring Base Address Register (AESDMA_ERBAR)
	 *   ERBA        31:4    RW Event Ring Base Address
	 *   the first TRB of the status will be write to this address
	 *   Reserved    3:0     RO Reserved
	 */
	unsigned int AESDMA_ERBAR;

	/* 123.23 AES DMA Event Ring De-queue Pointer Register (AESDMA_ERDPR)
	 *   ERDP        31:4    RW Event Ring De-queue Pointer
	 *   Indicates the TRB address of which the CPU is
	 *   processing now
	 *   Reserved    3:0     RO Reserved
	 */
	unsigned int AESDMA_ERDPR;

	/* 123.24 AES DMA Ring Control and Status Register (AESDMA_RCSR)
	 *   EN          31      RW Auto DMA enable
	 *   	To enable the auto DMA feature
	 *   ERF         30      RW1C Event ring Full
	 *   	Indicates the Event Ring has been writing full
	 *   Reserved    29:16   RO  Reserved
	 *   Size        15:0    RW Event Ring Size
	 *   	HWwill write to ERBA if the size reaches this value
	 *   	and ERDP != ERBA (number of trbs)
	 */
	unsigned int AESDMA_RCSR;

	/* 123.25 AES DMA Ring Trig Register (AESDMA_RTR)
	 *   Reserved    31:1    RO Reserved
	 *   CRT         0       RW Command Ring Trig
	 *   After SW write a 1 to this bit, HW will start transfer
	 *   TRBs until the ring is empty or stopped
	 */
	unsigned int AESDMA_RTR;

	/* 123.26 HASH DMA Command Ring Control Register (HASHDMA_CRCR)
	 *   CRPTR       31:5    RW  Command Ring Pointer
	 *   The command ring should be 32bytes aligned
	 *   CP          4       RW Cycle bit Position
	 *   0:Word 0[0] 1:Word7[31]
	 *   CRR         3       RO Command Ring Running
	 *   Indicates the command ring is running, SW can only
	 *   change the pointer when this bit is cleared
	 *   Reserved    2       RO Reserved
	 *   CS          1       RW Command Ring Stop
	 *   Write 1 to stop the command ring
	 *   RCS         0       RW Ring Cycle State
	 *   Indicates the initial state of ring cycle bit
	 */
	unsigned int HASHDMA_CRCR;

	/* 123.27 HASH DMA Event Ring Base Address Register (HASHDMA_ERBAR)
	 *   ERBA        31:4    RW Event Ring Base Address
	 *   The first TRB of the status will be write to this address
	 *   Reserved    3:0     RO Reserved
	 */
	unsigned int HASHDMA_ERBAR;

	/* 123.28 HASH DMA Event Ring De-queue Pointer Register (HASHDMA_ERDPR)
	 *   ERDP        31:4    RW Event Ring De-queue Pointer
	 *   Indicates the TRB address of which the CPU is processing now
	 *   Reserved    3:0     RO Reserved
	 */
	unsigned int HASHDMA_ERDPR;

	/* 123.29 HASH DMA Ring Control and Status Register (HASHDMA_RCSR)
	 *   EN          31      RW Auto DMA enable
	 *   To enable the auto DMA feature
	 *   ERF         30      RW1C Event ring Full
	 *   Indicates the Event Ring has been writing full
	 *   Reserved    29:16   RO Reserved
	 *   Size 15:0 RW Event Ring Size
	 *   HW will write to ERBA if the size reaches this value and ERDP != ERBA
	 */
	unsigned int HASHDMA_RCSR;

	/* 123.30 HASH DMA Ring Trig Register (HASHDMA_RTR)
	 *   Reserved    31:1    RO Reserved
	 *   CRT         0       RW Command Ring Trig
	 *   After SW write a 1 to this bit, HW will start transfer
	 *   TRBs until the ring is empty or stopped
	 */
	unsigned int HASHDMA_RTR;

	/* 123.31 */
	unsigned int reserved;

	/* 124.0 SEC IP Version (VERSION)
	 *   VERSION 31:0 RO the date of version
	 */
	unsigned int VERSION;

	/* 124.1 Interrupt Enable (SECIE)
	 *   Reserve     31:7    RO Reserve
	 *   AES_TRB_IE  6       RW AES TRB done interrupt enable
	 *   HASH TRB IE 5       RW HASH TRB done interrupt enable
	 *   AES_ERF_IE  4       RW AES Event Ring Full interrupt enable
	 *   HASH_ERF_IE 3       RW HASH Event Ring Full interrupt enable
	 *   AES_DMA_IE  2       RW AES DMA finish interrupt enable
	 *   HASH_DMA_IE 1       RW HASH DMA finish interrupt enable
	 *   RSA_DMA_IE  0       RW RSA DMA finish interrupt enable
	 */
	unsigned int SECIE;

	/* 124.2 Interrupt Flag (SECIF)
	 *   Reserve     31:7    RO Reserve
	 *   AES_TRB_IF  6       RW AES TRB done interrupt flag
	 *   HASH_TRB_IF 5       RW HASH TRB done interrupt flag
	 *   AES_ERF_IF  4       RW AES Event Ring Full interrupt flag
	 *   HASH_ERF_IF 3       RW HASH Event Ring Full interrupt flag
	 *   AES_DMA_IF  2       RW AES DMA finish interrupt flag
	 *   HASH_DMA_IF 1       RW HASH DMA finish interrupt flag
	 *   RSA_DMA_IF  0       RW RSA DMA finish interrupt flag
	 */
	unsigned int SECIF;

	/*
	 *   HASH_RESET  2       RW write 0 to reset hash hw
	 *   RSA_RESET   1       RW write 0 to reset rsa hw
	 *   AES_RESET   0       RW write 0 to reset aes hw
	 */
	unsigned int SECRESET;
};

/*
 *  TRB types
 */
#define TRB_NORMAL              (1)
#define TRB_LINK                (2)



/*
 *  macros for trb.para.mode
 */
#define M_MMASK         0x7F            /* mode mask */
/* AES */
#define M_AES_ECB       0x00000000
#define M_AES_CBC       0x00000001
#define M_AES_CTR       (0x00000002 | (128 << 24))
#define M_AES_GCTR      (0x00000002 | (32 << 24))
#define M_CHACHA20      0x00000003
#define M_ENC           (0 << 7)        /* 0: encrypt */
#define M_DEC           (1 << 7)        /* 1: decrypt */
#define M_KEYLEN(x)     ((((x)>>2)&0xff)<<16)
/* HASH */
#define M_MD5           0x00000000
#define M_SHA3          0x00000001
#define M_SHA3_224      0x00000001
#define M_SHA3_256      0x00010001
#define M_SHA3_384      0x00020001
#define M_SHA3_512      0x00030001
#define M_GHASH         0x00000002
//#define M_SHA2          0x00000003
//#define M_SHA2_256      0x00000003
//#define M_SHA2_512      0x00010003
//#define M_POLY1305      0x00000004
#define M_UPDATE        (0 << 7)        /* 0: update */
#define M_FINAL         (1 << 7)        /* 1: final  */
#define M_PADDED        (1 << 8)        /* only for poly1305 */
					/* 1: padded  0:no padded */

typedef enum {
	SP_CRYPTO_AES = 0x20180428,
	SP_CRYPTO_HASH,
	SP_CRYPTO_RSA,
} crypto_type_t;

struct trb_s {
	/* Cycle bits. indicates the current cycle of the ring */
	void *priv;

	/* Completion Code. only use in event trb
	 *   0 Invalid Indicates this field has not been updated
	 *   1 Success indicates the transfer is successfully completed
	 */
	unsigned int cc:1;

	/* Toggle Cycle bit. Used in link TRB only.
	 *   indicates the cycle bits will be toggle in trb_next segment.
	 */
	unsigned int tc:1;

	/* Interrupt On Complete.
	 *   when this bit is set, controller will set an interrupt
	 *   after this TRB is transmitted.
	 */
	unsigned int ioc:1;

	unsigned int rsv1:1;

	/* TRB type:
	 *   0x1: Normal. Normal TRB used in trb_ring.
	 *   0x2: Link. Link TRB to link to trb_ring segments
	 */
	unsigned int type:4;

	unsigned int rsv2:8;

	/*  Plain text size in bytes.
	 * indicates the read/write data bytes of this TRB.
	 */
	unsigned int size:16;

	/* For link TRB indicates the trb_next segment address.
	 * or  Source data pointer(depend on ENDC)
	 */
	dma_addr_t sptr;

	/* Destination data pointer (depend on ENDC)
	 */
	dma_addr_t dptr;

	/* Parameter
	 */
	unsigned int mode;
	dma_addr_t iptr; /* Initial Vector/Counter (IV/ICB) pointer */
	dma_addr_t kptr; /* AES/GHASH: Key/Sub-Key pointer */

	/* Cycle bits. indicates the current cycle of the ring */
	unsigned int c;

}__attribute__((__packed__));

typedef struct trb_s trb_t;

typedef struct trb_ring_s {
	trb_t *trb;

	/* get trb at tail, put trb at head */
	trb_t *head;
	trb_t *tail;
	trb_t *link;

//	struct mutex lock;
//	struct semaphore sem;

	unsigned int trigger_count;
	unsigned int irq_count;
} trb_ring_t;

typedef struct crypto_work_s {
	crypto_type_t type;
	trb_ring_t *ring;
//	volatile bool done;
//	wait_queue_head_t wait;
} crypto_work_t;

struct crypto_dev_s {
	volatile struct sp_crypto_reg *reg;
	unsigned int irq;
	unsigned int initialized;
	trb_ring_t *aes_ring;
	trb_ring_t *hash_ring;

	/* rsa related */
//	struct mutex rsa_lock;
//	wait_queue_head_t rsa_wait;
	volatile unsigned int rsa_done;
	unsigned int rsa_nbytes;
	unsigned char* rsa_n;
	unsigned char* rsa_e;
	unsigned char* rsa_a;
	unsigned char* rsa_x;
	unsigned char* rsa_p2;
};

//int crypto_work_init(crypto_work_t *ctx, crypto_type_t type);
//void crypto_work_deinit(crypto_work_t *ctx);
//int crypto_work_do(crypto_work_t *ctx, bool wait);
//int crypto_work_waitdone(crypto_work_t *ctx);
//int crypto_work_add(crypto_work_t *ctx,
//	dma_addr_t src, dma_addr_t dst, dma_addr_t iv, dma_addr_t key,
//	unsigned int len, unsigned int mode, unsigned char ioc);
//int crypto_do_expmod(unsigned char *x, unsigned char *a, unsigned char *e,
//	unsigned char *n, unsigned int size, unsigned char big_endian);

//int crypto_compare_array(unsigned char *a,unsigned char *b,unsigned int size,unsigned char reverse);
//void crypto_copy_array(unsigned char *dst,unsigned char *src,unsigned int size,unsigned char reverse);
//void crypto_rand_array(unsigned char array[], unsigned int size);
//long long crypto_mont_w(unsigned char *mod);
//void  crypto_dump_buf(const char *name, void *buf, int size);


#endif //__CRYPTO_DRV_H

