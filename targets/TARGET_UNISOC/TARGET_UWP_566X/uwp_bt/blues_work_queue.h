
#ifndef APPS_SYSTEM_BT_BLUES_WORK_QUEUE_H_
#define APPS_SYSTEM_BT_BLUES_WORK_QUEUE_H_
#include "osi/uki_work.h"
int blues_work_queue_init(void);
int blues_work_queue_uninit(void);
extern uki_work_t *g_default_work;


#endif /* APPS_SYSTEM_BT_BLUES_WORK_QUEUE_H_ */
