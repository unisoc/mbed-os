/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include "mbed_retarget.h"
#include "uwp_intc.h"
#include "UWP_5661.h"
#include "uwp_log.h"


/* convenience defines */
struct __intc_cb {
	uwp_intc_callback_t cb;
	void *data;
};

struct intc_uwp_data {
	struct __intc_cb irq[INTC_MAX_CHANNEL];
	struct __intc_cb fiq[INTC_MAX_CHANNEL];
};

static struct intc_uwp_data intc_uwp_dev_data;
static struct intc_uwp_data aon_intc_uwp_dev_data;

void uwp_intc_set_irq_callback(int channel,
		uwp_intc_callback_t cb, void *arg)
{
	if (intc_uwp_dev_data.irq[channel].cb != NULL) {
		printk("IRQ %d callback has been registered.\n", channel);
		return;
	}

	intc_uwp_dev_data.irq[channel].cb = cb;
	intc_uwp_dev_data.irq[channel].data = arg;
}

void uwp_intc_unset_irq_callback(int channel)
{
	intc_uwp_dev_data.irq[channel].cb = NULL;
	intc_uwp_dev_data.irq[channel].data = NULL;
}

void uwp_intc_set_fiq_callback(int channel,
		uwp_intc_callback_t cb, void *arg)
{
	if (intc_uwp_dev_data.fiq[channel].cb != NULL) {
		printk("FIQ %d callback has been registered.\n", channel);
		return;
	}

	intc_uwp_dev_data.fiq[channel].cb = cb;
	intc_uwp_dev_data.fiq[channel].data = arg;
}

void uwp_intc_unset_fiq_callback(int channel)
{
	intc_uwp_dev_data.fiq[channel].cb = NULL;
	intc_uwp_dev_data.fiq[channel].data = NULL;
}

void uwp_aon_intc_set_irq_callback(int channel,
		uwp_intc_callback_t cb, void *arg)
{

	if (aon_intc_uwp_dev_data.irq[channel].cb != NULL) {
		printk("IRQ %d callback has been registered.\n", channel);
		return;
	}

	aon_intc_uwp_dev_data.irq[channel].cb = cb;
	aon_intc_uwp_dev_data.irq[channel].data = arg;
}

void uwp_aon_intc_unset_irq_callback(int channel)
{

	aon_intc_uwp_dev_data.irq[channel].cb = NULL;
	aon_intc_uwp_dev_data.irq[channel].data = NULL;
}

#if 0
DEVICE_INIT(intc_uwp, CONFIG_INTC_UWP_DEVICE_NAME,
		    intc_uwp_init, &intc_uwp_dev_data, NULL,
		    PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);

DEVICE_INIT(aon_intc_uwp, CONFIG_AON_INTC_UWP_DEVICE_NAME,
		    aon_intc_uwp_init, &aon_intc_uwp_dev_data, NULL,
		    PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
#endif

static void intc_uwp_irq(void)
{
	int ch;
	for (ch = 0; ch < INTC_MAX_CHANNEL; ch++) {
		if (uwp_irq_status(ch) && intc_uwp_dev_data.irq[ch].cb) {
			uwp_irq_disable(ch);
			intc_uwp_dev_data.irq[ch].cb(ch, intc_uwp_dev_data.irq[ch].data);
			uwp_irq_enable(ch);
		}
	}
}

static void intc_uwp_fiq(void)
{
	int ch;
	for (ch = 0; ch < INTC_MAX_CHANNEL; ch++) {
		if (uwp_fiq_status(ch) && intc_uwp_dev_data.fiq[ch].cb) {
			uwp_fiq_disable(ch);
			intc_uwp_dev_data.fiq[ch].cb(ch, intc_uwp_dev_data.fiq[ch].data);
			uwp_fiq_enable(ch);
		}
	}
}


static void aon_intc_uwp_irq(void)
{
	int ch;
	for (ch = 0; ch < INTC_MAX_CHANNEL; ch++) {
		if (uwp_aon_irq_status(ch) && aon_intc_uwp_dev_data.irq[ch].cb) {
			uwp_aon_irq_disable(ch);
			aon_intc_uwp_dev_data.irq[ch].cb(ch, aon_intc_uwp_dev_data.irq[ch].data);
			uwp_aon_irq_enable(ch);
		}
	}
}

void intc_uwp_init(void)
{
    NVIC_DisableIRQ(INT_IRQ_IRQn);
	NVIC_DisableIRQ(INT_FIQ_IRQn);

	uwp_sys_enable(BIT(APB_EB_INTC));
	uwp_sys_reset(BIT(APB_EB_INTC));

    NVIC_SetVector(INT_IRQ_IRQn,intc_uwp_irq);
	NVIC_SetPriority(INT_IRQ_IRQn,0x1FUL);
	NVIC_EnableIRQ(INT_IRQ_IRQn);

    NVIC_SetVector(INT_FIQ_IRQn, intc_uwp_fiq);
	NVIC_SetPriority(INT_FIQ_IRQn,0x1FUL);
	NVIC_EnableIRQ(INT_FIQ_IRQn);

}


void aon_intc_uwp_init(void)
{
    NVIC_DisableIRQ(AON_IRQn);

	uwp_aon_enable(BIT(AON_EB_INTC));
	uwp_aon_reset(BIT(AON_RST_INTC));
    
	NVIC_SetVector(AON_IRQn,aon_intc_uwp_irq);
	NVIC_SetPriority(AON_IRQn,0x1FUL);
	NVIC_EnableIRQ(AON_IRQn);
	
}



