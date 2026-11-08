/** 
  *****************************************************************************
  * @file    		    : timer_util.h
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : Header of timer_util file.
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __TIMER_UTIL_H
#define __TIMER_UTIL_H

/* Includes ------------------------------------------------------------------*/
#include "il005define.h"


/* Defines -------------------------------------------------------------------*/


/* Variables -----------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/   
extern void delay_us(unsigned long long us);
extern void delay_ms(unsigned long long ms);

#endif /*__TIMER_UTIL_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/


