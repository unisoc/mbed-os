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
#include "uwp_intc.h"


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

static inline void uwp_aon_irq_enable(u32_t channel)
{
	sci_reg_or(REG_AON_IRQ_ENABLE, BIT(channel));
}
static void gpio_port_irq_enable(gpio_irq_t *obj){
    u32_t base = obj->port_base;
    switch(base){
        case BASE_AON_GPIOP0:
            uwp_aon_intc_set_irq_callback(AON_INT_GPIO0, gpio_uwp_isr, NULL);
            uwp_ictl_irq_enable(AON_IRQ_TYPE, AON_INT_GPIO0);
            break;
        case BASE_AON_GPIOP1:
            uwp_aon_intc_set_irq_callback(AON_INT_GPIO1, gpio_uwp_isr, NULL);
            uwp_ictl_irq_enable(AON_IRQ_TYPE, AON_INT_GPIO1);
            break;
        case BASE_AON_GPIOP2:
            uwp_aon_intc_set_irq_callback(AON_INT_GPIO2, gpio_uwp_isr, NULL);
            uwp_ictl_irq_enable(AON_IRQ_TYPE, AON_INT_GPIO2);
            break;
        default:
            obj->port_base = BASE_AON_GPIOP0;
            uwp_aon_intc_set_irq_callback(AON_INT_GPIO0, gpio_uwp_isr, NULL);
            uwp_ictl_irq_enable(AON_IRQ_TYPE, AON_INT_GPIO0);
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





