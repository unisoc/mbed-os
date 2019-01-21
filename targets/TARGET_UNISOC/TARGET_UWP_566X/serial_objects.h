/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MBED_SERIAL_OBJECTS_H
#define MBED_SERIAL_OBJECTS_H

#include "hal_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

struct serial_s {
    u32_t base;
    u32_t sys_clk_freq;
    u32_t baud_rate;
};


#ifdef __cplusplus
}
#endif

#endif
