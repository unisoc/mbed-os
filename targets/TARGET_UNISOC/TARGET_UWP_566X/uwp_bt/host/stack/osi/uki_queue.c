/* hci_core.c - HCI core Bluetooth handling */
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "uki_queue.h"

uki_fixed_queue_t *uki_fixed_queue_new(void)
{
    uki_fixed_queue_t *fixed_queue = (uki_fixed_queue_t *)uki_malloc(sizeof(uki_fixed_queue_t));
    fixed_queue->list = uki_list_create(1);
    return fixed_queue;
}

void uki_fixed_queue_free(uki_fixed_queue_t *queue)
{
    uki_list_destroy(queue->list);
    uki_free(queue);
}

void *uki_fixed_queue_dequeue(uki_fixed_queue_t *queue)
{
    return uki_list_pop(queue->list);
}

void uki_fixed_queue_enqueue(uki_fixed_queue_t *queue, void *data)
{
    uki_list_insert_end(queue->list, data);
}

int uki_fixed_queue_count(uki_fixed_queue_t *queue)
{
    return uki_list_count(queue->list);
}
