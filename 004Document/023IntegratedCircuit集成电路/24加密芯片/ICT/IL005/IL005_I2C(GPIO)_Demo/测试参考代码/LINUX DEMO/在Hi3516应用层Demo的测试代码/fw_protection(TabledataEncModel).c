/** 
  *****************************************************************************
  * @file    		    : fw_protection.c
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 09-JULY-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : Firmware Protection Example
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#include <string.h>
#include <stdlib.h>
#include "fw_protection.h"
#include "timer_util.h"
#include "ictk_lib.h"
#include "il005communication.h"
#include "il005command.h"
#include "sha256.h"






#define FACTORY_KEY_ID                          0x8000
#define ENCRYPT_ISSUE_KEY                        1
#define DEBUG									 1
uint8_t g_fw_key[32];

static const uint8_t SHARED_KEY1[]={(uint8_t)(0xA3+0xB6),(uint8_t)(0x25+0xC6),(uint8_t)(0xDE+0x5E),(uint8_t)(0x46+0x0A),(uint8_t)(0xBE+0x34),(uint8_t)(0x58+0x7D),(uint8_t)(0xAC+0x85),(uint8_t)(0x94+0x00),(uint8_t)(0x1C+0x19),(uint8_t)(0xB0+0xA7),
                                   (uint8_t)(0x2D+0x08),(uint8_t)(0x4E+0xC2),(uint8_t)(0xF7+0xA8),(uint8_t)(0x08+0xC8),(uint8_t)(0xCE+0xEA),(uint8_t)(0x4F+0xF3),(uint8_t)(0xD1+0x54),(uint8_t)(0x33+0x04),(uint8_t)(0xD1+0x51),(uint8_t)(0xBC+0x02),
                                   (uint8_t)(0x9B+0xA4),(uint8_t)(0xA9+0xC6),(uint8_t)(0x59+0xEC),(uint8_t)(0x19+0x3B),(uint8_t)(0x05+0xD8),(uint8_t)(0x59+0x7C),(uint8_t)(0x14+0x45),(uint8_t)(0x1F+0xB1),(uint8_t)(0xE8+0x41),(uint8_t)(0x00+0x29),
                                   (uint8_t)(0xB4+0x3C),(uint8_t)(0xA8+0x81)};   
static const uint8_t SHARED_KEY2[];
static const uint8_t SHARED_KEY3[];
static const uint8_t LED_PATTERN_TBL[];

static const uint8_t SHARED_ENC1[]={0xB6,0xC6,0x5E,0x0A,0x34,0x7D,0x85,0x00,0x19,0xA7,
                                   0x08,0xC2,0xA8,0xC8,0xEA,0xF3,0x54,0x04,0x51,0x02,
                                   0xA4,0xC6,0xEC,0x3B,0xD8,0x7C,0x45,0xB1,0x41,0x29,
                                   0x3C,0x81};

uint8_t DECRYPTED_DATA[32 + 100];

IL005_StatusTypeDef get_sector_key(uint8_t target_id, uint16_t address, uint8_t *sector_key, uint8_t *shared_key);
static void randam_number_generation(uint8_t *rand_buf, uint8_t rand_num);

static const unsigned char KEY_TO_WRITE[] = {
  0xA3, 0x25, 0xDE, 0x46, 0xBE, 0x58, 0xAC, 0x94, 0x1C, 0xB0, 0x2D, 0x4E, 0xF7, 0x08, 0xCE, 0x4F, 
  0xD1, 0x33, 0xD1, 0xBC, 0x9B, 0xA9, 0x59, 0x19, 0x05, 0x59, 0x14, 0x1F, 0xE8, 0x00, 0xB4, 0xA8, 

  0xF8, 0x9A, 0x53, 0xD7, 0xA8, 0xB3, 0xDF, 0xF8, 0x3C, 0x48, 0x4B, 0x7C, 0xDE, 0xBB, 0x12, 0xAF, 
  0xF8, 0xA4, 0x4E, 0x09, 0x1C, 0xD6, 0x4B, 0x60, 0xF9, 0x52, 0x4F, 0x01, 0xCA, 0x6B, 0xF0, 0x54, 

  0x00, 0xA5, 0xD7, 0x20, 0xCE, 0xE3, 0x15, 0x90, 0x7C, 0xAE, 0x84, 0xAF, 0xFD, 0x50, 0xF6, 0x9B, 
  0x17, 0xCD, 0x42, 0xB6, 0xB9, 0x5C, 0x93, 0x46, 0xA1, 0x72, 0xFA, 0xC9, 0x8A, 0x91, 0x2A, 0x90, 

  0xC0, 0xEF, 0x9F, 0x3F, 0x78, 0xAC, 0x66, 0x54, 0x98, 0xDC, 0x84, 0x1F, 0x9B, 0x36, 0xA1, 0x3C, 
  0xB6, 0xED, 0x35, 0x88, 0x8C, 0x46, 0xD3, 0x08, 0xC9, 0xA8, 0xEB, 0x62, 0xB9, 0x93, 0xEF, 0x3F, 

  0x3A, 0xC7, 0xD3, 0x51, 0x87, 0xAC, 0x8D, 0x29, 0xA1, 0x3E, 0x69, 0x4E, 0xCB, 0xE0, 0xC5, 0x23, 
  0x68, 0xA3, 0x9E, 0xF4, 0xA0, 0x5F, 0xD6, 0xAE, 0x08, 0xAB, 0xE4, 0x45, 0xCC, 0x25, 0xE1, 0x90, 

  0x81, 0x26, 0xCC, 0xF7, 0x96, 0x6B, 0xCB, 0xAF, 0x4F, 0xC6, 0xD5, 0x31, 0x81, 0x1B, 0x20, 0x94, 
  0x8A, 0x4E, 0xFA, 0x76, 0x53, 0x16, 0xFA, 0x26, 0x8A, 0x44, 0x82, 0x41, 0xE7, 0x7E, 0x10, 0xC9, 

  0xBB, 0xA6, 0x64, 0xF6, 0x97, 0x40, 0x02, 0x7F, 0x93, 0x7A, 0x1B, 0x88, 0xE6, 0x34, 0x76, 0x9C, 
  0xCF, 0x0E, 0x07, 0xE4, 0x77, 0xD6, 0x91, 0xEC, 0x68, 0x79, 0x4A, 0xBF, 0xC1, 0x99, 0xC9, 0x67,

  0x48, 0x14, 0x40, 0xFB, 0x25, 0x76, 0x96, 0xAA, 0x64, 0x3F, 0x8D, 0x4E, 0xEB, 0x48, 0x03, 0xA3, 
  0x83, 0x22, 0x1F, 0x82, 0x68, 0x6C, 0xB1, 0xDC, 0xC5, 0x67, 0x89, 0x15, 0x36, 0x18, 0x4A, 0xB3};

static const unsigned char FACTORY_KEY[] = {
  0x48, 0x5B, 0x3A, 0x15, 0xAB, 0x41, 0x81, 0x52, 0x48, 0x2D, 0x54, 0x82, 0xA0, 0xB9, 0x3A, 0x35, 
  0x04, 0x48, 0xF6, 0x15, 0x70, 0x7D, 0x87, 0x49, 0xF9, 0x3B, 0x89, 0xF6, 0x77, 0x05, 0x85, 0x73};

IL005_StatusTypeDef issueKey()
{
  IL005_StatusTypeDef ret_code;
  uint8_t temp[160];
  uint8_t il005_SN[13];  
  int i;
  int j;
  int iState;
  uint32_t uiFixed;

  for (iState = 0; iState < 5; iState++)
  {
    if (iState > 0)
    {
      if (iState > 2){
         delay_ms(2000);
      	}
      else{
        il005c_idle();
      	}
    }
	
    ret_code = il005c_wakeup(WAKE_LOW_DURATION);      // IL005 Wakeup : IL005 Wake low duration Min. 340us
    if(ret_code != IL005_OK)
      {continue;}
    
    memset(temp, 0x00, sizeof(temp));  
    ret_code = il005_Read(temp, 0x80, 0x0000);        // Serial Number Read
    if(ret_code != IL005_OK)                       
      {continue;}

    memcpy(il005_SN, temp + 1, 13);
    
    ret_code = il005_Read(temp, 0x00, 0x0015);        // 4bytes mode 0x15
    if(ret_code != IL005_OK)                       
      {continue;}

    uiFixed = temp[1 + 2];
    if (temp[1 + 3] == 0x55)
    {

      temp[64] = 0xC8;								// IIC address(default)
      temp[65] = 0x07;								// Please refer to the datasheet P34 (about encrypt read command)
      temp[66] = 0x55;								// OTP setting(default)
      temp[67] = 0x00;								//OWI setting(default)

      ret_code = il005_Write(temp, 0x00, 4, temp + 64, 0); //block 4
      if(ret_code != IL005_OK)                       
        continue;
	
      temp[64] = 0x80;								//80  = Never Read 
      temp[65] = 0x80;								//80  = Never Write&Diversify
      temp[66] = 0xC0;								//C0 = Encrypted Read, Use ReadKey store in = 0
      temp[67] = 0xA0;								//A0 = Never Write ,To be able to Diversify(Self-generated)& need MAC.  '0' Don't care
	  
      ret_code = il005_Write(temp, 0x00, 5, temp + 64, 0); //block 5
      if(ret_code != IL005_OK)                       
        continue;
     
      temp[66] = 0xC2;                            //C2 = Encrypted Read, Use ReadKey store in = 2
      temp[67] = 0xA2;                            //A2 = Never Write ,To be able to Diversify(Self-generated)& need MAC.  '2' Don't care

      ret_code = il005_Write(temp, 0x00, 6, temp + 64, 0); //block 6
      if(ret_code != IL005_OK)                       
        continue;


     
      temp[66] = 0xC4;                            //C4 = Encrypted Read, Use ReadKey store in = 4
      temp[67] = 0xA4;                            //A4 = Never Write ,To be able to Diversify(Self-generated)& need MAC.  '4' Don't care

      ret_code = il005_Write(temp, 0x00, 7, temp + 64, 0); //block 7
      if(ret_code != IL005_OK)                       
        continue;
      
      temp[66] = 0x80;                            //80 = Never Read, Never Write&Diversify
      temp[67] = 0x80;                            //80 = Never Read, Never Write&Diversify
      ret_code = il005_Write(temp, 0x00, 8, temp + 64, 0); //block 8
      if(ret_code != IL005_OK)                       
        continue;
      
      ret_code = il005_Fix(temp, 0x80, 0x00);     //fix setup area without CRC
      if(ret_code != IL005_OK)                       
        continue;
    }
    
    if (uiFixed == 0x55)
    {
      for (i = 0; i < 8; i++)    //Because KEY_TO_WRITE have 8 Group , so i < 8 .if your key have 10,please set i <10
      {
		//This is Encrypt ISSUE "KEY_TO_WRITE" array
#if ENCRYPT_ISSUE_KEY
        randam_number_generation(temp + 64, 20);        // Generate Random Number 
        il005c_idle();                     // IL005 Idle
        il005c_wakeup(WAKE_LOW_DURATION);
        
        ret_code = il005_GenNonce(temp, 0x01, temp + 64);  // GenNonce Command
        if(ret_code != IL005_OK)
          break;

        memcpy(temp + 1 + 32, temp + 64, 20);
        temp[53] = 0x16; //ins of genNonce
        temp[54] = 0x01; //P1
        temp[55] = 0;    //P2
        sha256(temp + 1, 55, temp + 64);   // random nonce
         
        ret_code = il005_GenDigest(temp, 0x02, FACTORY_KEY_ID, NULL);  // GenDigest Command
        if(ret_code != IL005_OK)  
         break;
        
        // the input of SHA256 when generating a digest consists of:
        memcpy(temp, FACTORY_KEY, 32);
        memset(temp + 32, 0x00, 32);
        temp[32] = 0x15; //ins of genDigest
        temp[33] = 0x02; //P1 = data area
        temp[34] = (uint8_t)(FACTORY_KEY_ID); //P2
        temp[35] = (uint8_t)(FACTORY_KEY_ID >> 8); //P2
        temp[36] = il005_SN[12];
        temp[37] = il005_SN[0];
        temp[38] = il005_SN[1]; 
  //      memcpy(temp + 64, random, 32);   // random nonce
    
        // End of the input of SHA256
        
        sha256(temp, 32 + 32 + 32, temp + 96);  //temporary.data

        memcpy(temp, temp + 96, 32);            //temporary.data
        memset(temp + 32, 0x00, 32);
        temp[32] = 0x12; //ins of write
        temp[33] = 0xC2; //P1 = 32bytes, encrypted, data area
        temp[34] = (uint8_t)(i << 3); //P2
        temp[35] = (uint8_t)0;        //P2
        temp[36] = il005_SN[12];
        temp[37] = il005_SN[0];
        temp[38] = il005_SN[1];
        memcpy(temp + 32 + 32, (uint8_t*)KEY_TO_WRITE + i * 32, 32);
        sha256(temp, 32 + 32 + 32, temp + 128);  //MAC
        for(j = 0; j < 32; j++)         // encrypted key = Digest XOR write key
          temp[j] = temp[j + 96] ^ KEY_TO_WRITE[i * 32 + j];
        
        ret_code = il005_Write(temp, 0xC2, i << 3, temp, temp + 128);
        if((ret_code != IL005_OK) || (temp[1] != 0))
          break;
      }
#else
	//This is Clear ISSUE "KEY_TO_WRITE" array
	 for(j = 0; j < 32; j++)         // encrypted key = Digest XOR write key
          temp[j] = KEY_TO_WRITE[i * 32 + j];
        ret_code = il005_Write(temp, 0x82, i << 3, temp, NULL);
        if((ret_code != IL005_OK) || (temp[1] != 0))
          break;
	
#endif
      if (i < 8)
        continue;
      
      ret_code = il005_Fix(temp, 0x81, 0x00);     //fix data area without CRC
      if(ret_code != IL005_OK)                       
        continue;
    }
    break;
  }
  
  il005c_idle();                     // IL005 Idle

  return ret_code;
}
//This is a Encrypt data from PC Tool before coding this array. And If the array decrypted successfully, you can use the plain data to correct operation your function 
static const uint8_t SHARED_KEY3[]={
  0x26, 0x24, 0x0C, 0xAF, 0xE3, 0xB8, 0x78, 0x50, 0x6D, 0x27, 0x15, 0x40, 0x1E, 0x13, 0x3F, 0x97, 
  0x68, 0x9F, 0x62, 0x3D, 0x8B, 0x76, 0x1F, 0xCD, 0x04, 0xC6, 0xE5, 0xED, 0x99, 0x6B, 0xE6, 0x9F, 
  0x81, 0x62, 0xB7, 0x57, 0x51, 0x05, 0xA9, 0xE4, 0x5B, 0xFE, 0x23, 0x96, 0xBE, 0x34, 0x15, 0x94, 
  0xFF, 0xF9, 0x49, 0x33, 0xCE, 0xD0, 0x95, 0x95, 0x38, 0x6A, 0x19, 0x8A, 0xAD, 0xA9, 0x93, 0x9E, 
  0xBA, 0x6F, 0x60, 0x24, 0x90, 0xD4, 0xDA, 0x84, 0x60, 0x5D, 0x75, 0xBB, 0x3B, 0x24, 0x5B, 0xB4, 
  0xDE, 0x9A, 0x2D, 0x93, 0x3D, 0x16, 0xFA, 0x8C, 0x3A, 0xD4, 0x16, 0xC9, 0xEE, 0x1C, 0x6C, 0x5C, 
  0xE2, 0x60, 0x0D, 0x3B, 0x85, 0xB7, 0x7B, 0x1C
}; 



//This is a Encrypt data from PC Tool before coding this array. And If the array decrypted successfully, you can use the plain data to correct operation your function 
static const uint8_t SHARED_KEY2[]={
  0xF8, 0x0B, 0x84, 0xF7, 0x26, 0x50, 0xF4, 0x98, 0x40, 0xF6, 0x39, 0xD3, 0xDB, 0x95, 0xE4, 0x4A, 
  0x1F, 0x69, 0x90, 0xAD, 0xA5, 0x32, 0x48, 0x26, 0x9A, 0x20, 0xB5, 0xCA, 0xC0, 0xFA, 0x1A, 0x3C, 
  0x88, 0xA0, 0xAD, 0xD1, 0x18, 0x57, 0xDF, 0x02, 0x34, 0x46, 0x4B, 0x92, 0xAE, 0xCD, 0xEE, 0xBD, 
  0x68, 0x72, 0x4E, 0x1F, 0x54, 0xCC, 0x4B, 0xBA, 0x89, 0x72, 0xB1, 0x21, 0x3A, 0xC5, 0xF0, 0x84, 
  0x78, 0xDA, 0x53, 0x69, 0xD8, 0x03, 0x21, 0xA8, 0xAC, 0x18, 0x4B, 0xDC, 0x92, 0xCB, 0x12, 0xC1, 
  0x88, 0x24, 0xB2, 0x89, 0x8C, 0xBA, 0x4B, 0xF0, 0x77, 0xF2, 0x4F, 0x9F, 0xBA, 0x1B, 0x10, 0xE4
};

/**
  * @brief  This function is example code for firmware protection.
            This function is in ram and restore firmware key2.
  */
void user_pattern1(uint8_t* temp)
{ 
  int i; 
  get_sector_key(2, 0x18, g_fw_key, (uint8_t *)DECRYPTED_DATA);   // Use SHARED_KEY2  get the FW_key2;
  
  
/************************************/
//Decrypt CTR mode in main RAM function
/************************************/
  memset(temp, 0x00, 16);
  memcpy(temp + 16, g_fw_key, 32);   //key

  for (i = 0; i < sizeof(SHARED_KEY3); i++)
  {
    if ((i & 31) == 0) // i % 32 == 0
    { //counter 2bytes
        temp[14] = (uint8_t)(i >> 13);    // i/32 >> 8
        temp[15] = (uint8_t)(i >> 5);     // i/32
        sha256(temp, 48, temp + 64);
    }
    ((uint8_t *)DECRYPTED_DATA)[i] = ((uint8_t *)SHARED_KEY3)[i] ^ (temp + 64)[i & 31];  //use the FW_KEY2 decrypt "Encrypt(shared3+data2)"
  }
  
}


/**
  * @brief  This function is example code for firmware protection.
            This function is in ram and restore firmware key3.
  */
void user_pattern2(void)
{  
  get_sector_key(4, 0x28, g_fw_key, (uint8_t *)DECRYPTED_DATA);   //SHARED_KEY3);
}



void led_pattern4(void)
{ 

}

//This is a Encrypt data from PC Tool before coding this array. And If the array decrypted successfully, you can use the plain data to correct operation your function 
static const uint8_t LED_PATTERN_TBL[]={
  0x91, 0xE2, 0xFC, 0xFF, 
  0xDA, 0x63, 0x2B, 0x59, 
  0x0F, 0xCE, 0x2B, 0x39, 
  0xF1, 0xF1, 0x20, 0xA0, 
  0xE6, 0x42, 0xFA, 0x96, 
  0x23, 0x22, 0x06, 0x2A, 
  0x9A, 0xCC, 0xB2, 0x51, 
  0x89, 0x6E, 0x70, 0x47, 
  0xC1, 0x36, 0x34, 0xE7, 
  0xC6, 0xB5, 0xAB, 0xCF, 
  0x01, 0xE6, 0x05, 0xEF, 
  0xE1, 0x3B, 0x30, 0xB4, 
  0x9A, 0xE2, 0xCA, 0xA6, 
  0x1D, 0x26, 0x5A, 0x0A, 
  0xCA, 0x74, 0x7E, 0x71, 
  0x57, 0xC2, 0x10, 0x09, 
  0xEF, 0x66, 0xCC, 0x49, 
  0xE6, 0xAB, 0x35, 0xEF, 
  0x60, 0x7A, 0xE4, 0x71, 
  0xF0, 0x5B, 0x81, 0xAC, 
  0xCB, 0x8E, 0x07, 0x36,
  0xC6, 0x2A, 0xFC, 0x66
};


/**
  * @brief  This function decrypt firmware for using firmware keys.
  * @param  encrypt_data
  * @param  encrypt_data_length 
  * @param  decrypt_data
  * @param  key 
  */
void decrypt(uint8_t *encrypt_data, uint16_t encrypt_data_length, uint8_t *decrypt_data, uint8_t *key)
{
  int i = 0;
  
  while (encrypt_data_length > i)
  {
    decrypt_data[i] = encrypt_data[i] - key[i & 31];
    i++;
    if (encrypt_data_length <= i)
      break;
    
    decrypt_data[i] = encrypt_data[i] + key[i & 31];
    i++;
    if (encrypt_data_length <= i)
      break;
    
    decrypt_data[i] = encrypt_data[i] ^ key[i & 31];
    i++;
  }
}

static void randam_number_generation(uint8_t *rand_buf, uint8_t rand_num)
{
  uint8_t i;
  uint8_t nTmp, n;
  
  for(i=0; i < rand_num; i++)
    rand_buf[i] = (uint8_t)rand();

  for(i=0; i < rand_num; i++)
  {
    n = (uint8_t)rand()%rand_num;
    nTmp = rand_buf[i];
    rand_buf[i]= rand_buf[n];
    rand_buf[n]= nTmp;
  }
}


IL005_StatusTypeDef get_sector_key(uint8_t target_id, uint16_t address, uint8_t *sector_key, uint8_t *shared_key)
{
  IL005_StatusTypeDef ret_code;
  uint8_t temp[128];
  uint8_t host_random[32];
  uint8_t il005_SN[32];  
  uint8_t read_key[32];
  uint8_t temp_key[32];
  uint8_t i;
  uint8_t mac_cal_data[32] = {0x15,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                              0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                              0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                              0x00,0x00};  
  
  ret_code = il005c_wakeup(WAKE_LOW_DURATION);      // IL005 Wakeup : IL005 Wake low duration Min. 340us
  if(ret_code != IL005_OK)
    return ret_code;
  
  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_Read(temp, 0x80, 0x0000);        // Serial Number Read
  if(ret_code != IL005_OK)                       
    return ret_code;

  memcpy(il005_SN, temp+1, 32);
  
  randam_number_generation(host_random, 32);        // Generate Random Number 
  
  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_GenNonce(temp, 0x03, host_random);  // GenNonce Command
  if(ret_code != IL005_OK)
    return ret_code;

  memset(temp, 0x00, sizeof(temp));    
  ret_code = il005_GenDigest(temp, 0x02, target_id, NULL);  // GenDigest Command
  if(ret_code != IL005_OK)  
    return ret_code;
  
  memset(temp, 0x00, sizeof(temp));  
  ret_code = il005_Read(temp, 0x82, address);   // Read data area key
  if(ret_code != IL005_OK)                       
    return ret_code;
  
  ret_code = il005c_idle();                     // IL005 Idle
  if(ret_code != IL005_OK)
    return ret_code;
  
  memcpy(read_key, temp+1, 32);        
  
  // the input of SHA256 when generating a digest consists of:
  mac_cal_data[2] = target_id;                  
  mac_cal_data[4] = il005_SN[12];
  mac_cal_data[5] = il005_SN[0];
  mac_cal_data[6] = il005_SN[1]; 
  
  memset(temp, 0x00, sizeof(temp));
  
  memcpy(temp, shared_key, 32);
  memcpy(temp+32, mac_cal_data, 32);
  memcpy(temp+32+32, host_random, 32);
  // End of the input of SHA256
  
  sha256(temp, 32+32+32, temp_key);
  
  for( i = 0; i < 32; i++)         // Sector key = Digest XOR Read key
    sector_key[i] = temp_key[i] ^ read_key[i];
  
  return ret_code;  
}
/**
  * @brief  This function is use decrypt_data Demo sample code.
  */
int led_pattern_data(uint8_t* temp, int length)
{ 
	
/*
  int i;
  for(i = 0; i < length; i += 2)
  {
    if (temp[i] & 0x40)
      LED1_ON;
    else
      LED1_OFF;

    if (temp[i] & 0x20)
      LED2_ON;
    else
      LED2_OFF;

    if (temp[i] & 0x10)
      LED3_ON;
    else
      LED3_OFF;
    
    delay_ms(((temp[i] & 0x0F) << 8) + temp[i + 1]);
  }
*/
  return 0;
}

/**
  * @brief  This function is a main function of firmware protection sample code.
  */
void fw_protection(void)
{
  IL005_StatusTypeDef ret_code;
  
  int i,j;
  uint8_t temp[128];
  uint8_t il005_SN[32];
  uint8_t il005_MacOut[32];  
  uint8_t mac_challege_data[32];  
  uint8_t mac_cal_data[15] = {0x08,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                              0x00,0x00,0x00,0x00,0x00}; // Please refer to the datasheet P36 about Calculated parameters
  uint8_t sha[32];
  
#if  DEBUG
  // only test chip ,or Issue chip by on board model  
  static int flag = 1;  
  if(flag)
  {
  	issueKey();
	flag = 0;
  }
#endif

  for (j = 0 ; j < 10 ;j++ )   // main loop
  {
    ret_code = il005c_wakeup(WAKE_LOW_DURATION);      // IL005 Wakeup : IL005 Wake low duration Min. 340us
    if(ret_code != IL005_OK)
    {
      delay_ms(150);
      continue;
    }
      
    memset(temp, 0x00, sizeof(temp));  
    ret_code = il005_Read(temp, 0x80, 0x0000);			// Read IL005 Setup Area first 32bytes. it can get IL005 SN
    if(ret_code != IL005_OK)
    {
      il005c_idle();                     // IL005 Idle
      continue;
    }

    memcpy(il005_SN, temp+1, 32);  
    
    randam_number_generation(mac_challege_data, 32);  // generate a random number
    
    memset(temp, 0x00, sizeof(temp));  
    ret_code = il005_GenMAC(temp, 0x40, 0x0000, mac_challege_data); // GenMAC Command ,use 0x40 model[will use the SN data & don't use OTP data]， "0x0000" mean that will use the KEY from sector 0 
    if(ret_code != IL005_OK)
    {
      il005c_idle();                     // IL005 Idle
      continue;
    }
    
    ret_code = il005c_idle(); // IL005 Idle
    if(ret_code != IL005_OK)
    {
      il005c_idle();                     // IL005 Idle
      continue;
    }
    
    memcpy(il005_MacOut, temp+1, 32);    
    

    // genrate MAC 
    memset(temp, 0x00, sizeof(temp));  
    memcpy(temp , SHARED_KEY1, 32);
    for ( i=0; i < 32; i++)
        temp[i] -= SHARED_ENC1[i];
    memcpy(temp+32, mac_challege_data, 32);
    memcpy(temp+32+32, mac_cal_data, 15);
    memcpy(temp+32+32+15, &il005_SN[12], 1);
    memcpy(temp+32+32+15+1, &il005_SN[8], 4);
    memcpy(temp+32+32+15+1+4, &il005_SN[0], 4);
    
    sha256(temp, 32+32+15+1+4+4, sha);
    ARRAY_PRINTF("sha256", sha, 32);  
    PRINTF("\r\n Mutual Authentication using GenMAC \r\n");
    for(i = 0; ; )
    {

      if (((int*)il005_MacOut)[i] == ((int*)sha)[i])      // compare result in main code
      {
        i++;
        if (i >= 4)                                       // infinite loop if not authenticated
          break;
      }
	  else 
		break;
    }
	if (i >=4)
	{
		PRINTF("\r\n sha OK ");
                delay_ms(5000);
	}
	else
	continue;

///########################Data Encrypt Model#######################//
/**
	MCU must be store the "shared key 1" ,"Encrypt (shared key2 + data1)","Encrypt (shared key3 + data2)""Encrypt data3"
	IL005 must be store the "shared key 1/2/3" "fw_key 1/2/3"
*/
    if (get_sector_key(0, 0x08, g_fw_key, (uint8_t *)temp) != IL005_OK) //use SHARED_KEY1 to get the "FW_key1";
    {
      il005c_idle();                     // IL005 Idle
      continue;
    }
    decrypt((uint8_t *)SHARED_KEY2, sizeof(SHARED_KEY2), (uint8_t *)DECRYPTED_DATA, g_fw_key); // use "FW_key1" decrypt ""Encrypt (shared key2 + data1)""
    led_pattern_data(DECRYPTED_DATA + 32, sizeof(SHARED_KEY2) - 32);  // only use the data2 function, please don't use compare operation
	
    user_pattern1(temp);  // get share key3
    
   
    user_pattern2();  // get fw_key3

    decrypt((uint8_t *)LED_PATTERN_TBL, sizeof(LED_PATTERN_TBL), (uint8_t *)DECRYPTED_DATA, g_fw_key);  //use the fw_key3 decrypt "Encrypt data3"
    
    
    led_pattern_data(DECRYPTED_DATA, sizeof(LED_PATTERN_TBL)); // // only use the data3 function, please don't use compare operation
	break;
  }
  
}

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/