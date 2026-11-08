/****************************(C) COPYRIGHT 2016 ICTK Co., LTD.*************************************/
/* program		  	: g3test.h 				  											                                 	*/
/* processor 	  	: STM32F405RGT																	                               	*/
/* compiler		  	: IAR ARM 7.7 															                               			*/
/* program BY	  	: bckang																		                                    */
/* date			  	  : 2016.02.02																		                                */
/* copyright	  	: ICTK Co., LTD																                              		*/
/**************************************************************************************************/
#ifndef __G3_TEST_H
#define __G3_TEST_H

/* Includes ------------------------------------------------------------------*/
//#include "stm32f4xx_hal.h"
#include "g3_define.h"

/* Defines ------------------------------------------------------------------*/

/* Functions ------------------------------------------------------------------*/
void SM2_Enc_Dec(void);
void SM2_Sign_Verify();
void Sign_Verify_AES(void);
void Sign_Verify_ECC(void);
void Sign_Verify_SM4(void);
void Dynamic_Auth(void);
void Diversify(void);
void Issue_Certificate(void);
void Certificate_To_PublicKey(void);
void Certificate_To_Temp(void);
void Enc_Dec_test_SM4(void);
void Enc_Dec_test_AES128(void);	
void SHA256_test(void);
void Enc_Dec_ECIES_test(void);
void ECDH_test(void);
void SM2_Enc_Dec_512(void);
void SM2_Enc_Dec_1024(void);
void SM2_Sign_Verify_512();

/* Exported functions --------------------------------------------------------*/   
void G3_test_menu(void);

#endif /*__IG3TEST_H*/

/****************************(C) COPYRIGHT 2016 ICTK Co., LTD.*************************************/