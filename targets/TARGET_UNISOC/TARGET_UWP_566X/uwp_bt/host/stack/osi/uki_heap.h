#ifndef APPS_SYSTEM_BT_HOST_STACK_OSI_UKI_HEAP_H_
#define APPS_SYSTEM_BT_HOST_STACK_OSI_UKI_HEAP_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef struct
{
    int index;
    void *arg;
} uki_heap_elem_t;
typedef struct
{
    int big_or_little;  //0 little 1 big
    int maxcount;
    int ccount;
    int (*compare)(void *arg1, void *arg2);   //0 equle 1 big -1 small
    uki_heap_elem_t *array;
} uki_heap_t;

uki_heap_t *uki_heap_create(int count, int (*compare)(void *arg1, void *arg2), int bigorlittle);
int uki_heap_destroy(uki_heap_t *handle);
int uki_heap_insert(uki_heap_t *handle, void *arg);
void *uki_heap_pop(uki_heap_t *handle);
void *uki_heap_top(uki_heap_t *handle);
int uki_heap_remove(uki_heap_t *handle, void *arg);

void uki_heap_test(void);

#ifdef __cplusplus
}
#endif

#endif /* APPS_SYSTEM_BT_HOST_STACK_OSI_UKI_HEAP_H_ */
