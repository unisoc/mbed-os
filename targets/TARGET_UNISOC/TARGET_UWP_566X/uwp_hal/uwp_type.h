/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __UWP_TYPE_H
#define __UWP_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#define __ramfunc static inline;
#define TRUE  (1)
#define FALSE (0)
#define CLR_BIT(reg, bit) ((reg) &= ~(1<<(bit)))
#define SET_BIT(reg, bit) ((reg) |=  (1<<(bit)))


typedef unsigned int   u32_t;
typedef unsigned short u16_t;
typedef unsigned char  u8_t;

typedef enum{
	TIMER_MODE_FREE,
	TIMER_MODE_PERIOD,
}TIMER_MODE_T;

typedef enum{
	TIMER_MODE_32BIT = 0,
	TIMER_MODE_64BIT,
	TIMER_MODE_MAXA
}TIMER_WID_MODE_T;


#ifdef __cplusplus
}
#endif

#endif
