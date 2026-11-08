/****************************(C) COPYRIGHT 2016 ICTK Co., LTD.*************************************/
/* program		  	: test.h 				  											                                       	*/
/* processor 	  	: STM32F405RGT																	                               	*/
/* compiler		  	: IAR ARM 7.7 															                               			*/
/* program BY	  	: bckang																		                                    */
/* date			  	  : 2016.02.02																		                                */
/* copyright	  	: ICTK Co., LTD																                              		*/
/**************************************************************************************************/
#ifndef __TEST_H
#define __TEST_H

/* Includes ------------------------------------------------------------------*/
//#include "stm32f4xx_hal.h"

/* Defines ------------------------------------------------------------------*/
#define TEST_END_OPERATION      0x03
#define TEST_NO_OPERATION       0xFF
#define TEST_ENTER_OPERATION    0x0D

/* Exported functions --------------------------------------------------------*/   
void test_menu(void);

#endif /*__TEST_H*/

/****************************(C) COPYRIGHT 2016 ICTK Co., LTD.*************************************/