/* hci_core.c - HCI core Bluetooth handling */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>

#include "uki_log.h"
#include "uki_queue.h"
#include <bluetooth/hci.h>

static uki_fixed_queue_t *log_fixed_queue;
static pthread_t log_print_th;
static int uki_log_enable = 0;

int bt_log_level = LOG_LEVEL_INFO;

typedef struct {
	unsigned char level;
	unsigned char type;
	char data[];
} uki_log_buf_t;
#define LOG_BUF_MAX_SIZE 1024

void *uki_log_print_th(const void *p)
{
	printf("%s start\n", __func__);
	uki_log_buf_t *buf;
	uki_log_enable = 1;
	while (1) {
		buf = uki_fixed_queue_dequeue(log_fixed_queue);
		printf("%s\n", buf->data);
		uki_free(buf);
	}
	return NULL;
}

int uki_log_init(void)
{
	log_fixed_queue = uki_fixed_queue_new();
	pthread_create(&log_print_th, NULL, uki_log_print_th, NULL);
	return 0;
}

void uki_log_print(unsigned char level, unsigned char type, const char* fmt, ...)
{
	va_list ap;
	uki_log_buf_t *buf = uki_malloc(sizeof(uki_log_buf_t) + LOG_BUF_MAX_SIZE);
	buf->level = level;
	buf->type = type;

	va_start(ap, fmt);
	vsnprintf(buf->data, LOG_BUF_MAX_SIZE, fmt, ap);
	va_end(ap);
	*(buf->data + LOG_BUF_MAX_SIZE) = 0;

	if (uki_log_enable) {
		uki_fixed_queue_enqueue(log_fixed_queue, buf);
	} else {
		printf("[ER]%s\n", buf->data);
		uki_free(buf);
	}
}


static void hex_dump(char *tag, unsigned char *bin, size_t binsz)
{
  char *str, hex_str[]= "0123456789ABCDEF";
  size_t i;

  str = (char *)uki_malloc(binsz * 3);
  if (!str) {
    return;
  }

  for (i = 0; i < binsz; i++) {
      str[(i * 3) + 0] = hex_str[(bin[i] >> 4) & 0x0F];
      str[(i * 3) + 1] = hex_str[(bin[i]     ) & 0x0F];
      str[(i * 3) + 2] = ' ';
  }
  str[(binsz * 3) - 1] = 0x00;
  if (tag)
	  UKI_DBG("%s %s", tag, str);
  else
	  UKI_DBG("%s", str);
  uki_free(str);
}

void hex_dump_block(char *tag, unsigned char *bin, size_t binsz)
{
#define HEX_DUMP_BLOCK_SIZE 20
	int loop = binsz / HEX_DUMP_BLOCK_SIZE;
	int tail = binsz % HEX_DUMP_BLOCK_SIZE;
	int i;

	if (!loop) {
		hex_dump(tag, bin, binsz);
		return;
	}

	for (i = 0; i < loop; i++) {
		hex_dump(tag, bin + i * HEX_DUMP_BLOCK_SIZE, HEX_DUMP_BLOCK_SIZE);
	}

	if (tail)
		hex_dump(tag, bin + i * HEX_DUMP_BLOCK_SIZE, tail);
}

