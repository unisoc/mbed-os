#ifndef __UKI_ABS__H
#define __UKI_ABS__H
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
//0 nuttx 1 mbed
#define PROJ_NUTTX 0
#define PROJ_MBED 1

#define CUR_PROJECT PROJ_MBED


#define UKI_THREAD_SIG_FLAG 1

void uki_abs_init();

//for pthread
typedef int pthread_t;

typedef struct pthread_attr_t
{
  unsigned char priority;            /* Priority of the pthread */
  unsigned char policy;              /* Pthread scheduler policy */
  //uint8_t inheritsched;        /* Inherit parent priority/policy? */


  //uint8_t low_priority;        /* Low scheduling priority */
  //uint8_t max_repl;            /* Maximum pending replenishments */



  //cpu_set_t affinity;          /* Set of permitted CPUs for the thread */


  unsigned int stacksize;            /* Size of the stack allocated for the pthread */


  //struct timespec repl_period; /* Replenishment period */
  //struct timespec budget;      /* Initial budget */
  int sched_priority;
}pthread_attr_t;

struct sched_param
{
  int sched_priority;                   /* Base thread priority */


  int sched_ss_low_priority;            /* Low scheduling priority for sporadic
                                         * server */
  //struct timespec sched_ss_repl_period; /* Replenishment period for sporadic * server. */
  //struct timespec sched_ss_init_budget; /* Initial budget for sporadic server */
  int sched_ss_max_repl;                /* Maximum pending replenishments for
                                         * sporadic server. */
};



typedef struct pthread_mutex_t
{
    void *p;
} pthread_mutex_t;


typedef struct pthread_mutexattr_t
{
    void *p;
} pthread_mutexattr_t;


typedef struct pthread_cond_t
{
    void *p;
} pthread_cond_t;

typedef struct pthread_condattr_t
{
    void *p;
} pthread_condattr_t;



int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (const void *), void *arg);

int pthread_join(pthread_t thread, void **retval);
pthread_t pthread_self(void);
int pthread_setname_np(pthread_t thread, const char *name);
int pthread_detach(pthread_t thread);

int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);

//todo: cant use
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *cond_attr);
//int pthread_cond_signal(pthread_cond_t *cond);
//int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
//int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);


int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

int mbed_thread_wait(int sig, int ms);
int mbed_thread_signal(pthread_t pid, int flag);
//for sem


typedef struct sem_t
{
    void *p;
} sem_t;


int sem_init(sem_t *sem, unsigned int value, unsigned int max);

int sem_destroy(sem_t *sem);

int sem_post(sem_t *sem);

int sem_wait(sem_t *sem);

int sem_trywait(sem_t *sem);

int sem_timedwait(sem_t *sem, const struct timespec *re_timeout);

int mbed_cover_abstime2retime(struct timespec *abs_timeout);

//for time
//
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

int clock_gettime(unsigned char clk_id, struct timespec *tp);



#ifdef __cplusplus
}
#endif

#endif
