/*
 * Copyright (c) 2017, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "stdio.h"
#include "uwpWiFiInterface.h"

int main(void)
{
	UWPWiFiInterface wifi;
    WiFiAccessPoint res;
    printf("simple wifi test ...\r\n");
	wifi.init();
    wifi.scan(&res,1);
    wifi.connect("TP-LINK_E304", NULL, NSAPI_SECURITY_NONE,8);
    printf("ip address:%s\r\n",wifi.get_ip_address());
    osDelay(10000);
    wifi.disconnect();
    osDelay(10000);
    wifi.connect("TP-LINK_E304", NULL, NSAPI_SECURITY_NONE,8);
    printf("ip address:%s\r\n",wifi.get_ip_address());
    while(1);
    return 0;
}

