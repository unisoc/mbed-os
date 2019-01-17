
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#ifdef __CYGWIN__
#else
//#include <sys/prctl.h>
#endif

#include "uki_work.h"
#include "uki_log.h"
#include "uki_utils.h"
#define WORK_DEBUG 0

static int work_list_cb(void *arg);
static int work_delay_list_cb(void *arg);

uki_work_t *uki_work_create(void)
{
    uki_work_t *handle = 0;
    handle = (typeof(handle)) uki_malloc(sizeof(uki_work_t));
    assert(handle);
    handle->timer = uki_timer_create(32);

    if (handle->timer == 0)
    {
        BT_INFO("handle->timer null");
    }

    //handle->list = uki_list_create();

    return handle;
}

int uki_work_destroy(uki_work_t *handle, int sync)
{
    if(handle && handle->timer)
    {
        uki_timer_destory_interface(handle->timer, sync);
    }

    /*
        while (elem = (uki_work_elem *)uki_list_pop(handle->list))
        {
            uki_work_elem_free(elem);
        }

        uki_list_destroy(handle->list);
        */
    uki_free(handle);
    return 0;
}

int uki_work_elem_create(uki_work_elem *elem, uki_work_cb cb, void *arg, int delayms)
{
    uki_timer_work *twork = 0;
    memset(elem, 0, sizeof(*elem));
    elem->arg = arg;
    elem->cb = cb;
#if WORK_DEBUG
    zclog("=== create elem %p", elem);
#endif



    if (delayms <= 0)
    {
        elem->is_delay_work = 0;
    }
    else
    {
        elem->is_delay_work = 1;
        twork = uki_timer_create_work(work_delay_list_cb, elem, delayms, 0);
        assert(twork);
#if WORK_DEBUG
        zclog("create twork %p", twork);
#endif
        if (twork == 0)
        {
            zclog("handle->timer null");
        }

        elem->twork = twork;
    }

    return 0;
}

int uki_work_elem_free(uki_work_elem *handle)
{
#if WORK_DEBUG
    zclog("=== free elem %p", handle);
#endif
    if (handle->twork)
    {
        uki_timer_work_free(handle->twork);
    }

    memset(handle, 0, sizeof(*handle));
    //uki_free(handle);
    return 0;
}

int uki_work_add(uki_work_t *handle, uki_work_elem *elem)
{
    int ret = 0;
    elem->whandle = handle;
#if WORK_DEBUG
    zclog("=== add elem %p", elem);
#endif
    if (elem->is_delay_work == 0)
    {
        if (!elem->flag)
        {
            elem->flag = 1;
            ret = uki_timer_send_call(handle->timer, work_list_cb, elem);
        }

    }
    else
    {
        if (elem->flag)
        {
            uki_timer_del_work_interface(handle->timer, elem->twork, 1);
        }

        elem->flag = 1;
#if WORK_DEBUG
        zclog("twork cb %p workelem cb %p", elem->twork->cb, elem->cb);
#endif
        ret = uki_timer_add_work_interface(handle->timer, elem->twork, 0);

    }

    return ret;
}

int uki_work_del(uki_work_t *handle, uki_work_elem *elem, int sync)
{
    int ret = 0;
#if WORK_DEBUG
    zclog("=== del elem %p", elem);
#endif
    if (elem->is_delay_work == 1)
    {
        ret = uki_timer_del_work_interface(handle->timer, elem->twork, sync);
        //uki_timer_work_free(elem->twork);
    }

    return ret;
}
int uki_work_get_remaining(uki_work_t *handle, uki_work_elem *elem)
{
    if (elem->is_delay_work)
    {
        return uki_timer_get_work_remaining_interface(handle->timer, elem->twork, 1);
    }

    return 0;
}
static int work_list_cb(void *arg)
{
    uki_work_elem *elem = (uki_work_elem *) arg;

    if (elem->cb)
    {
        (*elem->cb)(elem);
    }


    elem->flag = 0;

    return 0;
}

static int work_delay_list_cb(void *arg)
{
    uki_work_elem *elem = (uki_work_elem *) arg;


    if (elem->cb)
    {
        (*elem->cb)(arg);
    }

    elem->flag = 0;
    return 0;
}

static void *work_thread(const void *arg)
{
    uki_work_t *handle = (uki_work_t *)arg;

    uki_timer_run(handle->timer);
    return 0;
}

pthread_t uki_work_run(uki_work_t *handle, char *name)
{
    pthread_create(&handle->pt, 0, work_thread, (void *)handle);
#if defined (__CYGWIN__)
#else
    pthread_setname_np(handle->pt, name);
#endif
    return handle->pt;
}

#if 1
static uki_work_t *testhandle;

static inline int timespec_sub(struct timespec *result, struct timespec *v1, struct timespec *v2 )
{
    result->tv_sec = v1->tv_sec - v2->tv_sec;
    result->tv_nsec = v1->tv_nsec - v2->tv_nsec;

    if (result->tv_nsec < 0)
    {
        result->tv_nsec += 1000000000;
        result->tv_sec--;
    }

    return 0;
}
#define SET_TIMER_FUNC(EXPR) \
    static int EXPR(void *arg)   \
    {   \
        uki_work_elem *elem = (typeof(elem))arg;\
        static struct timespec ts = {0};    \
        struct timespec cur = {0};  \
        struct timespec tmp = {0};  \
        clock_gettime(CLOCK_MONOTONIC, &cur);   \
        timespec_sub(&tmp, &cur, &ts);  \
        ts = cur;   \
        zclog("cbarg %s lastsub %ld heapcount", (char *)(elem->arg), (long)(tmp.tv_sec * 1000 + tmp.tv_nsec / 1000000)); \
        return 0;   \
    }

SET_TIMER_FUNC(testcb1)
/*
SET_TIMER_FUNC(testcb2)
SET_TIMER_FUNC(testcb3)
SET_TIMER_FUNC(testcb4)
SET_TIMER_FUNC(testcb5)
SET_TIMER_FUNC(testcb6)
SET_TIMER_FUNC(testcb7)
*/
static uki_work_elem s_elem;
static void kwork_cb_test(k_work *arg)
{
    k_work *work = (k_work *)arg;
    static struct timespec ts = {0};
    struct timespec cur = {0};
    struct timespec tmp = {0};
    clock_gettime(CLOCK_MONOTONIC, &cur);
    timespec_sub(&tmp, &cur, &ts);
    ts = cur;
    zclog("cbarg %p lastsub %ld heapcount", work->arg, (long)(tmp.tv_sec * 1000 + tmp.tv_nsec / 1000000));
}

void uki_work_test(void)
{

    pthread_t pid;
    int i = 0;
    testhandle = uki_work_create();

    memset(&s_elem, 0, sizeof(s_elem));
    uki_work_elem_create(&s_elem, testcb1, "1zczczczczczczczcz", 1000);
    uki_work_add(testhandle, &s_elem);
    pid = uki_work_run(testhandle, "test work thread");

    for (i = 0; i < 5; ++i)
    {
        sleep(2);
        uki_work_add(testhandle, &s_elem);
    }

    sleep(2);

    uki_work_add(testhandle, &s_elem);
    uki_work_del(testhandle, &s_elem, 1);
    sleep(2);

    uki_work_destroy(testhandle, 0);
    pthread_join(pid, 0);
    zclog("main thread exit");

    {
        testhandle = uki_work_create();
        memset(&s_elem, 0, sizeof(s_elem));
        k_delayed_work_init(&s_elem, kwork_cb_test);
        k_delayed_work_submit_to_queue(testhandle, &s_elem, 1000);

        pid = (typeof(pid))k_work_q_start(testhandle, 0, 0, 0);

        for (i = 0; i < 5; ++i)
        {
            sleep(2);
            //uki_work_add(testhandle, &elem);
            zclog("k_delayed_work_submit_to_queue");
            k_delayed_work_submit_to_queue(testhandle, &s_elem, 1000);
            zclog("k_delayed_work_cancel");
            k_delayed_work_cancel(&s_elem);
            k_delayed_work_cancel(&s_elem);
        }

        sleep(2);

        uki_work_add(testhandle, &s_elem);
        uki_work_del(testhandle, &s_elem, 1);
        sleep(2);

        uki_work_destroy(testhandle, 1);
        pthread_join(pid, 0);
        zclog("main thread exit");
    }



}


size_t k_work_q_start(struct k_work_q *work_q, k_thread_stack_t *stack,
                                    size_t stack_size, int prio)
{
    return (size_t)uki_work_run(work_q, "k_work_pthread");
}

void k_work_init_inter(struct k_work *work, k_work_handler_t handler)
{
    uki_work_elem_create(work, (uki_work_cb)handler, work, 0);
}



void k_work_submit(struct k_work *work)
{
    uki_work_add(g_default_work, work);
}

void k_work_submit_to_queue(struct k_work_q *work_q, struct k_work *work)
{
    uki_work_add(work_q, work);
}

void k_delayed_work_init_inter(struct k_delayed_work *work,
        k_work_handler_t handler)
{
    uki_work_elem_create(work, (uki_work_cb)handler, work, 0);
}



int k_delayed_work_submit_inter(struct k_delayed_work *work,
        int delay)
{
    k_delayed_work tmp;
    memset(&tmp, 0, sizeof(tmp));
    memcpy(&tmp, work, sizeof(tmp));

    uki_work_del(work->whandle, work, 1);
    uki_work_elem_free(work);
    uki_work_elem_create(work, (uki_work_cb)tmp.cb, (void *)tmp.arg, delay);

    return uki_work_add(g_default_work, work);
}



int k_delayed_work_submit_to_queue(struct k_work_q *work_q,
        struct k_delayed_work *work,
        int delay)
{
    k_delayed_work tmp;
    memset(&tmp, 0, sizeof(tmp));
    memcpy(&tmp, work, sizeof(tmp));
    uki_work_del(work->whandle, work, 1);
    uki_work_elem_free(work);
    uki_work_elem_create(work, (uki_work_cb)tmp.cb, (void *)tmp.arg, delay);
    return uki_work_add(work_q, work);
}

int k_delayed_work_cancel(struct k_delayed_work *work)
{
    if (!work->whandle)
    {
        return 0;
    }

    return uki_work_del(work->whandle, work, 1);
}
int k_work_pending(struct k_work *work)
{
    return work->flag;
}
int k_delayed_work_remaining_get(struct k_delayed_work *work)
{
    return uki_work_get_remaining(g_default_work, work);
}

#endif
