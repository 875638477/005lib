/** 
  *****************************************************************************
  * @file    		    : ictk_define.h
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 7.7
  * @brief          : Definitions for Configurable Values of the ICTK Library
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __ICTKDEFINE_H
#define __ICTKDEFINE_H

/* Defines ------------------------------------------------------------------*/
#define ICTK_DEFINE
#define STM32_HAL_SYSTEMTICK

#define DISABLE_INTERRUPT()     __disable_interrupt(); //HAL_SuspendTick();  HAL_NVIC_DisableIRQ(OTG_FS_IRQn)
#define ENABLE_INTERRUPT()      __enable_interrupt(); //HAL_ResumeTick(); HAL_NVIC_EnableIRQ(OTG_FS_IRQn) //HAL_InitTick (TICK_INT_PRIORITY); HAL_NVIC_EnableIRQ(OTG_FS_IRQn)

// Interface(ITB-100 <-> Chip)
#define G3_COM_I2C                     0x00
#define G3_COM_OWI                     0x01
#define G3_COM_SPI                     0x02
#define G3_COM_TEST_SPI                0x03
#define G3_COM_MCO                     0x04


#define MAX_HANDLER_BUF_SIZE  512

// Test Interface(PC <-> IEB-100)
#if 1
  #define STM32_USBD_CDC          // ST Virtual COM Port(Need a ST STM32 Virtual COM Port Driver,
#else                             // Serial Terminal Program like the HyperTerminal or the TeraTerm)
  #define STM32_USBD_CUSTOM_HID   // ST Custom HID(Need a HID Communication Program)
#endif   

/* Variables ----------------------------------------------------------------*/
extern uint8_t ieb_interface_flag;


#endif /*__ICTKDEFINE_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/
