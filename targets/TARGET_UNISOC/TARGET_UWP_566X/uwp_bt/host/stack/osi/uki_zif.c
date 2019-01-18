/* hci_core.c - HCI core Bluetooth handling */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "uki_utils.h"
#include "uki_types.h"
#include "uki_zif.h"

#define CMD_BUF_SIZE BT_BUF_RX_SIZE
#define MOD_NUMBER      256
#define DEFAULT_SIZE    128

#define ZIF_DEBUG 0

int bt_rand(void *buf, size_t len)
{
	int seed = 0;
	int cur_num = 0;
	int seq_size = 0;
	u8_t seq[DEFAULT_SIZE];

	seq_size = (DEFAULT_SIZE > len) ? len : DEFAULT_SIZE;

	seed = (int)time(NULL);
	srand(seed);
	//printf("length = %d seed = %d\n", seq_size, seed);

	memset(seq, 0, sizeof(seq));
	for (int i = 0; i < seq_size; i++) {
		cur_num = rand() % MOD_NUMBER;
		//printf("0x%02x\n", cur_num);
		seq[i] = cur_num;
	}
	memcpy(buf, seq, seq_size);
	return 0;
}


const char *bt_hex(const void *buf, size_t len)
{
	static const char hex[] = "0123456789ABCDEF";
	static char hexbufs[4][129];
	static u8_t curbuf;
	const u8_t *b = buf;
	char *str;
	int i;

	str = hexbufs[curbuf++];
	curbuf %= ARRAY_SIZE(hexbufs);

	len = min(len, (sizeof(hexbufs[0]) - 1) / 2);

	for (i = 0; i < len; i++) {
		str[i * 2]     = hex[b[i] >> 4];
		str[i * 2 + 1] = hex[b[i] & 0xf];
	}

	str[i * 2] = '\0';

	return str;
}


const char *bt_addr_str(const bt_addr_t *addr)
{
	static char bufs[2][BT_ADDR_STR_LEN];
	static u8_t cur;
	char *str;

	str = bufs[cur++];
	cur %= ARRAY_SIZE(bufs);
	bt_addr_to_str(addr, str, sizeof(bufs[cur]));

	return str;
}

const char *bt_addr_le_str(const bt_addr_le_t *addr)
{
	static char bufs[2][BT_ADDR_LE_STR_LEN];
	static u8_t cur;
	char *str;

	str = bufs[cur++];
	cur %= ARRAY_SIZE(bufs);
	bt_addr_le_to_str(addr, str, sizeof(bufs[cur]));

	return str;
}

struct net_buf *net_buf_alloc(void)
{
    struct net_buf *buf = 0;
    buf = uki_malloc(sizeof(struct net_buf));
   // assert(buf);
    if(buf)
    {
        buf->size = CMD_BUF_SIZE;
        buf->__buf = uki_malloc(buf->size);
        buf->ref = 1;
    }
#if ZIF_DEBUG
    BT_INFO("%s alloc %p", __func__, buf);
#endif
    return buf;
}

size_t net_buf_simple_tailroom(struct net_buf_simple *buf)
{
	return buf->size - net_buf_simple_headroom(buf) - buf->len;
}

void net_buf_simple_init(struct net_buf_simple *buf,
                       size_t reserve_head)
{
    /*
    if (!buf->__buf) {
        buf->__buf = (u8_t *)buf + sizeof(*buf);
    }

    buf->data = buf->__buf + reserve_head;
    buf->len = 0;
    */
}

void *net_buf_simple_add(struct net_buf_simple *buf, size_t len)
{
	u8_t *tail = net_buf_simple_tail(buf);
	//assert(net_buf_simple_tailroom(buf) >= len);

	buf->len += len;
	return tail;
}

void *net_buf_simple_add_mem(struct net_buf_simple *buf, const void *mem,
			     size_t len)
{
	return memcpy(net_buf_simple_add(buf, len), mem, len);
}

u8_t *net_buf_simple_add_u8(struct net_buf_simple *buf, u8_t val)
{
	u8_t *u8;
	u8 = net_buf_simple_add(buf, 1);
	*u8 = val;

	return u8;
}

void net_buf_simple_reserve(struct net_buf_simple *buf, size_t reserve)
{
	//assert(buf);
	//assert(buf->len == 0);

	buf->data = buf->__buf + reserve;
}

void *net_buf_simple_pull(struct net_buf_simple *buf, size_t len)
{
	//assert(buf->len >= len);

	buf->len -= len;
	return buf->data += len;
}

u8_t net_buf_simple_pull_u8(struct net_buf_simple *buf)
{
	u8_t val;

	val = buf->data[0];
	net_buf_simple_pull(buf, 1);

	return val;
}

u16_t net_buf_simple_pull_le16(struct net_buf_simple *buf)
{
	u16_t val;

	val = UNALIGNED_GET((u16_t *)buf->data);
	net_buf_simple_pull(buf, sizeof(val));

	return sys_le16_to_cpu(val);
}

void *net_buf_simple_push(struct net_buf_simple *buf, size_t len)
{
	net_buf_simple_headroom(buf);

	buf->data -= len;
	buf->len += len;
	return buf->data;
}

void net_buf_simple_push_u8(struct net_buf_simple *buf, u8_t val)
{
	u8_t *data = net_buf_simple_push(buf, 1);

	*data = val;
}

struct net_buf *net_buf_ref(struct net_buf *buf)
{
	//assert(buf);
#if ZIF_DEBUG
	BT_INFO("ref %p %d", buf, buf->ref);
#endif
	buf->ref++;

	return buf;
}

size_t net_buf_simple_headroom(struct net_buf_simple *buf)
{
	return buf->data - buf->__buf;
}

void net_buf_unref(struct net_buf *buf)
{
#if ZIF_DEBUG
    BT_INFO("unref %p %d", buf, buf->ref);
#endif
    if( --(buf->ref) == 0)
    {
        uki_free(buf->__buf);
        uki_free(buf);
    }

}

void net_buf_simple_add_le16(struct net_buf_simple *buf, u16_t val)
{
	val = sys_cpu_to_le16(val);
	memcpy(net_buf_simple_add(buf, sizeof(val)), &val, sizeof(val));
}

struct net_buf *net_buf_slist_get(sys_slist_t *list)
{
	struct net_buf *buf, *frag;

	buf = (void *)sys_slist_get(list);

	if (!buf) {
		return NULL;
	}

	/* Get any fragments belonging to this buffer */
	for (frag = buf; (frag->flags & NET_BUF_FRAGS); frag = frag->frags) {
		frag->frags = (void *)sys_slist_get(list);

		/* The fragments flag is only for list-internal usage */
		frag->flags &= ~NET_BUF_FRAGS;
	}

	/* Mark the end of the fragment list */
	frag->frags = NULL;

	return buf;
}



void k_queue_append_list(struct k_queue *queue, void *head, void *tail)
{
	//unsigned int key = irq_lock();
	if (head) {
		sys_slist_append_list(&queue->data_q, head, tail);
	}
	//irq_unlock(key);
}

void k_queue_insert(struct k_queue *queue, void *prev, void *data)
{
	sys_slist_insert(&queue->data_q, prev, data);
}


void net_buf_slist_put(sys_slist_t *list, struct net_buf *buf)
{
	struct net_buf *tail;

	for (tail = buf; tail->frags; tail = tail->frags) {
		tail->flags |= NET_BUF_FRAGS;
	}

	sys_slist_append_list(list, &buf->node, &tail->node);
}


atomic_val_t atomic_set(atomic_t *target, atomic_val_t value)
{
	atomic_val_t ret;
	ret = *target;
	*target = value;
	return ret;
}


atomic_val_t atomic_get(const atomic_t *target)
{
	return *target;
}

atomic_val_t atomic_and(atomic_t *target, atomic_val_t value)
{
	atomic_val_t ret;
	ret = *target;
	*target &= value;
	return ret;
}

atomic_val_t atomic_or(atomic_t *target, atomic_val_t value)
{
	atomic_val_t ret;

	ret = *target;
	*target |= value;

	return ret;
}

atomic_val_t atomic_inc(atomic_t *target)
{
	atomic_val_t ret;

	ret = *target;
	(*target)++;
	return ret;
}

atomic_val_t atomic_dec(atomic_t *target)
{
	atomic_val_t ret;
	ret = *target;
	(*target)--;

	return ret;
}


void net_buf_simple_add_be32(struct net_buf_simple *buf, u32_t val)
{
	val = sys_cpu_to_be32(val);
	memcpy(net_buf_simple_add(buf, sizeof(val)), &val, sizeof(val));
}

u32_t net_buf_simple_pull_be32(struct net_buf_simple *buf)
{
	u32_t val;

	val = UNALIGNED_GET((u32_t *)buf->data);
	net_buf_simple_pull(buf, sizeof(val));

	return sys_be32_to_cpu(val);
}

u16_t net_buf_simple_pull_be16(struct net_buf_simple *buf)
{
	u16_t val;

	val = UNALIGNED_GET((u16_t *)buf->data);
	net_buf_simple_pull(buf, sizeof(val));

	return sys_be16_to_cpu(val);
}

void net_buf_simple_add_be16(struct net_buf_simple *buf, u16_t val)
{
	val = sys_cpu_to_be16(val);
	memcpy(net_buf_simple_add(buf, sizeof(val)), &val, sizeof(val));
}

void net_buf_simple_add_le32(struct net_buf_simple *buf, u32_t val)
{
    val = sys_cpu_to_le32(val);
    memcpy(net_buf_simple_add(buf, sizeof(val)), &val, sizeof(val));
}

s64_t _impl_k_uptime_get(void)
{
	return 0;
}

void net_buf_simple_push_be16(struct net_buf_simple *buf, u16_t val)
{
	val = sys_cpu_to_be16(val);
	memcpy(net_buf_simple_push(buf, sizeof(val)), &val, sizeof(val));
}
