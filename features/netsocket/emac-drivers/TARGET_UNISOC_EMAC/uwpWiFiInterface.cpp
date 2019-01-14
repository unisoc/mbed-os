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

#include "WIFIInterface.h"
#include "UWPWIFIInterface.h"
#include "nsapi_types.h"
#include "uwp_emac.h"
#include "uwp_wifi_cmdevt.h"


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
	printf("%s\r\n",__func__);
    uwp_mgmt_connect(ssid,pass,channel);
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

#if 0
    rda_msg msg;
	bool find = false;
	int i = 0;
	rda5981_scan_result bss;
	int ret = 0;

	if (ssid == NULL || ssid[0] == 0) {
		return NSAPI_ERROR_PARAMETER;
	}

    init();

	if(rda5981_check_scan_result_name(ssid) != 0) {
		for (i = 0; i< 5; i++) {
			rda5981_scan(NULL, 0, 0);
			if(rda5981_check_scan_result_name(ssid) == 0) {
				find = true;
				break;
			}
		}
	} else
		find = true;

	if (find == false) {
		printf("can not find the ap.\r\n");
		return NSAPI_ERROR_CONNECTION_TIMEOUT;
	}
	bss.channel = 15;
	rda5981_get_scan_result_name(&bss, ssid);
	if ((channel !=0) && (bss.channel != channel)) {
		printf("invalid channel bss.channel:%d set_channel:%d\n",
			bss.channel,channel);
		return NSAPI_ERROR_CONNECTION_TIMEOUT;
	}

    memcpy(gssid, ssid, strlen(ssid));
	if(pass[0] != 0)
        memcpy(gpass, pass, strlen(pass));
    memset(gbssid, 0, NSAPI_MAC_BYTES);
    gssid[strlen(ssid)] = gpass[strlen(pass)] = '\0';

    msg.type = WLAND_CONNECT;
    rda_mail_put(wland_msgQ, (void*)&msg, osWaitForever);
    ret = _interface->bringup(_dhcp,
            _ip_address[0] ? _ip_address : 0,
            _netmask[0] ? _netmask : 0,
            _gateway[0] ? _gateway : 0,
            DEFAULT_STACK,
            _blocking);

	if (ret == NSAPI_ERROR_DHCP_FAILURE)
		ret = NSAPI_ERROR_CONNECTION_TIMEOUT;
	return ret;
#endif
    return NSAPI_ERROR_OK;
}


nsapi_error_t UWPWiFiInterface::connect()
{
#if 0
    rda_msg msg;
	bool find = false;
	int i = 0;
	int ret = 0;
    init();

	if (_ssid[0] == 0)
		return NSAPI_ERROR_PARAMETER;

	if(rda5981_check_scan_result_name(_ssid) != 0) {
		for (i = 0; i< 5; i++) {
			rda5981_scan(NULL, 0, 0);
			if(rda5981_check_scan_result_name(_ssid) == 0) {
				find = true;
				break;
			}
		}
	} else
	    find = true;
	if (find == false) {
		printf("can not find the ap.\r\n");
		return NSAPI_ERROR_TIMEOUT;
	}

    memcpy(gssid, _ssid, strlen(_ssid));
	if (_pass[0]!= 0)
        memcpy(gpass, _pass, strlen(_pass));

    memset(gbssid, 0, NSAPI_MAC_BYTES);
    gssid[strlen(_ssid)] = gpass[strlen(_pass)] = '\0';

    msg.type = WLAND_CONNECT;
    rda_mail_put(wland_msgQ, (void*)&msg, osWaitForever);
    ret = _interface->bringup(_dhcp,
            _ip_address[0] ? _ip_address : 0,
            _netmask[0] ? _netmask : 0,
            _gateway[0] ? _gateway : 0,
            DEFAULT_STACK,
            _blocking);
	if (ret == NSAPI_ERROR_DHCP_FAILURE)
		ret = NSAPI_ERROR_CONNECTION_TIMEOUT;
	return ret;
#endif
    printf("%s\r\n",__func__);
    return NSAPI_ERROR_OK;
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
    printf("%s\r\n",__func__);
    init();
    ret = uwp_mgmt_scan(0, 0);
	printf("ap cnt:%d\r\n",ret);
    return 0;
#if 0
	int bss_num = 0, i;
    rda5981_scan_result *bss;
	nsapi_wifi_ap_t ap;

    init();

    rda5981_scan(NULL, 0, 0);
	bss_num = rda5981_get_scan_num();
	if (count != 0)
	    bss_num = (bss_num < count) ? bss_num : count;
	if (res) {
		bss = (rda5981_scan_result *)malloc(bss_num * sizeof(rda5981_scan_result));
	    rda5981_get_scan_result(bss, bss_num);
		for(i=0; i<bss_num; i++){
			memset(&ap, 0, sizeof(nsapi_wifi_ap_t));
		    memcpy(ap.bssid, bss[i].BSSID, 6);
		    memcpy(ap.ssid, bss[i].SSID, bss[i].SSID_len);
		    ap.channel = bss[i].channel;
		    ap.rssi = bss[i].RSSI;
		    if(bss[i].secure_type == ENCRYPT_NONE){
		        ap.security = NSAPI_SECURITY_NONE;
		    }else if(bss[i].secure_type & ENCRYPT_WEP){
		        ap.security = NSAPI_SECURITY_WEP;
		    }else if((bss[i].secure_type & (ENCRYPT_WPA_TKIP | ENCRYPT_WPA_CCMP)) && \
		                (bss[i].secure_type & (ENCRYPT_WPA2_TKIP | ENCRYPT_WPA2_CCMP))){
		        ap.security = NSAPI_SECURITY_WPA_WPA2;
		    }else if((bss[i].secure_type & (ENCRYPT_WPA_TKIP | ENCRYPT_WPA_CCMP))){
		        ap.security = NSAPI_SECURITY_WPA;
		    }else{
		        ap.security = NSAPI_SECURITY_WPA2;
		    }
			WiFiAccessPoint ap_temp(ap);
			memcpy(&res[i], &ap_temp, sizeof(WiFiAccessPoint));
		}
		free(bss);
	}
    return bss_num;
 #endif
}

WiFiInterface *WiFiInterface::get_default_instance() {
	static UWPWiFiInterface wifinet;
	return &wifinet;
}

