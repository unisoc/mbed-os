/******************************************************************************
 * @file     RDA5991H.h
 * @brief    CMSIS Cortex-M4 Core Peripheral Access Layer Header File for
 *           RDA RDA5991H Device Series
 * @version: V1.09
 * @date:    07. June 2018
 *
 * @note
 * Copyright (C) 2009 ARM Limited. All rights reserved.
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Cortex-M
 * processor based microcontrollers.  This file can be freely distributed
 * within development tools that are supporting such ARM based processors.
 *
 * @par
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/


#ifndef __UWP_5661H_H__
#define __UWP_5661H_H__

/*
 * ==========================================================================
 * ---------- Interrupt Number Definition -----------------------------------
 * ==========================================================================
 */

typedef enum IRQn
{
/******  Cortex-M4 Processor Exceptions Numbers ***************************************************/
  NonMaskableInt_IRQn           = -14,      /*!< 2 Non Maskable Interrupt                         */
  MemoryManagement_IRQn         = -12,      /*!< 4 Cortex-M4 Memory Management Interrupt          */
  BusFault_IRQn                 = -11,      /*!< 5 Cortex-M4 Bus Fault Interrupt                  */
  UsageFault_IRQn               = -10,      /*!< 6 Cortex-M4 Usage Fault Interrupt                */
  SVCall_IRQn                   = -5,       /*!< 11 Cortex-M4 SV Call Interrupt                   */
  DebugMonitor_IRQn             = -4,       /*!< 12 Cortex-M4 Debug Monitor Interrupt             */
  PendSV_IRQn                   = -2,       /*!< 14 Cortex-M4 Pend SV Interrupt                   */
  SysTick_IRQn                  = -1,       /*!< 15 Cortex-M4 System Tick Interrupt               */

/******  RDA5991H Specific Interrupt Numbers ******************************************************/
  INTCFIQ_IRQn                  = 0,        /*!< SPI Flash Interrupt                              */
  INTCIRQ_IRQn				    = 1, 	    /*!< SPI Flash Interrupt							  */
  WCI2_IRQn                     = 18,       /*!< SPI Flash Interrupt                              */
  AON_IRQn                      = 20,       /*!< AON Interrupt                                    */
  TIMER0_IRQn                   = 21,       /*!< SDIO Interrupt                                   */
  TIMER1_IRQn                   = 22,       /*!< USBDMA Interrupt                                 */
  TIMER2_IRQn                   = 23,       /*!< USB Interrupt                                    */
  SYSTEMTIMER_IRQn              = 26,       /*!< GPIO Interrupt                                   */
  UART0_IRQn                    = 35,       /*!< Timer Interrupt                                  */
  UART1_IRQn                    = 36,       /*!< UART0 Interrupt                                  */
  COMMONTIMER_IRQn              = 37,       /*!< MAC Hardware Interrupt                           */
  WATCHDOG_IRQn                 = 40,       /*!< UART1 Interrupt                                  */
} IRQn_Type;


/*
 * ==========================================================================
 * ----------- Processor and Core Peripheral Section ------------------------
 * ==========================================================================
 */

/* Configuration of the Cortex-M4 Processor and Core Peripherals */
#define __MPU_PRESENT             1         /*!< MPU present or not                               */
#define __NVIC_PRIO_BITS          5         /*!< Number of Bits used for Priority Levels          */
#define __Vendor_SysTickConfig    0         /*!< Set to 1 if different SysTick Config is used     */
#define __FPU_PRESENT             1         /*!< FPU present                                      */


#include "core_cm4.h"                       /* Cortex-M4 processor and core peripherals           */
#include "system_UWP5661.h"                 /* System Header                                      */


/******************************************************************************/
/*                Device Specific Peripheral registers structures             */
/******************************************************************************/

#if defined ( __CC_ARM   )
#pragma no_anon_unions
#endif


/******************************************************************************/
/*                         Peripheral memory map                              */
/******************************************************************************/


/* Base addresses                                                            */
//#define UWP_BOOT_FLASH_BASE   (0x00000000UL)
#define UWP_IRAM_BASE         (0x00100000UL)
#define UWP_DRAM_BASE         (0x00120000UL)
#define UWP_FLASH_BASE        (0x02000000UL)



/* AHB0 peripherals                                                           */


/******************************************************************************/
/*                         Peripheral declaration                             */
/******************************************************************************/


#endif /* __RDA5991H_H__ */
