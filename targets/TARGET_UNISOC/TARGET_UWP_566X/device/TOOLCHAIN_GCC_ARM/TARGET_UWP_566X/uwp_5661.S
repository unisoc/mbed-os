/* File: startup_ARMCM4.s
 * Purpose: startup file for Cortex-M3/M4 devices. Should use with
 *   GNU Tools for ARM Embedded Processors
 * Version: V1.1
 * Date: 17 June 2018
 *
 * Copyright (C) 2011 ARM Limited. All rights reserved.
 * ARM Limited (ARM) is supplying this software for use with Cortex-M3/M4
 * processor based microcontrollers.  This file can be freely distributed
 * within development tools that are supporting such ARM based processors.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 */
    .syntax unified
    .arch armv7-m

/* Memory Model
   The HEAP starts at the end of the DATA section and grows upward.

   The STACK starts at the end of the RAM and grows downward.

   The HEAP and stack STACK are only checked at compile time:
   (DATA_SIZE + HEAP_SIZE + STACK_SIZE) < RAM_SIZE

   This is just a check for the bare minimum for the Heap+Stack area before
   aborting compilation, it is not the run time limit:
   Heap_Size + Stack_Size = 0x80 + 0x80 = 0x100
 */
    .section .stack
    .align 3
#ifdef __STACK_SIZE
    .equ    Stack_Size, __STACK_SIZE
#else
    .equ    Stack_Size, 0x800
#endif
    .globl    __StackTop
    .globl    __StackLimit
__StackLimit:
    .space    Stack_Size
    .size __StackLimit, . - __StackLimit
__StackTop:
    .size __StackTop, . - __StackTop

    .section .heap
    .align 3
#ifdef __HEAP_SIZE
    .equ    Heap_Size, __HEAP_SIZE
#else
    .equ    Heap_Size, 0x28000
#endif
    .globl    __HeapBase
    .globl    __HeapLimit
__HeapBase:
    .space    Heap_Size
    .size __HeapBase, . - __HeapBase
__HeapLimit:
    .size __HeapLimit, . - __HeapLimit

    .section .isr_vector
    .align 2
    .globl __isr_vector
__isr_vector:
    .long    __StackTop            /* Top of Stack */
    .long    Reset_Handler         /* Reset Handler */
    .long    NMI_Handler           /* NMI Handler */
    .long    HardFault_Handler     /* Hard Fault Handler */
    .long    MemManage_Handler     /* MPU Fault Handler */
    .long    BusFault_Handler      /* Bus Fault Handler */
    .long    UsageFault_Handler    /* Usage Fault Handler */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    SVC_Handler           /* SVCall Handler */
    .long    DebugMon_Handler      /* Debug Monitor Handler */
    .long    0                     /* Reserved */
    .long    PendSV_Handler        /* PendSV Handler */
    .long    SysTick_Handler       /* SysTick Handler */

    /* External interrupts */
    .long    0                          /*  0:Reserved                      */
    .long    0                          /*  1:Reserved                      */
    .long    0                          /*  2:Reserved                      */
    .long    0                          /*  3:Reserved                      */
    .long    0                          /*  4:Reserved                      */
    .long    0                          /*  5:Reserved                      */
    .long    0                          /*  6:Reserved                      */
    .long    0                          /*  7:Reserved                      */
    .long    0                          /*  8:Reserved                      */
    .long    0                          /*  9:Reserved                      */
    .long    0                          /* 10:Reserved                      */
    .long    0                          /* 11:Reserved                      */
    .long    0                          /* 12:Reserved                      */
    .long    0                          /* 13:Reserved                      */
    .long    0                          /* 14:Reserved                      */
    .long    0                          /* 15:Reserved                      */
    .long    0                          /* 16:Reserved                      */
    .long    0                          /* 17:Reserved                      */
    .long   WCI2_IRQHandler             /* 18:WCI2_IRQHandler Flash         */
    .long   INT_IPI_IRQHandler          /* 19:IPI                           */
    .long   AON_IRQHandler              /* 20:AON                           */
    .long   TIMER0_IRQHandler           /* 21:TIMER0                        */
    .long   TIMER1_IRQHandler           /* 22:TIMER1                        */
    .long   TIMER2_IRQHandler           /* 23:TIMER2                        */
    .long    0                          /* 24:Reserved                      */
    .long    0                          /* 25:Reserved                      */
    .long   SYSTMR_IRQHandler           /* 26: SYSTEM TIMER                 */
    .long    0                          /* 27:Reserved                      */
    .long    0                          /* 28:Reserved                      */
    .long    0                          /* 29:Reserved                      */
    .long    0                          /* 30:Reserved                      */
    .long   WIFI_CAP_IRQHanler          /* 31:REQ_WIFI_CAP                  */
    .long   DPD_IRQHandler              /* 32:UART0                         */
    .long   MAC_IRQHandler              /* 33:MAC Hardware                  */
    .long    0                          /* 34:Reserved                      */
    .long   UART0_IRQHandler            /* 35:UART0                         */
    .long   UART1_IRQHandler            /* 36:AHB DMA                       */
    .long   COMTMR_IRQHandler           /* 37:COMMON TIMER                  */
    .long    0                          /* 38:Reserved                      */
    .long    0                          /* 39:Reserved                      */
    .long   WTHDOG_IRQHandler           /* 40:WATCHDOG                      */
    .long    0                          /* 41:Reserved                      */
    .long    0                          /* 42:Reserved                      */
    .long    0                          /* 43:Reserved                      */
    .long    0                          /* 44:Reserved                      */
    .long    0                          /* 45:Reserved                      */
    .long    0                          /* 46:Reserved                      */
    .long    0                          /* 47:Reserved                      */
    .long    0                          /* 48:Reserved                      */
    .long    0                          /* 49:Reserved                      */
    .long   GNSS2BTWIFI_IPI_IRQHandler  /* 50:GNSS2BTWIFI_IPI               */


    .size    __isr_vector, . - __isr_vector

    .text
    .thumb
    .thumb_func
    .align 2
    .globl    Reset_Handler
    .type    Reset_Handler, %function
Reset_Handler:
/*     Loop to copy data from read only memory to RAM. The ranges
 *      of copy from/to are specified by following symbols evaluated in
 *      linker script.
 *      _etext: End of code section, i.e., begin of data sections to copy from.
 *      __data_start__/__data_end__: RAM address range that data should be
 *      copied to. Both must be aligned to 4 bytes boundary.  */

    ldr    r1, =__etext
    ldr    r2, =__data_start__
    ldr    r3, =__data_end__

.Lflash_to_ram_loop:
    cmp     r2, r3
    ittt    lt
    ldrlt   r0, [r1], #4
    strlt   r0, [r2], #4
    blt    .Lflash_to_ram_loop

    ldr    r1, =__ramfunc_start__
    ldr    r2, =__ramtext_start__
    ldr    r3, =__ramtext_end__

.Lflash_to_ram_loop_text:
    cmp     r2, r3
    ittt    lt
    ldrlt   r0, [r1], #4
    strlt   r0, [r2], #4
    blt    .Lflash_to_ram_loop_text
    
    ldr    r0, =SystemInit
    blx    r0
    ldr    r0, =_start
    bx     r0

Soft_Reset:
    mov    r1, #0x04
    ldr    r0, [r1]
    bx     r0

    .pool
    .size Reset_Handler, . - Reset_Handler

    .text
/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
    .macro    def_default_handler    handler_name
    .align 1
    .thumb_func
    .weak    \handler_name
    .type    \handler_name, %function
\handler_name :
    b    .
    .size    \handler_name, . - \handler_name
    .endm

    def_default_handler    NMI_Handler
    def_default_handler    HardFault_Handler
    def_default_handler    MemManage_Handler
    def_default_handler    BusFault_Handler
    def_default_handler    UsageFault_Handler
    def_default_handler    SVC_Handler
    def_default_handler    DebugMon_Handler
    def_default_handler    PendSV_Handler
    def_default_handler    SysTick_Handler
    def_default_handler    Default_Handler

    .macro    def_irq_default_handler    handler_name
    .weak     \handler_name
    .set      \handler_name, Default_Handler
    .endm

    def_irq_default_handler     WCI2_IRQHandler
    def_irq_default_handler     INT_IPI_IRQHandler
    def_irq_default_handler     AON_IRQHandler
    def_irq_default_handler     TIMER0_IRQHandler
    def_irq_default_handler     TIMER1_IRQHandler
    def_irq_default_handler     TIMER2_IRQHandler
    def_irq_default_handler     SYSTMR_IRQHandler
    def_irq_default_handler     UART0_IRQHandler
    def_irq_default_handler     UART1_IRQHandler
    def_irq_default_handler     COMTMR_IRQHandler
    def_irq_default_handler     WTHDOG_IRQHandler
    def_irq_default_handler     WIFI_CAP_IRQHanler
    def_irq_default_handler     DPD_IRQHandler
    def_irq_default_handler     MAC_IRQHandler
    def_irq_default_handler     GNSS2BTWIFI_IPI_IRQHandler

    .end
