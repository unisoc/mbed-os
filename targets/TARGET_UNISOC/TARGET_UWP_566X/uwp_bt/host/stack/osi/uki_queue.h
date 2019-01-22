#ifndef __BLUETOOTH_UKI_QUEUE_H
#define __BLUETOOTH_UKI_QUEUE_H

//#include <semaphore.h>
#include "uki_utils.h"

#include "uki_list.h"


typedef struct {
    uki_list_t *list;
} uki_fixed_queue_t;

void *uki_fixed_queue_dequeue(uki_fixed_queue_t *queue);
void uki_fixed_queue_enqueue(uki_fixed_queue_t *queue, void *data);
uki_fixed_queue_t *uki_fixed_queue_new(void);
int uki_fixed_queue_count(uki_fixed_queue_t *queue);

#endif
