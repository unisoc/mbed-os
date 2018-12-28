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
#include "uwp_wifi_main.h"
extern void sleep_cal_lpo(unsigned int calms);
extern struct wifi_priv uwp_wifi;


/* This function is called after RAM initialization and before main. */
void mbed_sdk_init(void){
	struct spi_flash_struct flash_test;
	uwp_flash_init(&flash_test);
	uwp_cache_init();
}

void mbed_main(void){
	u32_t wrptr = 0x001eff0c;
	u32_t rdptr = 0x001eff08;
	sipc_init();
    ipi_uwp_init();
	//uwp_init(&uwp_wifi, WIFI_MODE_STA);
	//printf("%d %d\r\n",*(volatile u32_t*)wrptr,*(volatile u32_t*)rdptr);
}

