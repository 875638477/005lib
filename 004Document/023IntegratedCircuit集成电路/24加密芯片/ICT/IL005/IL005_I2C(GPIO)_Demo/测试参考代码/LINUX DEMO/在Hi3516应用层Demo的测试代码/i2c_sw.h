/** 
  *****************************************************************************
  * @file    		    : i2c_sw.h
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : Header of i2c_sw.c file.
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __I2C_SW_H
#define __I2C_SW_H

/* Includes ------------------------------------------------------------------*/
#include "il005define.h"   
#include "Hi3516_gpio.h"

/* Variables ------------------------------------------------------------------*/


/* Defines ------------------------------------------------------------------*/

#define SCL_H                 *((unsigned int *)scl_addr) = 1<<4
#define SCL_L                 *((unsigned int *)scl_addr) = 0<<4

#define SDA_H                 *((unsigned int *)sda_addr) = 1<<3
#define SDA_L                 *((unsigned int *)sda_addr) = 0<<3

#define SDA_READ()            hi3516a_pin_read()
#define SDA_RESET             0

#if 0 // Not used for raspberry pi
#define I2C_SPEED_1MHZ        21//23(define)
#define I2C_SPEED_400KHZ      63//65(define)
#define I2C_SPEED_100KHZ      273//275(define)
#define I2C_SPEED_50KHZ       553//?(define)
#endif

#define I2C_CLOCK_TIME        100  // Min. 17(variable) //19(define)

#define I2C_CLOCK_HIGH_TIME   I2C_CLOCK_TIME       // Min. 10(variable) // 14(define)
#define I2C_CLOCK_LOW_TIME    I2C_CLOCK_TIME       // Min. 10(variable) // 13(define)

extern uint16_t i2c_clock_time;
extern uint16_t i2c_clock_high_time;
extern uint16_t i2c_clock_low_time;


/* Exported functions --------------------------------------------------------*/   
void i2c_clock_set(uint16_t clock);
IL005_StatusTypeDef i2c_start(void);
void i2c_init(void);
void i2c_stop(void);
void i2c_ack(void);
void i2c_noack(void);
IL005_StatusTypeDef i2c_waitack(void);
IL005_StatusTypeDef i2c_wakeup(uint16_t delay_time);
void i2c_sendbyte(uint8_t sendbyte);
uint8_t i2c_receivebyte(void);


#endif /*__I2C_SW_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/
