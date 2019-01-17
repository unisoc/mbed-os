#include <stdio.h>
#include <stdlib.h>
#include "uki_heap.h"
#include "uki_log.h"
#include "uki_utils.h"





static int compare_heap(uki_heap_t *handle, int index1, int index2)
{
    if (0 > index1 || 0 > index2)
    {
        return -2;
    }

    if (index1 >= handle->ccount || index2 >= handle->ccount)
    {
        return -2;
    }

    return (*handle->compare)(handle->array[index1].arg, handle->array[index2].arg);
}

static int swap_elem(uki_heap_t *handle, int index1, int index2)
{
    void *tmp = 0;

    if (0 > index1 || 0 > index2 || index1 == index2)
    {
        return -2;
    }

    if (index1 >= handle->ccount || index2 >= handle->ccount)
    {
        return -2;
    }

    //handle->array[index1].index = index2;
    //handle->array[index2].index = index1;
    tmp = handle->array[index1].arg;
    handle->array[index1].arg = handle->array[index2].arg;
    handle->array[index2].arg = tmp;

    return 0;
}

static int adjust_up(uki_heap_t *handle, int index)
{
    int ret = 0;

    if (0 > index)
    {
        return -1;
    }

    if (index >= handle->ccount)
    {
        return -1;
    }

    ret = compare_heap(handle, index, (index - 1) / 2);

    if ((handle->big_or_little == 1 && ret == 1) ||
            (handle->big_or_little == 0 && ret == -1)
       )
    {
        swap_elem(handle, index, (index - 1) / 2);
        adjust_up(handle, (index - 1) / 2);
    }
    else
    {
        return -1;
    }

    return 0;

}
static int adjust_down(uki_heap_t *handle, int index)
{
    int ret = 0;
    int maxindex = 0, minindex = 0;
    int downindex = 0;

    if (0 > index)
    {
        return -1;
    }

    if (index * 2 + 2 < handle->ccount)
    {
        ret = compare_heap(handle, index * 2 + 1, index * 2 + 2);

        switch (ret)
        {
            case 1:
                maxindex = index * 2 + 1;
                minindex = index * 2 + 2;
                break;

            case -1:
                maxindex = index * 2 + 2;
                minindex = index * 2 + 1;
                break;

            default:
            case 0:
                maxindex = index * 2 + 1;
                minindex = index * 2 + 1;
                break;
        }

        if (handle->big_or_little == 1 )
        {
            downindex = maxindex;
        }

        else
        {
            downindex = minindex;
        }

    }
    else if (index * 2 + 1 < handle->ccount)
    {
        downindex = index * 2 + 1;
    }
    else
    {
        return -1;
    }


    ret = compare_heap(handle, index, downindex);

    if ((handle->big_or_little == 1 && ret == -1) ||
            (handle->big_or_little == 0 && ret == 1)
       )
    {
        swap_elem(handle, index, downindex);
        adjust_down(handle, downindex);
    }
    else
    {
        return -1;
    }

    return 0;

}

uki_heap_t *uki_heap_create(int count, int (*compare)(void *arg1, void *arg2), int bigorlittle)
{
    uki_heap_t *tmp = (uki_heap_t *)uki_malloc(sizeof(*tmp) * count);

    if (!tmp)
    {
        return 0;
    }

    tmp->array = (uki_heap_elem_t *)uki_malloc(sizeof(uki_heap_elem_t) * count);
    tmp->maxcount = count;
    tmp->compare = compare;
    tmp->big_or_little = bigorlittle;
    return tmp;
}

int uki_heap_destroy(uki_heap_t *handle)
{
    uki_free(handle->array);
    uki_free(handle);
    return 0;
}

int uki_heap_insert(uki_heap_t *handle, void *arg)
{
    int index = 0;

    if (!handle || !arg || handle->ccount == handle->maxcount)
    {
        return -1;
    }

    handle->array[handle->ccount].arg = arg;
    index = handle->array[handle->ccount].index = handle->ccount;
    handle->ccount++;
    adjust_up(handle, index);
    return 0;
}

void *uki_heap_pop(uki_heap_t *handle)
{
    void *tmp = 0;

    if (!handle || handle->ccount == 0)
    {
        return 0;
    }

    tmp = handle->array[0].arg;
    swap_elem(handle, 0, handle->ccount - 1);
    handle->ccount--;
    adjust_down(handle, 0);
    return tmp;
}

void *uki_heap_top(uki_heap_t *handle)
{
    void *tmp = 0;

    if (!handle || handle->ccount == 0)
    {
        return 0;
    }

    tmp = handle->array[0].arg;
    return tmp;
}

int uki_heap_remove(uki_heap_t *handle, void *arg)
{
    int i = 0;

    if (!handle || !arg || handle->ccount == 0)
    {
        return -1;
    }

    for (i = 0; i < handle->ccount; ++i)
    {
        if (handle->array[i].arg == arg)
        {
            swap_elem(handle, i, handle->ccount - 1);
            handle->ccount--;
            adjust_down(handle, i);
        }

    }

    return 0;

}

static int test_compare(void *arg1, void *arg2)
{
    if ((int)(size_t)arg1 > (int)(size_t)arg2)
    {
        return 1;
    }
    else if ((int)(size_t)arg1 == (int)(size_t)arg2)
    {
        return 0;
    }
    else
    {
        return -1;
    }

}
void uki_heap_test(void)
{
    int i = 0;
    int ret = 0;
    uki_heap_t *handle = uki_heap_create(32, test_compare, 1);
    uki_heap_insert(handle, (void *)5);
    uki_heap_insert(handle, (void *)3);
    uki_heap_insert(handle, (void *)6);
    uki_heap_insert(handle, (void *)8);
    uki_heap_insert(handle, (void *)4);
    uki_heap_insert(handle, (void *)1);
    uki_heap_remove(handle, (void *)6);

    for (i = 0; i < handle->ccount; ++i)
    {

    }

    for (i = 0; i < 6; ++i)
    {
        ret = (typeof(ret))(size_t)uki_heap_pop(handle);
    }

    uki_heap_destroy(handle);
}
