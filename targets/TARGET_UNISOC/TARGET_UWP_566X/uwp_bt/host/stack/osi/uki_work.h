
#ifndef UKI_WORK_H_
#define UKI_WORK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <pthread.h>
//#include <semaphore.h>
#include <string.h>
#include "uki_timer.h"
#include "uki_list.h"
#include "uki_log.h"
typedef uki_timer_cb uki_work_cb;
//typedef int (*uki_work_cb)(void *arg);

typedef struct uki_work_elem
{
    struct uki_work_t *whandle;
    int is_delay_work;
    uki_work_cb cb;
    void *arg;
    uki_timer_work *twork;

    int flag;
} uki_work_elem;

typedef struct uki_work_t
{
    pthread_t pt;
    uki_timer_t *timer;
    uki_list_t *list;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    sem_t sem;
} uki_work_t;

uki_work_t *uki_work_create(void);
int uki_work_destroy(uki_work_t *handle, int sync);
int uki_work_elem_create(uki_work_elem *elem, uki_work_cb cb, void *arg, int delayms);
int uki_work_del(uki_work_t *handle, uki_work_elem *elem, int sync);
int uki_work_add(uki_work_t *handle, uki_work_elem *elem);
int uki_work_get_remaining(uki_work_t *handle, uki_work_elem *elem);
pthread_t uki_work_run(uki_work_t *handle, char *name);
int uki_work_elem_free(uki_work_elem *handle);
void uki_work_test(void);




//
typedef void k_thread_stack_t;

#define k_work_q uki_work_t
#define k_work uki_work_elem
#define k_delayed_work uki_work_elem
//

extern uki_work_t *g_default_work;

typedef void (*k_work_handler_t)(struct k_work *work);




size_t k_work_q_start(struct k_work_q *work_q, k_thread_stack_t *stack,
                                    size_t stack_size, int prio);

void k_work_init_inter(struct k_work *work, k_work_handler_t handler);



void k_work_submit(struct k_work *work);

void k_work_submit_to_queue(struct k_work_q *work_q, struct k_work *work);

void k_delayed_work_init_inter(struct k_delayed_work *work,
        k_work_handler_t handler);


int k_delayed_work_submit_inter(struct k_delayed_work *work,
        int delay);


int k_delayed_work_submit_to_queue(struct k_work_q *work_q,
        struct k_delayed_work *work,
        int delay);


int k_delayed_work_cancel(struct k_delayed_work *work);

int k_work_pending(struct k_work *work);

int k_delayed_work_remaining_get(struct k_delayed_work *work);





#define k_work_init(work, handler) \
    k_work_init_inter(work, handler)

#define k_delayed_work_init(work, handler) \
    k_delayed_work_init_inter(work, handler)

#define k_delayed_work_submit(work, delay) \
    k_delayed_work_submit_inter((work), delay)



#ifdef __cplusplus
}
#endif


#endif /* UKI_WORK_H_ */
