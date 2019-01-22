#include <stdint.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "bt_hw_adapter.h"

#include "sipc.h"
#include "sblock.h"

#include "uki_abs.h"
#include "uki_zif.h"
#include <bluetooth/hci_driver.h>


#define SPRD_DP_RW_REG_SHIFT_BYTE 14
#define SPRD_DP_DMA_READ_BUFFER_BASE_ADDR 0x40280000
#define SPRD_DP_DMA_UARD_SDIO_BUFFER_BASE_ADDR (SPRD_DP_DMA_READ_BUFFER_BASE_ADDR + (1 << SPRD_DP_RW_REG_SHIFT_BYTE))
#define WORD_ALIGN 4

#define PACKET_TYPE		0
#define EVT_HEADER_TYPE		0
#define EVT_HEADER_EVENT	1
#define EVT_HEADER_SIZE		2
#define EVT_VENDOR_CODE_LSB	3
#define EVT_VENDOR_CODE_MSB	4
#define EVT_LE_META_SUBEVT  3
#define EVT_ADV_LENGTH     13

#define HCI_CMD			0x01
#define HCI_ACL			0x02
#define HCI_SCO			0x03
#define HCI_EVT			0x04
static u8_t adv_cache[50] = {0};

static pthread_t sipc_rx_thread_data;
static struct k_sem	event_sem;
extern struct net_buf *bt_buf_get_cmd_complete(void);
extern struct net_buf *bt_buf_get_rx(enum bt_buf_type type);

static inline int sprd_bt_write_word(unsigned int word)
{
  unsigned int *hwdec_addr = (unsigned int *)SPRD_DP_DMA_UARD_SDIO_BUFFER_BASE_ADDR;
  *hwdec_addr = word;
  //printf("wrte word: 0x%04X\n", word);
  return WORD_ALIGN;
}

int sprd_bt_write(unsigned char *data, int len)
{
  unsigned int *align_p, value;
  unsigned char *p;
  int i, word_num, remain_num;

  if (len <= 0)
    return len;

  word_num = len / WORD_ALIGN;
  remain_num = len % WORD_ALIGN;

  if (word_num) {
    for (i = 0, align_p = (unsigned int *)data; i < word_num; i++) {
    value = *align_p++;
    sprd_bt_write_word(value);
    }
  }

  if (remain_num) {
    value = 0;
    p = (unsigned char *) &value;
    for (i = len - remain_num; i < len; i++) {
      *p++ = *(data + i);
    }
    sprd_bt_write_word(value);
  }

  return len;
}

static void recv_callback(int ch)
{
	BT_DBG("recv_callback: %d", ch);
	if(ch == SMSG_CH_BT)
		k_sem_give(&event_sem);
}

static void bt_spi_handle_vendor_evt(u8_t *rxmsg)
{

}

static struct net_buf *alloc_adv_buf(unsigned char *src)
{
	struct net_buf *buf = NULL;
	u8_t length = src[EVT_ADV_LENGTH];

	if (!memcmp(adv_cache, src + EVT_ADV_LENGTH, length + 1)) {
		//BT_DBG("dropped surplus adv");
		return buf;
	} else {
		memcpy(adv_cache, src + EVT_ADV_LENGTH, length + 1);
		adv_cache[length + 1] = 0;
	}
	//BT_DBG("ADV REPORT, avail: %d, total: %d", le_adv_report_pool.avail_count, le_adv_report_pool.buf_count);
	buf = net_buf_alloc();
	if (buf == NULL) {
		BT_ERR("alloc adv buffer failed");
		adv_cache[0] = 0;
		return buf;
	}
	net_buf_reserve(buf, CONFIG_BT_HCI_RESERVE);
	bt_buf_set_type(buf, BT_BUF_EVT);

	return buf;
}



static void *rx_thread(const void *p1)
{
	int ret;
	u32_t left_length;
	struct net_buf *buf;
	unsigned char *rxmsg = NULL;
	struct bt_hci_acl_hdr acl_hdr;

	while (1) {
		//BT_DBG("wait for data");
		k_sem_take(&event_sem, K_FOREVER);
		struct sblock blk;
		ret = sblock_receive(0, SMSG_CH_BT, &blk, 0);
		if (ret < 0) {
			BT_ERR("sblock recv error");
			continue;
		}

		HCI_DUMP("<- ", blk.addr, blk.length);

		left_length = blk.length;

		do {
			rxmsg = ((unsigned char*)blk.addr) + blk.length - left_length;

			//BT_DBG("handle rx data +++");
			switch (rxmsg[PACKET_TYPE]) {
			case HCI_EVT:
				switch (rxmsg[EVT_HEADER_EVENT]) {
				case BT_HCI_EVT_VENDOR:
					{
						bt_spi_handle_vendor_evt(rxmsg);
						left_length -= rxmsg[EVT_HEADER_SIZE] + 3;
						BT_DBG("left: vendor %d", (int)left_length);
						if (!left_length) {
							goto rx_continue;
						} else {
							continue;
						}
					}
				case BT_HCI_EVT_CMD_COMPLETE:
				case BT_HCI_EVT_CMD_STATUS:
					buf = bt_buf_get_cmd_complete();
					break;
				default:
					if (rxmsg[EVT_HEADER_EVENT] == BT_HCI_EVT_LE_META_EVENT
						&& rxmsg[EVT_LE_META_SUBEVT] == BT_HCI_EVT_LE_ADVERTISING_REPORT) {
						buf = alloc_adv_buf(rxmsg);
						if (!buf) {
							left_length -= rxmsg[EVT_HEADER_SIZE] + 3;
							if (!left_length)
								goto rx_continue;
							else
								continue;
						}
					} else {
						buf = bt_buf_get_rx(BT_BUF_EVT);
					}
					break;
				}

				net_buf_add_mem(buf, &rxmsg[1],
						rxmsg[EVT_HEADER_SIZE] + 2);

				left_length -= rxmsg[EVT_HEADER_SIZE] + 3;
				break;
			case HCI_ACL:
				buf = bt_buf_get_rx(BT_BUF_ACL_IN);
				memcpy(&acl_hdr, &rxmsg[1], sizeof(acl_hdr));
				net_buf_add_mem(buf, &acl_hdr, sizeof(acl_hdr));
				net_buf_add_mem(buf, &rxmsg[5],
						sys_le16_to_cpu(acl_hdr.len));
				left_length -= sys_le16_to_cpu(acl_hdr.len) + 5;
				break;
			default:
				BT_ERR("Unknown BT buf type %d", rxmsg[0]);
				goto rx_continue;
			}

			//BT_DBG("handle rx data ---");
			if (rxmsg[PACKET_TYPE] == HCI_EVT &&
			    bt_hci_evt_is_prio(rxmsg[EVT_HEADER_EVENT])) {
				bt_recv_prio(buf);
			} else {
				bt_recv(buf);
			}
			BT_DBG("left: %d", (int)left_length);
		} while (left_length);
rx_continue:;
		sblock_release(0, SMSG_CH_BT, &blk);
	}
    return NULL;
}

int bt_sipc_init(void)
{
    k_sem_init(&event_sem, 0, UINT_MAX);

    sblock_create(0, SMSG_CH_BT,BT_TX_BLOCK_NUM, BT_TX_BLOCK_SIZE,
				BT_RX_BLOCK_NUM, BT_RX_BLOCK_SIZE);

	sblock_register_callback(SMSG_CH_BT, recv_callback);

 	pthread_create(&sipc_rx_thread_data, NULL, rx_thread, NULL);

/*
	k_thread_create(&tx_thread_data, tx_thread_stack,
			K_THREAD_STACK_SIZEOF(tx_thread_stack),
			(k_thread_entry_t)tx_thread, NULL, NULL, NULL,
			K_PRIO_COOP(CONFIG_BT_RX_PRIO),
			0, K_NO_WAIT);
	*/
	return 0;
}

