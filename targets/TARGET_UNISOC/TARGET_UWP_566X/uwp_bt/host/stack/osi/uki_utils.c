/* hci_core.c - HCI core Bluetooth handling */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include "uki_utils.h"
#include "uki_log.h"

#define UTILS_DEBUG 0

void *uki_malloc(size_t size)
{

    void *ret = 0;
    ret = calloc(1, size);
/*
#if UTILS_DEBUG
#if defined (__CYGWIN__)
#elif 1
    {
        char name[32] = {0};
        pthread_getname_np(pthread_self(), name);

        if(strcasecmp(name, "blues") && strcasecmp(name, "log_print_th"))
        {
            printf("%s %p size %d\n", __func__, ret, size);
        }
    }
#else
    printf("%s %p size %d\n", __func__, ret, size);
#endif
#endif*/
    return ret;
}

void uki_free(void *p)
{
    free(p);
/*
#if UTILS_DEBUG
#if defined (__CYGWIN__)
#elif 1
    {
        char name[32] = {0};
        pthread_getname_np(pthread_self(), name);

        if(strcasecmp(name, "blues") && strcasecmp(name, "log_print_th"))
        {
            printf("%s %p\n", __func__, p);
        }
    }
#else
    printf("%s %p size %d\n", __func__, ret, size);
#endif
#endif
*/
}



#define CASE_RETURN_STR(const) case const: return #const;
#define CASE_HCI_CMD_STR(const) case BT_HCI_OP_##const: return #const;
#define CASE_HCI_EVT_STR(const) case BT_HCI_EVT_##const: return #const;
#define CASE_LE_META_STR(const) case BT_HCI_EVT_##const: return #const;
#define CASE_RT_STR(hd, const) case hd##const: return #const;

#if 0
const char* dump_hci_event(int event)
{
    switch (event) {
        CASE_HCI_EVT_STR(VENDOR)
        CASE_HCI_EVT_STR(INQUIRY_COMPLETE)
        CASE_HCI_EVT_STR(CONN_COMPLETE)
        CASE_HCI_EVT_STR(CONN_REQUEST)
        CASE_HCI_EVT_STR(DISCONN_COMPLETE)
        CASE_HCI_EVT_STR(AUTH_COMPLETE)
        CASE_HCI_EVT_STR(REMOTE_NAME_REQ_COMPLETE)
        CASE_HCI_EVT_STR(ENCRYPT_CHANGE)
        CASE_HCI_EVT_STR(REMOTE_FEATURES)
        CASE_HCI_EVT_STR(REMOTE_VERSION_INFO)
        CASE_HCI_EVT_STR(CMD_COMPLETE)
        CASE_HCI_EVT_STR(CMD_STATUS)
        CASE_HCI_EVT_STR(ROLE_CHANGE)
        CASE_HCI_EVT_STR(NUM_COMPLETED_PACKETS)
        CASE_HCI_EVT_STR(PIN_CODE_REQ)
        CASE_HCI_EVT_STR(LINK_KEY_REQ)
        CASE_HCI_EVT_STR(LINK_KEY_NOTIFY)
        CASE_HCI_EVT_STR(DATA_BUF_OVERFLOW)
        CASE_HCI_EVT_STR(INQUIRY_RESULT_WITH_RSSI)
        CASE_HCI_EVT_STR(REMOTE_EXT_FEATURES)
        CASE_HCI_EVT_STR(SYNC_CONN_COMPLETE)
        CASE_HCI_EVT_STR(EXTENDED_INQUIRY_RESULT)
        CASE_HCI_EVT_STR(ENCRYPT_KEY_REFRESH_COMPLETE)
        CASE_HCI_EVT_STR(IO_CAPA_REQ)
        CASE_HCI_EVT_STR(IO_CAPA_RESP)
        CASE_HCI_EVT_STR(USER_CONFIRM_REQ)
        CASE_HCI_EVT_STR(USER_PASSKEY_REQ)
        CASE_HCI_EVT_STR(USER_PASSKEY_NOTIFY)
        CASE_HCI_EVT_STR(LE_META_EVENT)
        CASE_HCI_EVT_STR(AUTH_PAYLOAD_TIMEOUT_EXP)
        default:
        return "UNKNOW";
    }
}


const char* dump_le_meta_event(int event)
{
    switch (event) {
        CASE_LE_META_STR(LE_CONN_COMPLETE)
        CASE_LE_META_STR(LE_ADVERTISING_REPORT)
        CASE_LE_META_STR(LE_CONN_UPDATE_COMPLETE)
        CASE_RT_STR(BT_HCI_EV_, LE_REMOTE_FEAT_COMPLETE)
        CASE_LE_META_STR(LE_LTK_REQUEST)
        CASE_LE_META_STR(LE_CONN_PARAM_REQ)
        CASE_LE_META_STR(LE_DATA_LEN_CHANGE)
        CASE_LE_META_STR(LE_P256_PUBLIC_KEY_COMPLETE)
        CASE_LE_META_STR(LE_GENERATE_DHKEY_COMPLETE)
        CASE_LE_META_STR(LE_ENH_CONN_COMPLETE)
        CASE_LE_META_STR(LE_DIRECT_ADV_REPORT)
        CASE_LE_META_STR(LE_PHY_UPDATE_COMPLETE)
        CASE_LE_META_STR(LE_EXT_ADVERTISING_REPORT)
        CASE_LE_META_STR(LE_PER_ADV_SYNC_ESTABLISHED)
        CASE_LE_META_STR(LE_PER_ADV_SYNC_LOST)
        CASE_LE_META_STR(LE_SCAN_TIMEOUT)
        CASE_LE_META_STR(LE_ADV_SET_TERMINATED)
        CASE_LE_META_STR(LE_SCAN_REQ_RECEIVED)
        CASE_LE_META_STR(LE_CHAN_SEL_ALGO)
        default:
        return "UNKNOW";
    }
}

const char* dump_cmd(int opcode)
{
    switch (opcode) {
        CASE_HCI_CMD_STR(INQUIRY)
        CASE_HCI_CMD_STR(CONNECT)
        CASE_HCI_CMD_STR(DISCONNECT)
        CASE_HCI_CMD_STR(CONNECT_CANCEL)
        CASE_HCI_CMD_STR(ACCEPT_CONN_REQ)
        CASE_HCI_CMD_STR(SETUP_SYNC_CONN)
        CASE_HCI_CMD_STR(ACCEPT_SYNC_CONN_REQ)
        CASE_HCI_CMD_STR(REJECT_CONN_REQ)
        CASE_HCI_CMD_STR(LINK_KEY_REPLY)
        CASE_HCI_CMD_STR(LINK_KEY_NEG_REPLY)
        CASE_HCI_CMD_STR(PIN_CODE_REPLY)
        CASE_HCI_CMD_STR(PIN_CODE_NEG_REPLY)
        CASE_HCI_CMD_STR(AUTH_REQUESTED)
        CASE_HCI_CMD_STR(SET_CONN_ENCRYPT)
        CASE_HCI_CMD_STR(REMOTE_NAME_REQUEST)
        CASE_HCI_CMD_STR(REMOTE_NAME_CANCEL)
        CASE_HCI_CMD_STR(READ_REMOTE_FEATURES)
        CASE_HCI_CMD_STR(READ_REMOTE_EXT_FEATURES)
        CASE_HCI_CMD_STR(READ_REMOTE_VERSION_INFO)
        CASE_HCI_CMD_STR(IO_CAPABILITY_REPLY)
        CASE_HCI_CMD_STR(USER_CONFIRM_REPLY)
        CASE_HCI_CMD_STR(USER_CONFIRM_NEG_REPLY)
        CASE_HCI_CMD_STR(USER_PASSKEY_REPLY)
        CASE_HCI_CMD_STR(USER_PASSKEY_NEG_REPLY)
        CASE_HCI_CMD_STR(IO_CAPABILITY_NEG_REPLY)
        CASE_HCI_CMD_STR(SET_EVENT_MASK)
        CASE_HCI_CMD_STR(RESET)
        CASE_HCI_CMD_STR(WRITE_LOCAL_NAME)
        CASE_HCI_CMD_STR(WRITE_PAGE_TIMEOUT)
        CASE_HCI_CMD_STR(WRITE_SCAN_ENABLE)
        CASE_HCI_CMD_STR(SET_CTL_TO_HOST_FLOW)
        CASE_HCI_CMD_STR(HOST_BUFFER_SIZE)
        CASE_HCI_CMD_STR(HOST_NUM_COMPLETED_PACKETS)
        CASE_HCI_CMD_STR(WRITE_INQUIRY_MODE)
        CASE_HCI_CMD_STR(WRITE_SSP_MODE)
        CASE_HCI_CMD_STR(LE_WRITE_LE_HOST_SUPP)
        CASE_HCI_CMD_STR(WRITE_SC_HOST_SUPP)
        CASE_HCI_CMD_STR(READ_AUTH_PAYLOAD_TIMEOUT)
        CASE_HCI_CMD_STR(WRITE_AUTH_PAYLOAD_TIMEOUT)
        CASE_HCI_CMD_STR(READ_LOCAL_VERSION_INFO)
        CASE_HCI_CMD_STR(READ_SUPPORTED_COMMANDS)
        CASE_HCI_CMD_STR(READ_LOCAL_EXT_FEATURES)
        CASE_HCI_CMD_STR(READ_LOCAL_FEATURES)
        CASE_HCI_CMD_STR(READ_BUFFER_SIZE)
        CASE_HCI_CMD_STR(READ_BD_ADDR)
        CASE_HCI_CMD_STR(READ_RSSI)
        CASE_HCI_CMD_STR(READ_ENCRYPTION_KEY_SIZE)
        CASE_HCI_CMD_STR(LE_SET_EVENT_MASK)
        CASE_HCI_CMD_STR(LE_READ_BUFFER_SIZE)
        CASE_HCI_CMD_STR(LE_READ_LOCAL_FEATURES)
        CASE_HCI_CMD_STR(LE_SET_ADV_PARAM)
        CASE_HCI_CMD_STR(LE_READ_ADV_CHAN_TX_POWER)
        CASE_HCI_CMD_STR(LE_SET_ADV_DATA)
        CASE_HCI_CMD_STR(LE_SET_SCAN_RSP_DATA)
        CASE_HCI_CMD_STR(LE_SET_ADV_ENABLE)
        CASE_HCI_CMD_STR(LE_SET_SCAN_ENABLE)
        CASE_HCI_CMD_STR(LE_CREATE_CONN)
        CASE_HCI_CMD_STR(LE_CREATE_CONN_CANCEL)
        CASE_HCI_CMD_STR(LE_READ_WL_SIZE)
        CASE_HCI_CMD_STR(LE_CLEAR_WL)
        CASE_HCI_CMD_STR(LE_ADD_DEV_TO_WL)
        CASE_HCI_CMD_STR(LE_REM_DEV_FROM_WL)
        CASE_HCI_CMD_STR(LE_CONN_UPDATE)
        CASE_HCI_CMD_STR(LE_SET_HOST_CHAN_CLASSIF)
        CASE_HCI_CMD_STR(LE_READ_REMOTE_FEATURES)
        CASE_HCI_CMD_STR(LE_ENCRYPT)
        CASE_HCI_CMD_STR(LE_RAND)
        CASE_HCI_CMD_STR(LE_START_ENCRYPTION)
        CASE_HCI_CMD_STR(LE_LTK_REQ_REPLY)
        CASE_HCI_CMD_STR(LE_LTK_REQ_NEG_REPLY)
        CASE_HCI_CMD_STR(LE_READ_SUPP_STATES)
        CASE_HCI_CMD_STR(LE_RX_TEST)
        CASE_HCI_CMD_STR(LE_TX_TEST)
        CASE_HCI_CMD_STR(LE_TEST_END)
        CASE_HCI_CMD_STR(LE_CONN_PARAM_REQ_REPLY)
        CASE_HCI_CMD_STR(LE_CONN_PARAM_REQ_NEG_REPLY)
        CASE_HCI_CMD_STR(LE_SET_DATA_LEN)
        CASE_HCI_CMD_STR(LE_READ_DEFAULT_DATA_LEN)
        CASE_HCI_CMD_STR(LE_WRITE_DEFAULT_DATA_LEN)
        CASE_HCI_CMD_STR(LE_P256_PUBLIC_KEY)
        CASE_HCI_CMD_STR(LE_GENERATE_DHKEY)
        CASE_HCI_CMD_STR(LE_ADD_DEV_TO_RL)
        CASE_HCI_CMD_STR(LE_REM_DEV_FROM_RL)
        CASE_HCI_CMD_STR(LE_CLEAR_RL)
        CASE_HCI_CMD_STR(LE_READ_RL_SIZE)
        CASE_HCI_CMD_STR(LE_READ_PEER_RPA)
        CASE_HCI_CMD_STR(LE_READ_LOCAL_RPA)
        CASE_HCI_CMD_STR(LE_SET_ADDR_RES_ENABLE)
        CASE_HCI_CMD_STR(LE_SET_RPA_TIMEOUT)
        CASE_HCI_CMD_STR(LE_READ_MAX_DATA_LEN)
        CASE_HCI_CMD_STR(LE_READ_PHY)
        CASE_HCI_CMD_STR(LE_SET_DEFAULT_PHY)
        CASE_HCI_CMD_STR(LE_ENH_RX_TEST)
        CASE_HCI_CMD_STR(LE_ENH_TX_TEST)
        CASE_HCI_CMD_STR(LE_SET_ADV_SET_RANDOM_ADDR)
        CASE_HCI_CMD_STR(LE_SET_EXT_ADV_PARAM)
        CASE_HCI_CMD_STR(LE_SET_EXT_ADV_DATA)
        CASE_HCI_CMD_STR(LE_SET_EXT_SCAN_RSP_DATA)
        CASE_HCI_CMD_STR(LE_SET_EXT_ADV_ENABLE)
        CASE_HCI_CMD_STR(LE_READ_MAX_ADV_DATA_LEN)
        CASE_HCI_CMD_STR(LE_READ_NUM_ADV_SETS)
        CASE_HCI_CMD_STR(LE_REMOVE_ADV_SET)
        CASE_HCI_CMD_STR(CLEAR_ADV_SETS)
        CASE_HCI_CMD_STR(LE_SET_PER_ADV_PARAM)
        CASE_HCI_CMD_STR(LE_SET_PER_ADV_DATA)
        CASE_HCI_CMD_STR(LE_SET_PER_ADV_ENABLE)
        CASE_HCI_CMD_STR(LE_SET_EXT_SCAN_PARAM)
        CASE_HCI_CMD_STR(LE_SET_EXT_SCAN_ENABLE)
        CASE_HCI_CMD_STR(LE_EXT_CREATE_CONN)
        CASE_HCI_CMD_STR(LE_PER_ADV_CREATE_SYNC)
        CASE_HCI_CMD_STR(LE_PER_ADV_CREATE_SYNC_CANCEL)
        CASE_HCI_CMD_STR(LE_PER_ADV_TERMINATE_SYNC)
        CASE_HCI_CMD_STR(LE_ADD_DEV_TO_PER_ADV_LIST)
        CASE_HCI_CMD_STR(LE_REM_DEV_FROM_PER_ADV_LIST)
        CASE_HCI_CMD_STR(LE_CLEAR_PER_ADV_LIST)
        CASE_HCI_CMD_STR(LE_READ_PER_ADV_LIST_SIZE)
        CASE_HCI_CMD_STR(LE_READ_TX_POWER)
        CASE_HCI_CMD_STR(LE_READ_RF_PATH_COMP)
        CASE_HCI_CMD_STR(LE_WRITE_RF_PATH_COMP)
        CASE_HCI_CMD_STR(LE_SET_PRIVACY_MODE)
        default:
        if (opcode >> 8 == 0xFC) {
            return "VENDOR";
        } else
            return "UNKNOW";
    }
}
#endif

/**
 * @brief Swap one buffer content into another
 *
 * Copy the content of src buffer into dst buffer in reversed order,
 * i.e.: src[n] will be put in dst[end-n]
 * Where n is an index and 'end' the last index in both arrays.
 * The 2 memory pointers must be pointing to different areas, and have
 * a minimum size of given length.
 *
 * @param dst A valid pointer on a memory area where to copy the data in
 * @param src A valid pointer on a memory area where to copy the data from
 * @param length Size of both dst and src memory areas
 */
void sys_memcpy_swap(void *dst, const void *src, size_t length)
{
	u8_t *pdst = (u8_t *)dst;
	const u8_t *psrc = (const u8_t *)src;

	__ASSERT(((psrc < pdst && (psrc + length) <= pdst) ||
		  (psrc > pdst && (pdst + length) <= psrc)),
		 "Source and destination buffers must not overlap");

	psrc += length - 1;

	for (; length > 0; length--) {
		*pdst++ = *psrc--;
	}
}

/**
 * @brief Swap buffer content
 *
 * In-place memory swap, where final content will be reversed.
 * I.e.: buf[n] will be put in buf[end-n]
 * Where n is an index and 'end' the last index of buf.
 *
 * @param buf A valid pointer on a memory area to swap
 * @param length Size of buf memory area
 */
void sys_mem_swap(void *buf, size_t length)
{
	size_t i;

	for (i = 0; i < (length/2); i++) {
		u8_t tmp = ((u8_t *)buf)[i];

		((u8_t *)buf)[i] = ((u8_t *)buf)[length - 1 - i];
		((u8_t *)buf)[length - 1 - i] = tmp;
	}
}

#if 0
list_t *list_new(void) {
  return (list_t *)uki_malloc(sizeof(list_t));
}

void list_free(list_t *list) {
  if (!list)
    return;

  list_clear(list);
  uki_free(list);
}

int list_is_empty(const list_t *list) {
  return (list->length == 0);
}

static inline void *list_node(const list_node_t *node) {
  return node->data;
}

static inline list_node_t *list_begin(const list_t *list) {
  return list->head;
}

static inline list_node_t *list_end(UNUSED_ATTR const list_t *list) {
  return NULL;
}

static inline list_node_t *list_next(const list_node_t *node) {
  return node->next;
}

static inline list_node_t *list_free_node_(list_t *list, list_node_t *node) {
  list_node_t *next = node->next;

  uki_free(node);
  --list->length;

  return next;
}

int list_contains(const list_t *list, const void *data) {
  for (const list_node_t *node = list_begin(list); node != list_end(list); node = list_next(node)) {
    if (list_node(node) == data)
      return true;
  }
  return false;
}

void *list_front(const list_t *list) {
  return list->head->data;
}

int list_append(list_t *list, void *data) {
  list_node_t *node = (list_node_t *)uki_malloc(sizeof(list_node_t));
  if (!node)
    return false;
  node->next = NULL;
  node->data = data;
  if (list->tail == NULL) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }
  ++list->length;
  return true;
}

int list_remove(list_t *list, void *data) {
  if (list_is_empty(list))
    return false;

  if (list->head->data == data) {
    list_node_t *next = list_free_node_(list, list->head);
    if (list->tail == list->head)
      list->tail = next;
    list->head = next;
    return true;
  }

  for (list_node_t *prev = list->head, *node = list->head->next; node; prev = node, node = node->next)
    if (node->data == data) {
      prev->next = list_free_node_(list, node);
      if (list->tail == node)
        list->tail = prev;
      return true;
    }

  return false;
}
#endif

