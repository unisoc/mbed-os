/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MBED_UWP_WIFI_MAIN_H
#define MBED_UWP_WIFI_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "hal_type.h"
#include "uwp_def.h"
#include "uwp_wifi_drv.h"

//#define WIFI_MODE_NONE (0)
//#define WIFI_MODE_STA (1)
typedef enum{
    WIFI_MODE_NONE = 0,
    WIFI_MODE_STA,
	WIFI_MODE_AP
}UWP_WIFI_MODE_T;

typedef enum{
    STA_SCAN_TYPE = 0,
}UWP_MGMT_MODE_T;

#define WIFI_MODE_AP (2)
/* #define WIFI_MODE_APSTA (3) */
/* #define WIFI_MODE_MONITOR (4) */

#define MAX_WIFI_DEV_NUM (2)

#define WIFI_DEV_STA (0)
#define WIFI_DEV_AP (1)

#define ETH_ALEN (6)
#define IPV4_LEN (4)

// TODO: need porting
struct wifi_device {
	/* bool connecting; */
	bool connected;
	bool opened;
	u8_t mode;
	u8_t mac[ETH_ALEN];
	u8_t ipv4_addr[IPV4_LEN];
    u8_t max_sta_num;
    u8_t max_blacklist_num;
	scan_result_cb_t scan_result_cb;
	connect_cb_t connect_cb;
	disconnect_cb_t disconnect_cb;
	//new_station_t new_station_cb;
	void *iface;
};

struct wifi_priv {
	struct wifi_device wifi_dev[MAX_WIFI_DEV_NUM];
	u32_t cp_version;
	bool initialized;
};

static inline void uwp_save_addr_before_payload(u32_t payload, void *addr)
{
	u32_t *pkt_ptr;

	pkt_ptr = (u32_t *)(payload - 4);
	*pkt_ptr = (u32_t)addr;
}

static inline u32_t uwp_get_addr_from_payload(u32_t payload)
{
	u32_t *ptr;

	ptr = (u32_t *)(payload - 4);

	return *ptr;
}
/* extern struct adapter wifi_pAd; */


//struct wifi_device *get_wifi_dev_by_dev(struct device *dev);
/* int wifi_ifnet_sta_init(struct adapter *pAd); */
/* int wifi_ifnet_ap_init(struct adapter *pAd); */
/* struct netif *wifi_ifnet_get_interface(struct adapter *pAd,int ctx_id); */

int uwp_cp_init(void);
int uwp_mgmt_open(void);
void uwp_mgmt_empty_scan_result_list();
int uwp_mgmt_scan(uint8_t band, uint8_t channel, const char* ssid);
bool uwp_mgmt_scan_result_name(const char *name);
int uwp_mgmt_get_scan_result(void *buf, int num);
int uwp_mgmt_connect(const char *ssid, const char *password, uint8_t channel);
int uwp_mgmt_tx(uint8_t *pkt, uint32_t pkt_len);
int uwp_mgmt_getmac(uint8_t * addr);
int uwp_mgmt_disconnect(void);

#ifdef __cplusplus
}
#endif

#endif /* UWP_WIFI_MAIN_H */
