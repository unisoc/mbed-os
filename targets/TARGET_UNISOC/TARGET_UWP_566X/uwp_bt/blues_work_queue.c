#include "osi/uki_log.h"
#include "osi/uki_work.h"

#include "blues_work_queue.h"

uki_work_t *g_default_work;

int blues_work_queue_init(void)
{
    g_default_work = uki_work_create();
    uki_work_run(g_default_work, "g_default_work");
    return 0;
}
int blues_work_queue_uninit(void)
{
    if (g_default_work)
    {
        uki_work_destroy(g_default_work, 1);
        g_default_work = 0;
    }
    return 0;
}
