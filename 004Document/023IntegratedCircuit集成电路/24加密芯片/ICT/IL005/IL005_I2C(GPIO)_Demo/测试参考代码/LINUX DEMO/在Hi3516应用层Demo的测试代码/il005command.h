/** 
  *****************************************************************************
  * @file    		    : il005command.h
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 6.6
  * @brief          : Header of il005command.c file.
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __IL005COMMAND_H
#define __IL005COMMAND_H

/* Includes ------------------------------------------------------------------*/
#include "il005define.h"


/* Exported functions --------------------------------------------------------*/   
IL005_StatusTypeDef il005_Select(uint8_t *rx_buffer, uint8_t owi_address);
IL005_StatusTypeDef il005_Read(uint8_t *rx_buffer, uint8_t area, uint16_t address);
IL005_StatusTypeDef il005_GenMAC(uint8_t *rx_buffer, uint8_t mode,
                                 uint16_t targetid, uint8_t *challenge);
IL005_StatusTypeDef il005_GenHMAC(uint8_t *rx_buffer, uint8_t mode, uint16_t targetid);
IL005_StatusTypeDef il005_Write(uint8_t *rx_buffer,	uint8_t area, uint16_t address,
                                uint8_t *value, uint8_t *mac);
IL005_StatusTypeDef il005_GenDigest(uint8_t *rx_buffer,	uint8_t area,
                                    uint16_t targetid, uint8_t *otherdata);
IL005_StatusTypeDef il005_GenNonce(uint8_t *rx_buffer, uint8_t mode, uint8_t *inputnum);
IL005_StatusTypeDef il005_Fix(uint8_t *rx_buffer, uint8_t area, uint16_t crc);
IL005_StatusTypeDef il005_GenRand(uint8_t *rx_buffer);
IL005_StatusTypeDef il005_Diversify(uint8_t *rx_buffer, uint8_t sourceflag,
                                    uint8_t targetkeyid, uint8_t *mac);
IL005_StatusTypeDef il005_UpdateEtc(uint8_t *rx_buffer, uint8_t mode, uint8_t newvalue);
IL005_StatusTypeDef il005_VerifyMAC(uint8_t *rx_buffer,	uint8_t mode, uint8_t key_id,
                        uint8_t *client_challenge, uint8_t *client_responsemac, uint8_t *otherdata);
IL005_StatusTypeDef il005_Version(uint8_t *rx_buffer);
IL005_StatusTypeDef il005_ExtractPUF(uint8_t *rx_buffer, uint8_t targetid);
IL005_StatusTypeDef il005_AuthPUF(uint8_t *rx_buffer, uint8_t targetid, uint8_t *challange);
IL005_StatusTypeDef il005_Encrypt(uint8_t *rx_buffer, uint8_t mode, uint16_t source, uint8_t *data);
IL005_StatusTypeDef il005_Decrypt(uint8_t *rx_buffer, uint8_t mode, uint16_t source, uint8_t *data);
IL005_StatusTypeDef il005_SHA256(uint8_t *rx_buffer, uint8_t mode, uint8_t *message);

IL005_StatusTypeDef il005_test_data_in_out(uint8_t *rx_buffer, uint8_t *input_buffer);


#endif /*__IL005COMMAND_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/