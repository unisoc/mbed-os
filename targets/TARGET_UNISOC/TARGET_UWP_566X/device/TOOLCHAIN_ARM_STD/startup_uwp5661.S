;/*****************************************************************************
; * @file:    startup_UWP5661.s
; * @purpose: CMSIS Cortex-M4 Core Device Startup File
; *           for the UNISOC UWP566X Device Series
; * @version: V1.02, modified for mbed
; * @date:    07. June 2018
; *------- <<< Use Configuration Wizard in Context Menu >>> ------------------
; *
; * Copyright (C) 2009 ARM Limited. All rights reserved.
; * ARM Limited (ARM) is supplying this software for use with Cortex-M3
; * processor based microcontrollers.  This file can be freely distributed
; * within development tools that are supporting such ARM based processors.
; *
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; *****************************************************************************/

                PRESERVE8
                THUMB

; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                IMPORT  |Image$$ARM_LIB_STACK$$ZI$$Limit| [WEAK]
                EXPORT  __Vectors

__Vectors       DCD     |Image$$ARM_LIB_STACK$$ZI$$Limit| ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     MemManage_Handler         ; MPU Fault Handler
                DCD     BusFault_Handler          ; Bus Fault Handler
                DCD     UsageFault_Handler        ; Usage Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     DebugMon_Handler          ; Debug Monitor Handler
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

                ; External Interrupts
                DCD     0                         ;  0:Reserved  
                DCD     0                         ;  1:Reserved
                DCD     0                         ;  2:Reserved  
                DCD     0                         ;  3:Reserved 
                DCD     0                         ;  4:Reserved  
                DCD     0                         ;  5:Reserved
                DCD     0                         ;  6:Reserved  
                DCD     0                         ;  7:Reserved 
                DCD     0                         ;  8:Reserved  
                DCD     0                         ;  9:Reserved
                DCD     0                         ; 10:Reserved  
                DCD     0                         ; 11:Reserved 
                DCD     0                         ; 12:Reserved  
                DCD     0                         ; 13:Reserved
                DCD     0                         ; 14:Reserved  
                DCD     0                         ; 15:Reserved 
                DCD     0                         ; 16:Reserved
                DCD     0                         ; 17:Reserved
                DCD     WCI2_IRQHandler           ; 18:WCI2_IRQHandler Flash
                DCD     INT_IPI_IRQHandler        ; 19:IPI
                DCD     AON_IRQHandler            ; 20:AON
                DCD     TIMER0_IRQHandler         ; 21:TIMER0
                DCD     TIMER1_IRQHandler         ; 22:TIMER1
                DCD     TIMER2_IRQHandler         ; 23:TIMER2
                DCD     0                         ; 24:Reserved
                DCD     0                         ; 25:Reserved
                DCD     SYSTMR_IRQHandler         ; 26: SYSTEM TIMER
                DCD     0                         ; 27:Reserved
                DCD     0                         ; 28:Reserved
                DCD     0                         ; 29:Reserved
                DCD     0                         ; 30:Reserved
                DCD     WIFI_CAP_IRQHanler        ; 31:REQ_WIFI_CAP
                DCD     DPD_IRQHandler            ; 32:UART0
                DCD     MAC_IRQHandler            ; 33:MAC Hardware
                DCD     0                         ; 34:Reserved
                DCD     UART0_IRQHandler          ; 35:UART0
                DCD     UART1_IRQHandler          ; 36:AHB DMA
                DCD     COMTMR_IRQHandler         ; 37:COMMON TIMER
                DCD     0                         ; 38:Reserved
                DCD     0                         ; 39:Reserved
                DCD     WTHDOG_IRQHandler         ; 40:WATCHDOG
                DCD     0                         ; 41:Reserved
                DCD     0                         ; 42:Reserved
                DCD     0                         ; 43:Reserved  
                DCD     0                         ; 44:Reserved
                DCD     0                         ; 45:Reserved  
                DCD     0                         ; 46:Reserved 
                DCD     0                         ; 47:Reserved  
                DCD     0                         ; 48:Reserved
                DCD     0                         ; 49:Reserved 
                DCD     GNSS2BTWIFI_IPI_IRQHandler; 50:GNSS2BTWIFI_IPI



                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler         [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler          [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler        [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler          [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

Default_Handler PROC

                EXPORT  WCI2_IRQHandler           [WEAK]
                EXPORT  AON_IRQHandler            [WEAK]
                EXPORT  TIMER0_IRQHandler         [WEAK]
                EXPORT  TIMER1_IRQHandler         [WEAK]
                EXPORT  TIMER2_IRQHandler         [WEAK]
                EXPORT  SYSTMR_IRQHandler         [WEAK]
                EXPORT  UART0_IRQHandler          [WEAK]
                EXPORT  UART1_IRQHandler          [WEAK]
                EXPORT  COMTMR_IRQHandler         [WEAK]
                EXPORT  WTHDOG_IRQHandler         [WEAK]
                EXPORT  WIFI_CAP_IRQHanler        [WEAK]
                EXPORT  DPD_IRQHandler            [WEAK]
                EXPORT  MAC_IRQHandler            [WEAK]
                EXPORT  GNSS2BTWIFI_IPI_IRQHandler[WEAK]

WCI2_IRQHandler
AON_IRQHandler
TIMER0_IRQHandler
TIMER1_IRQHandler
TIMER2_IRQHandler
SYSTMR_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
COMTMR_IRQHandler
WTHDOG_IRQHandler
INT_IPI_IRQHandler
WIFI_CAP_IRQHanler
DPD_IRQHandler
MAC_IRQHandler
GNSS2BTWIFI_IPI_IRQHandler

                B       .

                ENDP

                ALIGN
                END
