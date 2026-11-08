/** 
  *****************************************************************************
  * @file    		    : G3define.h
  * @author         : Miller IC Trust Key China
  * @version        : V1.1.0
  * @date           : 2-July-2018
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 7.7
  * @brief          : Definitions for Configurable Values of the G3 Library
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __G3_DEFINE_H
#define __G3_DEFINE_H

//#include "stm32f4xx_hal.h"

/* Defines ------------------------------------------------------------------*/
#if 1       // select interface for communicating with IL005.
  #define G3_COMM_I2C
#else
#define G3_COMM_SPI
#endif

#define G3_I2C_NORMAL_FREQ      100000  //100KHZ, 正常数据读写时使用此时钟
// I2C Time Out
#define G3_I2C_TIME_OUT         1000



// I2C Default Device Address
#define G3_DEFAULT_I2C_DEVICE_ADDRESS  0xC8

// I2C Wordaddress    
#define G3_I2C_WORDADDRESS_RESET       0x00
#define G3_I2C_WORDADDRESS_SLEEP       0x01
#define G3_I2C_WORDADDRESS_IDLE        0x02
#define G3_I2C_WORDADDRESS_COMMAND     0x03

// OWI Flag
#define G3_OWI_FLAG_COMMAND            0x77
#define G3_OWI_FLAG_TRANSMIT           0x88
#define G3_OWI_FLAG_IDLE               0xBB
#define G3_OWI_FLAG_SLEEP              0xCC

// SEND & RECEIVE Retry Count
#define G3_SEND_RETRY_COUNT            0
#define G3_RECEIVE_RETRY_COUNT         200
      
// Wakeup pulse duration      
#define WAKE_LOW_DURATION                 100    // tWLO : 80us
#define WAKE_HIGH_DURATION                10000   // tWHI : 10ms
#define EXEC_MIN_DELAY                    3      // 3ms
            
#define G3_LENGTH_INDEX                0
#define G3_INSCODE_INDEX               1
#define G3_P1_INDEX                    2
#define G3_P2_INDEX                    3
#define G3_OPTIONALDATA_INDEX          5

// command INS Code definitions
#define G3_READ                         0x80
#define G3_WRITE                        0x81
#define G3_VERIFY_PWD                   0x82
#define G3_CHANGE_PWD                   0x83
#define G3_GET_CHAL                     0x84
#define G3_INIT_PRIV_KEY                0x85
#define G3_SIGN                         0x86
#define G3_VERIFY                       0x87
#define G3_ENCRYPT                      0x88
#define G3_DECRYPT                      0x89
#define G3_SESSION                      0x8A
#define G3_DIVERSIFY                    0x8B
#define G3_GET_PUB_KEY                  0x8C
#define G3_CERT                         0x8D
#define G3_ISSUE_CERT                   0x8E
#define G3_ECDH                         0x90
#define G3_TLS_MAC_ENCRYPT              0x91
#define G3_TLS_DECRYCT                  0x92
#define G3_TLS_GET_HANDSHAKE_DIGEST     0x93
#define G3_HASH                         0x94
#define G3_RESET                        0x9F


// parameter range definitions


// Response Size definitions
#define G3_CMD_SIZE_MIN                7
#define G3_CMD_SIZE_MAX                84

// Response Size definitions
#define G3_RSP_SIZE_MIN                4        // minimum number of bytes in response
#define G3_RSP_SIZE_MAX                243       // maximum size of response packet
#define G3_RSP_SIZE_VAL                7        // size of response packet containing four bytes of data


// Select command definitions
#define SELECT_LENGTH                     G3_CMD_SIZE_MIN  // Select command packet size

// Read command definitions
#define READ_LENGTH                       G3_CMD_SIZE_MIN  // Read command packet size
#define READ_AREA_MASK                    0x83    // Read area bits 2 to 6 are 0.
#define READ_AREA_MODE_32_BYTES           0x80    // Read mode: 32 bytes


// G3 I/O Error Code
typedef enum 
{
  G3_OK                          = 0x00,
  G3_ERROR                       = 0x01,
  G3_BUSY                        = 0x02,
  G3_TIMEOUT                     = 0x03,
  G3_RESYNC_WITH_WAKEUP          = 0x04,
  G3_WAKEUP_FAIL                 = 0x05,
  G3_I2C_START_FAIL              = 0x06,
  G3_I2C_NO_ACK                  = 0x07,
  G3_OWI_SEND_FAIL               = 0x08,
  G3_OWI_RECEIVE_FAIL            = 0x09,
  G3_COMM_FAIL                   = 0x0A,
  G3_CRC_ERROR                   = 0x0B,
  G3_PARAMETER_ERROR             = 0x0C,
  G3_INVALID_SIZE                = 0x0D,
  G3_NO_DATA                     = 0x0E,
  G3_INVAILD_DATA                = 0x0F,
  G3_INVAILD_COMMAND             = 0x10,
  G3_CERT_ERROR                  = 0x11,
  
} G3_StatusTypeDef;


//START ENUM

typedef enum  
{
	SND_N_RECV=0,
	SND=1,
	RECV=2,	 
}  EN_SND_RECV_MODE;   	

typedef enum  
{
	AND=0,
	OR=1,	 
}  EN_AND_OR;   	

typedef enum  
{
	CONDITION=0,
	FREE=1,
	FORBIDDEN=2,	 
}  EN_FORBIDDEN_TYPE;   	

typedef enum  
{
	ALL=0,
	MASK_N_ENC =1,
	ENC=2,
	ENC_MAC=3,	 
}  EN_RW ;   	

typedef enum  
{
	SECT_VOID=0,
	SECT_ST_ECC_PUF=1,
	SECT_ECC_PRV=2,
	SECT_ECC_PUB=3,
	SECT_AES128=4,
	SECT_AES256=5,
	SECT_SM4=6,
	SECT_SHA256=7,
	SECT_PASSWORD=8,
	SECT_DATA=9,
	SECT_COUNTER=10,	 
}  EN_DATA_SECTOR_TYPE;   	

	

typedef enum  
{
	INHERIT=0,
	SELF=1,	 
}  EN_DIVERSIFY_MODE;   	

typedef enum
{
	SECTOR_KEY = 0,
	SESSION_KEY = 1,
} EN_KEY_TYPE;

typedef enum  
{
	BL_CBC=0,
	BL_CTR=1,
	BL_GCM=3,	 
        BL_SM2_INIT=0x20,
        BL_SM2_UPDATE=0x21,
        BL_SM2_FINAL=0x22,
}  EN_BLOCK_MODE;   	

typedef enum  
{
	TO_TEMP=0,
	TO_KEY_SECTOR=1,	 
}  EN_CERTIFICATION_WRITE_MODE;   	

typedef enum  
{
	SIGN_ECDSA_EXT_SHA256=0,
        SIGN_SM2sign_EXT_SM3=0,
	SIGN_ECDSA_WITH_SHA256=1,
        SIGN_SM2sign_WITH_SM3=1,
	SIGN_HMAC=2,
	SIGN_SYMM=3,	 
	SIGN_SESSION_SYMM=4,
}  EN_SIGN_OPTION;   	

typedef enum  
{
	VERIFY_ECDSA_EXT_SHA256=0,
        VERIFY_SM2sign_EXT_SM3=0,
	VERIFY_ECDSA_WITH_SHA256=1,
        VERIFY_SM2sign_WITH_SM3=1,
	VERIFY_HMAC=2,
	VERIFY_SYMM=3,
	VERIFY_SESSION_SYMM=4,
	VERIFY_EXT_PUB_ECDSA_EXT_SHA256=0x10,
	VERIFY_EXT_PUB_ECDSA_WITH_SHA256=0x11,	 
}  EN_VERIFY_OPTION;   	

typedef enum  
{
	DYN_AUTH_ECDSA_SHA256=0x21,
	DYN_AUTH_HMAC=0x22,
	DYN_AUTH_SYMM=0x23,
	DYN_AUTH_CERT_PUB_ECDSA_SHA256=0x31,	 
}  EN_DYNAMIC_AUTH;   	

typedef enum  
{
	USE_CERT_PUB_ECDSA_EXT_SHA256=0,
	USE_CERT_PUB_ECDSA_WITH_SHA256=1,	 
}  EN_VERIFY_TYPE;   	

typedef enum  
{
	SETUP_AREA=0,
	KEY_AREA=1,
	DATA_AREA_0=2,
	DATA_AREA_1=3,	 
}  EN_AREA_TYPE;   	

typedef enum  
{
	PLAIN_TEXT=0,
	CBC=1,
	CTR=2,
	CCM=3,
	GCM=4,
	CBC_with_MAC=5,
	CTR_with_MAC=6,
	SESSION_KEY_CBC=0x11,
	SESSION_KEY_CTR=0x12,
	SESSION_KEY_CCM=0x13,
	SESSION_KEY_GCM=0x14,
	SESSION_KEY_CBC_with_MAC=0x15,
	SESSION_KEY_CTR_with_MAC=0x16,
	MASKED=0xFF,	 
}  EN_RW_INST_OPTION;   	

typedef enum  
{
	KEY_SECTOR=0,
	TEMP_PUBLIC_KEY=1,	 
}  EN_PUB_TYPE;   	

typedef enum  
{
	ISCRT_KEY_AREA=0,
	ISCRT_DATA_AREA_0=1,
	ISCRT_DATA_AREA_1=2,	 
}  EN_ISSUE_CERT_AREA_TYPE;   	

typedef enum  
{
	CHANGE_CIPHER_SPEC=20,
	ALERT=21,
	HANDSHAKE=22,
	APPLICATION_DATA=23,	 
}  EN_CONTENT_TYPE;   	

typedef enum  
{
	SSL_3_0=0x0300,
	TLS_1_0=0x0301,
	TLS_1_1=0x0302,
	TLS_1_2=0x0303,	 
}  EN_TLS_VERSION;   	

typedef enum  
{
	NORMAL_ECDH=0x0000,
	GEN_TLS_BLOCK=0x0011,
	SET_TLS_SESSION_KEY=0x0012,	 
}  EN_ECDH_MODE;   	

typedef enum  
{
	SYMM_KEY=0x80,
	FACTORY_AES=0x90,
	FACTORY_SM4=0x91,
	EXT_SESSION_KEY_AES=0xA0,
	EXT_SESSION_KEY_SM4=0xA1,
	EXT_PUB_KEY=0xA2,	 
}  EN_SESSION_MODE;   	

typedef enum  
{
	HSM_CLIENT=0x0000,
	HSM_SERVER=0x0001,	 
}  EN_HANDSHAKE_MODE;   	

typedef enum  
{
        SHA256 = 0x00,
        SM3    = 0x01,
}  EN_SEL_HASH_ALG;   	

typedef enum  
{
	Initialize=0x0000,
	Update=0x0001,
	Finalize=0x00FF,
}  EN_HASH_MODE;   	

//END ENUM

//START STRUCTURE

#define MAKEWORD(l_,h_)   ((uint16_t)(((uint8_t)(l_))|(((uint16_t)((uint8_t)(h_)))<<8)))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
//END STRUCTURE

#define uint8_t unsigned char
#define int8_t signed char
#define uint16_t unsigned short
#define int16_t signed short
#define uint32_t unsigned long
#define int32_t signed long
#define uint64_t unsigned long long
#define int64_t signed long long


#endif /*__G3DEFINE_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/
