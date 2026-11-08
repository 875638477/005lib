/** 
  *****************************************************************************
  * @file    		    : i2c_sw.c
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : Functions of I2C Hardware Dependent Part of IL005 Physical Layer
  *                   Using GPIO For Communication
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#include "i2c_sw.h"
#include "timer_util.h"
#include "ictk_lib.h"

uint16_t i2c_clock_time = I2C_CLOCK_TIME;
uint16_t i2c_clock_high_time = I2C_CLOCK_HIGH_TIME;
uint16_t i2c_clock_low_time = I2C_CLOCK_LOW_TIME;

/**
  * @brief  This function initializes and enables the I2C peripheral.
  */
void i2c_init(void)
{
#if 0
  GPIO_InitTypeDef GPIO_InitStruct;
  
  /*Configure GPIO pin : I2C1_Pin */  
  GPIO_InitStruct.Pin = I2C1_SCL | I2C1_SDA;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(I2C1_GPIO_Port, &GPIO_InitStruct);      
  HAL_GPIO_WritePin(I2C1_GPIO_Port, I2C1_SDA | I2C1_SCL, GPIO_PIN_SET);    
#else  	
//  pinMode(SDA_PIN, INPUT);
 // pullUpDnControl(SDA_PIN, PUD_OFF);
 // pinMode(SCL_PIN, OUTPUT);
 // pullUpDnControl(SCL_PIN, PUD_OFF);
 hi3516a_i2c_init();
#endif
}

/**
  * @brief  Software delay that determines the I2C bit rate.
  * @param  ulCount specifies the delay time length
  */
static void i2c_delay(uint32_t ulCount)
{
#if 0
    __asm("    subs    r0, #1\n"
          "    bne.n   i2c_delay\n"
          "    bx      lr");
#else
    uint32_t i;
DISABLE_INTERRUPT(); 
//    for(i=ulCount;i>0;i--);
delay_us(1);
ENABLE_INTERRUPT();    
#endif
}

/**
  * @brief  SCL clock frequency
  * @param  clock High or Low period of the SCL clock
  */
void i2c_clock_set(uint16_t clock)
{
  i2c_clock_time = clock;
  i2c_clock_high_time = clock;
  i2c_clock_low_time = clock;  
}

/**
  * @brief  This function creates a Start condition (SDA low, then SCL low).
  * @return status of the operation
  */
IL005_StatusTypeDef i2c_start(void)
{ 
  pinMode(SDA_PIN, OUTPUT);    
	SDA_H;
	SCL_H;
	i2c_delay(i2c_clock_high_time);
	if(SDA_READ() == SDA_RESET)
    return IL005_I2C_START_FAIL;
  
	SDA_L;
	i2c_delay(i2c_clock_low_time);
	if(SDA_READ() != SDA_RESET)  
    return IL005_I2C_START_FAIL;
  
	SCL_L;
	i2c_delay(i2c_clock_low_time);
  
	return IL005_OK;
}

/**
  * @brief  This function creates a Stop condition (SCL high, then SDA high).
  */
void i2c_stop(void)
{
  pinMode(SDA_PIN, OUTPUT);    
	SCL_L;
	SDA_L;
	i2c_delay(i2c_clock_low_time);
	SCL_H;
	i2c_delay(i2c_clock_high_time);
	SDA_H;
	i2c_delay(i2c_clock_high_time);
}


/**
  * @brief  Enable acknowledging data.
  */
void i2c_ack(void)
{	
  pinMode(SDA_PIN, OUTPUT);    
	SDA_L;
	i2c_delay(i2c_clock_low_time);
	SCL_H;
	i2c_delay(i2c_clock_high_time);
	SCL_L;
}

/**
  * @brief  Disable acknowledging data for the last byte.
  */
void i2c_noack(void)
{	
  pinMode(SDA_PIN, OUTPUT);    
	SDA_H;
	i2c_delay(i2c_clock_high_time);
	SCL_H;
	i2c_delay(i2c_clock_high_time);
	SCL_L;
}

/**
  * @brief  This function checks acknowledging data.
  * @return status of the operation
  */
IL005_StatusTypeDef i2c_waitack(void)
{
  pinMode(SDA_PIN, INPUT);      
	i2c_delay(i2c_clock_high_time);
	SCL_H;
	i2c_delay(i2c_clock_high_time);
	if(SDA_READ() != SDA_RESET)  
	{
		SCL_L;
   	return IL005_I2C_NO_ACK;
	}
	SCL_L;
	return IL005_OK;
}

/**
  * @brief  This I2C function generates a Wake-up pulse.
  * @param  delay_time Wake-up low duration
  * @return status of the operation
  */
IL005_StatusTypeDef i2c_wakeup(uint16_t delay_time)
{
  pinMode(SDA_PIN, OUTPUT);  
	SDA_H;
	SCL_H;
	i2c_delay(i2c_clock_high_time);
	if(SDA_READ() == SDA_RESET)  
{
PRINTF("1 : %X\r\n", SDA_READ()); 
    return IL005_WAKEUP_FAIL;
 }   
	SDA_L;
	i2c_delay(i2c_clock_low_time);
	if(SDA_READ() != SDA_RESET)
{
PRINTF("2"); 
    return IL005_WAKEUP_FAIL;
 }   

	SCL_L;  
	delay_us(delay_time);
  
	SCL_H;
	i2c_delay(i2c_clock_high_time);
	SDA_H;
	i2c_delay(i2c_clock_high_time);  

	return IL005_OK;
}

/**
  * @brief  This function sends one byte to an I2C device.
  * @param  sendbyte one byte to send
  */
void i2c_sendbyte(uint8_t sendbyte)
{
  uint8_t i=8;
  
  pinMode(SDA_PIN, OUTPUT);      
  while(i--)
  {
    SCL_L;
    i2c_delay(i2c_clock_low_time>>1);    
    if(sendbyte&0x80)
      SDA_H;  
    else 
      SDA_L;   
    sendbyte<<=1;
    i2c_delay(i2c_clock_low_time>>1);
    SCL_H;
    i2c_delay(i2c_clock_high_time);
  }
  SCL_L;
}

/**
  * @brief  This function receives one byte from an I2C device.
  * @return received byte
  */
uint8_t i2c_receivebyte(void) 
{ 
  uint8_t i=8;
  uint8_t receivebyte=0;
  
  pinMode(SDA_PIN, INPUT);  
  while(i--)
  {
    receivebyte<<=1;      
    SCL_L;
    i2c_delay(i2c_clock_low_time);
    SCL_H;
    i2c_delay(i2c_clock_high_time);	
    if(SDA_READ() != SDA_RESET)
      receivebyte|=0x01;
  } 
  SCL_L;   
     
  return receivebyte;
}

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/
