/** 
  *****************************************************************************
  * @file    		    : il005hal_i2c.h
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : Header of il005hal_i2c.c file.
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __IL005HAL_I2C_H
#define __IL005HAL_I2C_H

/* Includes ------------------------------------------------------------------*/
#include "il005define.h"

/* Variables ------------------------------------------------------------------*/
extern uint8_t u8IL005_I2C_ADDRESS;


/* Defines ------------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/   
IL005_StatusTypeDef il005p_wakeup(uint16_t delay_time);
IL005_StatusTypeDef il005p_reset(void);
IL005_StatusTypeDef il005p_sleep(void);
IL005_StatusTypeDef il005p_idle(void);
IL005_StatusTypeDef il005p_send_command(uint8_t *command);
IL005_StatusTypeDef il005p_receive_response(uint8_t *response);

#endif /*__IL005HAL_I2C_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/

