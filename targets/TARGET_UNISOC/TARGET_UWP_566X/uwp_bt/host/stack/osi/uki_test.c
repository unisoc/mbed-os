#include "osi/uki_heap.h"
#include "osi/uki_list.h"
#include "osi/uki_timer.h"
#include "osi/uki_work.h"

#include "uki_test.h"
#include "uki_log.h"
#if 0
int bt_osi_test(int argc, char *argv[])
{
    int opt = 0;
    int select = 0;
    //int i = 0;
    BTD("%s argc %d ++", __func__, argc);

    while ((opt = getopt(argc, argv, "c:")) >= 0)
    {
        BTD("%c ++",opt);
        switch (opt)
        {
        case 'c':
            select = atoi(optarg);
            BTD("%s ++",optarg);
            switch (select)
            {
            case 0:
                uki_list_test();
                break;
            case 1:
                uki_heap_test();
                break;
            case 2:
                uki_timer_test();
                break;
            case 3:
                uki_work_test();
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }

    }

    return 0;
}
#endif