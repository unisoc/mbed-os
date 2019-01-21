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
#include "cmsis.h"
#include "hal_sfc.h"
#include "uwp_intc.h"
#include "sipc.h"
#include "ipi.h"

/* This function is called after RAM initialization and before main. */
extern int uwp_flash_init(struct spi_flash_struct *dev);
void mbed_sdk_init(void){
    struct spi_flash_struct flash_test;
    uwp_cache_init();
    uwp_flash_init(&flash_test);
    intc_uwp_init();
    aon_intc_uwp_init();
}

void mbed_main(void){
    sipc_init();
    ipi_uwp_init();
}

