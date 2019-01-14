#ifndef MBED_UWP_SYS_WRAPPER_H
#define MBED_UWP_SYS_WRAPPER_H

#include "cmsis_os2.h"
#include "rtx_os.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UWP_SYS_DEBUG
#ifdef  UWP_SYS_DEBUG
#define UWP_SYS_PRINT(fmt, ...) do {\
            mbed_error_printf(fmt, ##__VA_ARGS__);\
    } while (0)
#else
#define UWP_SYS_PRINT(fmt, ...)
#endif

/* semepahore */
void *k_sem_create(uint32_t max_count, uint32_t initial_count);
int k_sem_release(void *sem);
int k_sem_acquire(void *sem, uint32_t ms);
int k_sem_delete(void *sem);

/* mutex */
void *k_mutex_create(void);
int k_mutex_lock(void *mutex, uint32_t ms);
int k_mutex_unlock(void *mutex);
int k_mutex_delete(void *mutex);


/* thread */
void *k_thread_create(const char *name, void (*thread_fn)(void *arg), void *arg, void *stack_mem, uint32_t stack_size, int priority);
int k_thread_terminate(void *tid);

/* msg queue  */
struct UWP_MSG_STRUCT{
    unsigned int type;
    unsigned int arg1;
    unsigned int arg2;
    unsigned int arg3;
};
typedef struct UWP_MSG_STRUCT* uwp_wifi_msg_t;
void *k_msg_create(unsigned int queuesz);
int k_msg_put(void *msgid, uwp_wifi_msg_t *msg, unsigned int ms);
int k_msg_get(void *msgid, uwp_wifi_msg_t *msg, unsigned int ms);

/* sys list   */

/* for zephyr */
#define  k_sleep  osDelay
#define K_FOREVER osWaitForever

#ifndef NULL
#define NULL ((void *)0)
#endif

struct _snode{
    struct _snode *next;
};
typedef struct _snode sys_snode_t;
struct _slist{
    sys_snode_t *head;
	sys_snode_t *tail;
};
typedef struct _slist sys_slist_t;

static inline void sys_slist_init(sys_slist_t *list){
    list->head = NULL;
	list->tail = NULL;
}

/* global variable for test */
//extern void *sys_sem;
//extern void *sys_mutex;


#ifdef __cplusplus
}
#endif

#endif

