#ifndef __BLUETOOTH_UKI_TYPES_H
#define __BLUETOOTH_UKI_TYPES_H

#include <errno.h>
#include <pthread.h>
#include <stdint.h>

typedef signed char         s8_t;
typedef signed short        s16_t;
typedef int32_t          s32_t;
typedef signed long long    s64_t;

typedef unsigned char       u8_t;
typedef unsigned short      u16_t;
typedef uint32_t        u32_t;
typedef unsigned long long  u64_t;

typedef int atomic_t;
typedef atomic_t atomic_val_t;

#define UINT_MAX    0xFFFFFFFFU

#define NOINIT noinit
#ifndef __COUNTER__
#define __COUNTER__ __LINE__
#endif
#define _STRINGIFY(x) #x
#define STRINGIFY(s) _STRINGIFY(s)
#define ___in_section(a, b, c) \
	__attribute__((section("." _STRINGIFY(a)			\
				"." _STRINGIFY(b)			\
				"." _STRINGIFY(c))))
#define __in_section(a, b, c) ___in_section(a, b, c)

#define __in_section_unique(seg) ___in_section(seg, __FILE__, __COUNTER__)
#define __noinit		__in_section_unique(NOINIT)


#define ATOMIC_INIT(i) (i)
#define ATOMIC_BITS (sizeof(atomic_val_t) * 8)
#define ATOMIC_MASK(bit) (1 << ((bit) & (ATOMIC_BITS - 1)))
#define ATOMIC_ELEM(addr, bit) ((addr) + ((bit) / ATOMIC_BITS))

#define ATOMIC_DEFINE(name, num_bits) \
	atomic_t name[1 + ((num_bits) - 1) / ATOMIC_BITS]

#ifndef BIT
#define BIT(n)  (1UL << (n))
#endif

#ifndef typeof
#define typeof  __typeof__
#endif


#define BIT_MASK(n) (BIT(n) - 1)

#define NET_BUF_FRAGS        BIT(0)

#define K_NO_WAIT 0
#define K_FOREVER 0xFFFFFFFFU

#ifndef __packed
  #define __packed __attribute__((__packed__))
#endif /* __packed */

#ifndef __aligned
#define __aligned(x)	__attribute__((__aligned__(x)))
#endif

#define UNALIGNED_GET(p)						\
__extension__ ({							\
	struct  __attribute__((__packed__)) {				\
		__typeof__(*(p)) __v;					\
	} *__p = (__typeof__(__p)) (p);					\
	__p->__v;							\
})


#define IS_ENABLED(config_macro) _IS_ENABLED1(config_macro)
#define _IS_ENABLED1(config_macro) _IS_ENABLED2(_XXXX##config_macro)
#define _XXXX1 _YYYY,
#define _IS_ENABLED2(one_or_two_args) _IS_ENABLED3(one_or_two_args 1, 0)
#define _IS_ENABLED3(ignore_this, val, ...) val


#define ZERO_OR_COMPILE_ERROR(cond) ((int) sizeof(char[1 - 2 * !(cond)]) - 1)
#define IS_ARRAY(array) \
	ZERO_OR_COMPILE_ERROR( \
		!__builtin_types_compatible_p(__typeof__(array), \
					      __typeof__(&(array)[0])))
#define ARRAY_SIZE(array) \
	((unsigned long) (IS_ARRAY(array) + \
		(sizeof(array) / sizeof((array)[0]))))


#ifndef CONTAINER_OF
#define CONTAINER_OF(ptr, type, field) \
	((type *)(((char *)(ptr)) - offsetof(type, field)))
#endif


enum bt_buf_type {
	/** HCI command */
	BT_BUF_CMD,
	/** HCI event */
	BT_BUF_EVT,
	/** Outgoing ACL data */
	BT_BUF_ACL_OUT,
	/** Incoming ACL data */
	BT_BUF_ACL_IN,
};


struct net_buf_simple {
	u8_t *data;
	u16_t len;
	u16_t size;
	u8_t *__buf;
};



struct _snode {
	struct _snode *next;
};

typedef struct _snode sys_snode_t;

struct _dnode {
	union {
		struct _dnode *head; /* ptr to head of list (sys_dlist_t) */
		struct _dnode *next; /* ptr to next node    (sys_dnode_t) */
	};
	union {
		struct _dnode *tail; /* ptr to tail of list (sys_dlist_t) */
		struct _dnode *prev; /* ptr to previous node (sys_dnode_t) */
	};
};
typedef struct _dnode sys_dlist_t;

struct _slist {
	sys_snode_t *head;
	sys_snode_t *tail;
};
typedef struct _slist sys_slist_t;

typedef struct {
	sys_dlist_t waitq;
} _wait_q_t;


typedef u64_t unative_t;

struct _sfnode {
	unative_t next_and_flags;
};

typedef struct _sfnode sys_sfnode_t;

struct _sflist {
	sys_sfnode_t *head;
	sys_sfnode_t *tail;
};
typedef struct _sflist sys_sflist_t;

struct k_queue {
	sys_slist_t data_q;
	union {
		_wait_q_t wait_q;

		//_POLL_EVENT;
	};

    //_OBJECT_TRACING_NEXT_PTR(k_queue);
};

struct k_fifo {
	struct k_queue _queue;
};

struct net_buf {
	union {
		sys_snode_t node;
		struct net_buf *frags;
	};

	u8_t ref;
	u8_t flags;
	u8_t pool_id;
	pthread_mutex_t lock;
	union {
		struct {
			u8_t *data;
			u16_t len;
			u16_t size;
			u8_t *__buf;
		};
		struct net_buf_simple b;
	};
	u8_t user_data[5];
};

struct net_buf_simple_state {
	u16_t offset;
	u16_t len;
};



#endif
