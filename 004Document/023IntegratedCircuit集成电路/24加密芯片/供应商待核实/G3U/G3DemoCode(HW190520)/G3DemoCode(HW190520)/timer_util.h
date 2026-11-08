/** 
  *****************************************************************************
  * @file    		    : timer_util.h
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 7.7
  * @brief          : Header of timer_util file.
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __TIMER_UTIL_H
#define __TIMER_UTIL_H

/* Includes ------------------------------------------------------------------*/
#include "g3_define.h"


/* Defines -------------------------------------------------------------------*/


/* Variables -----------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/   
extern void delay_us(uint32_t us);
extern void delay_ms(uint32_t ms);


#endif /*__TIMER_UTIL_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/


