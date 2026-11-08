/** 
  *****************************************************************************
  * @file    		    : owi_sw.h
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : Header of owi_sw.c file.
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __IL005TEST_H
#define __IL005TEST_H

/* Includes ------------------------------------------------------------------*/
#include "il005define.h"

/* Defines ------------------------------------------------------------------*/
#define TEST_END_OPERATION      0x03
#define TEST_NO_OPERATION       0xFF
#define TEST_ENTER_OPERATION    0x0D

/* Exported functions --------------------------------------------------------*/   
IL005_StatusTypeDef test_wakeup(void);
IL005_StatusTypeDef test_idle(void);
IL005_StatusTypeDef test_sleep(void);
void il005_menu(void);

#endif /*__IL005TEST_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/