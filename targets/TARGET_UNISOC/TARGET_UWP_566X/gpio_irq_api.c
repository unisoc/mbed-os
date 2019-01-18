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
#include <stddef.h>

#include "gpio_irq_api.h"
#include "mbed_error.h"
#include "cmsis.h"
#include "objects.h"

#if 0
#if DEVICE_INTERRUPTIN
#if defined(GPIO_IRQ_DEBUG)
#include "mbed_interface.h"
#endif /* GPIO_IRQ_DEBUG */

#define GPIO_PINNUM             28
#define NONE                    (uint32_t)NC
#define GPIO_INT_CTRL_REG       (RDA_GPIO->INTCTRL)
#define GPIO_INT_SEL_REG        (RDA_GPIO->INTSEL)
#define GPIO_DATA_IN_REG        (RDA_GPIO->DIN)

typedef enum {
    GPIO_IRQ_CH0,
    GPIO_IRQ_CH1,
    CHANNEL_NUM
} GPIO_IRQ_IDX_T;

static uint32_t channel_ids[CHANNEL_NUM] = {0};
static uint32_t channel_pinidxs[CHANNEL_NUM] = {0};
static uint8_t  channel_bothedge_flag[CHANNEL_NUM] = {0};
static gpio_irq_handler irq_handler[CHANNEL_NUM] = {NULL};

#if defined(GPIO_IRQ_DEBUG)
static uint32_t exception_cntr;
#endif /* GPIO_IRQ_DEBUG */

static GPIO_IRQ_IDX_T gpio_irq_ava_chidx(void)
{
    GPIO_IRQ_IDX_T ret;
    for (ret = GPIO_IRQ_CH0; ret < CHANNEL_NUM; ret++) {
        if (0 == channel_ids[ret])
            break;
    }
    return ret;
}

static uint32_t gpio_irq_pinidx(PinName pin)
{
    uint8_t idx;
    const uint32_t pinmap_gpio_irq[GPIO_PINNUM] = {
        /* GPIO 0 ~ 13 */
        PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7, PB_8, PB_9, PA_8, PA_9, PC_0, PC_1,
        /* GPIO 14 ~ 21, Not support interrupt */
        NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
        /* GPIO 22 ~ 27 */
        PD_0, PD_1, PD_2, PD_3, PA_0, PA_1
    };

    for (idx = 0; idx < GPIO_PINNUM; idx++) {
        if (pinmap_gpio_irq[idx] ==  NONE) {
            continue;
        } else if (pinmap_gpio_irq[idx] == (uint32_t)pin) {
            break;
        }
    }

    if (GPIO_PINNUM == idx) {
        error("The pin cannot generate interrupt");
    }
    return idx;
}

static void handle_interrupt_in(void)
{
    /* Read current interrupt register */
    uint32_t int_ctrl = GPIO_INT_CTRL_REG;
    uint32_t din_val  = GPIO_DATA_IN_REG;
    uint32_t idx;

    if (int_ctrl & (0x01UL << 16)) {
        GPIO_INT_CTRL_REG |= (0x01UL << 16);
        while (GPIO_INT_CTRL_REG & (0x01UL << 16));
    }
    for (idx = GPIO_IRQ_CH0; idx < CHANNEL_NUM; idx++) {
        if (int_ctrl & (0x01UL << (21 + idx))) {
            gpio_irq_event flagRiseFall_1, flagRiseFall_2;
            GPIO_INT_CTRL_REG |= (0x01UL << (17 + idx)); // clear int flag
            flagRiseFall_1 = (int_ctrl & (0x01UL << (2 + idx))) ? IRQ_RISE : IRQ_FALL;
            flagRiseFall_2 = (din_val & (0x01UL << channel_pinidxs[idx])) ? IRQ_RISE : IRQ_FALL;
            if (flagRiseFall_1 == flagRiseFall_2) {
                if (channel_bothedge_flag[idx]) {
                    GPIO_INT_CTRL_REG ^= (0x01UL << (2 + idx));
                }
                irq_handler[idx](channel_ids[idx], flagRiseFall_1);
#if defined(GPIO_IRQ_DEBUG)
                exception_cntr = 0;
#endif /* GPIO_IRQ_DEBUG */
            }
#if defined(GPIO_IRQ_DEBUG)
            else {
                exception_cntr++;
                if (exception_cntr >= 2) {
                    exception_cntr = 0;
                    mbed_error_printf("invalid gpio irq: %d,%d\r\n", (int)flagRiseFall_1, (int)flagRiseFall_2);
                }
            }
#endif /* GPIO_IRQ_DEBUG */
        }
    }
}
extern unsigned char aon_port0_inited;
int gpio_irq_init(gpio_irq_t *obj, PinName pin, gpio_irq_handler handler, uint32_t id)
{
#if 0
    uint32_t regval;

    if (pin == NC) return -1;

    obj->ch = (uint16_t)gpio_irq_ava_chidx();
    MBED_ASSERT(CHANNEL_NUM != obj->ch);

    irq_handler[obj->ch] = handler;

    channel_ids[obj->ch] = id;
    channel_pinidxs[obj->ch] = gpio_irq_pinidx(pin);

    regval = RDA_GPIO->INTSEL & ~(0x3FFUL << 10);
    RDA_GPIO->INTSEL = regval |  (0x3FFUL << 10);
#endif

    u32_t int_type;
	
    if(!aon_port0_inited){
       gpio_uwp_p0_init(obj, pin);
	}
    uwp_aon_irq_enable(AON_INT_GPIO0);

	uwp_gpio_set_dir(obj->base, BIT(pin), GPIO_DIR_IN);
	if (obj->flags & GPIO_INT) {
		if (obj->flags & GPIO_INT_EDGE) {
			int_type = GPIO_TRIGGER_BOTH_EDGE;
		} 
		else { /* GPIO_INT_LEVEL */
			if (obj->flags & GPIO_INT_ACTIVE_HIGH) {
				int_type = GPIO_TRIGGER_LEVEL_HIGH;
			} 
			else {
				int_type = GPIO_TRIGGER_LEVEL_LOW;
			}
		}

		uwp_gpio_int_set_type(obj->base, BIT(pin), int_type);
		uwp_gpio_int_clear(obj->base, BIT(pin));
		uwp_gpio_int_enable(obj->base, BIT(pin));
	}
	uwp_gpio_enable(obj->base, BIT(pin));

    NVIC_SetVector(AON_IRQn, (uint32_t)handle_interrupt_in);
    NVIC_SetPriority(AON_IRQn, 0x1FUL);
    NVIC_EnableIRQ(AON_IRQn);
    return 0;
}

void gpio_irq_free(gpio_irq_t *obj)
{
    channel_ids[obj->ch] = 0;
}

void gpio_irq_set(gpio_irq_t *obj, gpio_irq_event event, uint32_t enable)
{
    uint32_t reg_val;
    uint16_t intEn;
    MBED_ASSERT(1 >= obj->ch);

    if (IRQ_RISE == event) {
        obj->flagR = (uint8_t)enable;
    } else {
        obj->flagF = (uint8_t)enable;
    }
    if (obj->flagR && obj->flagF)
        channel_bothedge_flag[obj->ch] = 1U;
    else
        channel_bothedge_flag[obj->ch] = 0U;
    if (obj->flagR || obj->flagF)
        intEn = 1;
    else
        intEn = 0;

    if (0 == intEn) {
        GPIO_INT_CTRL_REG &= ~(0x01UL << (6 + obj->ch));
    } else {
        /* Set interrupt select reg */
        reg_val = GPIO_INT_SEL_REG & ~(0x1FUL << (5 * obj->ch));
        GPIO_INT_SEL_REG = reg_val | ((0x1FUL & ((channel_pinidxs[obj->ch] >= 22) ? (channel_pinidxs[obj->ch] - 8) : channel_pinidxs[obj->ch])) << (5 * obj->ch));

        /* Set interrupt control reg */
        reg_val = GPIO_INT_CTRL_REG & ~(0x01UL << (2 + obj->ch));
        GPIO_INT_CTRL_REG = reg_val | (((0U == channel_bothedge_flag[obj->ch]) && (1U == obj->flagR)) ? (0x01UL << (2 + obj->ch)) : (0x00UL))
                                    | (0x01UL << (6 + obj->ch));
    }
}

void gpio_irq_enable(gpio_irq_t *obj)
{
    NVIC_EnableIRQ(GPIO_IRQn);
}

void gpio_irq_disable(gpio_irq_t *obj)
{
    NVIC_DisableIRQ(GPIO_IRQn);
}

#endif /* DEVICE_INTERRUPTIN */
#endif

// TODO: number of can generate gpio pins 
//#define CONFIG_GPIO_LEVEL_TRIGGER
#define PIN_NUMBER  3
#define PORT_NUMBER 3
#define GPIO_BOTH_EDGE 0x03

static gpio_irq_handler irq_handler[PIN_NUMBER] = {0};
static u32_t pin_id[PIN_NUMBER] = {0};
static u8_t pin_edge[PIN_NUMBER] = {0};
static u32_t CH2PORT_Table[3] = { BASE_AON_GPIOP2, BASE_AON_GPIOP1, BASE_AON_GPIOP0};
static u32_t pin_irq_enabled = 0UL;

static void gpio_uwp_isr(int channel, void *arg){
    u8_t  port_channel = channel - 8;
    u32_t base = CH2PORT_Table[port_channel];
    u32_t int_status = uwp_gpio_int_status(base, 1);
    u32_t int_enabled = int_status & pin_irq_enabled;
    u32_t value = 0;
    gpio_irq_event event;
    uwp_gpio_int_disable(base, int_status);
    uwp_gpio_int_clear(base, int_status);
    for(int i=0; i < PIN_NUMBER; i++){
        if((int_enabled & (0x1UL << i))){
            switch(pin_edge[i]){
                case 1:
                case 2: event = pin_edge[i];                             break;
                case 3: value = ((uwp_gpio_read(base, 0x1UL<<i)) >> (i));
                        event = value == 0 ? IRQ_FALL : IRQ_RISE;       break;
                default:event = 4;                                       break;
            }
        //mbed_error_printf("event:%d val:%d\r\n", event, value);
        if(irq_handler[i] != NULL)
            irq_handler[i](pin_id[i], event);
        }
    }
    uwp_gpio_int_enable(base, int_status);
}

static void gpio_port_irq_enable(gpio_irq_t *obj){
    u32_t base = obj->port_base;
    switch(base){
        case BASE_AON_GPIOP0:
            uwp_aon_intc_set_irq_callback(AON_INT_GPIO0, gpio_uwp_isr, NULL);
            uwp_aon_irq_enable(AON_INT_GPIO0);
            break;
        case BASE_AON_GPIOP1:
            uwp_aon_intc_set_irq_callback(AON_INT_GPIO1, gpio_uwp_isr, NULL);
            uwp_aon_irq_enable(AON_INT_GPIO1);
            break;
        case BASE_AON_GPIOP2:
            uwp_aon_intc_set_irq_callback(AON_INT_GPIO2, gpio_uwp_isr, NULL);
            uwp_aon_irq_enable(AON_INT_GPIO2);
            break;
        default:
            obj->port_base = BASE_AON_GPIOP0;
            uwp_aon_intc_set_irq_callback(AON_INT_GPIO0, gpio_uwp_isr, NULL);
            uwp_aon_irq_enable(AON_INT_GPIO0);
            break;
    }
}

int gpio_irq_init(gpio_irq_t *obj, PinName pin, gpio_irq_handler handler, uint32_t id)
{
    obj->pin = pin;
    gpio_port_irq_enable(obj);
    irq_handler[pin] = handler;
    pin_id[pin] = id;
    pin_irq_enabled |= BIT(pin);
    
    return 0;
}

void gpio_irq_disable(gpio_irq_t *obj){
        uwp_gpio_int_disable(obj->port_base, BIT(obj->pin));
}

void gpio_irq_enable(gpio_irq_t *obj){
        uwp_gpio_int_enable(obj->port_base, BIT(obj->pin));
}

// TODO: more need to do
void gpio_irq_free(gpio_irq_t *obj){
    pin_id[obj->pin] = 0;
    pin_irq_enabled &= ~ BIT(obj->pin);
    irq_handler[obj->pin] = NULL;
    pin_edge[obj->pin] = 0;
}

void gpio_irq_set(gpio_irq_t *obj, gpio_irq_event event, uint32_t enable){
    u8_t port_channel = 0;
    u32_t int_type;
    int flag = 0;

     if(enable){
         if(event == IRQ_RISE){
             pin_edge[obj->pin] |= IRQ_RISE;
             int_type = GPIO_TRIGGER_HIGH_EDGE;
         }
        else if(event == IRQ_FALL){
            pin_edge[obj->pin] |= IRQ_FALL;
            int_type = GPIO_TRIGGER_LOW_EDGE;
        }
     }else{}

     if(!enable){
          gpio_irq_disable(obj);
    }
    else{
            uwp_gpio_set_dir(obj->port_base, BIT(obj->pin), GPIO_DIR_INPUT);

            if ((pin_edge[obj->pin] & GPIO_BOTH_EDGE) == GPIO_BOTH_EDGE)
                int_type = GPIO_TRIGGER_BOTH_EDGE;
            else if( event == IRQ_RISE)
                int_type = GPIO_TRIGGER_HIGH_EDGE;
            else
                int_type = GPIO_TRIGGER_LOW_EDGE;

#ifdef CONFIG_GPIO_LEVEL_TRIGGER
            if(event == IRQ_RISE){
                    int_type = GPIO_TRIGGER_LEVEL_HIGH;
                    pin_edge[obj->pin] = 4UL;
            }
            else{
                    int_type = GPIO_TRIGGER_LEVEL_LOW;
                    pin_edge[obj->pin] = 8UL;
            }
#endif
            //printf("int_type:%d\r\n",int_type);
            uwp_gpio_int_set_type(obj->port_base, BIT(obj->pin), int_type);
            uwp_gpio_int_clear(obj->port_base, BIT(obj->pin));
            uwp_gpio_int_enable(obj->port_base, BIT(obj->pin));
            uwp_gpio_enable(obj->port_base, BIT(obj->pin));
            // TODO: ???
            unsigned int *p = (unsigned int*)(0x40840010);
            *p = *p | (1 << 8);

            uwp_gpio_input_enable(obj->port_base, BIT(obj->pin));
    }
}





