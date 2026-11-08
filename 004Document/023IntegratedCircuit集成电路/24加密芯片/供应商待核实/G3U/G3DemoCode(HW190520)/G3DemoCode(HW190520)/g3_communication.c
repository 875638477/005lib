/** 
  *****************************************************************************
  * @file    		    : G3communication.c
  * @author         : Miller IC Trust Key China
  * @version        : V1.1.0
  * @date           : 2-July-2018
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 7.7
  * @brief          : Communication Layer of G3 Library
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#include <string.h> // needed for memset(), memcmp()

#include "g3_communication.h"
#include "g3_hal_i2c.h"
#include "i2c_sw.h"

#include "timer_util.h"
#include "led.h"
#include "gpio.h"
#include "ictk_lib.h"

uint16_t u16_G3_Send_Retry_Count = G3_SEND_RETRY_COUNT;     // Retry Count for sending command.
uint16_t u16_G3_Recv_Retry_Count = G3_RECEIVE_RETRY_COUNT;  // Retry Count for receiving respons.

uint8_t u8_G3_Resp_time = EXEC_MIN_DELAY;  // delay time between command and response.

static I2C_HandleTypeDef I2cHandle;
uint8_t wakeup_response[4] = {0x04,0x11,0x33,0x43}; 

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////Wakeup////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
G3_StatusTypeDef G3_wakeup(void)  //G3芯片唤醒，函数内部实现：SDA先拉低大于80us,然后再拉高延时大于10ms
{
  G3_StatusTypeDef ret_code;
  uint8_t result[6];

  //将I2C设置成G3_I2C_NORMAL_FREQ速率，100KHZ, G3唤醒时时钟不能大于100KHZ
  MX_I2C1_Set_Frq(&I2cHandle, G3_I2C_NORMAL_FREQ);

  //此处，I2C往""0x00地址设备""，发送一个byte的0x00数据，使SDA有一个低电平，时间大于80us
  HAL_I2C_Master_Transmit(&I2cHandle, 0x00, (uint8_t*)0x00, 1, 1);         //Wake up low Delay

  //使SDA有一个高电平10ms左右延时
  delay_ms(10);       //Wake up high Delay 

  //此处，从""0xC8地址设备""(即G3默认的地址)，读取4个byte数据(注意传入的地址是否有偏移，不同的平台可能地址转换不一样)
  //从G3读取4个Byte数据，如果返回结果为{0x04,0x11,0x33,0x43}，则表示G3唤醒成功
  HAL_I2C_Master_Receive( &I2cHandle, G3_DEFAULT_I2C_DEVICE_ADDRESS, result, 4, G3_I2C_TIME_OUT);
  if(memcmp(result, wakeup_response, 4) == 0)
  {
    ret_code = G3_OK;
	PRINTF("\r\n > G3 WAKE\r\n");
  }
  else
  {
    ret_code = G3_WAKEUP_FAIL;
	PRINTF("\r\n > WAKE Error : 0x%X \r\n", ret_code);
  }
  return ret_code;
 }


G3_StatusTypeDef G3_reset(void)
{
  G3_StatusTypeDef ret_code = G3_OK;
  uint8_t G3_I2C_WORDADDRESS_RESET = 0x00;
  
  if(HAL_OK == HAL_I2C_Master_Transmit(&I2cHandle, G3_DEFAULT_I2C_DEVICE_ADDRESS, &G3_I2C_WORDADDRESS_RESET, 1, G3_I2C_TIME_OUT)) 
  {
	return ret_code;  
  }
  else
  {
	ret_code = G3_ERROR;
	return ret_code;  
  }
}


G3_StatusTypeDef G3_idle(void)
{
  G3_StatusTypeDef ret_code = G3_OK;
  uint8_t G3_I2C_WORDADDRESS_IDLE = 0x02;
  
  if(HAL_OK == HAL_I2C_Master_Transmit(&I2cHandle, G3_DEFAULT_I2C_DEVICE_ADDRESS, &G3_I2C_WORDADDRESS_IDLE, 1, G3_I2C_TIME_OUT)) 
  {
	return ret_code;  
  }
  else
  {
	ret_code = G3_ERROR;
	return ret_code;  
  }
}


G3_StatusTypeDef G3_sleep(void)
{
  G3_StatusTypeDef ret_code = G3_OK;
  uint8_t G3_I2C_WORDADDRESS_SLEEP = 0x01;
  
  if(HAL_OK == HAL_I2C_Master_Transmit(&I2cHandle, G3_DEFAULT_I2C_DEVICE_ADDRESS, &G3_I2C_WORDADDRESS_SLEEP, 1, G3_I2C_TIME_OUT)) 
  {
  	return ret_code;  
  }
  else
  {
    ret_code = G3_ERROR;
  	return ret_code;  
  }
}



G3_StatusTypeDef g3_c_send_command(uint8_t *command)
{
  G3_StatusTypeDef ret_code;
  uint16_t u16_retry_cnt;
  uint8_t command_buf[265];

  command_buf[0] = G3_I2C_WORDADDRESS_COMMAND;
  memcpy(command_buf+1, command, command[0]);
  
  for(u16_retry_cnt = 0; u16_retry_cnt < G3_SEND_RETRY_COUNT; u16_retry_cnt++)
  {  
    //通过硬件I2C接口，发送数据包command_buf，command[0]+1为command_buf数据长度
    if(HAL_OK == (ret_code = HAL_I2C_Master_Transmit(&I2cHandle, G3_DEFAULT_I2C_DEVICE_ADDRESS, command_buf, command[0]+1, G3_I2C_TIME_OUT)))
    {
      ret_code = G3_OK;
      return ret_code;
    }
    else
    {
      ret_code = G3_I2C_NO_ACK ;
    }
    
    delay_ms(EXEC_MIN_DELAY);
  }
  
  return ret_code; 
}

G3_StatusTypeDef g3_c_receive_response(uint8_t *response)
{
  G3_StatusTypeDef ret_code = G3_OK;
  uint16_t u16_retry_cnt;
  
  for(u16_retry_cnt = 0; u16_retry_cnt < G3_SEND_RETRY_COUNT; u16_retry_cnt++)
  {
    //通过硬件I2C接口，先从G3读取一个byte数据,response[0]变成G3要返回的数据包长度
    if(HAL_OK == HAL_I2C_Master_Receive(&I2cHandle, G3_DEFAULT_I2C_DEVICE_ADDRESS, response, 1, G3_I2C_TIME_OUT))
    {
      delay_ms(EXEC_MIN_DELAY);
      if(response[0] != 0xFF)
      {
        //通过硬件I2C接口，将剩下的G3要返回的数据读完
        HAL_I2C_Master_Receive(&I2cHandle, G3_DEFAULT_I2C_DEVICE_ADDRESS, response+1, response[0]-1, G3_I2C_TIME_OUT);
        ret_code = G3_OK;
        return ret_code;
      }
    }
    else
    {
      ret_code = G3_I2C_NO_ACK ;
    }
    
    delay_ms(EXEC_MIN_DELAY);  
  }
  
  return ret_code;  
}


/**
  * @brief  This function sends a command to the G3 device and receives a response from the G3 device.
  * @param  command pointer to command buffer 
  * @param  response pointer to response buffer 
  * @return status of the operation
  */
G3_StatusTypeDef g3_c_send_and_receive(uint8_t *command, uint8_t *response)
{
  G3_StatusTypeDef ret_code = G3_OK;
  
  // Send Command.
  ret_code = g3_c_send_command(command);    // send command
  if(ret_code != G3_OK)
    return ret_code;

  // Wait minimum command execution time and then start polling for a response.
  delay_ms(u8_G3_Resp_time);
  
  ret_code = g3_c_receive_response(response);   // receive response
  
  return ret_code;
}

/****************************(C) COPYRIGHT 2016 ICTK Co., LTD.*************************************/
