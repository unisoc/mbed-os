/* LWIP implementation of NetworkInterfaceAPI
 * Copyright (c) 2015 ARM Limited
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

#include "WiFiInterface.h"
#include "uwpWiFiInterface.h"
#include "nsapi_types.h"
#include "uwp_emac.h"
#include "uwp_wifi_cmdevt.h"
#include "uwp_wifi_api.h"

/* Interface implementation */
//WiFiInterface::WiFiInterface(EMAC &emac, OnboardNetworkStack &stack) :
//{
//}

//RDAWiFiInterface *RDAWiFiInterface::get_target_default_instance()
//{
//    static RDAWiFiInterface wifinet;
//    return wifinet;
//}


nsapi_error_t UWPWiFiInterface::set_channel(uint8_t channel)
{
#if 0
	int ret= 0;
    init();

	if (channel > 13)
		return NSAPI_ERROR_PARAMETER;

	if (channel == 0){
		_channel = 0;
		return NSAPI_ERROR_OK;
	}

	ret = rda5981_set_channel(channel);
	if (ret == 0) {
		_channel = channel;
		return NSAPI_ERROR_OK;
	} else
		return NSAPI_ERROR_TIMEOUT;
#endif
    printf("%s\r\n",__func__);
    return NSAPI_ERROR_OK;
}

int8_t UWPWiFiInterface::get_rssi()
{
#if 0
	return rda5981_get_rssi();
#endif
    printf("%s\r\n",__func__);
    return 0;
}

nsapi_error_t UWPWiFiInterface::init()
{
#if 1
    if (!_interface) {
        if (!_emac.power_up()) {
            printf("power up failed!\n");
        }
        int ret = uwp_mgmt_open();
		if(ret != 0)
            printf("wifi open failed\r\n");
        nsapi_error_t err = _stack.add_ethernet_interface(_emac, true, &_interface);
        if (err != NSAPI_ERROR_OK) {
            _interface = NULL;
            return err;
        }
        _interface->attach(_connection_status_cb);
    }
#endif
    return NSAPI_ERROR_OK;
}
// TODO:need confirm
nsapi_error_t UWPWiFiInterface::set_credentials(const char *ssid, const char *pass,
                nsapi_security_t security)
{
	if(ssid == 0 || strlen(ssid) == 0)
		return NSAPI_ERROR_PARAMETER;
	if(security != NSAPI_SECURITY_NONE && (pass == 0 || strlen(pass) == 0))
		return NSAPI_ERROR_PARAMETER;
	if(strlen(ssid) > 32 || strlen(pass) > 63)
		return NSAPI_ERROR_PARAMETER;
    memcpy((void*)_ssid, (void*)ssid, strlen(ssid));
	_ssid[strlen(ssid)] = '\0';
    memcpy((void*)_pass, (void*)pass, strlen(pass));
	_pass[strlen(pass)] = '\0';
    _security = security;
    return NSAPI_ERROR_OK;
}

nsapi_error_t UWPWiFiInterface::connect(const char *ssid, const char *pass,
                nsapi_security_t security, uint8_t channel)
{
    int ret;
    bool find = false;
    int retry_count = 4;
	printf("%s\r\n",__func__);

    if (ssid == NULL || ssid[0] == 0) {
        return NSAPI_ERROR_PARAMETER;
    }

    init();

    /*because cp will check whether there is a "ssid" in scan_result list, if not
    the connect cmd will return an error. */
    if (uwp_mgmt_scan_result_name(ssid) == false) {
        uwp_mgmt_scan(0, 0, NULL);
        if(uwp_mgmt_scan_result_name(ssid) == false) {
            while(retry_count-- > 0) {
                //maybe it's a hidden ssid, transfer ssid to probe it.
                uwp_mgmt_scan(0, 0, ssid);
                if(uwp_mgmt_scan_result_name(ssid) == true) {
                    find = true;
                    break;
                }
            }
        } else
            find = true;
    } else
        find = true;

    if (!find) {
        printf("no %s cached in scan result list!\r\n", ssid);
        return NSAPI_ERROR_NO_SSID;
    }

    ret = uwp_mgmt_connect(ssid,pass,channel);
    if (ret) {
        printf("uwp_mgmt_connect failed:%d\n", ret);
        return ret;
    }
    osDelay(300);

    ret = _interface->bringup(_dhcp,
            _ip_address[0] ? _ip_address : 0,
            _netmask[0] ? _netmask : 0,
            _gateway[0] ? _gateway : 0,
            DEFAULT_STACK,
            _blocking);
	if (ret == NSAPI_ERROR_DHCP_FAILURE)
		ret = NSAPI_ERROR_CONNECTION_TIMEOUT;
	return ret;
}


nsapi_error_t UWPWiFiInterface::connect()
{
    printf("%s\r\n",__func__);
    return connect(_ssid, _pass, _security, _channel);
}
// TODO: return value
nsapi_error_t UWPWiFiInterface::disconnect()
{
#if 0
    rda_msg msg;

    if(sta_state < 2)
        return NSAPI_ERROR_NO_CONNECTION;

    init();

    msg.type = WLAND_DISCONNECT;
    rda_mail_put(wland_msgQ, (void*)&msg, osWaitForever);
    if (_interface) {
        return _interface->bringdown();
    }
#endif   
    printf("%s\r\n",__func__);
    int ret = uwp_mgmt_disconnect();
    if(_interface)
        return _interface->bringdown();
}
// TODO: only test scan
nsapi_size_or_error_t UWPWiFiInterface::scan(WiFiAccessPoint *res, nsapi_size_t count)
{
    int ret;
    init();
    ret = uwp_mgmt_scan(0, 0, NULL);
    struct event_scan_result *bss = (struct event_scan_result *)malloc(ret * sizeof(struct event_scan_result));
    if(bss == NULL)
        return NSAPI_ERROR_NO_MEMORY;
    memset(bss, 0, ret * sizeof(struct event_scan_result));
    uwp_mgmt_get_scan_result(bss, ret);
    for(int i=0; i<ret; i++){
        printf("%-32s    %-10s    %-2d  %02x:%02x:%02x:%02x:%02x:%02x\r\n",
            bss[i].ssid, security2str(bss[i].encrypt_mode), bss[i].rssi,
            (bss[i].bssid)[0],(bss[i].bssid)[1],(bss[i].bssid)[2],
            (bss[i].bssid)[3],(bss[i].bssid)[4],(bss[i].bssid)[5]);
    }
    free(bss);
    return 0;
}

WiFiInterface *WiFiInterface::get_default_instance() {
	static UWPWiFiInterface wifinet;
	return &wifinet;
}

