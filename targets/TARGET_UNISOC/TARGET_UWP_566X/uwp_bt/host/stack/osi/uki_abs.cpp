#include "mbed.h"
#include "uki_abs.h"
#include <stdlib.h>
typedef struct thread_arg
{
    void *(*func) (const void *);
    void *arg;
} thread_arg;

static Timer globel_timer;


static void s_thread_func(const void *arg)
{
    thread_arg *tmp = (typeof(tmp))arg;
    (*tmp->func)(tmp->arg);
    free(tmp);
    return;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (const void *), void *arg)
{
    thread_arg *atmp = (typeof(atmp))calloc(sizeof(*atmp), 1);
    atmp->func = start_routine;
    atmp->arg = arg;
    //todo: priority
    Thread *tmp =NULL;
    if(attr)
        tmp = new Thread(s_thread_func, atmp, osPriorityNormal, attr->stacksize);
    else
        tmp = new Thread(s_thread_func, atmp, osPriorityNormal, 1024);
    *thread = (typeof(*thread))tmp;
    return 0;
}

int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param)
{
    if(!attr || !param)
    {
        return -1;
    }

    attr->sched_priority = param->sched_priority;
    return 0;
}

int pthread_join(pthread_t thread, void **retval)
{
    Thread *tmp = reinterpret_cast<Thread *>(thread);
    return tmp->join();

}
pthread_t pthread_self(void)
{
    //todo: cant not impl
    return 0;
}
int pthread_setname_np(pthread_t thread, const char *name)
{
    //todo: cant not impl
    return 0;
}
int pthread_detach(pthread_t thread)
{
    return 0;
}
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *cond_attr)
{
    return 0;
}

int mbed_thread_signal(pthread_t pid, int flag)
{
    Thread *tmp = reinterpret_cast<Thread *>(pid);
    return tmp->flags_set(flag);
}

#if 0
int pthread_cond_signal(pthread_cond_t *cond)
{
    return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{

    return 0;
}
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    return 0;
}
#endif
int pthread_cond_destroy(pthread_cond_t *cond)
{
    return 0;
}

int pthread_condattr_init(pthread_condattr_t *attr)
{
    return 0;
}
int pthread_condattr_destroy(pthread_condattr_t *attr)
{
    return 0;
}
int mbed_thread_wait(int sig, int ms)
{
    return ::rtos::ThisThread::flags_wait_all_for(sig, ms);
}



int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *arg)
{
    Mutex *tmp = new Mutex();
    mutex->p = tmp;
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    Mutex *tmp = reinterpret_cast<Mutex *>(mutex->p);
    return tmp->lock();
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    Mutex *tmp = reinterpret_cast<Mutex *>(mutex->p);
    return tmp->unlock();
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    Mutex *tmp = reinterpret_cast<Mutex *>(mutex->p);
    delete tmp;
    return 0;
}


int sem_init(sem_t *sem, unsigned int value, unsigned int max)
{
    Semaphore *tmp = 0;
    if(max == 0)
    {
        tmp = new Semaphore(value);
    }
    else
    {
        tmp = new Semaphore(value, max);
    }

    (sem->p) = tmp;
    return 0;
}

int sem_destroy(sem_t *sem)
{
    Semaphore *tmp = reinterpret_cast<Semaphore *>(sem->p);
    delete tmp;
    tmp = NULL;
    sem = NULL;
    return 0;
}

int sem_post(sem_t *sem)
{
    Semaphore *tmp = reinterpret_cast<Semaphore *>(sem->p);
    tmp->release();
    return 0;
}

int sem_wait(sem_t *sem)
{
    Semaphore *tmp = reinterpret_cast<Semaphore *>(sem->p);
    tmp->wait(osWaitForever);
    return 0;
}

int sem_trywait(sem_t *sem)
{
    Semaphore *tmp = reinterpret_cast<Semaphore *>(sem->p);
    return tmp->wait_until(0);
}

int sem_timedwait(sem_t *sem, const struct timespec *re_timeout)
{
    Semaphore *tmp = reinterpret_cast<Semaphore *>(sem->p);
    int us = globel_timer.read_us();
    us = re_timeout->tv_sec * 1000000 + re_timeout->tv_nsec / 1000 + us;

    if (us < 0)
    {
        us = 0;
    }
    return tmp->wait_until(us / 1000);
}

int mbed_cover_abstime2retime(struct timespec *abs_timeout)
{
    int us = globel_timer.read_us();
    us = abs_timeout->tv_sec * 1000000 + abs_timeout->tv_nsec / 1000 - us;
    return us / 1000;
}

int clock_gettime(unsigned char clk_id, struct timespec *tp)
{
    int us = globel_timer.read_us();
    tp->tv_sec = us / 1000000;
    tp->tv_nsec = (us % 1000000) * 1000;
    return 0;
}


void uki_abs_init()
{
    globel_timer.start();
}
