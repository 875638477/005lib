/** 
  *****************************************************************************
  * @file    		    : timer_util.c
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 7.7
  * @brief          : Timer Utility Functions
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#include "timer_util.h"

/**
  * @brief This function receives one byte from an I2C device.
  * @param ulCount  number of ticks to delay
  */
static void _delay_tick(uint32_t ulCount)
{
  __asm("    subs    r0, #1\n"
        "    bne.n   _delay_tick\n"
        "    bx      lr");
}

/**
  * @brief This function delays for a number of microseconds.
  * @param us number of microseconds to delay
  */
void delay_us(uint32_t us)
{
  if(us == 0) return;
  _delay_tick(us * (168000000 / 3 / 1000000) - 11);
}

/**
  * @brief This function receives one byte from an I2C device.
  * @param ms number of milliseconds to delay
  */
void delay_ms(uint32_t ms)
{
  delay_us(ms * 1000);
}

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/