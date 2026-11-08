/** 
  *****************************************************************************
  * @file    		    : ictk_lib.h
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 7.7
  * @brief          : Header of ictk_lib.c file.
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __ICTK_LIB_H
#define __ICTK_LIB_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "ictk_define.h"

/* Defines ------------------------------------------------------------------*/
#if 1
  #define PRINTF iprintf
  #define CMD_PRINTF cmd_printf
  #define RSP_PRINTF rsp_printf
  #define ARRAY_PRINTF array_printf
  #define GETC igetc
#else
  #define PRINTF(args...)
	#define CMD_PRINTF(args...)
  #define RSP_PRINTF(args...)
  #define ARRAY_PRINTF(args...)
  #define GETC igetc
#endif

/* Exported functions --------------------------------------------------------*/   
void iprintf(char *fmt,...);
void cmd_printf(char *fmt, uint8_t *cmd);
void rsp_printf(char *fmt, uint8_t *rsp);
void array_printf(char *fmt, uint8_t *array, uint16_t array_cnt);
uint8_t igetc(uint8_t *rxch);
uint32_t mEnterInput(uint8_t length);
uint8_t mgets(uint8_t *pBuf, uint16_t length);

#endif /*__ICTK_LIB_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/