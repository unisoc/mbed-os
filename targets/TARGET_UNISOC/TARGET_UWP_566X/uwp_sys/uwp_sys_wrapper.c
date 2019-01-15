#include "uwp_sys_wrapper.h"
#include <stdio.h>

//void *sys_sem = NULL;
//void *sys_mutex = NULL;

void *k_sem_create(uint32_t max_count, uint32_t initial_count){
	osSemaphoreId_t sem = NULL;
    osRtxSemaphore_t *obj_sem = malloc(sizeof(osRtxSemaphore_t));
	osSemaphoreAttr_t attr_t_sem = {0};
	memset(obj_sem, 0, sizeof(osRtxSemaphore_t));
	attr_t_sem.cb_mem = obj_sem;
	attr_t_sem.cb_size = sizeof(osRtxSemaphore_t);
	sem = osSemaphoreNew(max_count, initial_count, &attr_t_sem);
	if(sem == NULL)
		UWP_SYS_PRINT("create sem error\r\n");
	return (void *)sem;
}

int k_sem_release(void *sem){
    osStatus_t ret = -1;
    ret = osSemaphoreRelease((osSemaphoreId_t)sem);
	if(ret == osErrorResource)
		UWP_SYS_PRINT("WRN:maximum sem count has been reached\r\n");
    else if(ret != osOK)
        UWP_SYS_PRINT("release sem error:%d\r\n",ret);
	return (int)ret;
}

int k_sem_acquire(void * sem, uint32_t ms){
    osStatus_t ret = -1;
	ret = osSemaphoreAcquire((osSemaphoreId_t)sem, ms);
	if((ret != osOK) && (ret != osErrorTimeout))
		UWP_SYS_PRINT("acquire sem error:%d\r\n",ret);
	return (int)ret;
}

int k_sem_delete(void *sem){
    osStatus_t ret = -1;
	ret = osSemaphoreDelete((osSemaphoreId_t)sem);
	if(ret != osOK)
		UWP_SYS_PRINT("delete sem error:%d\r\n",ret);
	free(sem);
	return (int)ret;
}

void *k_mutex_create(void){
	osMutexId_t id = NULL;
    osRtxMutex_t *obj_mutex = malloc(sizeof(osRtxMutex_t));
	memset(obj_mutex, 0, sizeof(osRtxMutex_t));
	osMutexAttr_t attr_mutex = {0};
	attr_mutex.name = "mutex";
	attr_mutex.cb_mem = obj_mutex;
	attr_mutex.cb_size = sizeof(osRtxMutex_t);
	attr_mutex.attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust;
	id = osMutexNew(&attr_mutex);
	if(id == NULL)
		UWP_SYS_PRINT("create mutex error\r\n");
	return (void *)id;
}

int k_mutex_lock(void *mutex, uint32_t ms){
    osStatus_t ret;
	ret = osMutexAcquire((osMutexId_t)mutex, ms);
	if(ret != osOK)
		UWP_SYS_PRINT("mutex:%p acquire failed:%d\r\n",mutex,ret);
	return (int)ret;
}

int k_mutex_unlock(void *mutex){
    osStatus_t ret;
	ret = osMutexRelease((osMutexId_t)mutex);
	if(ret != osOK)
		UWP_SYS_PRINT("mutex unlock error:%d\r\n",ret);
	return (int)ret;
}

int k_mutex_delete(void *mutex){
    osStatus_t ret;
	ret = osMutexDelete((osMutexId_t)mutex);
	free(mutex);
	if(ret != osOK)
		UWP_SYS_PRINT("mutex delete error:%d\r\n",ret);
	return (int)ret;
}

void *k_thread_create(const char *name, void (*thread_fn)(void *arg), void *arg, void *stack_mem, uint32_t stack_size, int priority){
	osThreadId_t thread_id;
    osRtxThread_t *thread_obj = malloc(sizeof(osRtxThread_t));
	if(stack_mem == NULL){
        stack_mem = malloc(stack_size);
	}
	if(thread_obj==NULL || stack_mem==NULL){
        UWP_SYS_PRINT("sys malloc error\r\n");
		return NULL;
	}
	osThreadAttr_t thread_attr = {
		.stack_mem = stack_mem,
		.stack_size = stack_size,
		.cb_mem = thread_obj,
		.cb_size = sizeof(osRtxThread_t),
		.priority = priority,
        .name = name
	};
	thread_id = osThreadNew(thread_fn, arg, &thread_attr);
	if(thread_id == NULL){
        UWP_SYS_PRINT("create thread failed\r\n");
	}
	return (void *)thread_id;
}
// TODO: porting
int k_thread_terminate(void *tid){
    return 0;
}

void *k_msg_create(unsigned int queuesz){
    osRtxMessageQueue_t *_obj_mem = malloc(sizeof(osRtxMessageQueue_t));
    void *internal_data = malloc(queuesz*(sizeof(uwp_wifi_msg_t)+sizeof(osRtxMessage_t)));
    if(_obj_mem==NULL || internal_data==NULL){
        UWP_SYS_PRINT("sys malloc error\r\n");
        return NULL;
}
    memset(_obj_mem, 0, sizeof(osRtxMessageQueue_t));
    osMessageQueueAttr_t attr = { 0 };
    attr.mq_mem = internal_data;
    attr.mq_size = (queuesz*(sizeof(uwp_wifi_msg_t)+sizeof(osRtxMessage_t)));
    attr.cb_mem = _obj_mem;
    attr.cb_size = sizeof(osRtxMessageQueue_t);
    osMessageQueueId_t id = osMessageQueueNew(queuesz, sizeof(uwp_wifi_msg_t), &attr);
    if(id == NULL)
        UWP_SYS_PRINT("create msg queue failed\r\n");
    return id;
}

int k_msg_put(void *msgid, uwp_wifi_msg_t *msg, unsigned int ms){
    osStatus_t ret;
    ret = osMessageQueuePut(msgid, msg, 0, ms);
    if(ret != osOK)
        UWP_SYS_PRINT("msg put failed\r\n");
    return (int)ret;
}

int k_msg_get(void *msgid, uwp_wifi_msg_t *msg, unsigned int ms){
    osStatus_t ret;
    ret = osMessageQueueGet(msgid, msg, NULL, ms);
    if(ret != osOK)
        UWP_SYS_PRINT("msg get failed\r\n");
    return (int)ret;
}

void list_add_tail(struct list_head *node, struct list_head *list){
    struct list_head *p_node = list;
    while(p_node->next != list)
        p_node = p_node->next;
    __LIST_ADD(node,p_node,list);
}

