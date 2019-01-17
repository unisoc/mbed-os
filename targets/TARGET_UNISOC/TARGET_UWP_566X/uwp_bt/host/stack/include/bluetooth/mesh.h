/** @file
 *  @brief Bluetooth Mesh Profile APIs.
 */

/*
 * Copyright (c) 2017 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BT_MESH_H
#define __BT_MESH_H

#include <osi/uki_utils.h>
#include <osi/uki_zif.h>
#include <osi/uki_log.h>
#include <osi/uki_types.h>
#include <osi/uki_work.h>
#include <osi/uki_timer.h>
#include <osi/uki_settings.h>
#include <stddef.h>
#include <errno.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
struct net_buf_pool
{

};


struct k_thread
{
    char data[sizeof(pthread_t)];
} __attribute__ ((aligned (1)));
typedef void (*k_thread_user_cb_t)(const struct k_thread *thread, void *user_data);


static inline void k_thread_foreach(k_thread_user_cb_t user_cb, void *user_data) { };


#define net_buf_alloc(...) net_buf_alloc()

#include <bluetooth/mesh/access.h>
#include <bluetooth/mesh/main.h>
#include <bluetooth/mesh/cfg_srv.h>
#include <bluetooth/mesh/health_srv.h>
#include <bluetooth/mesh/cfg_cli.h>
#include <bluetooth/mesh/health_cli.h>
#include <bluetooth/mesh/proxy.h>

#endif /* __BT_MESH_H */
