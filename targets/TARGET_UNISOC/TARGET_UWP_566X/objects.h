/* mbed Microcontroller Library
 * Copyright (c) 2006-2018 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MBED_OBJECTS_H
#define MBED_OBJECTS_H

#include "PinNames.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
    uint32_t port_base;
    PinName pin;
} gpio_t;

struct gpio_irq_s {
    PinName  pin;
    uint32_t    port_base;
};

typedef enum{
    TIMER_MODE_FREE,
    TIMER_MODE_PERIOD
}TIMER_MODE_T;

typedef enum{
    TIMER_MODE_32BIT = 0,
    TIMER_MODE_64BIT,
    TIMER_MODE_MAXA
}TIMER_WID_MODE_T;

struct my_timer_s{
    uint32_t base;
    TIMER_MODE_T mode;
    TIMER_WID_MODE_T wid_mode;
    uint32_t clk_freq;
    uint32_t load_count;
    uint32_t load_count_h;
    uint32_t irq_op;
};

#ifdef __cplusplus
}
#endif

#endif
