/** 
  *****************************************************************************
  * @file    		    : il005communication.h
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : Header of il005communication.c file.
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __IL005COMMUNICATION_H
#define __IL005COMMUNICATION_H

/* Includes ------------------------------------------------------------------*/
#include "il005define.h"

/* Defines -------------------------------------------------------------------*/


/* Variables -----------------------------------------------------------------*/
extern uint16_t u16_IL005_Send_Retry_Count;
extern uint16_t u16_IL005_Recv_Retry_Count;
extern uint8_t  u8_IL005_Resp_time;


/* Exported functions --------------------------------------------------------*/   
IL005_StatusTypeDef il005c_wakeup(uint16_t delay_time);
IL005_StatusTypeDef il005c_reset(void);
IL005_StatusTypeDef il005c_sleep(void);
IL005_StatusTypeDef il005c_idle(void);
IL005_StatusTypeDef il005c_send_command(uint8_t *command);
IL005_StatusTypeDef il005c_receive_response(uint8_t *response);
IL005_StatusTypeDef il005c_send_and_receive(uint8_t *command, uint8_t *response);

#endif /*__IL005COMMUNICATION_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/


