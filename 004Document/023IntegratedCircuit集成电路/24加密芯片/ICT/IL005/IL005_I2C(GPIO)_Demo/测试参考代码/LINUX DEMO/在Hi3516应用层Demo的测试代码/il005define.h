/** 
  *****************************************************************************
  * @file    		    : il005define.h
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : Definitions for Configurable Values of the IL005 Library
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __IL005DEFINE_H
#define __IL005DEFINE_H

/* Defines ------------------------------------------------------------------*/
#define SDA_PIN 8
#define SCL_PIN 9

#define INPUT 0
#define OUTPUT 1

//#define SYSTEMINTERRUPT_DISABLE   // Not used for raspberry pi
#define DISABLE_INTERRUPT()    // piLock(0)
#define ENABLE_INTERRUPT()      //piUnlock(0)

// I2C Default Device Address
#define IL005_DEFAULT_I2C_DEVICE_ADDRESS  0xC8

// I2C Wordaddress    
#define IL005_I2C_WORDADDRESS_RESET       0x00
#define IL005_I2C_WORDADDRESS_SLEEP       0x01
#define IL005_I2C_WORDADDRESS_IDLE        0x02
#define IL005_I2C_WORDADDRESS_COMMAND     0x03

// OWI Flag
#define IL005_OWI_FLAG_COMMAND            0x77
#define IL005_OWI_FLAG_TRANSMIT           0x88
#define IL005_OWI_FLAG_IDLE               0xBB
#define IL005_OWI_FLAG_SLEEP              0xCC

// SEND & RECEIVE Retry Count
#define IL005_SEND_RETRY_COUNT            0
#define IL005_RECEIVE_RETRY_COUNT         200
      
// Wakeup pulse duration      
#define WAKE_LOW_DURATION                 1000    // tWLO : 340us
#define WAKE_HIGH_DURATION                5000   // tWHI : 2.3ms
#define EXEC_MIN_DELAY                    1     // 1ms
            
#define IL005_LENGTH_INDEX                0
#define IL005_INSCODE_INDEX               1
#define IL005_P1_INDEX                    2
#define IL005_P2_INDEX                    3
#define IL005_OPTIONALDATA_INDEX          5

// command INS Code definitions
#define IL005_SELECT                      0x01
#define IL005_READ                        0x02
#define IL005_GENMAC                      0x08
#define IL005_GENHMAC                     0x11
#define IL005_WRITE                       0x12
#define IL005_GENDIGEST                   0x15
#define IL005_GENNONCE                    0x16
#define IL005_FIX                         0x17
#define IL005_GENRAND                     0x1B
#define IL005_DIVERSIFY                   0x1C
#define IL005_UPDATEETC                   0x20
#define IL005_VERIFYMAC                   0x28
#define IL005_VERSION                     0x30
#define IL005_EXTRACTPUF                  0x40
#define IL005_AUTHPUF                     0x42
#define IL005_ENCRYPT                     0x44
#define IL005_DECRYPT                     0x46
#define IL005_SHA256                      0x47
#define IL005_SOFTRESET                   0xFF

// parameter range definitions
#define IL005_KEY_ID_MAX                  15       // maximum value for key id
#define IL005_OTP_BLOCK_MAX               1        // maximum value for OTP block

// Response Size definitions
#define IL005_CMD_SIZE_MIN                7
#define IL005_CMD_SIZE_MAX                84

// Response Size definitions
#define IL005_RSP_SIZE_MIN                4        // minimum number of bytes in response
#define IL005_RSP_SIZE_MAX                43       // maximum size of response packet
#define IL005_RSP_SIZE_VAL                7        // size of response packet containing four bytes of data

#define IL005_CRC_SIZE                    2
#define IL005_RSPLEN_CRC_SIZE             3

#define SELECT_RSP_SIZE                   (IL005_RSPLEN_CRC_SIZE + 1)   // response size of Select command
#define READ_4_RSP_SIZE                   (IL005_RSPLEN_CRC_SIZE + 4)   // response size of Read command when reading 4 bytes
#define READ_32_RSP_SIZE                  (IL005_RSPLEN_CRC_SIZE + 32)  // response size of Read command when reading 32 bytes
#define GENMAC_RSP_SIZE                   (IL005_RSPLEN_CRC_SIZE + 32)  // response size of GenMAC command
#define GENHMAC_RSP_SIZE                  (IL005_RSPLEN_CRC_SIZE + 32)  // response size of GenHMAC command
#define WRITE_RSP_SIZE                    (IL005_RSPLEN_CRC_SIZE + 1)   // response size of Write command
#define GENDIGEST_RSP_SIZE                (IL005_RSPLEN_CRC_SIZE + 1)   // response size of GenDigest command
#define GENNONCE_RSP_SIZE_SHORT           (IL005_RSPLEN_CRC_SIZE + 1)   // response size of GenNonce command with mode[0:1] = 3
#define GENNONCE_RSP_SIZE_LONG            (IL005_RSPLEN_CRC_SIZE + 32)  // response size of GenNonce command
#define FIX_RSP_SIZE                      (IL005_RSPLEN_CRC_SIZE + 1)   // response size of Fix command
#define GENRAND_RSP_SIZE                  (IL005_RSPLEN_CRC_SIZE + 32)  // response size of GenRand command
#define DIVERSIFY_RSP_SIZE                (IL005_RSPLEN_CRC_SIZE + 1)   // response size of Diversify command
#define UPDATEETC_RSP_SIZE                (IL005_RSPLEN_CRC_SIZE + 1)   // response size of UpdateExtra command
#define VERIFYMAC_RSP_SIZE                (IL005_RSPLEN_CRC_SIZE + 1)   // response size of VerifyMAC command
#define VERSION_RSP_SIZE                  (IL005_RSPLEN_CRC_SIZE + 4)   // response size of Version command returns 4 bytes
#define EXTRACTPUF_RSP_SIZE               (IL005_RSPLEN_CRC_SIZE + 32)  // response size of ExtractPUF command
#define AUTHPUF_RSP_SIZE                  (IL005_RSPLEN_CRC_SIZE + 32)  // response size of AuthPUF command
#define ENCRYPT_RSP_SIZE_START            (IL005_RSPLEN_CRC_SIZE + 40)  // response size of Encrypt command
#define ENCRYPT_RSP_SIZE_CONTINUE         (IL005_RSPLEN_CRC_SIZE + 32)  // response size of Encrypt command
#define DECRYPT_RSP_SIZE                  (IL005_RSPLEN_CRC_SIZE + 32)  // response size of Decrypt command
#define SHA256_RSP_SIZE_INIT              (IL005_RSPLEN_CRC_SIZE + 1)   // response size of SHA256 command
#define SHA256_RSP_SIZE_CAL               (IL005_RSPLEN_CRC_SIZE + 32)  // response size of SHA256 command

// area definitions
#define IL005_AREA_SETUP                  0x00     // Setup area
#define IL005_AREA_OTP                    0x01     // OTP (One Time Programming) area
#define IL005_AREA_DATA                   0x02     // Data area
#define IL005_AREA_LENGTH_FLAG            0x80     // Area bit 7 set: Access 32 bytes, otherwise 4 bytes.
#define IL005_AREA_ACCESS_4               4        // Read or write 4 bytes.
#define IL005_AREA_ACCESS_32              32       // Read or write 32 bytes.
#define IL005_ADDRESS_AREA_SETUP          0x1F     // Address bits 5 to 7 are 0 for Setup area.
#define IL005_ADDRESS_AREA_OTP            0x0F     // Address bits 4 to 7 are 0 for OTP area.
#define IL005_ADDRESS_MASK                0x007F   // Address bit 7 to 15 are always 0.

// Select command definitions
#define SELECT_LENGTH                     IL005_CMD_SIZE_MIN  // Select command packet size

// Read command definitions
#define READ_LENGTH                       IL005_CMD_SIZE_MIN  // Read command packet size
#define READ_AREA_MASK                    0x83    // Read area bits 2 to 6 are 0.
#define READ_AREA_MODE_32_BYTES           0x80    // Read mode: 32 bytes

// GenMAC command definitions
#define GENMAC_LENGTH_SHORT               IL005_CMD_SIZE_MIN  // GenMAC command packet size without challenge
#define GENMAC_LENGTH_LONG                39      // GenMAC command packet size with challenge
#define GENMAC_MODE_BLOCK2_TEMP           0x01    // GenMAC mode bit   0: second SHA block from Temporary.Data
//#define GENMAC_MODE_BLOCK1_TEMP           0x02    // GenMAC mode bit   1: first SHA block from Temporary.Data
//#define GENMAC_MODE_SOURCE_FLAG_MATCH     0x04    // GenMAC mode bit   2: match TempKey.SourceFlag
//#define GENMAC_MODE_PASSTHROUGH           0x07    // GenMAC mode bit 0-2: pass-through mode
//#define GENMAC_MODE_INCLUDE_OTP_88        0x10    // GenMAC mode bit   4: include first 88 OTP bits
//#define GENMAC_MODE_INCLUDE_OTP_64        0x20    // GenMAC mode bit   5: include first 64 OTP bits
//#define GENMAC_MODE_INCLUDE_SN            0x50    // GenMAC mode bit   6: include serial number
#define GENMAC_CHALLENGE_SIZE             32      // GenMAC size of challenge
#define GENMAC_MODE_MASK                  0x77    // GenMAC mode bits 3 and 7 are 0.

// GenHMAC command definitions
#define GENHMAC_LENGTH                    IL005_CMD_SIZE_MIN  // GenHMAC command packet size
#define GENHMAC_MODE_MASK                 0x74    // GenHMAC mode bits 0, 1, 3, and 7 are 0.

// Write command definitions
//#define WRITE_LENGTH_SHORT                11      // Write command packet size with short data and no MAC
//#define WRITE_LENGTH_LONG                 39      // Write command packet size with long data and no MAC
//#define WRITE_LENGTH_SHORT_MAC            43      // Write command packet size with short data and MAC
#define WRITE_LENGTH_LONG_MAC             71      // Write command packet size with long data and MAC
#define WRITE_MAC_SIZE                    32      // Write MAC size
#define WRITE_AREA_MASK                   0xC3    // Write area bits 2 to 5 are 0.
#define WRITE_AREA_WITH_MAC               0x40    // Write area bit 6: write encrypted with MAC
#define WRITE_RSP_SUCCESS                 0x00    // If successful, will return as 0x00.

// GenDigest command definitions
#define GENDIGEST_LENGTH                  IL005_CMD_SIZE_MIN    // GenDigest command packet size without "other data"
#define GENDIGEST_LENGTH_DATA             11      // GenDigest command packet size with "other data"
#define GENDIGEST_OTHER_DATA_SIZE         4       // GenDigest size of "other data"
#define GENDIGEST_AREA_SETUP              0       // GenDigest area id SETUP
#define GENDIGEST_AREA_OTP                1       // GenDigest area id OTP
#define GENDIGEST_AREA_DATA               2       // GenDigest area id data
#define GENDIGEST_RSP_SUCCESS             0x00    // If successful, will return as 0x00.

// GenNonce command definitions
#define GENNONCE_LENGTH_SHORT             27      // GenNonce command packet size for 20 bytes of data
#define GENNONCE_LENGTH_LONG              39      // GenNonce command packet size for 32 bytes of data
//#define GENNONCE_MODE_MASK                3       // GenNonce mode bits 2 to 7 are 0.
//#define GENNONCE_MODE_SEED_UPDATE         0x00    // GenNonce mode: update seed
//#define GENNONCE_MODE_NO_SEED_UPDATE      0x01    // GenNonce mode: do not update seed
#define GENNONCE_MODE_INVALID             0x02    // GenNonce mode 2 is invalid.
#define GENNONCE_MODE_PASSTHROUGH         0x03    // GenNonce mode: pass-through
#define GENNONCE_INPUTNUM_SIZE            20      // GenNonce data length
#define GENNONCE_INPUTNUM_SIZE_PASSTHROUGH  32    // GenNonce data length in pass-through mode (mode = 3)
#define GENNONCE_RSP_SUCCESS              0x00    // If successful, will return as 0x00.

// Fix command definitions
#define FIX_LENGTH                        IL005_CMD_SIZE_MIN  // Fix command packet size
//#define FIX_AREA_NO_SETUP                0x01  // Fix zone is OTP or Data
#define FIX_AREA_NO_CRC                   0x80  // Fix command: Ignore summary.
#define FIX_AREA_MASK                     0x81  // Fix parameter 1 bits 2 to 6 are 0.
#define FIX_RSP_SUCCESS                   0x00  // If successful, will return as 0x00.

// GenRand command definitions
#define GENRAND_LENGTH                    IL005_CMD_SIZE_MIN  // GenRand command packet size

// Diversify command definitions
#define DIVERSIFY_LENGTH_SMALL            IL005_CMD_SIZE_MIN  // Diversify command packet size without MAC
#define DIVERSIFY_LENGTH_LARGE            39      // Diversify command packet size with MAC
#define DIVERSIFY_RANDOM_FLAG             4       // Diversify 1. parameter
#define DIVERSIFY_MAC_SIZE                32      // Diversify MAC size
#define DIVERSIFY_RSP_SUCCESS             0x00    // If successful, will return as 0x00.

// UpdateEtc command definitions
#define UPDATEETC_LENGTH                  IL005_CMD_SIZE_MIN  // UpdateEtc command packet size
#define UPDATEETC_CONFIG_BYTE_86          0x01    // UpdateExtra mode: update Config byte 86
#define UPDATEETC_RSP_SUCCESS             0x00    // If successful, will return as 0x00.

// VerifyMAC command definitions
#define VERIFYMAC_LENGTH                  84      // VerifyMAC command packet size
#define VERIFYMAC_CLIENT_CHALLENGE_INDEX  IL005_OPTIONALDATA_INDEX  // VerifyMAC command index for client challenge
#define VERIFYMAC_CLIENT_RESPONSE_INDEX   37      // VerifyMAC command index for client response
#define VERIFYMAC_DATA_INDEX              69      // VerifyMAC command index for other data
#define CHECKMAC_LENGTH                   84      // VerifyMAC command packet size
#define VERIFYMAC_MODE_MASK               0x27    // VerifyMAC mode bits 3, 4, 6, and 7 are 0.
#define VERIFYMAC_CLIENT_CHALLENGE_SIZE   32      // VerifyMAC size of client challenge
#define VERIFYMAC_CLIENT_RESPONSE_SIZE    32      // VerifyMAC size of client response
#define VERIFYMAC_OTHER_DATA_SIZE         13      // VerifyMAC size of "other data"
#define VERIFYMAC_RSP_SUCCESS             0x00    // If successful, will return as 0x00.

// Version command definitions
#define VERSION_LENGTH                    IL005_CMD_SIZE_MIN  // Version command packet size

// ExtractPUF command definitions
#define EXTRACTPUF_LENGTH                 IL005_CMD_SIZE_MIN  // Extract PUF command packet size

// AuthPUF command definitions
#define AUTHPUF_LENGTH                    23      // AuthPUF command packet size
#define AUTHPUF_CHALLANGE_SIZE            16      // AuthPUF Challange size

// Encrpyt command definitions
#define ENCRYPT_LENGTH                    39      // Encrypt command packet size
#define ENCRYPT_MODE_MASK                 0x01    // Encrypt mode bits 1 and 7 are 0.

// Decrypt command definitions
#define DECRYPT_LENGTH_SHORT              39      // Decrypt command packet size with chipher
#define DECRYPT_LENGTH_LONG               47      // Decrypt command packet size with iv and chipher
#define DECRYPT_MODE_MASK                 0x01    // Decrypt mode bits 1 and 7 are 0.
#define DECRYPT_CHIPER_SIZE               32      // Decrypt chipher size
#define DECRYPT_IV_CHIPER_SIZE            40      // Decrypt iv and cipher size

// SHA256 command definitions
#define SHA256_LENGTH_SHORT               IL005_CMD_SIZE_MIN  // SHA256 command packet size without hash
#define SHA256_LENGTH_LONG                71      // SHA256 command packet size with hash
#define SHA256_MODE_MASK                  0x01    // SHA256 mode bits 1 and 7 are 0.
#define SHA256_MESSAGE_SIZE               64      // SHA256 size of Message
#define SHA256_MODE_CALCULATION           0x01    // Add 64 bytes in the message parameter to the SHA256 context
#define SHA256_INIT_RSP_SUCCESS           0x00    // If successful, will return as 0x00.t size

// IL005 I/O Error Code
typedef enum 
{
  IL005_OK                          = 0x00,
  IL005_ERROR                       = 0x01,
  IL005_BUSY                        = 0x02,
  IL005_TIMEOUT                     = 0x03,
  IL005_RESYNC_WITH_WAKEUP          = 0x04,
  IL005_WAKEUP_FAIL                 = 0x05,
  IL005_I2C_START_FAIL              = 0x06,
  IL005_I2C_NO_ACK                  = 0x07,
  IL005_OWI_SEND_FAIL               = 0x08,
  IL005_OWI_RECEIVE_FAIL            = 0x09,
  IL005_COMM_FAIL                   = 0x0A,
  IL005_CRC_ERROR                   = 0x0B,
  IL005_PARAMETER_ERROR             = 0x0C,
  IL005_INVALID_SIZE                = 0x0D,
  IL005_NO_DATA                     = 0x0E,
  IL005_INVAILD_DATA                = 0x0F,
  IL005_INVAILD_COMMAND             = 0x10,
  IL005_CERT_ERROR                  = 0x11,
} IL005_StatusTypeDef;

#define uint8_t unsigned char
#define int8_t signed char
#define uint16_t unsigned short
#define int16_t signed short
#define uint32_t unsigned long
#define int32_t signed long
#define uint64_t unsigned long long
#define int64_t signed long long

#endif /*__IL005DEFINE_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/
