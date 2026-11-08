/****************************(C) COPYRIGHT 2016 ICTK Co., LTD.*************************************/
/* program		  	: ictk_lib.h 		      											                                   	*/
/* processor 	  	: STM32F405RGT																	                               	*/
/* compiler		  	: IAR ARM 6.6 															                               			*/
/* program BY	  	: bckang																		                                    */
/* date			  	  : 2016.02.02																		                                */
/* copyright	  	: ICTK Co., LTD																                              		*/
/**************************************************************************************************/
#ifndef __ICTK_LIB_H
#define __ICTK_LIB_H

/* Includes ------------------------------------------------------------------*/
//#include "stm32f4xx_hal.h"
#include "il005define.h"

/* Defines ------------------------------------------------------------------*/
#define TEST_PROCESS_PRINT 	1
#if TEST_PROCESS_PRINT
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
  #define GETC(args...)
#endif

/* Exported functions --------------------------------------------------------*/   
void iprintf(char *fmt,...);
void cmd_printf(char *fmt, uint8_t *cmd);
void rsp_printf(char *fmt, uint8_t *rsp);
void array_printf(char *fmt, uint8_t *array, uint8_t array_cnt);
uint8_t igetc(uint8_t *rxch);
uint32_t mEnterInput(uint8_t length);
uint8_t mgets(uint8_t *pBuf, uint16_t length);

#endif /*__ICTK_LIB_H*/

/****************************(C) COPYRIGHT 2016 ICTK Co., LTD.*************************************/

