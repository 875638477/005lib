/** 
  *****************************************************************************
  * @file    		    : il005hal_i2c.c
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : Functions for I2C Physical Hardware Independent Layer of IL005 Library
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#include "il005hal_i2c.h"
#include "i2c_sw.h"
#include "timer_util.h"

uint8_t u8IL005_I2C_ADDRESS = IL005_DEFAULT_I2C_DEVICE_ADDRESS;

#ifndef NULL
#define NULL (uint8_t*)0
#endif

/**
  * @brief  This function sends a I2C packet enclosed by a I2C start and stop to a IL005 device.
  * @param  instruction_flag value listed in instruction flag
  * @param  pData pointer to data buffer
  * @param  Size number of bytes in data buffer
  * @return status of the operation
  */
static IL005_StatusTypeDef il005p_i2c_send(uint8_t instruction_flag, uint8_t *pData, uint8_t Size)
{
  IL005_StatusTypeDef ret_code = IL005_OK;

  ret_code = i2c_start();
  if(ret_code != IL005_OK)
  {
    i2c_stop();
    return ret_code;
  }
  
  i2c_sendbyte(u8IL005_I2C_ADDRESS & 0xFE);
  
  ret_code = i2c_waitack();
  if(ret_code != IL005_OK)
  {
    i2c_stop();
    return ret_code;
  }

  i2c_sendbyte(instruction_flag);
  
  ret_code = i2c_waitack();
  if(ret_code != IL005_OK)
  {
    i2c_stop();
    return ret_code;
  }    


  while(Size--)
  {
    i2c_sendbyte(*pData);
    
    ret_code = i2c_waitack();
    if(ret_code != IL005_OK)
    {
      i2c_stop();
      return ret_code;
    }
    pData++;
  }
  i2c_stop();
  
  return ret_code;  
}

/**
  * @brief  This I2C function generates a Wake-up pulse and delays.
  * @param  delay_time Wake-up low duration 
  * @return status of the operation
  */
IL005_StatusTypeDef il005p_wakeup(uint16_t delay_time)
{
  IL005_StatusTypeDef ret_code = IL005_OK;

  ret_code = i2c_wakeup(delay_time);
  if(ret_code != IL005_OK)
    return ret_code;
  
  delay_us(WAKE_HIGH_DURATION);

  return ret_code;
}

/**
  * @brief  This I2C function resets the I/O buffer of the IL005 device.
  * @return status of the operation
  */
IL005_StatusTypeDef il005p_reset(void)  // I2C RESET Word Address Value : 0x00
{
  return il005p_i2c_send(IL005_I2C_WORDADDRESS_RESET, NULL, 0);
}

/**
  * @brief  This I2C function puts the IL005 device into low-power state.
  * @return status of the operation
  */
IL005_StatusTypeDef il005p_sleep(void)  // I2C SLEEP Word Address Value : 0x01
{
  return il005p_i2c_send(IL005_I2C_WORDADDRESS_SLEEP, NULL, 0);
}

/**
  * @brief  This I2C function puts the IL005 device into idle state.
  * @return status of the operation
  */
IL005_StatusTypeDef il005p_idle(void)  // I2C IDLE Word Address Value : 0x02 
{
  return il005p_i2c_send(IL005_I2C_WORDADDRESS_IDLE, NULL, 0);
}

/**
  * @brief  This I2C function sends a command to the device.
  * @param  command  pointer to command buffer
  * @return status of the operation
  */
IL005_StatusTypeDef il005p_send_command(uint8_t *command)  // I2C COMMAND Word Address Value : 0x03  
{  
//  if(command[0] > IL005_CMD_SIZE_MAX)
//    return IL005_INVALID_SIZE;  
 
  return il005p_i2c_send(IL005_I2C_WORDADDRESS_COMMAND, command, command[0]); 
}

/**
  * @brief  This I2C function receives a response from the IL005 device.
  * @param  response  pointer to rx buffer
  * @return status of the operation
  */
IL005_StatusTypeDef il005p_receive_response(uint8_t *response)
{
  IL005_StatusTypeDef ret_code = IL005_OK;  
  uint8_t temp_cnt;
  
  ret_code = i2c_start();
  if(ret_code != IL005_OK)
  {
    i2c_stop();
    return ret_code;
  }
  
  i2c_sendbyte(u8IL005_I2C_ADDRESS | 0x01);
  
  ret_code = i2c_waitack();
  if(ret_code != IL005_OK)
  {
    i2c_stop();
    return ret_code;
  }

  *response = i2c_receivebyte();
  if(*response == 0xFF)
  {
    i2c_noack();    
    i2c_stop();      
    return IL005_BUSY;      
  }
  else if((*response < IL005_RSP_SIZE_MIN ) || (*response > IL005_RSP_SIZE_MAX))
  {
    i2c_noack();    
    i2c_stop();         
    return IL005_INVALID_SIZE;
  }
  else
  {
    i2c_ack();          
    temp_cnt = response[0] - 1;    
    response++;
  }

  while(temp_cnt)
  {
    *response = i2c_receivebyte();
    
    if(temp_cnt == 1)
      i2c_noack();
    else
      i2c_ack(); 
    
    response++;
    temp_cnt--;
  }
  i2c_stop();    
  
  return ret_code;
}
/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/