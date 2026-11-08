/** 
  *****************************************************************************
  * @file    		    : test.c
  * @author         : Miller IC Trust Key China
  * @version        : V1.1.0
  * @date           : 2-July-2018
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : Firmware Protection Example
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */
#include <string.h>

#include "test.h"
#include "ictk_lib.h"

#include "g3_test.h"

//G3功能演示菜单
static void test_menu_display(void)
{
  PRINTF("\r\n***** ICTK G3 APPLICATION TEST MENU *****\r\n\r\n");
  PRINTF("1. G3 Wake-up Test  \r\n"); 
  PRINTF("2. G3 Sign&Verify using AES Authentication \r\n");
  PRINTF("3. G3 Sign&Verify using ECC Authentication \r\n");
  PRINTF("4. G3 Sign&Verify using SM4 Authentication \r\n");
  PRINTF("5. G3 Dynamic Authentication \r\n");
  PRINTF("6. G3 Diversify \r\n");
  PRINTF("7. G3 Issue Certificate \r\n");
  PRINTF("8. G3 Certificate To PublicKey \r\n");
  PRINTF("9. G3 Certificate To Temp \r\n"); 
  PRINTF("a. G3 Encrypt&Decrypt SM4 Test \r\n"); 
  PRINTF("b. G3 Encrypt&Decrypt AES128 Test \r\n"); 
  PRINTF("c. G3 ECDH Test \r\n"); 
  PRINTF("d. G3 SHA256 Test \r\n");
  PRINTF("i. G3 SM2_Encrypt&Decrypt Test 32 byte \r\n");
  PRINTF("j. G3 SM2_Encrypt&Decrypt Test 512 byte \r\n");
  PRINTF("k. G3 SM2_Encrypt&Decrypt Test 1024 byte \r\n");  
  PRINTF("l. G3 SM2_Sign&Verify Test \r\n");
  PRINTF("m. G3 SM2_Sign&Verify Test 512 byte ( external hash ) \r\n");  
  PRINTF("\r\n");
  PRINTF("\r\n********************************************\r\n");
  PRINTF("> ");
}

void test_menu(void)
{
  uint8_t ret_code=0;
  uint8_t sub_cmd;
  
  sub_cmd = TEST_NO_OPERATION;
  while(1)
  {
    ret_code = 0;
		if(GETC(&sub_cmd))
		{
			PRINTF("%c \r\n", sub_cmd);
		}
		switch(sub_cmd)
		{
    case '1':
      G3_wakeup();
	  G3_sleep();
    break; 
    case '2':
      Sign_Verify_AES();
    break;   
    case '3':
      Sign_Verify_ECC();
    break;   
    case '4':
      Sign_Verify_SM4();
    break; 
    case '5':
     Dynamic_Auth(); 
    break;         
    case '6':
      Diversify();
    break;    
    case '7':
      Issue_Certificate();
    break;    
    case '8':
      Certificate_To_PublicKey();
    break;    
    case '9':
      Certificate_To_Temp();
    break;  
    case 'a':
    case 'A':
     Enc_Dec_test_SM4(); 
    break;      
    case 'b':
    case 'B':      
     Enc_Dec_test_AES128();
    break;      
    case 'c':
    case 'C':       
      ECDH_test();
    break;        
    case 'd':
    case 'D':
      SHA256_test();
    break;                 
    case 'i':
    case 'I':
      SM2_Enc_Dec();
    break;  
    case 'j':
    case 'J':
      SM2_Enc_Dec_512();
    break;
    case 'k':
    case 'K':
      SM2_Enc_Dec_1024();      
    break;           
    case 'l':
    case 'L':
      SM2_Sign_Verify();
    break;  
    case 'm':
    case 'M':
      SM2_Sign_Verify_512();      
    break;      
    case TEST_ENTER_OPERATION:
    break;
    case TEST_NO_OPERATION:
    break;
    default:
      PRINTF("NOT VAILD\r\n\r\n> ");
      sub_cmd = TEST_NO_OPERATION;
    break;
		}
    if(sub_cmd != TEST_NO_OPERATION)
      {
      if(sub_cmd != TEST_ENTER_OPERATION && ret_code == 0)
        PRINTF("Press Enter...");
      while(sub_cmd != TEST_ENTER_OPERATION && ret_code == 0)
        {
        GETC(&sub_cmd);
        }
      PRINTF("\r\n");
      
      test_menu_display();
      sub_cmd = TEST_NO_OPERATION;
      }
}
}
/****************************(C) COPYRIGHT 2016 ICTK Co., LTD.*************************************/