/** 
  *****************************************************************************
  * @file    		    : il005communication.c
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : Communication Layer of IL005 Library
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#include <string.h> // needed for memset(), memcmp()

#include "il005communication.h"
#include "il005hal_i2c.h"
#include "i2c_sw.h"
#include "timer_util.h"
#include "ictk_lib.h"

uint16_t u16_IL005_Send_Retry_Count = IL005_SEND_RETRY_COUNT;     // Retry Count for sending command.
uint16_t u16_IL005_Recv_Retry_Count = IL005_RECEIVE_RETRY_COUNT;  // Retry Count for receiving respons.

uint8_t u8_IL005_Resp_time = EXEC_MIN_DELAY;  // delay time between command and response.

/**
  * @brief  This function wakes up a IL005 device and receives a response.
  * @param  delay_time Wake-up low duration 
  * @return status of the operation
  */
IL005_StatusTypeDef il005c_wakeup(uint16_t delay_time)
{
  IL005_StatusTypeDef ret_code;
  uint8_t wake_read_data[4] = {0x04,0x11,0x33,0x43};    // status code after wake-up
  uint8_t rtemp[IL005_RSP_SIZE_MAX];  

#ifdef SYSTEMINTERRUPT_DISABLE
  DISABLE_INTERRUPT();   // Disable Interrupt
#endif      
  ret_code = il005p_wakeup(delay_time);   // Wake-up signal
#ifdef SYSTEMINTERRUPT_DISABLE
  ENABLE_INTERRUPT();   // Enable Interrupt
#endif     
  if(ret_code != IL005_OK)
    return ret_code;  
  
  memset(rtemp, 0x00, sizeof(rtemp));
  ret_code = il005c_receive_response(rtemp);  
  if(ret_code == IL005_OK)
  {
    if(memcmp((char*)rtemp, (char*)wake_read_data, 4))// Confirms IL005 has received a normal proper Wake-up.
      return IL005_WAKEUP_FAIL;    
  }
  
  return ret_code;
}

/**
  * @brief  This function resets the I/O buffer of the IL005 device.
  * @return status of the operation
  */
IL005_StatusTypeDef il005c_reset(void)
{
  IL005_StatusTypeDef ret_code;
  
#ifdef SYSTEMINTERRUPT_DISABLE
  DISABLE_INTERRUPT();   // Disable Interrupt
#endif   
  ret_code = il005p_reset();    // reset
#ifdef SYSTEMINTERRUPT_DISABLE
  ENABLE_INTERRUPT();   // Enable Interrupt
#endif       
  
  return ret_code;
}

/**
  * @brief  This function puts the IL005 device into low-power state.
  * @return status of the operation
  */
IL005_StatusTypeDef il005c_sleep(void)
{
  IL005_StatusTypeDef ret_code; 
  
#ifdef SYSTEMINTERRUPT_DISABLE
  DISABLE_INTERRUPT();   // Disable Interrupt
#endif     
  ret_code = il005p_sleep();    // sleep
#ifdef SYSTEMINTERRUPT_DISABLE
  ENABLE_INTERRUPT();   // Enable Interrupt
#endif     
  
  return ret_code;  
}

/**
  * @brief  This function puts the IL005 device into idle state.
  * @return status of the operation
  */
IL005_StatusTypeDef il005c_idle(void)
{
  IL005_StatusTypeDef ret_code;  
  
#ifdef SYSTEMINTERRUPT_DISABLE
  DISABLE_INTERRUPT();    // Disable Interrupt
#endif     
  ret_code = il005p_idle();   // idle
#ifdef SYSTEMINTERRUPT_DISABLE
  ENABLE_INTERRUPT();   // Enable Interrupt
#endif     
  
  return ret_code;  
}

/**
  * @brief  This function sends a command to the IL005 device.
  * @param  command pointer to command buffer 
  * @return status of the operation
  */
IL005_StatusTypeDef il005c_send_command(uint8_t *command)
{
  IL005_StatusTypeDef ret_code = IL005_OK;
  uint16_t u16_retry_cnt;
  
  u16_retry_cnt = u16_IL005_Send_Retry_Count + 1;
  while(u16_retry_cnt > 0)
  {
#ifdef SYSTEMINTERRUPT_DISABLE
  DISABLE_INTERRUPT();    // Disable Interrupt
#endif       
    ret_code = il005p_send_command(command);   // send command
#ifdef SYSTEMINTERRUPT_DISABLE
  ENABLE_INTERRUPT();   // Enable Interrupt
#endif     
    if(ret_code == IL005_OK)
      break;
    else
      u16_retry_cnt = u16_retry_cnt - 1;
  }  
  
  return ret_code; 
}

/**
  * @brief  This function receives a response from the IL005 device.
  * @param  response pointer to response buffer 
  * @return status of the operation
  */
IL005_StatusTypeDef il005c_receive_response(uint8_t *response)
{
  IL005_StatusTypeDef ret_code = IL005_OK;
  uint16_t u16_retry_cnt;

  u16_retry_cnt = u16_IL005_Recv_Retry_Count + 1;
  while(u16_retry_cnt > 0)
  {
#ifdef SYSTEMINTERRUPT_DISABLE
  DISABLE_INTERRUPT();    // Disable Interrupt
#endif       
    ret_code = il005p_receive_response(response);   // receive response
#ifdef SYSTEMINTERRUPT_DISABLE
  ENABLE_INTERRUPT();   // Enable Interrupt
#endif         
    if(ret_code == IL005_OK)
      return ret_code;
    else
    {
      u16_retry_cnt = u16_retry_cnt - 1;
      delay_ms(u8_IL005_Resp_time);
    } 
  }
  
  return ret_code;  
}

/**
  * @brief  This function sends a command to the IL005 device and receives a response from the IL005 device.
  * @param  command pointer to command buffer 
  * @param  response pointer to response buffer 
  * @return status of the operation
  */
IL005_StatusTypeDef il005c_send_and_receive(uint8_t *command, uint8_t *response)
{
  IL005_StatusTypeDef ret_code = IL005_OK;
  
  // Send Command.
  ret_code = il005c_send_command(command);    // send command
  if(ret_code != IL005_OK)
    return ret_code;

  // Wait minimum command execution time and then start polling for a response.
  delay_ms(u8_IL005_Resp_time);
  
  ret_code = il005c_receive_response(response);   // receive response
  
  return ret_code;
}

/****************************(C) COPYRIGHT 2016 ICTK Co., LTD.*************************************/
