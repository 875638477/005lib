/** 
  *****************************************************************************
  * @file    		    : il005example.c
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : IL005 Example Functions of IL005 Application Layer
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#include <string.h>

#include "il005example.h"
#include "ictk_lib.h"
#include "il005communication.h"
#include "il005command.h"
#include "sha256.h"
#include "il005test.h"

static void test_Challange(void)     // Challange Command Test
{
  uint8_t ret_code;
  uint8_t temp[100];
  uint8_t sha[32];
  
  uint8_t mac_challege_data[32] = {0x00};
  uint8_t nonce_input_data[20] = {0x26,0x1B,0x74,0x4A,0x4A,0xE7,0x47,0x81,0xF5,0xE3,
                                  0xB1,0x5E,0xC2,0xA5,0xAB,0x8B,0x68,0xF3,0x15,0xF2};
  uint8_t tempkey_tail[3] = {0x16,0x00,0x00};

  uint8_t mac_cal_data[15] = {0x08,0x41,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                              0x00,0x00,0x00,0x00,0x00};

  uint8_t il005_SN[32];
  uint8_t il005_NonceOut[32];
  uint8_t il005_MacOut[32];
  uint8_t il005_TempKey[32];

  uint8_t il005_ShareKey[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  
  
  // Read SN
  ret_code = test_wakeup();                      // IL005 Wake
  if(ret_code != IL005_OK)
    return;   

  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_Read(temp, 0x80, 0x0000);
  if(ret_code != IL005_OK)                       // Error response
 	{
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);   
        return;   
}

  memcpy(il005_SN, temp+1, 32);
   
  // Nonce
  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_GenNonce(temp, 0x00, nonce_input_data);
  if(ret_code != IL005_OK)                       // Error response
 	{
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);   
        return;   
}   

  memcpy(il005_NonceOut, temp+1, 32);
  
  // MAC
  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_GenMAC(temp, 0x41, 0x0000, mac_challege_data);
  if(ret_code != IL005_OK)                       // Error response  
 	{
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);   
        return;   
}      

  ret_code = test_idle();                        // IL005 Idle
  if(ret_code != IL005_OK)
    return;

  memcpy(il005_MacOut, temp+1, 32);      

  memset(temp, 0x00, sizeof(temp));
  
  memcpy(temp, il005_NonceOut, 32);
  memcpy(temp+32, nonce_input_data, 20);
  memcpy(temp+32+20, tempkey_tail, 3);
  
  sha256(temp, 32+20+3, il005_TempKey);
  
  memset(temp, 0x00, sizeof(temp));
  
  memcpy(temp, il005_ShareKey, 32);
  memcpy(temp+32, il005_TempKey, 32);
  memcpy(temp+32+32, mac_cal_data, 15);
  memcpy(temp+32+32+15, &il005_SN[12], 1);
  memcpy(temp+32+32+15+1, &il005_SN[8], 4);
  memcpy(temp+32+32+15+1+4, &il005_SN[0], 4);
  
  sha256(temp, 32+32+15+1+4+4, sha);
  
  ARRAY_PRINTF("IL005 MAC", il005_MacOut, 32);
  ARRAY_PRINTF("HOST  MAC", sha, 32); 
}

static void sha(void)
{ 
  uint8_t tempbuf[512];
  uint8_t templen;
  uint8_t sha[32], sha2[32];

  
  memset(tempbuf, 0x00, sizeof(tempbuf));
  PRINTF("\r\n Data : ");
  templen = mgets(tempbuf, 250);
  
  sha256(tempbuf, templen, sha);    
  
  ARRAY_PRINTF("SHA1", sha, 32);   
}

static void il005_example_menu_display(void)
{
  PRINTF("\r\n***** ICTK IL005 Example MENU *****\r\n\r\n");
  PRINTF("1. Test Challange\r\n");
  PRINTF("2. Raspberry SHA256\r\n");
  PRINTF("q. Previous MENU\r\n");  
  PRINTF("\r\n***********************************\r\n");
  PRINTF("> ");
}

void il005_example_menu(void)
{
	uint8_t sub_cmd;
	uint8_t current_status=1;
  
  il005_example_menu_display();
  
  sub_cmd = TEST_NO_OPERATION;
  do
  {
		if(GETC(&sub_cmd))
		{
			PRINTF("%c \r\n", sub_cmd);
		}
		switch(sub_cmd)
		{
			case '1':
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_Challange();
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;      
			case '2':
        PRINTF("\r\n >>>TEST START>>> \r\n");
        sha();
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;      
			case 'q':
			case 'Q':
				return;
			break;       			 
      case TEST_ENTER_OPERATION:
      break;
			case TEST_END_OPERATION:
				current_status=0;
				PRINTF("TEST END !!\r\n");
			break;				
			case TEST_NO_OPERATION:
			break;
			default:
				PRINTF("NOT VAILD\r\n\r\n> ");
				sub_cmd = TEST_NO_OPERATION;
			break;
		}
		if((sub_cmd != TEST_END_OPERATION)&& (sub_cmd != TEST_NO_OPERATION))
    { 
      if(sub_cmd != TEST_ENTER_OPERATION)
        PRINTF("Press Enter...");
      while(sub_cmd != TEST_ENTER_OPERATION)
      {
        GETC(&sub_cmd);
      }
      PRINTF("\r\n");
			il005_example_menu_display();
      sub_cmd = TEST_NO_OPERATION;
		}    
	} while(current_status);
}

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/