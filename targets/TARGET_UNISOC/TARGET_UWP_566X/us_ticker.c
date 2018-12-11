/* mbed Microcontroller Library
 * Copyright (c) 2006-2018 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "us_ticker_api.h"
#include "mbed_critical.h"
#include "uwp_type.h"
#include "objects.h"

#define BIT_TIMER_RUN           1
#define BIT_TIMER_MODE          0
#define BIT_TIMER_WIDTH_SEL     16
#define TMR_INT_CLR             3
#define TMR_INT_MASK_STS        2
#define TMR_INT_RAW_STS         1
#define TMR_INT_EN              0

#define TIMER0_MAX_COUNT        (0xFFFFFFFF)        
#define TIMER0_SHIFTBITS        (0) 

typedef struct {
    volatile uint32_t load_l;
    volatile uint32_t load_h;
    volatile uint32_t value_l;
    volatile uint32_t value_h;
	volatile uint32_t ctl;
	volatile uint32_t clr;
	volatile uint32_t shdw_l;
	volatile uint32_t shdw_h;
}TimerReg;

typedef struct timer_s timer_t;

timer_t UsTicker = {
    .base = 0x40050000,
	.mode = TIMER_MODE_PERIOD,
	.wid_mode = TIMER_MODE_32BIT,
	.clk_freq = 26000000,
	.load_count = 0x018cba80,
	.load_count_h = 0x0,
	.irq_op = 1	
};

TimerReg *timer0 = (TimerReg *)0x40050000;

volatile uint32_t us_ticker_clrInt = 0;

static uint32_t us_ticker_inited = 0;
uint32_t us_ticker_soft_int_flag;
static uint32_t us_ticker_timestamp;
static uint32_t us_ticker_interruptCount;

static void us_ticker_irq_callback(void);

void us_ticker_init(void)
{
    if (us_ticker_inited) {
        us_ticker_disable_interrupt();
        return;
    }

    CLR_BIT(timer0->ctl, BIT_TIMER_RUN);
    if(UsTicker.mode == TIMER_MODE_FREE)
	    CLR_BIT(timer0->ctl, BIT_TIMER_MODE);
	else
	    SET_BIT(timer0->ctl, BIT_TIMER_MODE);
	if(UsTicker.wid_mode == TIMER_MODE_32BIT)
		CLR_BIT(timer0->ctl, BIT_TIMER_WIDTH_SEL);
	else
		SET_BIT(timer0->ctl, BIT_TIMER_WIDTH_SEL);
	if(UsTicker.irq_op)
		SET_BIT(timer0->clr, TMR_INT_EN);
	else
		CLR_BIT(timer0->clr, TMR_INT_EN);
	if(UsTicker.wid_mode){
        timer0->load_l = UsTicker.load_count;
		timer0->load_h = UsTicker.load_count_h;
	}
	else
		timer0->load_l = UsTicker.load_count;
	NVIC_SetVector(TIMER0_IRQn, us_ticker_irq_callback);
	NVIC_SetPriority(TIMER0_IRQn, 0x1FUL);
	NVIC_EnableIRQ(TIMER0_IRQn);
	SET_BIT(timer0->ctl, BIT_TIMER_RUN);
	us_ticker_inited = 1;
	us_ticker_soft_int_flag = 0;
	us_ticker_timestamp = 0;
	us_ticker_interruptCount = TIMER0_MAX_COUNT;
}

uint32_t us_ticker_read(void)
{
    if (!us_ticker_inited) {
        return 0 ;
    }
    uint32_t tick_readout = 0 ;

    core_util_critical_section_enter();
    uint32_t ticker = timer0->shdw_l >> TIMER0_SHIFTBITS ;

    if (us_ticker_interruptCount > ticker)
        tick_readout = (us_ticker_timestamp + us_ticker_interruptCount - ticker) % TIMER0_MAX_COUNT ;
    else
        tick_readout = (us_ticker_timestamp + TIMER0_MAX_COUNT + us_ticker_interruptCount - ticker) % TIMER0_MAX_COUNT ;
    core_util_critical_section_exit();

    return tick_readout;
}

void us_ticker_set_interrupt(timestamp_t timestamp)
{
    if (!us_ticker_inited) {
        return ;
    }
	
    uint32_t tmp_stamp = timestamp % TIMER0_MAX_COUNT ;

    core_util_critical_section_enter();
    us_ticker_timestamp = us_ticker_read();
    us_ticker_interruptCount = (tmp_stamp > us_ticker_timestamp) ? (tmp_stamp - us_ticker_timestamp):(tmp_stamp + TIMER0_MAX_COUNT - us_ticker_timestamp) ;
    CLR_BIT(timer0->ctl, BIT_TIMER_RUN);
	SET_BIT(timer0->clr, TMR_INT_CLR);
	SET_BIT(timer0->clr, TMR_INT_EN);
	timer0->load_l = us_ticker_interruptCount << TIMER0_SHIFTBITS;
	SET_BIT(timer0->ctl, BIT_TIMER_RUN);
    core_util_critical_section_exit();

    return ;
}

void us_ticker_fire_interrupt(void)
{
    if (!us_ticker_inited) {
        return ;
    }
	
	core_util_critical_section_enter();
    us_ticker_soft_int_flag = 1 ;
    NVIC_SetPendingIRQ(TIMER0_IRQn);
    core_util_critical_section_exit();
}

void us_ticker_disable_interrupt_help(void)
{
    if (!us_ticker_inited) {
        return ;
    }
	
	CLR_BIT(timer0->clr, TMR_INT_EN);

    return;
}

void us_ticker_disable_interrupt(void)
{
    if (!us_ticker_inited) {
        return ;
    }

	core_util_critical_section_enter();
	us_ticker_timestamp = us_ticker_read();
	us_ticker_interruptCount = TIMER0_MAX_COUNT;
    CLR_BIT(timer0->ctl, BIT_TIMER_RUN);
    SET_BIT(timer0->clr, TMR_INT_CLR);
	SET_BIT(timer0->clr, TMR_INT_EN);
	timer0->load_l = us_ticker_interruptCount;
    SET_BIT(timer0->ctl, BIT_TIMER_RUN);
    core_util_critical_section_exit();

    return;
}

void us_ticker_clear_interrupt(void)
{
    if (!us_ticker_inited) {
        return ;
    }
    SET_BIT(timer0->clr, TMR_INT_CLR);
}

const ticker_info_t* us_ticker_get_info()
{
    static const ticker_info_t info =
    {
        26000000,      // 5MHZ
        32            // 29 bit counter
    };
    return &info;
}

void us_ticker_free(void)
{
    if (!us_ticker_inited) {
        return ;
    }

    core_util_critical_section_enter();
    us_ticker_disable_interrupt_help();
    us_ticker_clear_interrupt();

    /* Disable timer */
    CLR_BIT(timer0->ctl, BIT_TIMER_RUN);

    us_ticker_inited = 0;
    us_ticker_timestamp = 0 ;
    us_ticker_interruptCount = 0 ;
    core_util_critical_section_exit();

    return;
}

void us_ticker_irq_callback()
{
    mbed_error_printf("%s\r\n",__func__);
    us_ticker_clear_interrupt () ;

    if (us_ticker_soft_int_flag == 1) {
        us_ticker_soft_int_flag = 0 ;
        return ;
    }

    core_util_critical_section_enter();
	CLR_BIT(timer0->ctl, BIT_TIMER_RUN);
	CLR_BIT(timer0->clr, TMR_INT_CLR);
    CLR_BIT(timer0->clr, TMR_INT_EN);
	timer0->shdw_l = TIMER0_MAX_COUNT;
	SET_BIT(timer0->ctl, BIT_TIMER_RUN);
    core_util_critical_section_exit();

    return;
}
