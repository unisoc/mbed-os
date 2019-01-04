/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MBED_UWP_TYPE_H
#define MBED_UWP_TYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __ramfunc static inline
#define TRUE  (1)
#define FALSE (0)
#define BIT(nr) (1UL << (nr))
#define CLR_BIT(reg, bit) ((reg) &= ~(1<<(bit)))
#define SET_BIT(reg, bit) ((reg) |=  (1<<(bit)))
#define SCI_ASSERT(a)
#define set_bits(value, addr)	\
	(*(volatile unsigned long *)(addr)) |= (value)

#define clr_bits(value, addr)	\
	(*(volatile unsigned long *)(addr)) &= ~(value)

#if defined(__arm__) && defined(__ARMCC_VERSION) 
    /* Keil uVision4 tools */
    #define PACK_STRUCT_BEGIN __packed
    #define PACK_STRUCT_STRUCT
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    #define ALIGNED(n)  __align(n)
#elif defined (__IAR_SYSTEMS_ICC__) 
    /* IAR Embedded Workbench tools */
    #define PACK_STRUCT_BEGIN __packed
    #define PACK_STRUCT_STRUCT
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    #define IAR_STR(a) #a
    #define ALIGNED(n) _Pragma(IAR_STR(data_alignment= ## n ##))
#else 
    /* GCC tools (CodeSourcery) */
    #define PACK_STRUCT_BEGIN
    #define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(fld) fld
    #define ALIGNED(n)  __attribute__((aligned (n)))
#endif


typedef unsigned int   u32_t;
typedef signed   int   s32_t;
typedef unsigned short u16_t;
typedef signed   short s16_t;
typedef unsigned char  u8_t;
typedef signed   char  s8_t;
typedef u32_t mem_addr_t;

typedef enum{
	TIMER_MODE_FREE,
	TIMER_MODE_PERIOD
}TIMER_MODE_T;

typedef enum{
	TIMER_MODE_32BIT = 0,
	TIMER_MODE_64BIT,
	TIMER_MODE_MAXA
}TIMER_WID_MODE_T;

static inline u32_t sys_read32(mem_addr_t addr)
{
	return *(volatile u32_t *)addr;
}

static inline void sys_write32(u32_t data, mem_addr_t addr)
{
	*(volatile u32_t *)addr = data;
}

#ifdef __cplusplus
}
#endif

#endif
