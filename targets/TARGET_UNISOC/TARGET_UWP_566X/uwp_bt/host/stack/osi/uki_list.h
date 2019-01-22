#ifndef UKI_LIST_H_
#define UKI_LIST_H_
#include <pthread.h>
//#include <semaphore.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct uki_list_elem
{
    void *arg;
    struct uki_list_elem *next;
    struct uki_list_elem *pre;
} uki_list_elem;


typedef struct uki_list_t
{
    int count;
    int is_sync;
    pthread_mutex_t mutex;
    sem_t sem;
    uki_list_elem *first;
    uki_list_elem *end;
} uki_list_t;
uki_list_t *uki_list_create(int is_sync);
int uki_list_destroy(uki_list_t *handle);
int uki_list_insert_first(uki_list_t *handle, void *arg);
int uki_list_insert_end(uki_list_t *handle, void *arg);
void *uki_list_first(uki_list_t *handle);
void *uki_list_pop_timeout(uki_list_t *handle, int ms);
void *uki_list_pop(uki_list_t *handle);
int uki_list_merge(uki_list_t *h1, uki_list_t *h2);
int uki_list_is_exist(uki_list_t *handle, void *arg);
void *uki_list_find_index(uki_list_t *handle, int index);
int uki_list_is_exist_with_cb_with_action(uki_list_t *handle, int (*compare)(void *, void *), void *arg, int action);
int uki_list_remove(uki_list_t *handle, void *arg);
int uki_list_remove_index(uki_list_t *handle, int index);
int uki_list_count(uki_list_t *handle);
void uki_list_test(void);

#ifdef __cplusplus
}
#endif
#endif /* UKI_LIST_H_ */
