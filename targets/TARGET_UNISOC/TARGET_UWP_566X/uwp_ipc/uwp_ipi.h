/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MBED_UWP_IPI_H
#define MBED_UWP_IPI_H

#include "uwp_hal_ipi.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef void (*uwp_ipi_callback_t) (void *data);
extern void uwp_ipi_set_callback(uwp_ipi_callback_t cb, void *arg);
extern void uwp_ipi_unset_callback(void);
void ipi_uwp_init(void);

static inline void uwp_ipi_irq_trigger(void)
{
	uwp_ipi_trigger(IPI_CORE_BTWF, IPI_TYPE_IRQ0);
}

#ifdef __cplusplus
}
#endif

#endif /* _INTC_UWP_H_ */
