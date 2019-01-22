
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <errno.h>
//#include <semaphore.h>
#include <string.h>
#include <pthread.h>
#include "uki_timer.h"
#include "uki_heap.h"
#include "uki_log.h"
#include "uki_utils.h"


#define USE_SEM 1
#define IS_NUTTX_IMPL 0

#define TIMER_DEBUG 0

#define IS_TYPE_SIGNED(my_type) ((((typeof(my_type))(0))-1) < 0)
#define IS_OVERFLOW(x1,x2) ( ((x1) + (x2) < (x1)) || ((x1) + (x2) < (x2)) )

typedef enum
{
    UKI_TIMER_ACTION__ADD,
    UKI_TIMER_ACTION__ADD_WORK,
    UKI_TIMER_ACTION__DEL,
    UKI_TIMER_ACTION__DEL_WORK,
    UKI_TIMER_ACTION__GET_WORK_REMAINING,
    UKI_TIMER_ACTION__DESTORY,
    UKI_TIMER_ACTION__EXTERN_CALL,
} UKI_TIMER_ACTION__ENUM;

typedef struct
{
    UKI_TIMER_MEM
    UKI_TIMER_ACTION__ENUM action;
    sem_t *sem;
    void *action_mem;
    int action_size;
} uki_timer_action;

typedef struct uki_timer_elem
{
    UKI_TIMER_MEM
    struct uki_timer_head_elem *head;
} uki_timer_elem;

#if TIMER_DEBUG
#define timespec_print(ts, arg)  \
    BT_INFO("%s sec %ld , msec %ld", (arg), ((long)(ts)->tv_sec), ((long)(ts)->tv_nsec))
#else
#define timespec_print(ts, arg)
#endif

#if CUR_PROJECT == PROJ_MBED
#define pthread_cond_signal(x) mbed_thread_signal(CONTAIN_OF(x, cont, uki_timer_t)->pid, UKI_THREAD_SIG_FLAG);
#endif

static inline int timespec_plus(struct timespec *result, struct timespec *v1, struct timespec *v2 )
{
    int flag = 0;
    result->tv_nsec = v1->tv_nsec + v2->tv_nsec;

    if (result->tv_nsec >= 1000000000 )
    {

        flag += 1;
        result->tv_nsec -= 1000000000;

    }

    result->tv_sec = v1->tv_sec + v2->tv_sec + flag;

    if (IS_OVERFLOW(v1->tv_sec, v2->tv_sec) || IS_OVERFLOW(v1->tv_sec + v2->tv_sec, flag))
    {
        return -1;
    }


    return 0;
}

static inline int timespec_sub(struct timespec *result, struct timespec *v1, struct timespec *v2 )
{
    int flag = 0;
    result->tv_nsec = v1->tv_nsec - v2->tv_nsec;

    if (v1->tv_nsec < v2->tv_nsec)
    {
        result->tv_nsec += 1000000000;
        flag = 1;
    }

    result->tv_sec = v1->tv_sec - v2->tv_sec - flag;

    if (v1->tv_sec < v2->tv_sec || v1->tv_sec - v2->tv_sec < flag)
    {
        return -1;
    }

    return 0;
}

static inline int timespec_compare(struct timespec *v1, struct timespec *v2)
{
    struct timespec tmp;
    int ret = 0;
    ret =  timespec_sub(&tmp, v1, v2);

    if (ret < 0)
    {
        return -1;
    }
    else if (tmp.tv_nsec == 0 && tmp.tv_sec == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

static inline int timespec_is_nearly(struct timespec *diff, struct timespec *v1, struct timespec *v2)
{
    struct timespec tmp;

    if (timespec_compare(v1, v2) >= 0)
    {
        timespec_sub(&tmp, v1, v2);
    }
    else
    {
        timespec_sub(&tmp, v2, v1);
    }

    if (timespec_compare(diff, &tmp) >= 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static int timer_elem_compare(void *arg1, void *arg2)
{
    uki_timer_head_elem *elem1 = (uki_timer_head_elem *)arg1;
    uki_timer_head_elem *elem2 = (uki_timer_head_elem *)arg2;
    return timespec_compare(&elem1->ts, &elem2->ts);

}

uki_timer_t *uki_timer_create(int count)
{
    uki_timer_t *tmp = 0;
    int i = 0;
    int ret = 0;

    if (count < 1)
    {
        return 0;
    }

    tmp = (uki_timer_t *)uki_malloc( sizeof(uki_timer_t));
    tmp->hl = (uki_heap_t *)uki_heap_create(count, timer_elem_compare, 0);
    tmp->head_list = (uki_timer_head_elem *)uki_malloc(sizeof(uki_timer_head_elem) * count);

    assert (tmp && tmp->hl && tmp->head_list);

    for ( i = 0; i < count; ++i)
    {
        tmp->head_list[i].list = uki_list_create(0);

        assert(tmp->head_list[i].list);

    }

    tmp->max_head_count = count;

    ret = pthread_mutex_init(&tmp->act_mutex, 0);
    assert(ret == 0);
    pthread_condattr_init(&tmp->ca);
#if IS_NUTTX_IMPL
    ret = pthread_condattr_setclock(&tmp->ca, CLOCK_MONOTONIC);
    assert(ret == 0);
    ret = pthread_cond_init(&tmp->cont, &tmp->ca);
    assert(ret == 0);
#else
    ret = pthread_cond_init(&tmp->cont, 0);
    assert(ret == 0);
#endif



#if USE_SEM
    ret = sem_init(&tmp->async_sem, 0, 1);
    assert(ret == 0);
#else
    pipe(tmp->fd);
    fcntl(tmp->fd[0], F_SETFL, fcntl(tmp->fd[0], F_GETFL, 0) & ~O_NONBLOCK);
    fcntl(tmp->fd[1], F_SETFL, fcntl(tmp->fd[1], F_GETFL, 0) & ~O_NONBLOCK);
#endif
    tmp->asyn_list = uki_list_create(0);
    assert(tmp->asyn_list);
    return tmp;
}

uki_timer_work *uki_timer_create_work(uki_timer_cb cb, void *arg, int ms, int persist)
{
    uki_timer_work *tmp = (uki_timer_work *) uki_malloc(sizeof(typeof(*tmp)));
    assert(tmp);

    if (tmp)
    {
        tmp->arg = arg;
        tmp->cb = cb;
        tmp->ms = ms;
        tmp->is_persist = persist;
    }

    return tmp;
}

int uki_timer_work_free(uki_timer_work *handle)
{

    if (handle->timer)
    {
        uki_timer_del_work_interface(handle->timer, handle, 1);
    }

    uki_free(handle);
    return 0;
}
static int uki_timer_destroy(uki_timer_t *handle)
{
    int i = 0;
    uki_timer_elem *elem = 0;
    uki_timer_head_elem *headlist = 0;
    uki_list_t *list;


    headlist = handle->head_list;

    for (i = 0; i < handle->max_head_count; ++i)
    {

        list = headlist[i].list;

        while ((elem = uki_list_pop(list)))
        {
            uki_free(elem);
        }

        uki_list_destroy(list);
    }

    uki_free(headlist);
    uki_heap_destroy(handle->hl);

    pthread_mutex_destroy(&handle->act_mutex);
    pthread_cond_destroy(&handle->cont);
    pthread_condattr_destroy(&handle->ca);
#if USE_SEM
    sem_destroy(&handle->async_sem);
#else
    close(handle->fd[0]);
    close(handle->fd[1]);
#endif

    while (uki_list_count(handle->asyn_list))
    {
        uki_free(uki_list_pop(handle->asyn_list));
    }

    uki_list_destroy(handle->asyn_list);
    uki_free(handle);
    return 0;
}

int uki_timer_destory_interface(uki_timer_t *handle, int sync)
{
    uki_timer_action *tmp = (uki_timer_action *) uki_malloc(sizeof(typeof(*tmp)));
    sem_t *sem = 0;
    tmp->action = UKI_TIMER_ACTION__DESTORY;


    if (sync)
    {
        sem = (sem_t *)uki_malloc(sizeof(sem_t));
        sem_init(sem, 0, 1);
        tmp->sem = sem;
    }

    pthread_mutex_lock(&handle->act_mutex);
    uki_list_insert_end(handle->asyn_list, tmp);
#if USE_SEM
    sem_post(&handle->async_sem);
#else
    write(handle->fd[1], "1", 1);
#endif
    pthread_cond_signal(&handle->cont);
    pthread_mutex_unlock(&handle->act_mutex);


    if (sync)
    {
        sem_wait(sem);
        sem_destroy(sem);
        uki_free(sem);
    }

    return 0;
}

static int uki_timer_add(uki_timer_t *handle, uki_timer_cb cb, void *arg, int ms, int persist)
{

    int realms = 0 ;
    int i = 0;
    int found = 0;
    struct timespec tmpts1 = {0};
    struct timespec tmpts2 = {0};
    struct timespec tmptsres = {0};
    uki_timer_head_elem *head = 0;
    uki_timer_head_elem *headlist = 0;
    uki_timer_elem *elem = 0;

    if (!handle || !cb || ms <= 0)
    {
        return -1;
    }

    realms = ms / UKI_TIMER_GAP;
    realms *= UKI_TIMER_GAP;
    tmpts1.tv_sec = realms / 1000;
    tmpts1.tv_nsec = (realms % 1000) * 1000000;
#if IS_NUTTX_IMPL
    clock_gettime(CLOCK_MONOTONIC, &tmpts2);
#else
    clock_gettime(CLOCK_REALTIME, &tmpts2);
#endif
    timespec_plus(&tmptsres, &tmpts1, &tmpts2);

    headlist = handle->head_list;

    for (i = 0; i < handle->max_head_count; ++i)
    {
        struct timespec nearlyts;
        nearlyts.tv_sec = UKI_TIMER_PRECISION / 1000;
        nearlyts.tv_nsec = UKI_TIMER_PRECISION % 1000 * 1000000;

        if ( uki_list_count(headlist[i].list) > 0 && timespec_is_nearly(&nearlyts, &tmptsres, &headlist[i].ts))
        {
            timespec_print(&tmpts2, "now");
            timespec_print(&tmptsres, "furture");
            timespec_print(&headlist[i].ts, "head");
            found = 1;
            break;
        }

    }

    elem = (uki_timer_elem *)uki_malloc(sizeof(uki_timer_elem));
    elem->arg = arg;
    elem->cb = cb;
    elem->is_persist = (persist ? 1 : 0);
    elem->ms = ms;

    if (found)
    {
        head = &headlist[i];
        uki_list_insert_end(head->list, elem);
#if TIMER_DEBUG
        BT_INFO("add index %d", i);
#endif
    }
    else
    {
        if (handle->used_head_count >= handle->max_head_count)
        {
            uki_free(elem);
            return -1;
        }

        handle->used_head_count++;

        for (i = 0; i < handle->max_head_count; ++i)
        {
            head = &headlist[i];

            if ( uki_list_count(head->list) == 0)
            {
                break;
            }
        }

        uki_list_insert_end(head->list, elem);
        head->ts = tmptsres;

        assert(0 == uki_heap_insert(handle->hl, (void *)head));
#if TIMER_DEBUG
        BT_INFO("insert index %d sec %ld nsec %ld index0 sec %ld nsec %ld", i, tmptsres.tv_sec, (long)tmptsres.tv_nsec,
              headlist[0].ts.tv_sec, (long)headlist[0].ts.tv_nsec);
#endif
    }


    return 0;
}

int uki_timer_add_interface(uki_timer_t *handle, uki_timer_cb cb, void *arg, int ms, int persist, int sync)
{
    uki_timer_action *tmp = (uki_timer_action *) uki_malloc(sizeof(uki_timer_action));
    tmp->action = UKI_TIMER_ACTION__ADD;
    tmp->arg = arg;
    tmp->cb = cb;
    tmp->ms = ms;
    tmp->is_persist = persist;

    pthread_mutex_lock(&handle->act_mutex);
    uki_list_insert_end(handle->asyn_list, tmp);
#if USE_SEM
    sem_post(&handle->async_sem);
#else
    write(handle->fd[1], "1", 1);
#endif
    pthread_cond_signal(&handle->cont);
    pthread_mutex_unlock(&handle->act_mutex);
    return 0;
}

static int work_cb(void *arg)
{
    uki_timer_work *work = (uki_timer_work *)arg;
#if TIMER_DEBUG
    BT_INFO("arg %p cb %p", arg, work->cb);
#endif
    return (*work->cb)(work->arg);
}

int uki_timer_add_work_interface(uki_timer_t *handle, uki_timer_work *work, int sync)
{
    int ret = 0;
    uki_timer_action *tmp = (uki_timer_action *) uki_malloc(sizeof(uki_timer_action));
    assert(tmp);
    tmp->action = UKI_TIMER_ACTION__ADD_WORK;
    tmp->arg = work;
    tmp->cb = work_cb;
    tmp->ms = work->ms;
    tmp->is_persist = work->is_persist;
    work->timer = handle;
#if TIMER_DEBUG
    BT_INFO("work %p work->cb %p", work, work->cb);
#endif
    pthread_mutex_lock(&handle->act_mutex);
    uki_list_insert_end(handle->asyn_list, tmp);
#if USE_SEM
    sem_post(&handle->async_sem);
#else
    write(handle->fd[1], "1", 1);
#endif
    ret = pthread_cond_signal(&handle->cont);
    assert(ret == 0);
    pthread_mutex_unlock(&handle->act_mutex);
    return 0;
}


static int timerlist_find(void *arg1, void *arg2)
{
    uki_timer_elem *elem = (uki_timer_elem *)arg1;
    return elem->arg == (typeof(elem->arg))arg2;
}

static int uki_timer_del(uki_timer_t *handle, uki_timer_cb cb, void *arg)
{
    int i = 0;
    int ret = 0;
    uki_list_t *list = 0;

    for (i = 0; i < handle->max_head_count; ++i)
    {
        list = handle->head_list[i].list;

        ret = uki_list_is_exist_with_cb_with_action(list, timerlist_find, arg, 1);

        if (ret && uki_list_count(list) == 0)
        {
            uki_heap_remove(handle->hl, &handle->head_list[i]);

            handle->used_head_count--;
        }
    }


    return 0;
}



int uki_timer_del_interface(uki_timer_t *handle, uki_timer_cb cb, void *arg, int sync)
{
    uki_timer_action *tmp = (uki_timer_action *) uki_malloc(sizeof(typeof(*tmp)));
    tmp->action = UKI_TIMER_ACTION__DEL;
    tmp->cb = cb;
    tmp->arg = arg;
    pthread_mutex_lock(&handle->act_mutex);
    uki_list_insert_end(handle->asyn_list, tmp);
#if USE_SEM
    sem_post(&handle->async_sem);
#else
    write(handle->fd[1], "1", 1);
#endif
    pthread_cond_signal(&handle->cont);
    pthread_mutex_unlock(&handle->act_mutex);
    return 0;
}


int uki_timer_del_work_interface(uki_timer_t *handle, uki_timer_work *work, int sync)
{
    uki_timer_action *tmp = (uki_timer_action *) uki_malloc(sizeof(uki_timer_action));
    sem_t *sem = 0;
    tmp->action = UKI_TIMER_ACTION__DEL_WORK;
    tmp->cb = work->cb;
    tmp->arg = work;

    if (sync && pthread_self() != handle->pid)
    {
        sem = (sem_t *)uki_malloc(sizeof(sem_t));
        sem_init(sem, 0, 1);
        tmp->sem = sem;
    }

    pthread_mutex_lock(&handle->act_mutex);
    uki_list_insert_end(handle->asyn_list, tmp);
#if USE_SEM
    sem_post(&handle->async_sem);
#else
    write(handle->fd[1], "1", 1);
#endif
    pthread_cond_signal(&handle->cont);
    pthread_mutex_unlock(&handle->act_mutex);

    if (sync && pthread_self() != handle->pid)
    {
        sem_wait(sem);
        sem_destroy(sem);
        uki_free(sem);
    }

    return 0;
}
int uki_timer_get_work_remaining_interface(uki_timer_t *handle, uki_timer_work *work, int sync)
{

    uki_timer_action *tmp = (uki_timer_action *) uki_malloc(sizeof(uki_timer_action));
    sem_t *sem = 0;
    //int ret = 0;
    struct timespec *remain = 0, tmpts, nowts, tmptmp;

    tmp->action = UKI_TIMER_ACTION__GET_WORK_REMAINING;
    tmp->cb = work->cb;
    tmp->arg = work;
    sync = 1;

    if (sync)
    {
        sem = (sem_t *)uki_malloc(sizeof(sem_t));
        sem_init(sem, 0, 1);
        tmp->sem = sem;
    }

    tmp->action_mem = remain = (typeof(remain))uki_malloc(sizeof(*remain));
    tmp->action_size = sizeof(*remain);
    pthread_mutex_lock(&handle->act_mutex);
    uki_list_insert_end(handle->asyn_list, tmp);
#if USE_SEM
    sem_post(&handle->async_sem);
#else
    write(handle->fd[1], "1", 1);
#endif
    pthread_cond_signal(&handle->cont);
    pthread_mutex_unlock(&handle->act_mutex);

    if (sync)
    {

        sem_wait(sem);
        sem_destroy(sem);
        uki_free(sem);
        memcpy(&tmpts, remain, sizeof(tmpts));
        uki_free(remain);
    }

    if (tmpts.tv_nsec == 0 && tmpts.tv_sec == 0)
    {
        return 0;
    }

#if IS_NUTTX_IMPL
    clock_gettime(CLOCK_MONOTONIC, &nowts);
#else
    clock_gettime(CLOCK_REALTIME, &nowts);
#endif
    timespec_sub(&tmptmp, &tmpts, &nowts);

    return tmptmp.tv_sec * 1000 + tmptmp.tv_nsec / 1000000;
}
int uki_timer_send_call(uki_timer_t *handle, uki_timer_cb cb, void *arg)
{
    uki_timer_action *tmp = (uki_timer_action *) uki_malloc(sizeof(uki_timer_action));
    tmp->action = UKI_TIMER_ACTION__EXTERN_CALL;
    tmp->cb = cb;
    tmp->arg = arg;

    pthread_mutex_lock(&handle->act_mutex);
    uki_list_insert_end(handle->asyn_list, tmp);
#if USE_SEM
    sem_post(&handle->async_sem);
#else
    write(handle->fd[1], "1", 1);
#endif
    pthread_cond_signal(&handle->cont);
    pthread_mutex_unlock(&handle->act_mutex);
    return 0;
}
int uki_timer_run(uki_timer_t *handle)
{

    uki_timer_head_elem *head = 0;
    uki_timer_elem *elem = 0;
    uki_heap_t *heap = 0;
    uki_timer_action *act = 0;

    struct timespec tmpsub = {0};
    struct timespec tsnow = {0};

    if (!handle)
    {
        return -1;
    }

    handle->pid = pthread_self();
    heap = handle->hl;


    while (1)
    {
#if USE_SEM

        while (sem_trywait(&handle->async_sem) == 0)
#else
        char asyncdata = 0;

        while (1 == read(handle->fd[0], &asyncdata, 1))
#endif
        {
            int need_exit = 0;
            pthread_mutex_lock(&handle->act_mutex);
            act = (typeof(act))(size_t)uki_list_pop(handle->asyn_list);
            pthread_mutex_unlock(&handle->act_mutex);
            clock_gettime(CLOCK_MONOTONIC, &tsnow);
#if TIMER_DEBUG
            BT_INFO("%ld recv action %d", tsnow.tv_sec, act->action);
#endif

            switch (act->action)
            {
                case UKI_TIMER_ACTION__ADD:
                case UKI_TIMER_ACTION__ADD_WORK:
#if TIMER_DEBUG
                    BT_INFO("UKI_TIMER_ACTION__ADD %d cb %p %p", act->action, act->cb, ((uki_timer_work *)(act->arg))->cb);
#endif
                    uki_timer_add(handle, act->cb, act->arg, act->ms, act->is_persist);
                    break;

                case UKI_TIMER_ACTION__DEL:
                case UKI_TIMER_ACTION__DEL_WORK:
                    uki_timer_del(handle, act->cb, act->arg);
                    break;

                case UKI_TIMER_ACTION__GET_WORK_REMAINING:
                {
                    int i = 0;
                    int ret = 0;
                    uki_list_t *list = 0;
                    memset(act->action_mem, 0, act->action_size);

                    for (i = 0; i < handle->max_head_count; ++i)
                    {
                        list = handle->head_list[i].list;

                        ret = uki_list_is_exist_with_cb_with_action(list, timerlist_find, act->arg, 1);

                        if (ret == 0)
                        {
                            memcpy(act->action_mem, &handle->head_list[i].ts, act->action_size);
                            break;
                        }
                    }


                }
                break;

                case UKI_TIMER_ACTION__DESTORY:
                    uki_timer_destroy(handle);
                    need_exit = 1;
                    break;

                case UKI_TIMER_ACTION__EXTERN_CALL:
                    (*act->cb)(act->arg);
                    break;

                default:
                    break;
            }


            if (act->sem)
            {
                sem_post(act->sem);
            }

            uki_free(act);

            if (need_exit)
            {
                BT_INFO("timer exit");
                return 0;
            }
        }

#if IS_NUTTX_IMPL
        clock_gettime(CLOCK_MONOTONIC, &tsnow);
#else
        clock_gettime(CLOCK_REALTIME, &tsnow);
#endif
        head = (typeof(head))uki_heap_top(heap);

        timespec_print(&tsnow, "now 2");

        if (!head)
        {
#if TIMER_DEBUG
            BT_INFO("wait signal");
#endif

#if CUR_PROJECT == PROJ_NUTTX
            pthread_mutex_lock(&handle->act_mutex);

            while (!uki_list_count(handle->asyn_list))
            {
                pthread_cond_wait(&handle->cont, &handle->act_mutex);
            }

            pthread_mutex_unlock(&handle->act_mutex);
#elif CUR_PROJECT == PROJ_MBED
            mbed_thread_wait(UKI_THREAD_SIG_FLAG, 0);
#else
#error "not impl"
#endif
            continue;

        }
        else if (timespec_compare(&tsnow, &head->ts) < 0)
        {
            int ret = 0;
            struct timespec tmpts;

            //timespec_sub(&tmpsub, &head->ts, &tsnow);
            tmpts = head->ts;
            timespec_print(&tsnow, "now");
            timespec_print(&tmpts, "wait to");


#if CUR_PROJECT == PROJ_NUTTX


            ret = pthread_mutex_lock(&handle->act_mutex);

            while (1)
            {

                ret = pthread_cond_timedwait(&handle->cont, &handle->act_mutex, &tmpts);

                if (ret == ETIMEDOUT)
                {
                    break;
                }
                else if (ret == 0)
                {
                    if (uki_list_count(handle->asyn_list))
                    {
                        break;
                    }
                }
                else
                {
                    BT_INFO("ret %d waitms %ld errno %d %s", ret, (long)(tmpsub.tv_sec * 1000 + tmpsub.tv_nsec / 1000000),
                          errno, strerror(errno));
                    assert(0);
                }
            }

            ret = pthread_mutex_unlock(&handle->act_mutex);

#elif CUR_PROJECT == PROJ_MBED
            int tmpmsmbed = mbed_cover_abstime2retime(&tmpts);

            if (tmpmsmbed <= 0)
            {
                break;
            }

            ret = mbed_thread_wait(UKI_THREAD_SIG_FLAG, tmpmsmbed);

            if ( !(ret & UKI_THREAD_SIG_FLAG))
            {
                break;
            }
            else if (ret & UKI_THREAD_SIG_FLAG)
            {
                break;
            }
            else
            {
                BT_INFO("flags_wait_all_for err");
                assert(0);
            }

#else
#error "not impl"
#endif
            //usleep(tmpsub.tv_sec * 1000000 + tmpsub.tv_nsec / 1000);
            continue;
        }
        else
        {
            int loop = 1;
            head = (typeof(head))uki_heap_pop(heap);

            do
            {

                elem = (typeof(elem))uki_list_pop(head->list);

                if (!elem)
                {
                    break;
                }

#if TIMER_DEBUG
                BT_INFO("elem->cb before %p", elem->cb);
#endif

                if (elem->cb)
                {
                    (*elem->cb)(elem->arg);
                }

#if TIMER_DEBUG
                BT_INFO("elem->cb after");
#endif

                if (!uki_list_count(head->list))
                {
                    loop = 0;
                    handle->used_head_count--;
                }

                if (elem->is_persist)
                {
                    assert(0 == uki_timer_add(handle, elem->cb, elem->arg, elem->ms, 1));
                }

                uki_free(elem);

            }
            while (loop);


        }
    }

    return 0;
}

#if 1
#include <pthread.h>
static uki_timer_t *testhandle = 0;
typedef struct
{
    char name[16];
    struct timespec ts;
} zc_work;

#define SET_TIMER_FUNC(EXPR) \
    static int EXPR(void *arg)   \
    {   \
        static struct timespec ts = {0};    \
        struct timespec cur = {0};  \
        struct timespec tmp = {0};  \
        clock_gettime(CLOCK_MONOTONIC, &cur);   \
        timespec_sub(&tmp, &cur, &ts);  \
        ts = cur;   \
        BT_INFO("curtime %ld cbarg %s lastsub %ld heapcount %d",cur.tv_sec, (char *)arg, (long)(tmp.tv_sec * 1000 + tmp.tv_nsec / 1000000), testhandle->hl->ccount); \
        return 0;   \
    }
/*
SET_TIMER_FUNC(testcb1)
SET_TIMER_FUNC(testcb2)
SET_TIMER_FUNC(testcb3)
SET_TIMER_FUNC(testcb4)
SET_TIMER_FUNC(testcb5)
SET_TIMER_FUNC(testcb6)
SET_TIMER_FUNC(testcb7)
*/
static int test_work_arg(void *arg)
{
    zc_work *work = (zc_work *)arg;

    struct timespec cur = {0};
    struct timespec tmp = {0};
    clock_gettime(CLOCK_MONOTONIC, &cur);
    timespec_sub(&tmp, &cur, &work->ts);
    work->ts = cur;
    BT_INFO("cur %ld %ld name %s timegap %ld heapcount %d", cur.tv_sec, cur.tv_nsec / 1000000,
          work->name, (long)(tmp.tv_sec * 1000 + tmp.tv_nsec / 1000000), testhandle->hl->ccount);
    return 0;
}

static void *test(void *arg)
{
    uki_timer_work *work = (uki_timer_work *)arg;
    int count = 5;

    while (count--)
    {

        sleep(3);

        uki_timer_del_work_interface(testhandle, work, 1);

        sleep(5);

        uki_timer_add_work_interface(testhandle, work, 1);




    }

    uki_timer_destory_interface(testhandle, 1);
    //BT_INFO("child thread %ld destroy timer", (long)(size_t)pthread_self());
    //uki_timer_destory_async(testhandle);
    return 0;
}

void uki_timer_test(void)
{
    pthread_t it = 0;
    uki_timer_work *work = 0;
    int i = 0;
    zc_work *tmp = 0;

    testhandle = (uki_timer_t *)uki_timer_create(16);
#if 1

    tmp = 0;

    for (i = 1; i < 6; ++i)
    {
        tmp = (zc_work *)calloc(1, sizeof(zc_work));
        sprintf(tmp->name, "%d", i);
        clock_gettime(CLOCK_MONOTONIC, &tmp->ts);
        work = uki_timer_create_work(test_work_arg, tmp, 1000, 1);
        uki_timer_add_work_interface(testhandle, work, 0);
    }

    for (i = 0; i < 1; ++i)
    {
        tmp = (zc_work *)calloc(1, sizeof(zc_work));
        sprintf(tmp->name, "%d", i);
        clock_gettime(CLOCK_MONOTONIC, &tmp->ts);
        work = uki_timer_create_work(test_work_arg, tmp, 1000, 1);
        uki_timer_add_work_interface(testhandle, work, 0);
    }

#else
    uki_timer_add_interface(testhandle, testcb1, "1", 1000, 1);
    uki_timer_add_interface(testhandle, testcb2, "2", 2000, 0);
    uki_timer_add_interface(testhandle, testcb3, "3", 3000, 0);
    uki_timer_add_interface(testhandle, testcb4, "4", 4000, 0);
    uki_timer_add_interface(testhandle, testcb5, "5", 5000, 0);
    uki_timer_add_interface(testhandle, testcb6, "6", 6000, 0);
    uki_timer_add_interface(testhandle, testcb7, "2", 2000, 0);
    //uki_timer_add_async(testhandle, testdelaywork, "2", 2000, 0);
#endif
    BT_INFO("start father %ld\n", (long)(size_t)pthread_self());
    pthread_create(&it, 0, test, (void *)work);
    uki_timer_run(testhandle);

    pthread_join(it, 0);
}


#endif

