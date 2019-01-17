#ifndef __BLUETOOTH_UKI_ZPHY_H
#define __BLUETOOTH_UKI_ZPHY_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "uki_types.h"

#include "osi/uki_queue.h"

#include <bluetooth/bluetooth.h>
//#include <bluetooth/hci.h>

#define BT_BUF_RX_SIZE (CONFIG_BT_HCI_RESERVE + CONFIG_BT_RX_BUF_LEN)

typedef int atomic_t;
typedef atomic_t atomic_val_t;
struct k_sem
{
    char data[sizeof(sem_t)];
} __attribute__ ((aligned (1)));
//#define k_sem_give sem_post
//#define k_sem_init sem_init


#ifdef __GNUC__
#define NUTTX_UNUSED __attribute__ ((unused))
#else
#define NUTTX_UNUSED
#endif
#define K_MSEC(ms)     (ms)
#define MSEC_PER_SEC 1000
#define K_SECONDS(s)   K_MSEC((s) * MSEC_PER_SEC)
#define K_MINUTES(m)   K_SECONDS((m) * 60)
#define K_HOURS(h)     K_MINUTES((h) * 60)


#define k_sleep(time) usleep(time * 1000)


#define CODE_UNREACHABLE __builtin_unreachable()
#define popcount(x) __builtin_popcount(x)

#define net_buf_add_be16(buf, val) net_buf_simple_add_be16(&(buf)->b, val)
#define net_buf_add_be32(buf, val) net_buf_simple_add_be32(&(buf)->b, val)

#define NET_BUF_SIMPLE_DEFINE(_name, _size)     \
	u8_t net_buf_data_##_name[_size];       \
	struct net_buf_simple _name = {         \
		.data   = net_buf_data_##_name, \
		.len    = 0,                    \
		.size   = _size,                \
		.__buf  = net_buf_data_##_name, \
	}

#define NET_BUF_SIMPLE_DEFINE_STATIC(_name, _size)        \
	static __noinit u8_t net_buf_data_##_name[_size]; \
	static struct net_buf_simple _name = {            \
		.data   = net_buf_data_##_name,           \
		.len    = 0,                              \
		.size   = _size,                          \
		.__buf  = net_buf_data_##_name,           \
	}


const char *bt_hex(const void *buf, size_t len);
const char *bt_addr_str(const bt_addr_t *addr);
const char *bt_addr_le_str(const bt_addr_le_t *addr);
int bt_rand(void *buf, size_t len);

struct net_buf *net_buf_alloc(void);
u8_t *net_buf_simple_add_u8(struct net_buf_simple *buf, u8_t val);
void net_buf_simple_reserve(struct net_buf_simple *buf, size_t reserve);
void net_buf_simple_init(struct net_buf_simple *buf,
                       size_t reserve_head);

void *net_buf_simple_add(struct net_buf_simple *buf, size_t len);
u8_t net_buf_simple_pull_u8(struct net_buf_simple *buf);
void *net_buf_simple_pull(struct net_buf_simple *buf, size_t len);
struct net_buf *net_buf_ref(struct net_buf *buf);
size_t net_buf_simple_headroom(struct net_buf_simple *buf);
void *net_buf_simple_add_mem(struct net_buf_simple *buf, const void *mem,
			     size_t len);
void *net_buf_simple_push(struct net_buf_simple *buf, size_t len);

void net_buf_unref(struct net_buf *buf);

void k_queue_append(struct k_queue *queue, void *data);
void net_buf_simple_add_le16(struct net_buf_simple *buf, u16_t val);
u16_t net_buf_simple_pull_le16(struct net_buf_simple *buf);
void net_buf_simple_push_u8(struct net_buf_simple *buf, u8_t val);
struct net_buf *net_buf_slist_get(sys_slist_t *list);
void net_buf_simple_push_be16(struct net_buf_simple *buf, u16_t val);
void net_buf_slist_put(sys_slist_t *list, struct net_buf *buf);
#define net_buf_headroom(buf) net_buf_simple_headroom(&(buf)->b)

static inline void net_buf_simple_reset(struct net_buf_simple *buf)
{
	buf->len  = 0;
	buf->data = buf->__buf;
}

static inline u8_t *net_buf_simple_tail(struct net_buf_simple *buf)
{
	return buf->data + buf->len;
}

size_t net_buf_simple_tailroom(struct net_buf_simple *buf);

static inline void *net_buf_user_data(struct net_buf *buf)
{
	return (void *)buf->user_data;
}

static inline void net_buf_simple_save(struct net_buf_simple *buf,
				       struct net_buf_simple_state *state)
{
	state->offset = net_buf_simple_headroom(buf);
	state->len = buf->len;
}

static inline void net_buf_simple_restore(struct net_buf_simple *buf,
					  struct net_buf_simple_state *state)
{
	buf->data = buf->__buf + state->offset;
	buf->len = state->len;
}

static inline void bt_buf_set_type(struct net_buf *buf, enum bt_buf_type type)
{
	*(u8_t *)net_buf_user_data(buf) = type;
}

static inline enum bt_buf_type bt_buf_get_type(struct net_buf *buf)
{
	return (enum bt_buf_type)(*(u8_t *)net_buf_user_data(buf));
}

static inline size_t net_buf_frags_len(struct net_buf *buf)
{
	size_t bytes = 0;

	while (buf) {
		bytes += buf->len;
		buf = buf->frags;
	}

	return bytes;
}

#define net_buf_add(buf, len) net_buf_simple_add(&(buf)->b, len)
#define net_buf_add_mem(buf, mem, len) net_buf_simple_add_mem(&(buf)->b, \
							      mem, len)
#define net_buf_add_u8(buf, val) net_buf_simple_add_u8(&(buf)->b, val)
#define net_buf_reserve(buf, reserve) net_buf_simple_reserve(&(buf)->b, \
							     reserve);
#define net_buf_pull_u8(buf) net_buf_simple_pull_u8(&(buf)->b)

#define net_buf_add_le16(buf, val) net_buf_simple_add_le16(&(buf)->b, val)
#define net_buf_pull_le16(buf) net_buf_simple_pull_le16(&(buf)->b)

#define net_buf_pull(buf, len) net_buf_simple_pull(&(buf)->b, len)
#define net_buf_tailroom(buf) net_buf_simple_tailroom(&(buf)->b)
#define net_buf_push(buf, len) net_buf_simple_push(&(buf)->b, len)
#define net_buf_push_u8(buf, val) net_buf_simple_push_u8(&(buf)->b, val)
inline void net_buf_put(uki_fixed_queue_t *queue, struct net_buf *buf)
{
    uki_fixed_queue_enqueue(queue, buf);
}

inline void *net_buf_get(uki_fixed_queue_t *queue, s32_t timeout)
{
    void *ret = 0;
    ret = uki_fixed_queue_dequeue(queue);
    return ret;
}

#define k_fifo_put_list(fifo, head, tail) \
	k_queue_append_list((struct k_queue *) fifo, head, tail)

#define k_fifo_put(fifo, data) \
	k_queue_append((struct k_queue *) fifo, data)


#ifndef __BYTE_ORDER__
#define __BYTE_ORDER__ XCHAL_MEMORY_ORDER
#endif
#ifndef __ORDER_BIG_ENDIAN__
#define __ORDER_BIG_ENDIAN__ XTHAL_BIGENDIAN
#endif
#ifndef __ORDER_LITTLE_ENDIAN__
#define __ORDER_LITTLE_ENDIAN__ XTHAL_LITTLEENDIAN
#endif

#define __bswap_16(x) ((u16_t) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))
#define __bswap_32(x) ((u32_t) ((((x) >> 24) & 0xff) | \
				   (((x) >> 8) & 0xff00) | \
				   (((x) & 0xff00) << 8) | \
				   (((x) & 0xff) << 24)))
#define __bswap_64(x) ((u64_t) ((((x) >> 56) & 0xff) | \
				   (((x) >> 40) & 0xff00) | \
				   (((x) >> 24) & 0xff0000) | \
				   (((x) >> 8) & 0xff000000) | \
				   (((x) & 0xff000000) << 8) | \
				   (((x) & 0xff0000) << 24) | \
				   (((x) & 0xff00) << 40) | \
				   (((x) & 0xff) << 56)))

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define sys_le16_to_cpu(val) (val)
#define sys_cpu_to_le16(val) (val)
#define sys_be16_to_cpu(val) __bswap_16(val)
#define sys_cpu_to_be16(val) __bswap_16(val)
#define sys_le32_to_cpu(val) (val)
#define sys_cpu_to_le32(val) (val)
#define sys_le64_to_cpu(val) (val)
#define sys_cpu_to_le64(val) (val)
#define sys_be32_to_cpu(val) __bswap_32(val)
#define sys_cpu_to_be32(val) __bswap_32(val)
#define sys_be64_to_cpu(val) __bswap_64(val)
#define sys_cpu_to_be64(val) __bswap_64(val)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define sys_le16_to_cpu(val) __bswap_16(val)
#define sys_cpu_to_le16(val) __bswap_16(val)
#define sys_be16_to_cpu(val) (val)
#define sys_cpu_to_be16(val) (val)
#define sys_le32_to_cpu(val) __bswap_32(val)
#define sys_cpu_to_le32(val) __bswap_32(val)
#define sys_le64_to_cpu(val) __bswap_64(val)
#define sys_cpu_to_le64(val) __bswap_64(val)
#define sys_be32_to_cpu(val) (val)
#define sys_cpu_to_be32(val) (val)
#define sys_be64_to_cpu(val) (val)
#define sys_cpu_to_be64(val) (val)
#else
#error "Unknown byte order"
#endif

static inline void sys_put_be16(u16_t val, u8_t dst[2])
{
	dst[0] = val >> 8;
	dst[1] = val;
}

static inline void sys_put_be32(u32_t val, u8_t dst[4])
{
	sys_put_be16(val >> 16, dst);
	sys_put_be16(val, &dst[2]);
}

static inline void sys_put_le16(u16_t val, u8_t dst[2])
{
	dst[0] = val;
	dst[1] = val >> 8;
}

static inline void sys_put_le32(u32_t val, u8_t dst[4])
{
	sys_put_le16(val, dst);
	sys_put_le16(val >> 16, &dst[2]);
}

static inline void sys_put_le64(u64_t val, u8_t dst[8])
{
	sys_put_le32(val, dst);
	sys_put_le32(val >> 32, &dst[4]);
}

static inline u16_t sys_get_be16(const u8_t src[2])
{
	return ((u16_t)src[0] << 8) | src[1];
}

static inline u32_t sys_get_be32(const u8_t src[4])
{
	return ((u32_t)sys_get_be16(&src[0]) << 16) | sys_get_be16(&src[2]);
}

static inline u16_t sys_get_le16(const u8_t src[2])
{
	return ((u16_t)src[1] << 8) | src[0];
}

static inline u32_t sys_get_le32(const u8_t src[4])
{
	return ((u32_t)sys_get_le16(&src[2]) << 16) | sys_get_le16(&src[0]);
}

static inline u64_t sys_get_le64(const u8_t src[8])
{
	return ((u64_t)sys_get_le32(&src[4]) << 32) | sys_get_le32(&src[0]);
}

static inline void sys_slist_append(sys_slist_t *list,
				    sys_snode_t *node)
{
	node->next = NULL;

	if (!list->tail) {
		list->tail = node;
		list->head = node;
	} else {
		list->tail->next = node;
		list->tail = node;
	}
}

static inline bool sys_slist_is_empty(sys_slist_t *list)
{
	return (!list->head);
}

static inline sys_snode_t *sys_slist_get_not_empty(sys_slist_t *list)
{
	sys_snode_t *node = list->head;

	list->head = node->next;
	if (list->tail == node) {
		list->tail = list->head;
	}

	return node;
}

static inline void sys_slist_append_list(sys_slist_t *list,
					 void *head, void *tail)
{
	if (!list->tail) {
		list->head = (sys_snode_t *)head;
		list->tail = (sys_snode_t *)tail;
	} else {
		list->tail->next = (sys_snode_t *)head;
		list->tail = (sys_snode_t *)tail;
	}
}

static inline sys_snode_t *sys_slist_get(sys_slist_t *list)
{
	return sys_slist_is_empty(list) ? NULL : sys_slist_get_not_empty(list);
}

static inline void sys_slist_prepend(sys_slist_t *list,
				     sys_snode_t *node)
{
	node->next = list->head;
	list->head = node;

	if (!list->tail) {
		list->tail = list->head;
	}
}

static inline void sys_slist_insert(sys_slist_t *list,
				    sys_snode_t *prev,
				    sys_snode_t *node)
{
	if (!prev) {
		sys_slist_prepend(list, node);
	} else if (!prev->next) {
		sys_slist_append(list, node);
	} else {
		node->next = prev->next;
		prev->next = node;
	}
}

static inline void sys_slist_remove(sys_slist_t *list,
				    sys_snode_t *prev_node,
				    sys_snode_t *node)
{
	if (!prev_node) {
		list->head = node->next;

		/* Was node also the tail? */
		if (list->tail == node) {
			list->tail = list->head;
		}
	} else {
		prev_node->next = node->next;

		/* Was node the tail? */
		if (list->tail == node) {
			list->tail = prev_node;
		}
	}

	node->next = NULL;
}

static inline void sys_slist_init(sys_slist_t *list)
{
	list->head = NULL;
	list->tail = NULL;
}

static inline sys_snode_t *sys_slist_peek_head(sys_slist_t *list)
{
	return list->head;
}

static inline sys_snode_t *sys_slist_peek_next_no_check(sys_snode_t *node)
{
	return node->next;
}

static inline sys_snode_t *sys_slist_peek_tail(sys_slist_t *list)
{
	return list->tail;
}

static inline sys_snode_t *sys_slist_peek_next(sys_snode_t *node)
{
	return node ? sys_slist_peek_next_no_check(node) : NULL;
}


#define SYS_SLIST_STATIC_INIT(ptr_to_list) {NULL, NULL}
#define SYS_DLIST_STATIC_INIT(ptr_to_list) {{(ptr_to_list)}, {(ptr_to_list)}}
#define _WAIT_Q_INIT(wait_q) { SYS_DLIST_STATIC_INIT(&(wait_q)->waitq) }

#define SYS_SLIST_CONTAINER(__ln, __cn, __n) \
	((__ln) ? CONTAINER_OF((__ln), __typeof__(*(__cn)), __n) : NULL)
#define SYS_SLIST_PEEK_HEAD_CONTAINER(__sl, __cn, __n) \
	SYS_SLIST_CONTAINER(sys_slist_peek_head(__sl), __cn, __n)
#define SYS_SLIST_PEEK_NEXT_CONTAINER(__cn, __n) \
        ((__cn) ? SYS_SLIST_CONTAINER(sys_slist_peek_next(&((__cn)->__n)), \
                          __cn, __n) : NULL)
#define SYS_SLIST_FOR_EACH_CONTAINER(__sl, __cn, __n)			\
	for (__cn = SYS_SLIST_PEEK_HEAD_CONTAINER(__sl, __cn, __n); __cn; \
	     __cn = SYS_SLIST_PEEK_NEXT_CONTAINER(__cn, __n))
#define SYS_SLIST_FOR_EACH_CONTAINER_SAFE(__sl, __cn, __cns, __n)	\
            for (__cn = SYS_SLIST_PEEK_HEAD_CONTAINER(__sl, __cn, __n), \
                 __cns = SYS_SLIST_PEEK_NEXT_CONTAINER(__cn, __n); __cn;    \
                 __cn = __cns, __cns = SYS_SLIST_PEEK_NEXT_CONTAINER(__cn, __n))
#define SYS_SLIST_FOR_EACH_NODE(__sl, __sn)				\
                    for (__sn = sys_slist_peek_head(__sl); __sn;            \
                         __sn = sys_slist_peek_next(__sn))

#define SYS_SLIST_PEEK_TAIL_CONTAINER(__sl, __cn, __n) \
                        SYS_SLIST_CONTAINER(sys_slist_peek_tail(__sl), __cn, __n)

#define _K_QUEUE_INITIALIZER(obj) \
	{ \
	.data_q = SYS_SLIST_STATIC_INIT(&obj.data_q), \
	.wait_q = _WAIT_Q_INIT(&obj.wait_q) \
	}

#define _K_FIFO_INITIALIZER(obj) \
	{ \
	._queue = _K_QUEUE_INITIALIZER(obj._queue) \
	}




static inline bool sys_slist_find_and_remove(sys_slist_t *list,
					     sys_snode_t *node)
{
	sys_snode_t *prev = NULL;
	sys_snode_t *test;

	SYS_SLIST_FOR_EACH_NODE(list, test) {
		if (test == node) {
			sys_slist_remove(list, prev, node);
			return true;
		}

		prev = test;
	}

	return false;
}

#define ATOMIC_DEFINE(name, num_bits) \
	atomic_t name[1 + ((num_bits) - 1) / ATOMIC_BITS]

atomic_val_t atomic_set(atomic_t *target, atomic_val_t value);
atomic_val_t atomic_get(const atomic_t *target);
atomic_val_t atomic_and(atomic_t *target, atomic_val_t value);
atomic_val_t atomic_or(atomic_t *target, atomic_val_t value);
atomic_val_t atomic_inc(atomic_t *target);
atomic_val_t atomic_dec(atomic_t *target);

static inline int atomic_test_bit(const atomic_t *target, int bit)
{
	atomic_val_t val = atomic_get(ATOMIC_ELEM(target, bit));

	return (1 & (val >> (bit & (ATOMIC_BITS - 1))));
}

static inline void atomic_set_bit(atomic_t *target, int bit)
{
	atomic_val_t mask = ATOMIC_MASK(bit);

	atomic_or(ATOMIC_ELEM(target, bit), mask);
}

static inline void atomic_clear_bit(atomic_t *target, int bit)
{
	atomic_val_t mask = ATOMIC_MASK(bit);

	atomic_and(ATOMIC_ELEM(target, bit), ~mask);
}

static inline int atomic_test_and_set_bit(atomic_t *target, int bit)
{
	atomic_val_t mask = ATOMIC_MASK(bit);
	atomic_val_t old;

	old = atomic_or(ATOMIC_ELEM(target, bit), mask);

	return (old & mask) != 0;
}

static inline int atomic_test_and_clear_bit(atomic_t *target, int bit)
{
	atomic_val_t mask = ATOMIC_MASK(bit);
	atomic_val_t old;

	old = atomic_and(ATOMIC_ELEM(target, bit), ~mask);

	return (old & mask) != 0;
}

static inline u32_t k_uptime_get_32(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000;
}

static inline s64_t k_uptime_get(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000;
}
static inline unsigned int find_msb_set(u32_t op)
{
    if (!op) {
        return 0;
    }

    return 32 - __builtin_clz(op);
}


static inline unsigned int find_lsb_set(u32_t op)
{
    return __builtin_ffs(op);
}

u32_t net_buf_simple_pull_be32(struct net_buf_simple *buf);
void net_buf_simple_add_be32(struct net_buf_simple *buf, u32_t val);
u16_t net_buf_simple_pull_be16(struct net_buf_simple *buf);
void net_buf_simple_add_be16(struct net_buf_simple *buf, u16_t val);
void net_buf_simple_add_le32(struct net_buf_simple *buf, u32_t val);
s64_t _impl_k_uptime_get(void);


static inline void k_sem_init(struct k_sem *sem, unsigned int initial_count, unsigned int limit)
{
    sem_init((sem_t *)sem, initial_count, limit);
}
static inline int k_sem_take(struct k_sem *sem, uint32_t timeout)
{
    if (K_FOREVER == timeout) {
        return sem_wait((sem_t *)sem);
    }
    if (K_NO_WAIT == timeout) {
        return sem_trywait((sem_t *)sem);
    }
    struct timespec ts;
    ts.tv_sec = timeout / 1000;
    ts.tv_nsec = (timeout % 1000) * 1000000;
    return sem_timedwait((sem_t *)sem, &ts);
}
static inline void k_sem_give(struct k_sem *sem)
{
    sem_post((sem_t *)sem);
}

#endif
