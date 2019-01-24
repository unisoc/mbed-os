#include "uwp_buf_mgmt.h"
#include "mbed_retarget.h"

//#define WIFI_LOG_DBG
#include "uwp_log.h"

static char wifi_reserve_ram[CONFIG_UWP_PKT_BUF_MAX * CONFIG_UWP_PKT_BUF_SIZE];
static struct list_head wifi_buf_list_employ;
static struct list_head wifi_buf_list_free;
static void *wifi_list_mutex = NULL;

int uwp_pkt_buf_init(void){
    int i = 0;
    struct list_head *p_head = NULL;
    uwp_pkt_buf *p = NULL;

    LOG_DBG("WIFI RESERVE BUF:%p",wifi_reserve_ram);

    wifi_list_mutex = k_mutex_create();

    if(wifi_list_mutex == NULL){
         return -1; /* mutex create failed */
	}

    int ret = k_mutex_lock(wifi_list_mutex, 0);
    if(ret != osOK)
        return -2; /* lock mutex failed */

    __INIT_LIST_HEAD(&wifi_buf_list_employ);
    __INIT_LIST_HEAD(&wifi_buf_list_free);

    p_head = &wifi_buf_list_free;
    for(i = 0; i < CONFIG_UWP_PKT_BUF_MAX; i++){
        p = (uwp_pkt_buf *) malloc(sizeof(uwp_pkt_buf));
        if(p == NULL){
            LOG_DBG("malloc failed");
            return -ENOMEM;
		}
        p->buf = wifi_reserve_ram + i * CONFIG_UWP_PKT_BUF_SIZE;

        LOG_DBG("PREPARE BUFF:%p", p->buf);
        list_add_tail(&p->list, p_head);
	}

    k_mutex_unlock(wifi_list_mutex);
    return i;
}

void *uwp_pkt_buf_get(void){
    struct list_head *p_head_free = &wifi_buf_list_free;
    struct list_head *p_head_employ = &wifi_buf_list_employ;
    struct list_head *p_node = NULL;
    int ret;

    ret = k_mutex_lock(wifi_list_mutex, 0);
    if(ret != osOK)
        return NULL; /* lock mutex failed */

    p_node = p_head_free->next;
    ret = list_del_node(p_node, p_head_free);
    if(ret != 0){
        k_mutex_unlock(wifi_list_mutex);
        LOG_DBG("get pkt buf error:%d", ret);
        return NULL;
	}

    list_add_tail(p_node, p_head_employ);
    k_mutex_unlock(wifi_list_mutex);

    LOG_DBG("GET BUF:%p", ((uwp_pkt_buf *)LIST_FIND_ENTRY(p_node, uwp_pkt_buf, list))->buf);
    return ((uwp_pkt_buf *)LIST_FIND_ENTRY(p_node, uwp_pkt_buf, list))->buf;
}

int uwp_pkt_buf_free(void *buf){
    struct list_head *p_head_free = &wifi_buf_list_free;
    struct list_head *p_head_employ = &wifi_buf_list_employ;
    struct list_head *p_del = NULL;
    int ret;

    ret = k_mutex_lock(wifi_list_mutex, 0);
    if(ret != osOK)
        return -2; /* mutex lock failed */

    p_del = p_head_employ->next;
    while((p_del != p_head_employ) && (((uwp_pkt_buf *)LIST_FIND_ENTRY(p_del, uwp_pkt_buf, list))->buf != buf))
        p_del = p_del->next;

    ret = list_del_node(p_del, p_head_employ);
    if(ret != 0){
        LOG_DBG("node doesn't exist");
        k_mutex_unlock(wifi_list_mutex);
        return ret;
	}

    list_add_tail(p_del, p_head_free);
    //LOG_DBG("FREE BUF:%p", buf);
    k_mutex_unlock(wifi_list_mutex);

    return 0;
}
