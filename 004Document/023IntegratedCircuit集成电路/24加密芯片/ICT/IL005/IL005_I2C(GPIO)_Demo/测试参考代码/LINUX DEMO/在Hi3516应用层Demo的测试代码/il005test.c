/** 
  *****************************************************************************
  * @file    		    : il005test.c
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : IL005 Test Functions of IL005 Application Layer
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#include <string.h>

#include "il005test.h"
#include "ictk_lib.h"
#include "il005communication.h"
#include "il005command.h"
#include "sha256.h"
#include "timer_util.h"
#include "il005example.h"

IL005_StatusTypeDef test_wakeup(void)
{
  IL005_StatusTypeDef ret_code;
  
  ret_code = il005c_wakeup(WAKE_LOW_DURATION);      // IL005 Wakeup : IL005 Wake low duration Min. 340us
  if(ret_code != IL005_OK)
  {
    PRINTF("\r\n > WAKE Error : 0x%X \r\n", ret_code);
    return ret_code;
  }
  else
    PRINTF("\r\n > IL005 WAKE \r\n");
  
  return ret_code;  
}

IL005_StatusTypeDef test_idle(void)
{
  IL005_StatusTypeDef ret_code;
  
  ret_code = il005c_idle();                        // IL005 Idle
  if(ret_code != IL005_OK)
  {
    PRINTF("\r\n > IDLE Error : 0x%X \r\n", ret_code); 
    return ret_code;
  }
  else
    PRINTF("\r\n > IL005 IDLE \r\n");
  
  return ret_code;
}

IL005_StatusTypeDef test_sleep(void)
{
  IL005_StatusTypeDef ret_code;
  
  ret_code = il005c_sleep();                        // IL005 Sleep
  if(ret_code != IL005_OK)
  {
    PRINTF("\r\n > SLEEP Error : 0x%X \r\n", ret_code); 
    return ret_code;
  }
  else
    PRINTF("\r\n > IL005 SLEEP \r\n");
  
  return ret_code;
}

static void read_setup_area(void)
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint8_t setup_area_data[88];
  uint8_t i;
  
  ret_code = test_wakeup();                       // IL005 Wake
  if(ret_code != IL005_OK)
    return;

  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_Read(temp, 0x80, 0x0000);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(setup_area_data, &temp[1], 32);
  
  ret_code = il005_Read(temp, 0x80, 0x0008);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&setup_area_data[32], &temp[1], 32);  
  
  ret_code = il005_Read(temp, 0x00, 0x0010);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&setup_area_data[64], &temp[1], 4);    
  
  ret_code = il005_Read(temp, 0x00, 0x0011);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&setup_area_data[68], &temp[1], 4);    

  ret_code = il005_Read(temp, 0x00, 0x0012);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&setup_area_data[72], &temp[1], 4);    

  ret_code = il005_Read(temp, 0x00, 0x0013);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&setup_area_data[76], &temp[1], 4);    

  ret_code = il005_Read(temp, 0x00, 0x0014);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&setup_area_data[80], &temp[1], 4);      
  
  ret_code = il005_Read(temp, 0x00, 0x0015);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&setup_area_data[84], &temp[1], 4);      
  
  ret_code = test_idle();                        // IL005 Idle
  if(ret_code != IL005_OK)
    return;
  
  PRINTF("\r\n>>>Setup Area<<<\r\n"); 
  for(i=0; i<88; i++)
  {
    if(i%4 == 0)
      iprintf(" 0x%02X :", i/4);
    PRINTF(" %02X", setup_area_data[i]); 
    if(((i+1)%4 == 0) && (i < 88 - 1))
        iprintf("\r\n");  
    if(i == 0x05*4 - 1 || i == 0x0D*4 - 1 || i == 0x11*4 - 1 || i == 0x15*4 - 1)
        iprintf("\r\n");      
  }  
  PRINTF("\r\n>>>>>>>><<<<<<<<\r\n");   
}

static void read_data_area(void)
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint8_t data_area_data[512];
  uint16_t i;
  
  ret_code = test_wakeup();                       // IL005 Wake
  if(ret_code != IL005_OK)
    return;   
  
  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_Read(temp, 0x82, 0x0000);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(data_area_data, &temp[1], 32);
  
  ret_code = il005_Read(temp, 0x82, 0x0008);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[32], &temp[1], 32);  
  
  ret_code = il005_Read(temp, 0x82, 0x0010);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[64], &temp[1], 32);    
  
  ret_code = il005_Read(temp, 0x82, 0x0018);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[96], &temp[1], 32);    
  
  ret_code = il005_Read(temp, 0x82, 0x0020);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[128], &temp[1], 32);    
  
  ret_code = il005_Read(temp, 0x82, 0x0028);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[160], &temp[1], 32);      
 
  ret_code = il005_Read(temp, 0x82, 0x0030);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[192], &temp[1], 32);    
  
  ret_code = il005_Read(temp, 0x82, 0x0038);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[224], &temp[1], 32);     
  
  ret_code = il005_Read(temp, 0x82, 0x0040);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[256], &temp[1], 32);    
  
  ret_code = il005_Read(temp, 0x82, 0x0048);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[288], &temp[1], 32);     
  
  ret_code = il005_Read(temp, 0x82, 0x0050);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[320], &temp[1], 32);    
  
  ret_code = il005_Read(temp, 0x82, 0x0058);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[352], &temp[1], 32);       

  ret_code = il005_Read(temp, 0x82, 0x0060);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[384], &temp[1], 32);    
  
  ret_code = il005_Read(temp, 0x82, 0x0068);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[416], &temp[1], 32);     
  
  ret_code = il005_Read(temp, 0x82, 0x0070);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[448], &temp[1], 32);    
  
  ret_code = il005_Read(temp, 0x82, 0x0078);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&data_area_data[480], &temp[1], 32);    
  
  ret_code = test_idle();                         // IL005 Idle
  if(ret_code != IL005_OK)
    return;
  
  PRINTF("\r\n>>>Data Area<<<\r\n"); 
  for(i=0; i<512; i++)
  {
    if(i%32 == 0)
      iprintf(" 0x%04X :", i/4);
    PRINTF(" %02X", data_area_data[i]); 
    if((((i+1)/16)%2 == 1) && ((i+1)%16 == 0))
      iprintf("\r\n         ");
    if(((i+1)%32 == 0) && (i < 512 - 1))
      iprintf("\r\n");  
  }  
  PRINTF("\r\n>>>>>>><<<<<<<\r\n");   
}

static void read_otp_area(void)
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint8_t otp_area_data[64];
  uint8_t i;
  
  ret_code = test_wakeup();
  if(ret_code != IL005_OK)
    return;   

  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_Read(temp, 0x81, 0x0000);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(otp_area_data, &temp[1], 32);
  
  ret_code = il005_Read(temp, 0x81, 0x0008);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  memcpy(&otp_area_data[32], &temp[1], 32);  
    
  ret_code = test_idle();                         // IL005 Idle
  if(ret_code != IL005_OK)
    return;
  
  PRINTF("\r\n>>>Otp Area<<<\r\n"); 
  for(i=0; i<64; i++)
  {
    if(i%32 == 0)
      iprintf(" 0x%04X :", i/4);
    PRINTF(" %02X", otp_area_data[i]); 
    if((((i+1)/16)%2 == 1) && ((i+1)%16 == 0))
      iprintf("\r\n         ");
    if(((i+1)%32 == 0) && (i < 64 - 1))
      iprintf("\r\n");  
  }  
  PRINTF("\r\n>>>>>><<<<<<\r\n");   
}

static void test_Select(uint8_t owi_address)    // Select Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint8_t new_value[4] = {0x77,0x77,0x77,0x77};
  uint8_t old_value[4];  
  
  ret_code = il005c_wakeup(WAKE_LOW_DURATION);      // IL005 Wakeup : IL005 Wake low duration Min. 340us
  if(ret_code != IL005_OK)
  {
    PRINTF("\r\n > WAKE Error : 0x%X \r\n", ret_code);
    return;
  }
  else
    PRINTF("\r\n > IL005 WAKE \r\n");       
  
  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_Select(temp, owi_address);       // Select Command
  if(ret_code != IL005_OK)
  {
//    (void)test_sleep();
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
    return;
  }
  
  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_Version(temp);                   // Version Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);  
  
  memset(temp, 0x00, sizeof(temp));    
  ret_code = il005_Read(temp, 0x00, 0x000E);                     // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code); 
  
  memcpy(old_value, &temp[1], 4);
  
  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_Write(temp, 0x00, 0x000E, new_value, NULL);  // Write Command
  if(ret_code != IL005_OK)
  {
    (void)test_sleep();    
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);  
    return;
  }
  
  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_Write(temp, 0x00, 0x000E, old_value, NULL);  // Write Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);  

  ret_code = test_idle();                           // IL005 Idle
  if(ret_code != IL005_OK)
    return; 
}

static void test_Read(uint8_t zone, uint16_t address)   // Read Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  
  memset(temp, 0x00, sizeof(temp));
  ret_code = test_wakeup();                        // IL005 Wake
  if(ret_code != IL005_OK)
    return;    
  
  ret_code = il005_Read(temp, zone, address);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  ret_code = test_idle();                          // IL005 Idle
  if(ret_code != IL005_OK)
    return;
}

static void test_GenMAC(void)   // GenMAC Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint8_t nonce_input_data[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                  0x00,0x00};
  
  ret_code = test_wakeup();                                 // IL005 Wake
  if(ret_code != IL005_OK)
    return;       

  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_GenNonce(temp, 0x03, nonce_input_data);  // GenNonce Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  
  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_GenMAC(temp, 0x07, 0x0000, NULL);        // GenMAC Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  ret_code = test_idle();                                   // IL005 Idle
  if(ret_code != IL005_OK)
    return;    
}

static void test_GenHMAC(void)  // GenHMAC Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  
  uint8_t nonce_input_data[20] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  
  
  ret_code = test_wakeup();                                // IL005 Wake
  if(ret_code != IL005_OK)
    return;      

  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_GenNonce(temp, 0x01, nonce_input_data); // GenNonce Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_GenHMAC(temp, 0x00, 0x0000);            // GenHMAC Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  ret_code = test_idle();                                  // IL005 Idle
  if(ret_code != IL005_OK)
    return; 
}

static void test_Write(void)  // Write Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint8_t new_value[4] = {0x77,0x77,0x77,0x77};
  uint8_t old_value[4];
  
  ret_code = test_wakeup();                                       // IL005 Wake
  if(ret_code != IL005_OK)
    return;   
  
  PRINTF("\r\n > Read SetupFixed. \r\n");
  memset(temp, 0x00, sizeof(temp));    
  ret_code = il005_Read(temp, 0x00, 0x0015);                      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  
  if(temp[4] != 0x55)
    PRINTF("\r\n Setup area is fixed...cannot write...Error... \r\n");
  else
    PRINTF("\r\n Setup area is not fixed. \r\n");
  
  PRINTF("\r\n Read Setup Area(0x0E). \r\n");  
  memset(temp, 0x00, sizeof(temp));    
  ret_code = il005_Read(temp, 0x00, 0x000E);                     // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code); 
  
  memcpy(old_value, &temp[1], 4);
  
  PRINTF("\r\n Write new value to Setup Area(0x0E). \r\n");  
  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_Write(temp, 0x00, 0x000E, new_value, NULL);  // Write Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  PRINTF("\r\n Read Setup Area(0x0E). \r\n");
  memset(temp, 0x00, sizeof(temp));    
  ret_code = il005_Read(temp, 0x00, 0x000E);                    // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  PRINTF("\r\n Write old value to Setup Area(0x0E). \r\n");    
  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_Write(temp, 0x00, 0x000E, old_value, NULL);  // Write Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  PRINTF("\r\n Read Setup Area(0x0E). \r\n");     
  memset(temp, 0x00, sizeof(temp));    
  ret_code = il005_Read(temp, 0x00, 0x000E);                   // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);  
  
  ret_code = test_idle();                                      // IL005 Idle
  if(ret_code != IL005_OK)
    return; 
}

static void test_GenDigest(void)    // GenDigest Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint8_t nonce_input_data[20] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  
  
  ret_code = test_wakeup();                                  // IL005 Wake
  if(ret_code != IL005_OK)
    return;       

  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_GenNonce(temp, 0x00, nonce_input_data);  // GenNonce Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  
  memset(temp, 0x00, sizeof(temp));    
  ret_code = il005_GenDigest(temp, 0x02, 0x00, NULL);       // GenDigest Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  ret_code = test_idle();                                   // IL005 Idle
  if(ret_code != IL005_OK)
    return;    
}

static void test_GenNonce(uint8_t mode)   // GenNonce Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint8_t nonce_input_data1[20] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};      
  uint8_t nonce_input_data2[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};      

  ret_code = test_wakeup();                                    // IL005 Wake
  if(ret_code != IL005_OK)
    return;  
  
  memset(temp, 0x00, sizeof(temp));  
  if(mode == 0x03)
    ret_code = il005_GenNonce(temp, mode, nonce_input_data2); // GenNonce Command  
  else
    ret_code = il005_GenNonce(temp, mode, nonce_input_data1); // GenNonce Command      
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  
  ret_code = test_idle();                                     // IL005 Idle
  if(ret_code != IL005_OK)
    return;
}

static void test_Fix(void)  // Fix Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  
  ret_code = test_wakeup();                        // IL005 Wake
  if(ret_code != IL005_OK)
    return;    
  
  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_Read(temp, 0x00, 0x0015);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  
  memset(temp, 0x00, sizeof(temp));  
  ret_code =  il005_Fix(temp, 0x80, 0x0000);      // Fix Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_Read(temp, 0x00, 0x0015);      // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  ret_code = test_idle();                         // IL005 Idle
  if(ret_code != IL005_OK)
    return; 
}

static void test_GenRand(void)    // GenRand Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  
  ret_code = test_wakeup();                        // IL005 Wake
  if(ret_code != IL005_OK)
    return;       
  
  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_GenRand(temp);            // RenRand Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
 
  ret_code = test_idle();                          // IL005 Idle
  if(ret_code != IL005_OK)
    return;  
}

static void test_Diversify(void)    // Diversify Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  
  uint8_t nonce_input_data[20] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  
  
  ret_code = test_wakeup();                                 // IL005 Wake
  if(ret_code != IL005_OK)
    return;  
  
  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_GenNonce(temp, 0x01, nonce_input_data);  // GenNonce Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_Diversify(temp, 0x00, 0x03, NULL);      // Diversify Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  ret_code = test_idle();                                  // IL005 Idle
  if(ret_code != IL005_OK)
    return;    
}

static void test_UpdateEtc(uint8_t owi_address)   // UpdateEtc Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  
  ret_code = test_wakeup();                            // IL005 Wake
  if(ret_code != IL005_OK)
    return;      

  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_Read(temp, 0x00, 0x0015);           // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  
  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_UpdateEtc(temp, 0x01, owi_address); // UpdateEtc Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);  

  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_Read(temp, 0x00, 0x0015);           // Read Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);  

  ret_code = test_idle();                              // IL005 Idle
  if(ret_code != IL005_OK)
    return;   
}

static void test_VerifyMAC(void)    // VerifyMAC Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint8_t clientchal[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  
  uint8_t clientresp[32];
  uint8_t otherdata[13] = {0x08,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                           0x00,0x00,0x00};    
  uint8_t nonce_input_data[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; 
  
  ret_code = test_wakeup();                                // IL005 Wake
  if(ret_code != IL005_OK)
    return;  

  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_GenNonce(temp, 0x03, nonce_input_data); // GenNonce Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  
  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_GenMAC(temp, 0x07, 0x0000, NULL);       // GenMAC Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);  
  
  memcpy(clientresp, &temp[1], 32);
  
  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_GenNonce(temp, 0x03, nonce_input_data); // GenNonce Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);  
  
  memset(temp, 0x00, sizeof(temp));    
  ret_code = il005_VerifyMAC(temp, 0x07, 0x00, clientchal, clientresp, otherdata); // VerifyMAC Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  ret_code = test_idle();                               // IL005 Idle
  if(ret_code != IL005_OK)
    return;    
}

void test_Version(void)   // Version Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  
  ret_code = test_wakeup();                      // IL005 Wake
  if(ret_code != IL005_OK)
    return;  

//  memset(temp, 0x00, sizeof(temp));
//  ret_code = il005_Version2(temp);                // Version Command
//  if(ret_code != IL005_OK)
//    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
//  
//  memset(temp, 0x00, sizeof(temp));         // 2nd Version Reponse Read, Maybe get error...
//  ret_code = il005_receive(temp);
//  if(ret_code != IL005_OK)
//      PRINTF("\r\n > Receive Error : 0x%X \r\n", ret_code);  
//  else
//     ARRAY_PRINTF("Receive", temp, temp[0]);    
//  
//  ret_code = il005_reset();                         // Reset
//  if(ret_code != IL005_OK)
//      PRINTF("\r\n > Reset Error : 0x%X \r\n", ret_code);  
//  
//  memset(temp, 0x00, sizeof(temp));  
//  ret_code = il005_receive(temp);               // 3rd Version Reponse Read, Maybe get Normal response.
//  if(ret_code != IL005_OK)
//      PRINTF("\r\n > Receive Error : 0x%X \r\n", ret_code);    
//  else
//     ARRAY_PRINTF("Receive", temp, temp[0]);       
  
  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_Version(temp);                // Version Command
  if(ret_code != IL005_OK)
 {
 	test_sleep();
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
	return;
}

  ret_code = test_idle();                        // IL005 Idle
  if(ret_code != IL005_OK)
    return;
}

static void test_ExtractPUF(void)    // ExtractPUF Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  
  ret_code = test_wakeup();                       // IL005 Wake
  if(ret_code != IL005_OK)
    return;    
  
  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_ExtractPUF(temp, 0x00);        // ExtractPUF Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  
  ret_code = test_idle();                         // IL005 Idle
  if(ret_code != IL005_OK)
    return; 
}

static void test_AuthPUF(void)   // AuthPUF Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint8_t authpuf_challange[16] = {0x29,0x2B,0x93,0xE1,0x1E,0x5F,0x1B,0x95,0xD9,0x00,
                                   0xB7,0x29,0xE0,0x55,0xD4,0x72};
  
  ret_code = test_wakeup();                                   // IL005 Wake
  if(ret_code != IL005_OK)
    return;    
  
  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_AuthPUF(temp, 0x00, authpuf_challange);    // AuthPUF Command
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  
  ret_code = test_idle();                                     // IL005 Idle
  if(ret_code != IL005_OK)
    return; 
}

static uint8_t decrypt_data[512];
static uint32_t decrypt_data_len;
static void test_Encrypt(uint8_t *encrypt_data, uint32_t len)   // Encrypt Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint32_t i,j=0;
  uint8_t mode;
  
  decrypt_data_len = len + 8;
  
  for(i=0; i<((len>>5) + 1); i++)
  {
    ret_code = test_wakeup();                                   // IL005 Wake
    if(ret_code != IL005_OK)
      return;    

    if(i == 0)
      mode = 0x00;
    else
      mode = 0x01;
    memset(temp, 0x00, sizeof(temp));
    ret_code = il005_Encrypt(temp, mode, 0x0002, encrypt_data + (i*32)); // Encrypt Command
    if(ret_code != IL005_OK)
      PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

    if(mode == 0x00)
    {
      memcpy(decrypt_data, &temp[1], 40);
      j = 40;
    }
    else
    {
      memcpy(decrypt_data + j, &temp[1], 32);
      j = j + 32;
    }

    ret_code = test_idle();                                     // IL005 Idle
    if(ret_code != IL005_OK)
      return; 
  }
  ARRAY_PRINTF("Encrypted Text", decrypt_data, j);  
}

static void test_Decrypt(void)    // Decrypt Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint32_t i,j=0;  
  uint8_t mode;    
  uint8_t plain[512];
  
  
  for(i=0; i<((decrypt_data_len>>5) + 1); i++)
  {
    ret_code = test_wakeup();                                   // IL005 Wake
    if(ret_code != IL005_OK)
      return;    
    
    if(i == 0)
      mode = 0x00;
    else
      mode = 0x01;    
    memset(temp, 0x00, sizeof(temp));
    ret_code = il005_Decrypt(temp, mode, 0x0002, decrypt_data + j); // Decrypt Command
    if(ret_code != IL005_OK)
      PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

    memcpy(plain + (i*32), &temp[1], 32);
    
    if(mode == 0x00)
      j = 40;
    else
      j = j + 32;
    
    ret_code = test_idle();                                     // IL005 Idle
    if(ret_code != IL005_OK)
      return; 
  }
  
  ARRAY_PRINTF("Plain Text", plain, decrypt_data_len - 8);  
}

static void test_SHA256(void)   // SHA256 Command Test
{
  uint8_t ret_code;
  uint8_t temp[512];
  uint8_t message_parameter[64] = {0x20,0x60,0xBB,0x61,0xA8,0xEA,0xE3,0xDB,0xC0,0xAB,
                                   0x68,0x6E,0xA0,0xB0,0x8F,0x6F,0x60,0xC0,0x98,0x38,
                                   0x67,0x2B,0x21,0x8B,0x57,0xF0,0x19,0x96,0x97,0x36,
                                   0x9F,0x76,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                   0x00,0x00,0x01,0x00};

  ret_code = test_wakeup();                               // IL005 Wake
  if(ret_code != IL005_OK)
    return;  
  
  memset(temp, 0x00, sizeof(temp));
  ret_code = il005_SHA256(temp, 0x00, NULL);              // SHA256 Command (mode Init : 0x00)
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);

  ret_code = il005_SHA256(temp, 0x01, message_parameter); // SHA256 Command (mode Compute : 0x01)
  if(ret_code != IL005_OK)
    PRINTF("\r\n > CMD Error : 0x%X \r\n", ret_code);
  
  ret_code = test_idle();                                 // IL005 Idle
  if(ret_code != IL005_OK)
    return;    
}

static void il005_test_menu_display(void)
{
  PRINTF("\r\n***** ICTK IL005 TEST MENU *****\r\n\r\n");
  PRINTF("1. Test Select      Command\r\n");  
  PRINTF("2. Test Read        Command\r\n");
  PRINTF("3. Test GenMAC      Command\r\n");  
  PRINTF("4. Test GenHMAC     Command\r\n"); 
  PRINTF("5. Test Write       Command\r\n");  
  PRINTF("6. Test GenDigest   Command\r\n");    
  PRINTF("7. Test GenNonce    Command\r\n");  
  PRINTF("8. Test Fix         Command\r\n");  
  PRINTF("9. Test GenRand     Command\r\n"); 
  PRINTF("a. Test Diversify   Command\r\n");  
  PRINTF("b. Test UpdateEtc   Command\r\n");
  PRINTF("c. Test VerifyMAC   Command\r\n");  
  PRINTF("d. Test Version     Command\r\n");
  PRINTF("e. Test ExtractPUF  Command\r\n");    
  PRINTF("f. Test AuthPUF     Command\r\n");    
  PRINTF("g. Test Encrypt     Command\r\n");  
  PRINTF("h. Test Decrypt     Command\r\n");    
  PRINTF("i. Test SHA256      Command\r\n");  
  PRINTF("q. Exit\r\n");  
  PRINTF("\r\n********************************\r\n");
  PRINTF("> ");
}

static void il005_test_menu(void)
{
	uint8_t sub_cmd;
	uint8_t current_status=1;
  uint8_t p1;
  uint16_t p2;
  uint8_t optional_data[100];
  uint8_t optional_data_len;
  
  il005_test_menu_display();
  
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
        PRINTF("\r\n>>>Input Parameter>>>\r\n");
        PRINTF("\r\n OWI ADDRESS(XX) : ");
        p1 = (uint8_t)mEnterInput(1);
        PRINTF("\r\n <<<END Input<<<\r\n");        
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_Select(p1);
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break; 
			case '2':
        PRINTF("\r\n>>>Input Parameter>>>\r\n");
        PRINTF("\r\n Area(XX)      : ");
        p1 = (uint8_t)mEnterInput(1);
        PRINTF("\r\n Address(XXXX) : ");        
        p2 = (uint16_t)mEnterInput(2);
        PRINTF("\r\n <<<END Input<<<\r\n");        
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_Read(p1, p2);
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;          
			case '3':
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_GenMAC();
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;            
			case '4':    
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_GenHMAC();
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;    
			case '5':
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_Write();
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;        
			case '6':
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_GenDigest();
        PRINTF("\r\n <<<TEST END<<< \r\n");      
      case TEST_ENTER_OPERATION:
      break;
			case '7':
        PRINTF("\r\n>>>Input Parameter>>>\r\n");
        PRINTF("\r\n Mode(XX) : ");
        p1 = (uint8_t)mEnterInput(1);
        PRINTF("\r\n <<<END Input<<<\r\n");            
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_GenNonce(p1);
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;  
			case '8':      
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_Fix();
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;     
			case '9':
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_GenRand();
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;    
      case 'a':
			case 'A':        
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_Diversify();
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;  
			case 'b':
			case 'B':    
        PRINTF("\r\n>>>Input Parameter>>>\r\n");
        PRINTF("\r\n OWI ADDRESS(XX) : ");
        p1 = (uint8_t)mEnterInput(1);
        PRINTF("\r\n <<<END Input<<<\r\n");          
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_UpdateEtc(p1);
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;       
			case 'c':
			case 'C':  
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_VerifyMAC();
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;     
			case 'd':
			case 'D':      
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_Version();
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;     
      case 'e':
			case 'E':      
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_ExtractPUF();
        PRINTF("\r\n <<<TEST END<<< \r\n");
      break;
      case 'f':
			case 'F':      
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_AuthPUF();
        PRINTF("\r\n <<<TEST END<<< \r\n");        
			break;          
      case 'g':
			case 'G':        
        memset(optional_data, 0x00, sizeof(optional_data));
        PRINTF("\r\n>>>Input Parameter>>>\r\n");
        PRINTF("\r\n Encrypt Data : ");
        optional_data_len = mgets(optional_data, 200);
        ARRAY_PRINTF("Encrypt Data", optional_data, optional_data_len);  
        PRINTF("\r\n <<<END Input<<<\r\n");         
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_Encrypt(optional_data, optional_data_len);
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;  
      case 'h':
			case 'H':      
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_Decrypt();
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;       
			case 'i':
			case 'I':      
        PRINTF("\r\n >>>TEST START>>> \r\n");
        test_SHA256();
        PRINTF("\r\n <<<TEST END<<< \r\n");
			break;   
      case 'q':
      case 'Q':
        return;
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
		if((sub_cmd != TEST_END_OPERATION) && (sub_cmd != TEST_NO_OPERATION))
    { 
      if(sub_cmd != TEST_ENTER_OPERATION)
        PRINTF("Press Enter...");
      while(sub_cmd != TEST_ENTER_OPERATION)
      {
        GETC(&sub_cmd);
      }
      PRINTF("\r\n");
			il005_test_menu_display();
      sub_cmd = TEST_NO_OPERATION;
		}        
	} while(current_status);
}

static void sha_test(void)
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

static void il005_normal_command(void)
{
  IL005_StatusTypeDef ret_code;
  uint8_t rtemp[512];  
  uint8_t tempbuf[200];
  uint8_t templen;
  uint8_t sub_cmd = TEST_NO_OPERATION;  
  uint16_t p2;

  PRINTF("\r\n len : ");
  tempbuf[0] = (uint8_t)mEnterInput(1);  
  
//  tempbuf[0] = 0x05;
  
  PRINTF("\r\n Opcode : ");
  tempbuf[1] = (uint8_t)mEnterInput(1);
  
  PRINTF("\r\n P1 : ");
  tempbuf[2] = (uint8_t)mEnterInput(1);
  PRINTF("\r\n P2 : ");        
  p2 = (uint16_t)mEnterInput(2);
  tempbuf[3]  = (uint8_t)(p2 >> 8);  
  tempbuf[4]  = (uint8_t)(p2 >> 0);  
  
  PRINTF("\r\n\r\n Data(y/n) : ");
  
  while(sub_cmd == TEST_NO_OPERATION)
  {
    if(GETC(&sub_cmd))
      PRINTF("%c", sub_cmd);
  }
  if(sub_cmd == 'y' || sub_cmd == 'Y')
  {
    PRINTF("\r\n Data : ");
    templen = mgets(&tempbuf[5], 100);
    tempbuf[0] = tempbuf[0] + templen;
  }
  PRINTF("\r\n <<<END Input<<<\r\n");        
  
  
  PRINTF("\r\n >>>TEST START>>> \r\n");
  
  ret_code = test_wakeup();                     // IL005 Wake
  if(ret_code != IL005_OK)
    return;        
  
  memset(rtemp, 0x00, sizeof(rtemp));
  ret_code = il005_test_data_in_out(rtemp, tempbuf);  
  
  if(ret_code != IL005_OK)   
  {
    PRINTF("\r\n > Error : 0x%X \r\n", ret_code);
    return;  
  }       
  
  memset(rtemp, 0x00, sizeof(rtemp));         // 2nd Version Reponse Read, Maybe get error...
  ret_code = il005c_receive_response(rtemp);
  if(ret_code != IL005_OK)
      PRINTF("\r\n > Receive Error : 0x%X \r\n", ret_code);  
  else
     ARRAY_PRINTF("Receive", rtemp, rtemp[0]);    
  
  ret_code = il005c_reset();                         // Reset
  if(ret_code != IL005_OK)
      PRINTF("\r\n > Reset Error : 0x%X \r\n", ret_code);  
  
  memset(rtemp, 0x00, sizeof(rtemp));  
  ret_code = il005c_receive_response(rtemp);               // 3rd Version Reponse Read, Maybe get Normal response.
  if(ret_code != IL005_OK)
      PRINTF("\r\n > Receive Error : 0x%X \r\n", ret_code);    
  else
     ARRAY_PRINTF("Receive", rtemp, rtemp[0]);      
  
  ret_code = test_idle();                        // IL005 Idle
  if(ret_code != IL005_OK)
    return;  
  
  PRINTF("\r\n <<<TEST END<<< \r\n");  
}

static void il005_menu_display(void)
{
  PRINTF("\r\n***** ICTK IL005 MAIN MENU *****\r\n\r\n");
  PRINTF("1. IL005 TEST \r\n");
  PRINTF("2. IL005 EXAMPLE \r\n");  
  PRINTF("a. IL005 Setup Area \r\n");
  PRINTF("b. IL005 Data Area \r\n");
  PRINTF("c. IL005 OTP Area \r\n");
  PRINTF("z. Command \r\n");
  PRINTF("\r\n********************************\r\n");
  PRINTF("> ");
}

void il005_menu(void)
{
  uint8_t ret_code=0;
	uint8_t sub_cmd;
  uint8_t temp[100];  
  
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
        il005_test_menu();
        ret_code = 1;
			break;   
			case '2':
        il005_example_menu();
        ret_code = 1;
			break;    			    
      case 'a':
      case 'A':
        read_setup_area();
        ret_code = 0;
      break;
      case 'b':
      case 'B':
        read_data_area();
        ret_code = 0;
      break;      
      case 'c':
      case 'C':
        read_otp_area();
        ret_code = 0;
      break;  
      case 'z':
      case 'Z':
        il005_normal_command();
        ret_code = 0;
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
			il005_menu_display();
      sub_cmd = TEST_NO_OPERATION;
		}
	}
}

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/