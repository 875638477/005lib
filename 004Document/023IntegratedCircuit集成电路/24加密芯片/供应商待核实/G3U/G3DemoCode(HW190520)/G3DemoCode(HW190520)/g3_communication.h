/** 
  *****************************************************************************
  * @file    		    : G3communication.h
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 7.7
  * @brief          : Header of G3communication.c file.
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __G3_COMMUNICATION_H
#define __G3_COMMUNICATION_H

/* Includes ------------------------------------------------------------------*/
#include "g3_define.h"

/* Defines -------------------------------------------------------------------*/


/* Variables -----------------------------------------------------------------*/
extern uint16_t u16_G3_Send_Retry_Count;
extern uint16_t u16_G3_Recv_Retry_Count;
extern uint8_t  u8_G3_Resp_time;


/* Exported functions --------------------------------------------------------*/   
G3_StatusTypeDef G3_wakeup(void);
G3_StatusTypeDef G3_idle(void);
G3_StatusTypeDef G3_sleep(void);
G3_StatusTypeDef G3_reset(void);


G3_StatusTypeDef g3_c_send_command(uint8_t *command);
G3_StatusTypeDef g3_c_receive_response(uint8_t *response);
G3_StatusTypeDef g3_c_send_and_receive(uint8_t *command, uint8_t *response);

#endif /*__G3COMMUNICATION_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/


