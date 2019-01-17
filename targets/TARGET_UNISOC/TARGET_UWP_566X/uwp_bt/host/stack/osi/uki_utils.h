#ifndef __BLUETOOTH_UKI_UTILS_H
#define __BLUETOOTH_UKI_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "uki_log.h"
#include "uki_types.h"

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#ifndef UNUSED_ATTR
#define UNUSED_ATTR __attribute__((unused))
#endif

#ifndef NET_BUF_SIMPLE_ASSERT
#define NET_BUF_SIMPLE_ASSERT(cond) assert(cond)
#endif

#define __ASSERT(test, fmt, ...)                                   \
	do {                                                       \
		if (!(test)) {                                     \
			BT_ERR(fmt, ##__VA_ARGS__);                \
		}                                                  \
	} while ((0))


#ifndef BT_ASSERT
//#define BT_ASSERT assert
#define BT_ASSERT(cond) if (!(cond)) { \
				BT_ERR("assert: '" #cond "' failed"); \
			}

#endif

#ifndef __ASSERT_NO_MSG
#define __ASSERT_NO_MSG(test) __ASSERT(test, "")
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif


const char* dump_cmd(int opcode);
const char* dump_le_meta_event(int opcode);
const char* dump_hci_event(int event);
void sys_memcpy_swap(void *dst, const void *src, size_t length);
void sys_mem_swap(void *buf, size_t length);

//typedef unsigned char       bool;


typedef struct
{
    uint16_t          event;
    uint16_t          len;
    uint16_t          offset;
    uint16_t          layer_specific;
    uint8_t           data[];
} bt_buffer_t;


void *uki_malloc(size_t size);
void uki_free(void *p);

#define UINT32_TO_STREAM(p, u32) {*(p)++ = (uint8_t)(u32); *(p)++ = (uint8_t)((u32) >> 8); *(p)++ = (uint8_t)((u32) >> 16); *(p)++ = (uint8_t)((u32) >> 24);}
#define UINT24_TO_STREAM(p, u24) {*(p)++ = (uint8_t)(u24); *(p)++ = (uint8_t)((u24) >> 8); *(p)++ = (uint8_t)((u24) >> 16);}
#define UINT16_TO_STREAM(p, u16) {*(p)++ = (uint8_t)(u16); *(p)++ = (uint8_t)((u16) >> 8);}
#define UINT8_TO_STREAM(p, u8)   {*(p)++ = (uint8_t)(u8);}


enum { DUAL_MODE = 0, CLASSIC_MODE, LE_MODE };
enum { DISABLE_BT = 0, ENABLE_BT };


#define UNUSED(x) (void)(x)

#if 0
typedef struct _list_node_t {
  struct _list_node_t *next;
  void *data;
} list_node_t;

typedef struct list_t {
  list_node_t *head;
  list_node_t *tail;
  size_t length;
} list_t;

int list_append(list_t *list, void *data);
int list_remove(list_t *list, void *data);
void *list_front(const list_t *list);
#endif

#endif
