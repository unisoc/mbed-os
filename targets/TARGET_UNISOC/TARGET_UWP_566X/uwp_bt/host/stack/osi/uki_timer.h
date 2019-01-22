
#ifndef UKI_TIMER_H_
#define UKI_TIMER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <pthread.h>
//#include <semaphore.h>
#include "uki_heap.h"
#include "uki_list.h"




#define UKI_TIMER_GAP 10        //ms
#define UKI_TIMER_PRECISION 1   //ms

#if UKI_TIMER_GAP <= UKI_TIMER_PRECISION
#error "UKI_TIMER_GAP <= UKI_TIMER_PRECISION"
#endif


#define CONTAIN_OF(p, mem, struc)   ((struc *)((unsigned int)(p) - (unsigned int)(&(((struc *)0)->mem))))


#define UKI_TIMER_MEM       \
    uki_timer_t *timer;      \
    uki_timer_cb cb;         \
    void *arg;              \
    int ms;                 \
    int is_persist;

typedef int (*uki_timer_cb)(void *arg);

typedef struct uki_timer_head_elem
{
    struct timespec ts;
    uki_list_t *list;
} uki_timer_head_elem;

typedef struct
{
    int max_head_count;
    int elem_count;
    int used_head_count;
    pthread_t pid;
    struct timespec ts;
    uki_timer_head_elem *head_list;
    uki_heap_t *hl;
    //uki_hashmap_t *hs;

    //for async
    pthread_mutex_t act_mutex;
    pthread_cond_t cont;
    pthread_condattr_t ca;
    sem_t async_sem;
    //int fd[2];  //not used by windows
    uki_list_t *asyn_list;
} uki_timer_t;


typedef struct
{
    UKI_TIMER_MEM
} uki_timer_work;

uki_timer_t *uki_timer_create(int count);
int uki_timer_destory_interface(uki_timer_t *handle, int sync);
uki_timer_work *uki_timer_create_work(uki_timer_cb cb, void *arg, int ms, int persist);
int uki_timer_work_free(uki_timer_work *handle);

int uki_timer_send_call(uki_timer_t *handle, uki_timer_cb cb, void *arg);
int uki_timer_del_work_interface(uki_timer_t *handle, uki_timer_work *work, int sync);
int uki_timer_del_interface(uki_timer_t *handle, uki_timer_cb cb, void *arg, int sync);
int uki_timer_get_work_remaining_interface(uki_timer_t *handle, uki_timer_work *work, int sync);
int uki_timer_add_work_interface(uki_timer_t *handle, uki_timer_work *work, int sync);
int uki_timer_add_interface(uki_timer_t *handle, uki_timer_cb cb, void *arg, int ms, int persist, int sync);
int uki_timer_destory_interface(uki_timer_t *handle, int sync);
int uki_timer_run(uki_timer_t *handle);
void uki_timer_test(void);
#ifdef __cplusplus
}
#endif

#endif /* UKI_TIMER_H_ */
