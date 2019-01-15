/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include "mbed_retarget.h"
#include "uwp_wifi_main.h"
#include "uwp_wifi_rf.h"
#include "uwp_wifi_cmdevt.h"
#include "uwp_sys_wrapper.h"
#include "uwp_wifi_drv.h"
#include "uwp_wifi_txrx.h"

//#define WIFI_LOG_DBG
//#define WIFI_DUMP
#include "uwp_log.h"

/*
 * We do not need <socket/include/socket.h>
 * It seems there is a bug in ASF side: if OS is already defining sockaddr
 * and all, ASF will not need to define it. Unfortunately its socket.h does
 * but also defines some NM API functions there (??), so we need to redefine
 * those here.
 */
#define __SOCKET_H__
#define HOSTNAME_MAX_SIZE (64)

#define MTU (1500)

#define SEC1 (1)
#define SEC2 (2)

#define DEV_DATA(dev) \
	((struct wifi_priv *)(dev)->driver_data)

void *g_wifi_mgmt_queue = NULL;
struct list_head g_scan_list;

static struct wifi_priv uwp_wifi_priv_data;
static struct wifi_priv uwp_wifi_dev;
static bool cp_initialized = false;

static int wifi_rf_init(void)
{
	int ret = 0;

	LOG_DBG("download the first section of config file");
	ret = wifi_cmd_load_ini(sec1_table, sizeof(sec1_table), SEC1);
	if (ret) {
		LOG_ERR("download first section ini fail,ret = %d", ret);
		return ret;
	}

	LOG_DBG("download the second section of config file");
	ret = wifi_cmd_load_ini(sec2_table, sizeof(sec2_table), SEC2);
	if (ret) {
		LOG_ERR("download second section ini fail,ret = %d", ret);
		return ret;
	}

	LOG_DBG("Load wifi ini success.");

	return 0;
}
#if 0
static int uwp_mgmt_open(struct device *dev)
{
	int ret;
	struct wifi_priv *priv;
	struct wifi_device *wifi_dev;

	if (!dev) {
		return -EINVAL;
	}

	wifi_dev = get_wifi_dev_by_dev(dev);
	if (!wifi_dev) {
		LOG_ERR("Unable to find wifi dev by dev %p", dev);
		return -EINVAL;
	}

	priv = DEV_DATA(dev);

	if (wifi_dev->opened) {
		return -EAGAIN;
	}

	ret = wifi_cmd_open(wifi_dev);
	if (ret) {
		return ret;
	}

	/* Open mode at first time */
	if (!priv->wifi_dev[WIFI_DEV_STA].opened
			&& !priv->wifi_dev[WIFI_DEV_AP].opened) {
		wifi_tx_empty_buf(TOTAL_RX_ADDR_NUM);
	}

	wifi_dev->opened = true;

	return 0;
}

static int uwp_mgmt_close(struct device *dev)
{
	int ret;
	struct wifi_priv *priv;
	struct wifi_device *wifi_dev;

	if (!dev) {
		return -EINVAL;
	}

	wifi_dev = get_wifi_dev_by_dev(dev);
	if (!wifi_dev) {
		LOG_ERR("Unable to find wifi dev by dev %p", dev);
		return -EINVAL;
	}

	priv = DEV_DATA(dev);

	if (!wifi_dev->opened) {
		return -EAGAIN;
	}

	ret = wifi_cmd_close(wifi_dev);
	if (ret) {
		return ret;
	}

	wifi_dev->opened = false;

	/* Both are closed */
	if (!priv->wifi_dev[WIFI_DEV_STA].opened
			&& !priv->wifi_dev[WIFI_DEV_AP].opened) {
		wifi_release_rx_buf();
	}

	/* Flush all callbacks */
	if (wifi_dev->new_station_cb) {
		wifi_dev->new_station_cb = NULL;
	}

	if (wifi_dev->scan_result_cb) {
		wifi_dev->scan_result_cb = NULL;
	}

	if (wifi_dev->connect_cb) {
		wifi_dev->connect_cb = NULL;
	}

	if (wifi_dev->disconnect_cb) {
		wifi_dev->disconnect_cb = NULL;
	}

	return 0;
}

static int uwp_mgmt_start_ap(struct device *dev,
			     struct wifi_drv_start_ap_params *params,
				 new_station_t cb)
{
	struct wifi_device *wifi_dev;

	if (!dev || !params) {
		return -EINVAL;
	}

	wifi_dev = get_wifi_dev_by_dev(dev);
	if (!wifi_dev) {
		LOG_ERR("Unable to find wifi dev by dev %p", dev);
		return -EINVAL;
	}

	if (wifi_dev->mode != WIFI_MODE_AP) {
		LOG_WRN("Improper mode %d to start_ap.",
				wifi_dev->mode);
		return -EINVAL;
	}

	if (wifi_dev->new_station_cb) {
		return -EAGAIN;
	}

	wifi_dev->new_station_cb = cb;

	return wifi_cmd_start_ap(wifi_dev, params);
}

static int uwp_mgmt_stop_ap(struct device *dev)
{
	struct wifi_device *wifi_dev;

	if (!dev) {
		return -EINVAL;
	}

	wifi_dev = get_wifi_dev_by_dev(dev);
	if (!wifi_dev) {
		LOG_ERR("Unable to find wifi dev by dev %p", dev);
		return -EINVAL;
	}

	if (wifi_dev->mode != WIFI_MODE_AP) {
		LOG_WRN("Improper mode %d to stop_ap.",
				wifi_dev->mode);
		return -EINVAL;
	}

	if (wifi_dev->new_station_cb) {
		wifi_dev->new_station_cb = NULL;
	}

	return wifi_cmd_stop_ap(wifi_dev);
}

static int uwp_mgmt_del_station(struct device *dev,
				u8_t *mac)
{
	return 0;
}

static int uwp_mgmt_scan(struct device *dev,
		struct wifi_drv_scan_params *params,
		scan_result_cb_t cb)
{
	struct wifi_device *wifi_dev;

	if (!dev || !params) {
		return -EINVAL;
	}

	wifi_dev = get_wifi_dev_by_dev(dev);
	if (!wifi_dev) {
		LOG_ERR("Unable to find wifi dev by dev %p", dev);
		return -EINVAL;
	}

	if (wifi_dev->mode != WIFI_MODE_STA) {
		LOG_WRN("Improper mode %d to scan.",
				wifi_dev->mode);
		return -EINVAL;
	}

	if (wifi_dev->scan_result_cb) {
		return -EAGAIN;
	}

	wifi_dev->scan_result_cb = cb;

	return wifi_cmd_scan(wifi_dev, params);

}

static int uwp_mgmt_get_station(struct device *dev,
		/* struct wifi_get_sta_req_params *params */
		u8_t *signal)
{
	return 0;
}

static int uwp_mgmt_connect(struct device *dev,
			    struct wifi_drv_connect_params *params,
				connect_cb_t con_cb,
				disconnect_cb_t discon_cb)
{
	struct wifi_device *wifi_dev;

	if (!dev || !params) {
		return -EINVAL;
	}

	wifi_dev = get_wifi_dev_by_dev(dev);
	if (!wifi_dev) {
		LOG_ERR("Unable to find wifi dev by dev %p", dev);
		return -EINVAL;
	}

	if (wifi_dev->mode != WIFI_MODE_STA) {
		LOG_WRN("Improper mode %d to connect.",
				wifi_dev->mode);
		return -EINVAL;
	}

	if (wifi_dev->connected) {
		LOG_WRN("Connect again in connected.");
	}

	wifi_dev->connect_cb = con_cb;
	wifi_dev->disconnect_cb = discon_cb;

	return wifi_cmd_connect(wifi_dev, params);
}

static int uwp_mgmt_disconnect(struct device *dev,
		disconnect_cb_t cb)
{
	struct wifi_device *wifi_dev;

	if (!dev) {
		return -EINVAL;
	}

	wifi_dev = get_wifi_dev_by_dev(dev);
	if (!wifi_dev) {
		LOG_ERR("Unable to find wifi dev by dev %p", dev);
		return -EINVAL;
	}

	if (wifi_dev->mode != WIFI_MODE_STA) {
		LOG_WRN("Improper mode %d to disconnect.",
				wifi_dev->mode);
		return -EINVAL;
	}

	if (!wifi_dev->connected) {
		LOG_WRN("Disconnect again in disconnected.");
	}

	wifi_dev->disconnect_cb = cb;

	return wifi_cmd_disconnect(wifi_dev);
}

static int uwp_mgmt_set_ip(struct device *dev, u8_t *ip, u8_t len)
{
	struct wifi_device *wifi_dev;

	if (!dev || !ip) {
		return -EINVAL;
	}

	wifi_dev = get_wifi_dev_by_dev(dev);
	if (!wifi_dev) {
		LOG_ERR("Unable to find wifi dev by dev %p", dev);
		return -EINVAL;
	}

	if (wifi_dev->mode != WIFI_MODE_STA) {
		LOG_WRN("Improper mode %d to connect.",
				wifi_dev->mode);
		return -EINVAL;
	}

	return wifi_cmd_set_ip(wifi_dev, ip, len);
}

static void uwp_iface_init(struct net_if *iface)
{
	struct wifi_device *wifi_dev;
	struct device *dev;

	if (!iface) {
		return;
	}

	dev = iface->if_dev->dev;
	wifi_dev = get_wifi_dev_by_dev(dev);
	if (!wifi_dev) {
		LOG_ERR("Unable to find wifi dev by dev %p", dev);
		return;
	}

	/* Net stack has not been fit for softap as of now. */
	if (wifi_dev->mode == WIFI_MODE_STA) {
		ethernet_init(iface);
	}

	net_if_set_link_addr(iface, wifi_dev->mac,
			sizeof(wifi_dev->mac),
			NET_LINK_ETHERNET);

	/* Store iface in wifi device */
	wifi_dev->iface = iface;

}

static int wifi_tx_fill_msdu_dscr(struct wifi_device *wifi_dev,
			   struct net_pkt *pkt, u8_t type, u8_t offset)
{
	u32_t addr = 0;
	u32_t reserve_len = net_pkt_ll_reserve(pkt);
	struct tx_msdu_dscr *dscr = NULL;

	net_pkt_set_ll_reserve(pkt,
			sizeof(struct tx_msdu_dscr) +
			net_pkt_ll_reserve(pkt));
	LOG_DBG("size msdu: %d", sizeof(struct tx_msdu_dscr));

	dscr = (struct tx_msdu_dscr *)net_pkt_ll(pkt);
	memset(dscr, 0x00, sizeof(struct tx_msdu_dscr));
	addr = (u32_t)dscr;
	SPRD_AP_TO_CP_ADDR(addr);
	dscr->next_buf_addr_low = addr;
	dscr->next_buf_addr_high = 0x0;

	dscr->tx_ctrl.checksum_offload = 0;
	dscr->common.type =
		(type == SPRDWL_TYPE_CMD ? SPRDWL_TYPE_CMD : SPRDWL_TYPE_DATA);
	dscr->common.direction_ind = TRANS_FOR_TX_PATH;
	dscr->common.buffer_type = 0;

	if (wifi_dev->mode == WIFI_MODE_STA) {
		dscr->common.interface = WIFI_DEV_STA;
	} else if (wifi_dev->mode == WIFI_MODE_AP) {
		dscr->common.interface = WIFI_DEV_AP;
	}

	dscr->pkt_len = reserve_len + net_pkt_get_len(pkt);
	dscr->offset = 11;
	/* TODO */
	dscr->tx_ctrl.sw_rate = (type == SPRDWL_TYPE_DATA_SPECIAL ? 1 : 0);
	dscr->tx_ctrl.wds = 0;
	/*TBD*/ dscr->tx_ctrl.swq_flag = 0;
	/*TBD*/ dscr->tx_ctrl.rsvd = 0;
	/*TBD*/ dscr->tx_ctrl.pcie_mh_readcomp = 1;
	dscr->buffer_info.msdu_tid = 0;
	dscr->buffer_info.mac_data_offset = 0;
	dscr->buffer_info.sta_lut_idx = 0;
	dscr->buffer_info.encrypt_bypass = 0;
	dscr->buffer_info.ap_buf_flag = 1;
	dscr->tx_ctrl.checksum_offload = 0;
	dscr->tx_ctrl.checksum_type = 0;
	dscr->tcp_udp_header_offset = 0;

	return 0;
}

static int uwp_iface_tx(struct net_if *iface, struct net_pkt *pkt)
{
	struct device *dev;
	struct net_buf *frag;
	struct wifi_device *wifi_dev;
	bool first_frag = true;
	u8_t *data_ptr;
	u16_t data_len;
	u16_t total_len;
	u32_t addr;
	u16_t max_len;

	if (!iface) {
		return -EINVAL;
	}

	dev = iface->if_dev->dev;
	wifi_dev = get_wifi_dev_by_dev(dev);
	if (!wifi_dev) {
		LOG_ERR("Unable to find wifi dev by dev %p", dev);
		net_pkt_unref(pkt);
		return -EINVAL;
	}

	if (!wifi_dev->opened
			&& wifi_dev->mode == WIFI_MODE_AP) {
		LOG_WRN("AP mode %d not opened.", wifi_dev->mode);
		net_pkt_unref(pkt);
		return -EWOULDBLOCK;
	} else if (!wifi_dev->connected
			&& wifi_dev->mode == WIFI_MODE_STA) {
		LOG_WRN("STA mode %d not connected.", wifi_dev->mode);
		net_pkt_unref(pkt);
		return -EWOULDBLOCK;
	}

	wifi_tx_fill_msdu_dscr(wifi_dev, pkt, SPRDWL_TYPE_DATA, 0);

	total_len = net_pkt_ll_reserve(pkt) + net_pkt_get_len(pkt);

	LOG_DBG("wifi tx data: %d bytes, reserve: %d bytes",
		    total_len, net_pkt_ll_reserve(pkt));

	for (frag = pkt->frags; frag; frag = frag->frags) {
		if (first_frag) {
			data_ptr = net_pkt_ll(pkt);
			data_len = net_pkt_ll_reserve(pkt) + frag->len;
			first_frag = false;
		} else {
			data_ptr = frag->data;
			data_len = frag->len;

		}

		addr = (u32_t)data_ptr;
		/* FIXME Save pkt addr before payload. */
		uwp_save_addr_before_payload(addr, (void *)pkt);

		SPRD_AP_TO_CP_ADDR(addr);

		max_len = MTU + sizeof(struct tx_msdu_dscr)
			+ 14 /* link layer header length */;
		if (data_len > max_len) {
			LOG_ERR("Exceed max length %d data_len %d",
					max_len, data_len);
			return -EINVAL;
		}

		wifi_tx_data((void *)addr, data_len);
	}

	return 0;
}


static const struct wifi_drv_api uwp_api = {
	.eth_api.iface_api.init		= uwp_iface_init,
	.eth_api.iface_api.send		= uwp_iface_tx,
	.open						= uwp_mgmt_open,
	.close						= uwp_mgmt_close,
	.scan						= uwp_mgmt_scan,
	.get_station				= uwp_mgmt_get_station,
	.connect					= uwp_mgmt_connect,
	.disconnect					= uwp_mgmt_disconnect,
	.notify_ip					= uwp_mgmt_set_ip,
	.start_ap					= uwp_mgmt_start_ap,
	.stop_ap					= uwp_mgmt_stop_ap,
	.del_station				= uwp_mgmt_del_station,
};
#endif
int uwp_init(struct wifi_priv *wifi_priv, UWP_WIFI_MODE_T wifi_mode)
{
	int ret;
	struct wifi_priv *priv = wifi_priv;
	struct wifi_device *wifi_dev = NULL;

	if (!wifi_priv) {
		return -EINVAL;
	}

	if (wifi_mode == WIFI_MODE_STA) {
		wifi_dev = &(priv->wifi_dev[WIFI_DEV_STA]);
		wifi_dev->mode = WIFI_MODE_STA;
	} else if (wifi_mode == WIFI_MODE_AP) {
		wifi_dev = &(priv->wifi_dev[WIFI_DEV_AP]);
		wifi_dev->mode = WIFI_MODE_AP;
	} else {
		LOG_ERR("Unknown WIFI DEV MODE");
	}
	
	if (!priv->initialized) {
		ret = uwp_mcu_init();
		if (ret) {
			LOG_ERR("Firmware download failed %i.",
					ret);
			return ret;
		}

        /* ensure smsg thread excute  */
        osDelay(100);

		wifi_cmdevt_init();
		wifi_txrx_init(priv);
		wifi_irq_init();

		k_sleep(400); /* FIXME: workaround */
		ret = wifi_rf_init();
		if (ret) {
			LOG_ERR("wifi rf init failed.");
			return ret;
		}
	
		ret = wifi_cmd_get_cp_info(priv);
		if (ret) {
			LOG_ERR("Get cp info failed.");
			return ret;
		}

		priv->initialized = true;

	}
	LOG_DBG("UWP WIFI driver Initialized");

	return 0;
}
#if 0
NET_DEVICE_INIT(uwp_ap, CONFIG_WIFI_AP_DRV_NAME,
		uwp_init, &uwp_wifi_priv_data, NULL,
		CONFIG_WIFI_INIT_PRIORITY,
		&uwp_api,
		ETHERNET_L2, NET_L2_GET_CTX_TYPE(ETHERNET_L2),
		MTU);

NET_DEVICE_INIT(uwp_sta, CONFIG_WIFI_STA_DRV_NAME,
		uwp_init, &uwp_wifi_priv_data, NULL,
		CONFIG_WIFI_INIT_PRIORITY,
		&uwp_api,
		ETHERNET_L2, NET_L2_GET_CTX_TYPE(ETHERNET_L2),
		MTU);
#endif

static int wifi_tx_fill_msdu_dscr(struct wifi_device *wifi_dev,
                void *pkt, void *pkt_len, u8_t type, u8_t offset)
{
    u32_t addr = 0;
    struct tx_msdu_dscr *dscr = NULL;
    dscr = (struct tx_msdu_dscr *)pkt;

    memset(dscr, 0x00, sizeof(struct tx_msdu_dscr));
    addr = (u32_t)dscr;
    SPRD_AP_TO_CP_ADDR(addr);
    dscr->next_buf_addr_low = addr;
    dscr->next_buf_addr_high = 0x0;

    dscr->tx_ctrl.checksum_offload = 0;
    dscr->common.type =
        (type == SPRDWL_TYPE_CMD ? SPRDWL_TYPE_CMD : SPRDWL_TYPE_DATA);
    dscr->common.direction_ind = TRANS_FOR_TX_PATH;
    dscr->common.buffer_type = 0;

    if (wifi_dev->mode == WIFI_MODE_STA) {
        dscr->common.interface = WIFI_DEV_STA;
    } else if (wifi_dev->mode == WIFI_MODE_AP) {
        dscr->common.interface = WIFI_DEV_AP;
    }

    dscr->pkt_len = pkt_len;
    dscr->offset = 11;
    /* TODO */
    dscr->tx_ctrl.sw_rate = (type == SPRDWL_TYPE_DATA_SPECIAL ? 1 : 0);
    dscr->tx_ctrl.wds = 0;
    /*TBD*/ dscr->tx_ctrl.swq_flag = 0;
    /*TBD*/ dscr->tx_ctrl.rsvd = 0;
    /*TBD*/ dscr->tx_ctrl.pcie_mh_readcomp = 1;
    dscr->buffer_info.msdu_tid = 0;
    dscr->buffer_info.mac_data_offset = 0;
    dscr->buffer_info.sta_lut_idx = 0;
    dscr->buffer_info.encrypt_bypass = 0;
    dscr->buffer_info.ap_buf_flag = 1;
    dscr->tx_ctrl.checksum_offload = 0;
    dscr->tx_ctrl.checksum_type = 0;
    dscr->tcp_udp_header_offset = 0;

    //DUMP_DATA(dscr,sizeof(struct tx_msdu_dscr));
    //DUMP_DATA(dscr+sizeof(struct tx_msdu_dscr), 14);
    return 0;
}

/* for uwp_cp_init uwp_mgmt_open 
 * if repeated operate
 * return correct value
 */
int uwp_cp_init(void){ 
    int ret = -1;
    if(cp_initialized){
        LOG_DBG("cp is already runing");
        return 0;
    }
    sipc_init();
    ipi_uwp_init();
    ret = uwp_init(&uwp_wifi_dev, WIFI_MODE_STA);
    if(ret == 0)
        cp_initialized = true;
    return ret;
}

extern int impl_empty_buf(int num);
int uwp_mgmt_open(void){
    int ret = -1;
    int i;
    if(uwp_wifi_dev.wifi_dev[0].opened == true){
        LOG_DBG("WIFI STA OPENED");
        return 0;
    }

    g_wifi_mgmt_queue = k_msg_create(10);
    if(g_wifi_mgmt_queue == NULL){
        LOG_ERR("malloc failed");
        return -ENOMEM;
    }

    __INIT_LIST_HEAD(&g_scan_list);

    ret = wifi_cmd_open(&(uwp_wifi_dev.wifi_dev[0]));
    if(ret == 0){
        for(i = 0; i < 10; i ++){
            ret = impl_empty_buf(1);
            if(ret != 0)
                break;
        }
    }
    uwp_wifi_dev.wifi_dev[0].opened = true;
    return ret;
}

int uwp_mgmt_scan(uint8_t band, uint8_t channel)
{
    int ret = -1;
    struct wifi_drv_scan_params para;
    para.band = band;
    para.channel = channel;
    uwp_wifi_msg_t msg = NULL;

    g_wifi_mgmt_queue = k_msg_create(10);
    if(g_wifi_mgmt_queue == NULL){
        LOG_ERR("malloc failed");
        return -ENOMEM;
    }

    ret = wifi_cmd_scan(&(uwp_wifi_dev.wifi_dev[0]), &para);
    if(ret != 0)
        return ret;
    k_msg_get(g_wifi_mgmt_queue, &msg, 10000);
    if(msg->type == STA_SCAN_TYPE)
        ret = msg->arg1;
    LOG_DBG("find ap:%d",ret);
    free(msg);
    return ret;
}

int uwp_mgmt_get_scan_result(void *buf, int num){
    int cnt = 0;
    struct list_head *p_node = NULL, *p_del = NULL;
    struct list_head *p_head = &g_scan_list;
    struct event_scan_result *data = (struct event_scan_result*)buf;

    p_node = p_head->next;
    while((p_node != p_head) && (cnt <= num)){
        memcpy(&data[cnt], (void *)(((uint32_t)p_node) - sizeof(struct event_scan_result)),
                        sizeof(struct event_scan_result));
        p_node = p_node->next;
        cnt ++;
    }

    p_node = p_head->next;
    while(p_node != p_head){
        p_node->next->prev = p_node->prev;
        p_node->prev->next = p_node->next;
        p_del = p_node;
        p_node = p_node->next;
        LOG_DBG("free scan:%x",(((uint32_t)p_del) - sizeof(struct event_scan_result)));
        free((void *)(((uint32_t)p_del) - sizeof(struct event_scan_result)));
    }

    return cnt;
}
int uwp_mgmt_connect(const char *ssid, const char *password, uint8_t channel)
{
    struct wifi_drv_connect_params para;
    memset(&para, 0, sizeof(para));
    para.channel = channel;
    para.ssid_length = strlen(ssid);
    para.psk_length = strlen(password);
    para.ssid = ssid;
    para.psk = password;
    LOG_DBG("ssid:%s psk:%s",para.ssid,para.psk);
    if (uwp_wifi_dev.wifi_dev[0].connected) {
        LOG_WRN("Connect again in connected.");
    }

    uwp_netif_cb_register(&(uwp_wifi_dev.wifi_dev[0]));

    return wifi_cmd_connect(&(uwp_wifi_dev.wifi_dev[0]), &para);
}

int uwp_mgmt_tx(uint8_t *pkt, uint32_t pkt_len)
{
    u8_t *data_ptr;
    u16_t data_len;
    u16_t total_len;
    u32_t addr;
    u16_t max_len;
    u8_t *debug_buf;
    int ret;
 
    wifi_tx_fill_msdu_dscr(&(uwp_wifi_dev.wifi_dev[0]), pkt, pkt_len, SPRDWL_TYPE_DATA, 0);

    data_ptr = (u32_t)pkt;
    /* FIXME Save pkt addr before payload. */
    //uwp_save_addr_before_payload(addr, (void *)pkt);

    SPRD_AP_TO_CP_ADDR(data_ptr);

    max_len = MTU + sizeof(struct tx_msdu_dscr) + 14 /* link layer header length */;

    if (pkt_len > max_len) {
        LOG_ERR("Exceed max length %d data_len %d", max_len, pkt_len);
            return -EINVAL;
    }

    debug_buf = (u8_t *)(data_ptr + sizeof(struct tx_msdu_dscr));
    LOG_DBG("DATA OUT:%02x:%02x:%02x:%02x:%02x:%02x <-- %02x:%02x:%02x:%02x:%02x:%02x addr:%p len:%d", 
            debug_buf[0],debug_buf[1],debug_buf[2],debug_buf[3],debug_buf[4],debug_buf[5],
                debug_buf[6],debug_buf[7],debug_buf[8],debug_buf[9],debug_buf[10],debug_buf[11],
                    pkt,pkt_len);

    wifi_tx_data((void *)data_ptr, pkt_len);

    return 0;
}

int uwp_mgmt_getmac(uint8_t *addr){
    memcpy(addr,uwp_wifi_dev.wifi_dev[0].mac,6);
    DUMP_DATA(addr,6);
    return 0;
}

int uwp_mgmt_disconnect(void)
{
    struct wifi_device *wifi_dev = &(uwp_wifi_dev.wifi_dev[0]);

    if(!wifi_dev->connected) {
        LOG_WRN("Disconnect again in disconnected.");
    }

    return wifi_cmd_disconnect(wifi_dev);
}

