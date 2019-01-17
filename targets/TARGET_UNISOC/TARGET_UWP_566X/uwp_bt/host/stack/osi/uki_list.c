#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "uki_list.h"

#include "uki_log.h"
#include "uki_utils.h"



#define IS_TYPE_SIGNED(my_type) ((((typeof(my_type))(0))-1) < 0)
#define IS_OVERFLOW(x1,x2) ( ((x1) + (x2) < (x1)) || ((x1) + (x2) < (x2)) )



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
uki_list_t *uki_list_create(int is_sync)
{
    uki_list_t *handle = (uki_list_t *)uki_malloc(sizeof(uki_list_t));
    handle->is_sync = is_sync;

    if (handle->is_sync)
    {
        sem_init(&handle->sem, 0, 1);
        pthread_mutex_init(&handle->mutex, 0);
    }

    return handle;
}

int uki_list_destroy(uki_list_t *handle)
{
    uki_list_elem *tmp = handle->first;
    uki_list_elem *next = 0;

    while (tmp)
    {
        next = tmp->next;
        uki_free(tmp);
        tmp = next;
    }

    if (handle->is_sync)
    {
        sem_destroy(&handle->sem);
        pthread_mutex_destroy(&handle->mutex);
    }

    uki_free(handle);
    return 0;
}

int uki_list_insert_first(uki_list_t *handle, void *arg)
{
    uki_list_elem *tmp = (uki_list_elem *)uki_malloc(sizeof(uki_list_elem));
    tmp->arg = arg;


    if (handle->is_sync)
    {
        pthread_mutex_lock(&handle->mutex);
    }

    tmp->next = handle->first;

    if (handle->first)
    {
        handle->first->pre = tmp;
    }
    else
    {
        handle->end = tmp;
    }

    tmp->pre = 0;
    handle->first = tmp;
    handle->count++;

    if (handle->is_sync)
    {
        pthread_mutex_unlock(&handle->mutex);
        sem_post(&handle->sem);
    }


    return 0;
}

int uki_list_insert_end(uki_list_t *handle, void *arg)
{
    uki_list_elem *tmp = (uki_list_elem *)uki_malloc(sizeof(uki_list_elem));
    //assert(tmp);
    tmp->arg = arg;

    if (handle->is_sync)
    {
        pthread_mutex_lock(&handle->mutex);
    }


    tmp->pre = handle->end;

    if (handle->end)
    {
        handle->end->next = tmp;
    }
    else
    {
        handle->first = tmp;
    }

    tmp->next = 0;
    handle->end = tmp;
    handle->count++;

    if (handle->is_sync)
    {
        pthread_mutex_unlock(&handle->mutex);
        sem_post(&handle->sem);
    }

    return 0;
}
void *uki_list_pop_timeout(uki_list_t *handle, int ms)
{
    uki_list_elem *ret = 0;
    void *arg = 0;

    if (handle->is_sync)
    {
        int sem_ret = 0;

        if (ms > 0)
        {
            struct timespec waitt;
            struct timespec now;
            struct timespec real;
            waitt.tv_sec = ms / 1000;
            waitt.tv_nsec = (ms % 1000) * 1000000;
            clock_gettime(CLOCK_REALTIME, &now);
            timespec_plus(&real, &now, &waitt);
            sem_ret = sem_timedwait(&handle->sem, &real);

            if (sem_ret < 0 && errno == EAGAIN)
            {
                goto uki_list_pop_timeout_err;
            }
        }
        else if (ms == 0)
        {
            sem_ret = sem_trywait(&handle->sem);
            if (sem_ret < 0 && errno == ETIMEDOUT)
            {
                goto uki_list_pop_timeout_err;
            }
        }
        else
        {
            sem_ret = sem_wait(&handle->sem);
        }

        pthread_mutex_lock(&handle->mutex);
    }

    ret = handle->first;

    if (!ret)
    {
        if (handle->is_sync)
        {
            pthread_mutex_unlock(&handle->mutex);
        }

        return 0;
    }

    if (ret->next)
    {
        ret->next->pre = ret->pre;
    }
    else
    {
        handle->end = 0;
    }

    handle->first = ret->next;

    arg = ret->arg;
    uki_free(ret);
    handle->count--;

    if (handle->is_sync)
    {
        pthread_mutex_unlock(&handle->mutex);
    }

    return arg;
uki_list_pop_timeout_err:
    return 0;
}
void *uki_list_pop(uki_list_t *handle)
{
    return uki_list_pop_timeout(handle, -1);
}

void *uki_list_first(uki_list_t *handle)
{
    uki_list_elem *ret = 0;

    if (handle->is_sync)
    {
        pthread_mutex_lock(&handle->mutex);
    }

    ret = handle->first;

    if (!ret)
    {
        if (handle->is_sync)
        {
            pthread_mutex_unlock(&handle->mutex);
        }

        return 0;
    }

    if (handle->is_sync)
    {
        pthread_mutex_unlock(&handle->mutex);
    }

    return ret->arg;
}

int uki_list_is_exist(uki_list_t *handle, void *arg)
{
    uki_list_elem *tmp = 0;
    int ret = 0;

    if (handle->is_sync)
    {
        pthread_mutex_lock(&handle->mutex);
    }

    tmp = handle->first;

    while (tmp)
    {
        tmp = tmp->next;

        if (tmp->arg == arg)
        {
            ret = 1;
        }
    }

    if (handle->is_sync)
    {
        pthread_mutex_unlock(&handle->mutex);
    }

    return ret;
}

int uki_list_merge(uki_list_t *h1, uki_list_t *h2)
{
    void *arg = 0;

    if (!h1 || !h2)
    {
        return -1;
    }

    while (arg = uki_list_pop(h2))
    {
        uki_list_insert_end(h1, arg);
    }

    uki_list_destroy(h2);

    return 0;
}

void *uki_list_find_index(uki_list_t *handle, int index)
{
    int i = 0;
    uki_list_elem *elem = 0;

    if (handle->is_sync)
    {
        pthread_mutex_lock(&handle->mutex);
    }


    if (index >= handle->count)
    {
        if (handle->is_sync)
        {
            pthread_mutex_unlock(&handle->mutex);
        }

        return 0;
    }

    //arg = handle->first;

    for (i = 0; i < index; i++)
    {
        elem = elem->next;
    }

    if (handle->is_sync)
    {
        pthread_mutex_unlock(&handle->mutex);
    }

    return elem->arg;
}

//action 0 nothing 1 remove
int uki_list_is_exist_with_cb_with_action(uki_list_t *handle, int (*compare)(void *, void *), void *arg, int action)
{
    uki_list_elem *tmp = 0 ;
    uki_list_elem *next = 0;
    int ret  = 0;

    if (handle->is_sync)
    {
        pthread_mutex_lock(&handle->mutex);
    }

    tmp =  handle->first;

    while (tmp)
    {
        next = tmp->next;

        if ((*compare)(tmp->arg, arg))
        {
            ret = 1;

            switch (action)
            {
                case 0:
                    break;

                case 1:

                    if (tmp->next)
                    {
                        tmp->next->pre = tmp->pre;
                    }
                    else
                    {
                        handle->end = tmp->pre;
                    }

                    if (tmp->pre)
                    {
                        tmp->pre->next = tmp->next;
                    }
                    else
                    {
                        handle->first = tmp->next;
                    }

                    uki_free(tmp);
                    handle->count--;
                    break;

                default:
                    break;
            }
        }

        tmp = next;
    }

    if (handle->is_sync)
    {
        pthread_mutex_unlock(&handle->mutex);
    }

    return ret;
}

int uki_list_remove(uki_list_t *handle, void *arg)
{
    uki_list_elem *tmp = 0;
    uki_list_elem *next = 0;

    if (handle->is_sync)
    {
        pthread_mutex_lock(&handle->mutex);
    }

    tmp = handle->first;

    while (tmp)
    {
        next = tmp->next;

        if (tmp->arg == arg)
        {
            if (tmp->pre)
            {
                tmp->pre->next = next;
            }
            else
            {
                handle->first = next;
            }

            if (next)
            {
                next->pre = tmp->pre;
            }
            else
            {
                handle->end = tmp->pre;
            }

            uki_free(tmp);
            handle->count--;
        }


        tmp = next;
    }

    if (handle->is_sync)
    {
        pthread_mutex_unlock(&handle->mutex);
    }

    return 0;
}

int uki_list_remove_index(uki_list_t *handle, int index)
{
    int i = 0;
    uki_list_elem *elem = 0, *pelem = 0;

    if (handle->is_sync)
    {
        pthread_mutex_lock(&handle->mutex);
    }


    if (!handle || index < 0 || index >= handle->count)
    {
        if (handle->is_sync)
        {
            pthread_mutex_unlock(&handle->mutex);
        }

        return -1;
    }

    for (i = 0, elem = uki_list_first(handle); i < index; ++i)
    {
        pelem = elem;
        elem = elem->next;
    }

    if (pelem)
    {
        pelem->next = elem->next;
    }
    else
    {
        handle->first = elem->next;
    }

    if (elem->next)
    {
        elem->next = pelem;
    }
    else
    {
        handle->end = elem->pre;
    }

    //arg = elem->arg;
    uki_free(elem);
    handle->count--;

    if (handle->is_sync)
    {
        pthread_mutex_unlock(&handle->mutex);
    }

    return 0;

}

int uki_list_count(uki_list_t *handle)
{
    int count = 0;

    if (handle->is_sync)
    {
        pthread_mutex_lock(&handle->mutex);
    }

    count = handle->count;

    if (handle->is_sync)
    {
        pthread_mutex_unlock(&handle->mutex);
    }

    return count;
}

#if 1
static int testc(void *arg1, void *arg2)
{
    return (int)(size_t)arg1 == (int)(size_t)arg2;
}
void uki_list_test(void)
{
    int i = 0;

    while (i++ < 8)
    {
        uki_list_t *handle = uki_list_create(0);
        int arg  = 0;
        uki_list_insert_first(handle, (void *)1);
        uki_list_insert_first(handle, (void *)2);
        uki_list_insert_first(handle, (void *)3);
        uki_list_insert_first(handle, (void *)4);
        uki_list_insert_first(handle, (void *)5);
        uki_list_insert_first(handle, (void *)6);
        uki_list_insert_first(handle, (void *)7);
        uki_list_insert_first(handle, (void *)4);
        uki_list_insert_first(handle, (void *)6);
        uki_list_remove(handle, (void *)6);
        uki_list_is_exist_with_cb_with_action(handle, testc, (void *)4, 1);

        while ((arg = (typeof(arg))(size_t)uki_list_pop(handle)))
        {
        }

        uki_list_destroy(handle);
    }

    while (i++ < 8)
    {
        uki_list_t *handle = uki_list_create(1);
        int arg  = 0;
        uki_list_insert_first(handle, (void *)1);
        uki_list_insert_first(handle, (void *)2);
        uki_list_insert_first(handle, (void *)3);
        uki_list_insert_first(handle, (void *)4);
        uki_list_insert_first(handle, (void *)5);
        uki_list_insert_first(handle, (void *)6);
        uki_list_insert_first(handle, (void *)7);
        uki_list_insert_first(handle, (void *)4);
        uki_list_insert_first(handle, (void *)6);
        uki_list_remove(handle, (void *)6);
        uki_list_is_exist_with_cb_with_action(handle, testc, (void *)4, 1);

        while ((arg = (typeof(arg))(size_t)uki_list_pop(handle)))
        {
        }

        uki_list_destroy(handle);
    }

}
#endif
