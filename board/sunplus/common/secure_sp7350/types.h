#ifndef __TYPE_H__
#define __TYPE_H__

#include <stddef.h>    // size_t
#include <stdint.h>    // uint32_t

typedef unsigned char		BYTE;

#define UINT64			u64
#define UINT32			u32
#define UINT16			u16
#define UINT8			u8
#define INT64			s64
#define INT32			s32
#define INT16			s16
#define INT8			s8
#define SINT32			s32
#define SINT16			s16
#define SINT8			s8

#define PASS 			0
#define FAIL			2

#define ROM_SUCCESS		PASS
#define ROM_FAIL		FAIL

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL    ((void *)0)
#endif

#define __ALIGN4       __attribute__((aligned(4)))
#define __ALIGN8       __attribute__((aligned(8)))

#endif // __TYPES_H__
