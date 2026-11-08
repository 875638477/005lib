/** 
  *****************************************************************************
  * @file    		    : il005command.c
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 7.7
  * @brief          : Command Marshaling Layer of IL005 Library
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#include <string.h>   // needed for memset(), memcpy()

#include "il005command.h"
#include "il005communication.h"
#include "ictk_lib.h"

//#define CMD_PRINTF(args...)
//#define RSP_PRINTF(args...)
//#deinfe ARRAY_PRINTF(args...)


/**
  * @brief  This function calculates CRC16.
  * @param  length number of bytes in buffer
  * @param  data pointer to data for which CRC should be calculated
  * @param  crc16 pointer to 16-bit CRC
  */
static void il005_calculate_crc16(uint8_t length, uint8_t *data, uint8_t *crc16)
{
	uint8_t counter;
	uint16_t crc16_register = 0;
  uint16_t polynomial = 0x8005;      // polynomial : 0x8005
	uint8_t shift_register;
	uint8_t data_bit, crc16_bit;

	for(counter = 0; counter < length; counter++)
  {
	  for(shift_register = 0x01; shift_register > 0x00; shift_register <<= 1)
    {
      data_bit = (data[counter] & shift_register) ? 1 : 0;
      crc16_bit = crc16_register >> 15;

      crc16_register <<= 1;
      if((data_bit ^ crc16_bit) != 0)
        crc16_register ^= polynomial;
	  }
	}
	crc16[0] = (uint8_t) (crc16_register);
	crc16[1] = (uint8_t) (crc16_register >> 8);
}

/**
  * @brief  This function checks the consistency of a response.
  * @param  response pointer to response
  * @return status of the consistency check
  */
static IL005_StatusTypeDef il005_check_crc16(uint8_t *response)
{
	uint8_t crc16[2];
	uint8_t length;

	length = response[0] - 2;
	il005_calculate_crc16(length, response, crc16);

	return (crc16[0] == response[length] && crc16[1] == response[length + 1])
          ? IL005_OK : IL005_CRC_ERROR;
}

/**
  * @brief  This function checks the consistency of a response.
  * @param  rx_buffer pointer to receive buffer
  * @param  owi_address Devices not matching this value will go into the idle state.
  * @return status of the operation
  */
IL005_StatusTypeDef il005_Select(uint8_t *rx_buffer, uint8_t owi_address)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[SELECT_LENGTH];

  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer)
		return IL005_PARAMETER_ERROR;

  // 2. Make Packet
	tx_buffer[IL005_LENGTH_INDEX]  = SELECT_LENGTH;
	tx_buffer[IL005_INSCODE_INDEX] = IL005_SELECT;
	tx_buffer[IL005_P1_INDEX]      = owi_address;
	tx_buffer[IL005_P2_INDEX]      = 0;
	tx_buffer[IL005_P2_INDEX + 1]  = 0;
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("Select", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("Select", rx_buffer);  
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != SELECT_RSP_SIZE)
    return IL005_INVALID_SIZE;  
  
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);  
}

/**
  * @brief  This function reads 4 bytes or 32 bytes from the EEPROM area.
  * @param  rx_buffer pointer to receive buffer
  * @param  area 0x00(Setup area), 0x01(OTP Area), 0x02(Data Area)
  * @param  address Address for targetSector
  * @return status of the operation
  */
IL005_StatusTypeDef il005_Read(uint8_t *rx_buffer, uint8_t area, uint16_t address)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[READ_LENGTH];
  uint8_t rx_size;
  
  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || ((area & ~READ_AREA_MASK) != 0)
				|| ((area & READ_AREA_MODE_32_BYTES) && (area == IL005_AREA_OTP)))
		return IL005_PARAMETER_ERROR;
  
//	if (area & IL005_ZONE_DATA)
//  {
//		address >>= 2;
//		if (address & 1)
//			// If we would just mask this bit, we would
//			// read from an address that was not intended.
//			return IL005_PARAMETER_ERROR;
//	}    

  // 2. Make Packet
	tx_buffer[IL005_LENGTH_INDEX]  = READ_LENGTH;
	tx_buffer[IL005_INSCODE_INDEX] = IL005_READ;
	tx_buffer[IL005_P1_INDEX]      = area;                                      
	tx_buffer[IL005_P2_INDEX]      = (uint8_t)(address & IL005_ADDRESS_MASK);
	tx_buffer[IL005_P2_INDEX + 1]  = 0;
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("Read", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("Read", rx_buffer);  
  
  // 5. Check Reponse Packet Size
	rx_size = (area & IL005_AREA_LENGTH_FLAG) ? READ_32_RSP_SIZE : READ_4_RSP_SIZE;    
  if (rx_buffer[0] != rx_size)
    return IL005_INVALID_SIZE;
 
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);
}

/**
  * @brief  This function calculates a SHA256 digest from keys stored in the device, challenge, or other internal data.
  * @param  rx_buffer pointer to receive buffer
  * @param  mode selects message fields
  * @param  targetid input of SHA generating response.
  * @param  challenge pointer to challenge (not used if mode bit 0 is set)
  * @return status of the operation
  */
IL005_StatusTypeDef il005_GenMAC(uint8_t *rx_buffer, uint8_t mode,
                                 uint16_t targetid, uint8_t *challenge)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[GENMAC_LENGTH_LONG];

  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || ((mode & ~GENMAC_MODE_MASK) != 0)
				|| (((mode & GENMAC_MODE_BLOCK2_TEMP) == 0) && !challenge))
		return IL005_PARAMETER_ERROR;
  
  // 2. Make Packet
	tx_buffer[IL005_LENGTH_INDEX]  = GENMAC_LENGTH_SHORT;
	tx_buffer[IL005_INSCODE_INDEX] = IL005_GENMAC;
	tx_buffer[IL005_P1_INDEX]      = mode;
	tx_buffer[IL005_P2_INDEX]      = (uint8_t)(targetid);
	tx_buffer[IL005_P2_INDEX + 1]  = (uint8_t)(targetid >> 8);
	if ((mode & GENMAC_MODE_BLOCK2_TEMP) == 0)
	{
		memcpy(&tx_buffer[IL005_OPTIONALDATA_INDEX], challenge, GENMAC_CHALLENGE_SIZE);
		tx_buffer[IL005_LENGTH_INDEX] = GENMAC_LENGTH_LONG;
	}
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("GenMAC", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("GenMAC", rx_buffer);  
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != GENMAC_RSP_SIZE)
    return IL005_INVALID_SIZE;  
    
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);
}

/**
  * @brief  This function calculates a HMAC/SHA256 input of a key stored in the Data sector, a challenge, and other information inside the device.
  * @param  rx_buffer pointer to receive buffer
  * @param  mode selects message fields
  * @param  targetid included to input of HMAC generating response.
  * @return status of the operation
  */
IL005_StatusTypeDef il005_GenHMAC(uint8_t *rx_buffer, uint8_t mode, uint16_t targetid)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[GENHMAC_LENGTH];

  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || ((mode & ~GENHMAC_MODE_MASK) != 0))
		return IL005_PARAMETER_ERROR;

  // 2. Make Packet
	tx_buffer[IL005_LENGTH_INDEX]  = GENHMAC_LENGTH;
	tx_buffer[IL005_INSCODE_INDEX] = IL005_GENHMAC;
	tx_buffer[IL005_P1_INDEX]      = mode;
	tx_buffer[IL005_P2_INDEX]      = (uint8_t)(targetid); 
	tx_buffer[IL005_P2_INDEX + 1]  = (uint8_t)(targetid >> 8);
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("GenHMAC", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("GenHMAC", rx_buffer);  
  
  // 5. Check Reponse Packet Size
  if(rx_buffer[0] != GENHMAC_RSP_SIZE)
    return IL005_INVALID_SIZE;  

  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);  
}

/**
  * @brief  This function can write either as 4 bytes or 32 bytes to one of the EEPROM areas of the device.
  * @param  rx_buffer pointer to receive buffer
  * @param  area 0x00(Setup area), 0x01(OTP Area), 0x02(Data Area)
  * @param  address Address for targetSector
  * @param  value pointer to 32 (zone bit 7 set) or 4 bytes of data
  * @param  mac pointer to MAC (ignored if zone is unlocked)
  * @return status of the operation
  */
IL005_StatusTypeDef il005_Write(uint8_t *rx_buffer,	uint8_t area, uint16_t address,
                                uint8_t *value, uint8_t *mac)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[WRITE_LENGTH_LONG_MAC];
  uint8_t length;

  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || !value || ((area & ~WRITE_AREA_MASK) != 0))
		return IL005_PARAMETER_ERROR;

//	if (area & IL005_AREA_DATA)
//  {
//		address >>= 2;
//		if (address & 1)
//			// If we would just mask this bit, we would
//			// write to an address that was not intended.
//			return IL005_PARAMETER_ERROR;
//	}  

  // 2. Make Packet
	tx_buffer[IL005_INSCODE_INDEX] = IL005_WRITE;
	tx_buffer[IL005_P1_INDEX]      = area;
	tx_buffer[IL005_P2_INDEX]      = (uint8_t)(address & IL005_ADDRESS_MASK);
	tx_buffer[IL005_P2_INDEX + 1]  = 0;  
  
  length = (area & IL005_AREA_LENGTH_FLAG) ? IL005_AREA_ACCESS_32 : IL005_AREA_ACCESS_4;
  memcpy(&tx_buffer[IL005_OPTIONALDATA_INDEX], value, length);

	if (mac != NULL)
	{
		memcpy(&tx_buffer[IL005_OPTIONALDATA_INDEX+length], mac, WRITE_MAC_SIZE);
		length = length + WRITE_MAC_SIZE;
	}

	tx_buffer[IL005_LENGTH_INDEX] = (uint8_t)(length + IL005_CRC_SIZE + 5);
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("Write", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("Write", rx_buffer);  
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != WRITE_RSP_SIZE)
    return IL005_INVALID_SIZE;  
  
  if (rx_buffer[1] != WRITE_RSP_SUCCESS)
    return IL005_INVAILD_DATA;   
  
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);
}

/**
  * @brief  This function combines data and other Temporary.Data in the device using SHA256.
  * @param  rx_buffer pointer to receive buffer
  * @param  area 0x00(Setup area), 0x01(OTP Area), 0x02(Data Area or Factory Key)
  * @param  targetid Factory key ID for Area block or Data area in Setup/OTP Area
  * @param  otherdata pointer to 4 bytes of data when using VerifyOnly key
  * @return status of the operation
  */
IL005_StatusTypeDef il005_GenDigest(uint8_t *rx_buffer,	uint8_t area,
                                    uint16_t targetid, uint8_t *otherdata)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[GENDIGEST_LENGTH_DATA];

  // 1. Check Parameter 
  if (!tx_buffer || !rx_buffer
     || ((area != GENDIGEST_AREA_SETUP) && (area != GENDIGEST_AREA_OTP) && (area != GENDIGEST_AREA_DATA)))
          return IL005_PARAMETER_ERROR;

  if ((area == GENDIGEST_AREA_OTP) && (targetid > IL005_OTP_BLOCK_MAX))
//     || ((area == GENDIGEST_AREA_DATA) && (targetid > IL005_KEY_ID_MAX)))  
          return IL005_PARAMETER_ERROR;
    
  // 2. Make Packet
  tx_buffer[IL005_INSCODE_INDEX] = IL005_GENDIGEST;
  tx_buffer[IL005_P1_INDEX]      = area;
	tx_buffer[IL005_P2_INDEX]      = (uint8_t)(targetid);
	tx_buffer[IL005_P2_INDEX + 1]  = (uint8_t)(targetid >> 8);
  if (otherdata != NULL)            
  {
    memcpy(&tx_buffer[IL005_OPTIONALDATA_INDEX], otherdata, GENDIGEST_OTHER_DATA_SIZE);
    tx_buffer[IL005_LENGTH_INDEX] = GENDIGEST_LENGTH_DATA;
  }
  else
    tx_buffer[IL005_LENGTH_INDEX] = GENDIGEST_LENGTH;
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("GenDigest", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("GenDigest", rx_buffer);   
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != GENDIGEST_RSP_SIZE)
    return IL005_INVALID_SIZE;  
  
  if (rx_buffer[1] != GENDIGEST_RSP_SUCCESS)
    return IL005_INVAILD_DATA;
  
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);  
}

/**
  * @brief  This function generates a nonce using internally generated random numbers and input data within the master.
  * @param  rx_buffer pointer to receive buffer
  * @param  mode controls the mechanism of the internal random number generator and seed update
  * @param  inputnum  pointer to system input
  * @return status of the operation
  */
IL005_StatusTypeDef il005_GenNonce(uint8_t *rx_buffer, uint8_t mode, uint8_t *inputnum)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[GENNONCE_LENGTH_LONG];
  uint8_t rx_size;
  
  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || !inputnum || (mode > GENNONCE_MODE_PASSTHROUGH)
      || (mode == GENNONCE_MODE_INVALID))
		return IL005_PARAMETER_ERROR;
  
  // 2. Make Packet
	tx_buffer[IL005_INSCODE_INDEX] = IL005_GENNONCE;
	tx_buffer[IL005_P1_INDEX]      = mode;
  tx_buffer[IL005_P2_INDEX]      = 0;
	tx_buffer[IL005_P2_INDEX + 1]  = 0;
  
	if(mode != GENNONCE_MODE_PASSTHROUGH)
	{
		memcpy(&tx_buffer[IL005_OPTIONALDATA_INDEX], inputnum, GENNONCE_INPUTNUM_SIZE);
		tx_buffer[IL005_LENGTH_INDEX] = GENNONCE_LENGTH_SHORT;
		rx_size = GENNONCE_RSP_SIZE_LONG;
	}
	else
	{
		memcpy(&tx_buffer[IL005_OPTIONALDATA_INDEX], inputnum, GENNONCE_INPUTNUM_SIZE_PASSTHROUGH);
		tx_buffer[IL005_LENGTH_INDEX] = GENNONCE_LENGTH_LONG;
		rx_size = GENNONCE_RSP_SIZE_SHORT;
	}  
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("GenNonce", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("GenNonce", rx_buffer);   
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != rx_size)
    return IL005_INVALID_SIZE;  
  
  if (mode == GENNONCE_MODE_PASSTHROUGH)
  {
    if (rx_buffer[1] != GENNONCE_RSP_SUCCESS)
      return IL005_INVAILD_DATA;
  }
  
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);
}

/**
  * @brief  This function change permissions for Setup or Data/OTP area.
  * @param  rx_buffer pointer to receive buffer
  * @param  area area to fix
  * @param  crc  CRC for that area
  * @return status of the operation
  */
IL005_StatusTypeDef il005_Fix(uint8_t *rx_buffer, uint8_t area, uint16_t crc)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[FIX_LENGTH];

  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || ((area & ~FIX_AREA_MASK) != 0)
				|| ((area & FIX_AREA_NO_CRC) && (crc != 0)))
		return IL005_PARAMETER_ERROR;

  // 2. Make Packet
	tx_buffer[IL005_LENGTH_INDEX]  = FIX_LENGTH;
	tx_buffer[IL005_INSCODE_INDEX] = IL005_FIX;
	tx_buffer[IL005_P1_INDEX]      = area & FIX_AREA_MASK;
	tx_buffer[IL005_P2_INDEX]      = (uint8_t)(crc);
	tx_buffer[IL005_P2_INDEX + 1]  = (uint8_t)(crc >> 8);
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer, 
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("Fix", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("Fix", rx_buffer);    
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != FIX_RSP_SIZE)
    return IL005_INVALID_SIZE;  
  
  if (rx_buffer[1] != FIX_RSP_SUCCESS)
    return IL005_INVAILD_DATA;

  // 6. Check checksum
	return il005_check_crc16(rx_buffer);  
}

/**
  * @brief  This function generates a random number to the master to use.
  * @param  rx_buffer pointer to receive buffer
  * @return status of the operation
  */
IL005_StatusTypeDef il005_GenRand(uint8_t *rx_buffer)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[GENRAND_LENGTH];

  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer)
		return IL005_PARAMETER_ERROR;

  // 2. Make Packet
	tx_buffer[IL005_LENGTH_INDEX]  = GENRAND_LENGTH;
	tx_buffer[IL005_INSCODE_INDEX] = IL005_GENRAND;
	tx_buffer[IL005_P1_INDEX]      = 0;
	tx_buffer[IL005_P2_INDEX]      = 0;
	tx_buffer[IL005_P2_INDEX + 1]  = 0;
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("GenRand", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("GenRand", rx_buffer);    
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != GENRAND_RSP_SIZE)
    return IL005_INVALID_SIZE;  
  
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);  
}

/**
  * @brief  This function combines a source key value and Temporary.Data using SHA256 and stores the results in targetSector.
  * @param  rx_buffer pointer to receive buffer
  * @param  sourceflag type of source key
  * @param  targetkeyid The Diversify key to be saved 
  * @param  mac ointer to optional MAC
  * @return status of the operation
  */
IL005_StatusTypeDef il005_Diversify(uint8_t *rx_buffer, uint8_t sourceflag,
                                    uint8_t targetkeyid, uint8_t *mac)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[DIVERSIFY_LENGTH_LARGE];
  
  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || ((sourceflag & ~DIVERSIFY_RANDOM_FLAG) != 0)
				 || (targetkeyid > IL005_KEY_ID_MAX))
		return IL005_PARAMETER_ERROR;

  // 2. Make Packet
	tx_buffer[IL005_INSCODE_INDEX] = IL005_DIVERSIFY;  
	tx_buffer[IL005_P1_INDEX]      = sourceflag;
	tx_buffer[IL005_P2_INDEX]      = targetkeyid;
	tx_buffer[IL005_P2_INDEX + 1]  = 0;
	if (mac != NULL)
	{
		memcpy(&tx_buffer[IL005_OPTIONALDATA_INDEX], mac, DIVERSIFY_MAC_SIZE);
		tx_buffer[IL005_LENGTH_INDEX] = DIVERSIFY_LENGTH_LARGE;
	}
	else
		tx_buffer[IL005_LENGTH_INDEX] = DIVERSIFY_LENGTH_SMALL;
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("Diversify", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("Diversify", rx_buffer);  
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != DIVERSIFY_RSP_SIZE)
    return IL005_INVALID_SIZE;  
  
  if (rx_buffer[1] != DIVERSIFY_RSP_SUCCESS)
    return IL005_INVAILD_DATA;

  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);  
}

/**
  * @brief  This function used to update the values within the Setup area (byte 84 and 85)
  *         after the Setup area is fixed and is also able to reduce the value of UseFlag.
  * @param  rx_buffer pointer to receive buffer
  * @param  mode 0: update Setup Area byte 85; 1: byte 86
  * @param  newvalue byte to write
  * @return status of the operation
  */
IL005_StatusTypeDef il005_UpdateEtc(uint8_t *rx_buffer, uint8_t mode, uint8_t newvalue)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[UPDATEETC_LENGTH];

  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || (mode > UPDATEETC_CONFIG_BYTE_86))
		return IL005_PARAMETER_ERROR;

  // 2. Make Packet
	tx_buffer[IL005_LENGTH_INDEX]  = UPDATEETC_LENGTH;
	tx_buffer[IL005_INSCODE_INDEX] = IL005_UPDATEETC;
	tx_buffer[IL005_P1_INDEX]      = mode;
	tx_buffer[IL005_P2_INDEX]      = newvalue;
	tx_buffer[IL005_P2_INDEX + 1]  = 0;
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("UpdateEtc", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("UpdateEtc", rx_buffer);   
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != UPDATEETC_RSP_SIZE)
    return IL005_INVALID_SIZE;  
  
  if (rx_buffer[1] != UPDATEETC_RSP_SUCCESS)
    return IL005_INVAILD_DATA;

  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);
}

/**
  * @brief  This function calculates a MAC value internally and compares it with a MAC that has already been received.
  * @param  rx_buffer pointer to receive buffer
  * @param  mode selects the hash inputs
  * @param  targetid slot index of key
  * @param  client_challenge pointer to client challenge (ignored if mode bit 0 is set)
  * @param  client_responsemac pointer to client response
  * @param  otherdata pointer to 13 bytes of data used in the client command
  * @return status of the operation
  */
IL005_StatusTypeDef il005_VerifyMAC(uint8_t *rx_buffer,	uint8_t mode, uint8_t targetid,
                         uint8_t *client_challenge, uint8_t *client_responsemac, uint8_t *otherdata)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[VERIFYMAC_LENGTH];
  
  // 1. Check Parameter 
	if (		// no null pointers allowed
				!tx_buffer || !rx_buffer || !client_responsemac || !otherdata
				// No reserved bits should be set.
				|| (mode | VERIFYMAC_MODE_MASK) != VERIFYMAC_MODE_MASK
				// targetid > 15 not allowed
				|| targetid > IL005_KEY_ID_MAX)
		return IL005_PARAMETER_ERROR;
  
  // 2. Make Packet
  tx_buffer[IL005_LENGTH_INDEX]  = VERIFYMAC_LENGTH;
	tx_buffer[IL005_INSCODE_INDEX] = IL005_VERIFYMAC;
	tx_buffer[IL005_P1_INDEX]      = mode & VERIFYMAC_MODE_MASK;
	tx_buffer[IL005_P2_INDEX]      = targetid;
	tx_buffer[IL005_P2_INDEX + 1]  = 0;

	if (client_challenge == NULL)
		memset(&tx_buffer[VERIFYMAC_CLIENT_CHALLENGE_INDEX], 0, VERIFYMAC_CLIENT_CHALLENGE_SIZE);
	else
		memcpy(&tx_buffer[VERIFYMAC_CLIENT_CHALLENGE_INDEX], client_challenge,
           VERIFYMAC_CLIENT_CHALLENGE_SIZE);  
  
	memcpy(&tx_buffer[VERIFYMAC_CLIENT_RESPONSE_INDEX], client_responsemac,
         VERIFYMAC_CLIENT_RESPONSE_SIZE);
	memcpy(&tx_buffer[VERIFYMAC_DATA_INDEX], otherdata, VERIFYMAC_OTHER_DATA_SIZE);  
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("VerifyMAC", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("VerifyMAC", rx_buffer);   
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != VERIFYMAC_RSP_SIZE)
    return IL005_INVALID_SIZE;  
  
  if (rx_buffer[1] != VERIFYMAC_RSP_SUCCESS)
    return IL005_INVAILD_DATA;

  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);
}

/**
  * @brief  This function can check the version information of the IL005 device.
  * @param  rx_buffer pointer to receive buffer
  * @return status of the operation
  */
IL005_StatusTypeDef il005_Version(uint8_t *rx_buffer)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[VERSION_LENGTH];
  
  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer)
		return IL005_PARAMETER_ERROR;
  
  // 2. Make Packet
  tx_buffer[IL005_LENGTH_INDEX]  = VERSION_LENGTH;
	tx_buffer[IL005_INSCODE_INDEX] = IL005_VERSION;
	tx_buffer[IL005_P1_INDEX]      = 0;
	tx_buffer[IL005_P2_INDEX]      = 0;
	tx_buffer[IL005_P2_INDEX + 1]  = 0;
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("Version", tx_buffer);
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("Version", rx_buffer);   
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != VERSION_RSP_SIZE)
    return IL005_INVALID_SIZE;    
  
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);
}

/**
  * @brief  This function extracts the PUF_AUTH data necessary to verify a MAC generated by the AuthPUF command.
  * @param  rx_buffer pointer to receive buffer
  * @param  targetid used to generate MAC of AuthPUF command
  * @return status of the operation
  */
IL005_StatusTypeDef il005_ExtractPUF(uint8_t *rx_buffer, uint8_t targetid)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[EXTRACTPUF_LENGTH];

  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer)
		return IL005_PARAMETER_ERROR;

  // 2. Make Packet
  tx_buffer[IL005_LENGTH_INDEX]  = EXTRACTPUF_LENGTH;  
	tx_buffer[IL005_INSCODE_INDEX] = IL005_EXTRACTPUF;
	tx_buffer[IL005_P1_INDEX]      = targetid;
	tx_buffer[IL005_P2_INDEX]      = 0;
	tx_buffer[IL005_P2_INDEX + 1]  = 0;
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer, 
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("ExtractPUF", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("ExtractPUF", rx_buffer);  
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != EXTRACTPUF_RSP_SIZE)
    return IL005_INVALID_SIZE;  
  
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);  
}

/**
  * @brief  This function generates a MAC by performing SHA256 using PUF value and input challenge from master.
  * @param  rx_buffer pointer to receive buffer
  * @param  targetid  to be used to generate MAC
  * @param  challange pointer to be used to generate MAC
  * @return status of the operation
  */
IL005_StatusTypeDef il005_AuthPUF(uint8_t *rx_buffer, uint8_t targetid, uint8_t *challange)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[AUTHPUF_LENGTH];

  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || !challange)
		return IL005_PARAMETER_ERROR;

  // 2. Make Packet
	tx_buffer[IL005_LENGTH_INDEX]  = AUTHPUF_LENGTH;
	tx_buffer[IL005_INSCODE_INDEX] = IL005_AUTHPUF;
	tx_buffer[IL005_P1_INDEX] = targetid;           // 
	tx_buffer[IL005_P2_INDEX] = 0;                  // 
	tx_buffer[IL005_P2_INDEX + 1] = 0;
  memcpy(&tx_buffer[IL005_OPTIONALDATA_INDEX], challange, AUTHPUF_CHALLANGE_SIZE);
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("AuthPUF", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("AuthPUF", rx_buffer);  
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != AUTHPUF_RSP_SIZE)
    return IL005_INVALID_SIZE;  
  
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);  
}

/**
  * @brief  This function is to encrypt input data with PUF or a key stored in Data area.
  * @param  rx_buffer pointer to receive buffer
  * @param  mode  0x00(Start), 0x01(Continue)
  * @param  source 
  * @param  data Plaintext
  * @return status of the operation
  */
IL005_StatusTypeDef il005_Encrypt(uint8_t *rx_buffer, uint8_t mode, uint16_t source, uint8_t *data)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[ENCRYPT_LENGTH];
  static uint8_t nonce[32];
  static uint32_t counter;
  uint8_t temp[IL005_CMD_SIZE_MAX];
  uint8_t i, j=0;

  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || !data || ((mode & ~ENCRYPT_MODE_MASK) != 0))
		return IL005_PARAMETER_ERROR;

  // 2. Make Packet
  if((source&0x0002) == 0x0002)
  {
    if(mode == 0x00)
    {
      counter = 0;
      ret_code = il005_GenRand(rx_buffer);
      if(ret_code != IL005_OK)
        return ret_code;      
      memset(nonce, 0x00, 32);
      memcpy(nonce, rx_buffer+1, 8);
      memcpy(nonce+12, &counter, 4);   
    }
    else
    {
      counter = counter + 1;
      memcpy(&nonce[12], &counter, 4);     
    }
    
    ret_code = il005_GenMAC(rx_buffer, 0x00, (source >> 8), nonce);
    if(ret_code != IL005_OK)
      return ret_code;
            
    if(mode == 0x00)
    {
      memcpy(temp, nonce, 8);          
      j = 8;
    }
    for(i = 0; i < 32; i++)      
      temp[i+j] = rx_buffer[i+1] ^ data[i];
      
    rx_buffer[0] = 1 + 32 + j + 2;
    memcpy(rx_buffer+1, temp, 32 + j);
    il005_calculate_crc16(rx_buffer[0]-2, rx_buffer, &rx_buffer[rx_buffer[0]-2]);  
    RSP_PRINTF("Encrypt", rx_buffer); 
    
    return IL005_OK;    
  }
  else
  {
    tx_buffer[IL005_LENGTH_INDEX]  = ENCRYPT_LENGTH;
    tx_buffer[IL005_INSCODE_INDEX] = IL005_ENCRYPT;
    tx_buffer[IL005_P1_INDEX]      = mode;
    tx_buffer[IL005_P2_INDEX]      = (uint8_t)(source);
    tx_buffer[IL005_P2_INDEX + 1]  = (uint8_t)(source >> 8);
    memcpy(&tx_buffer[IL005_OPTIONALDATA_INDEX], data, 32);
  }
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("Encrypt", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("Encrypt", rx_buffer);  
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != ((mode == 0x00) ? (ENCRYPT_RSP_SIZE_START) : (ENCRYPT_RSP_SIZE_CONTINUE)))
    return IL005_INVALID_SIZE;
  
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);  
}

/**
  * @brief  This function decrypts the cipher text that was generated by the Encrypt command and restores it back to plaintext.
  * @param  rx_buffer pointer to receive buffer
  * @param  mode  0x00(Start), 0x01(Continue)
  * @param  source 
  * @param  data Cipher or Nonce || Cipher
  * @return status of the operation
  */
IL005_StatusTypeDef il005_Decrypt(uint8_t *rx_buffer, uint8_t mode, uint16_t source, uint8_t *data)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[DECRYPT_LENGTH_LONG];
  static uint8_t nonce[32];
  static uint32_t counter;
  uint8_t temp[IL005_CMD_SIZE_MAX];
  uint8_t i, j=0;  

  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || !data || ((mode & ~DECRYPT_MODE_MASK) != 0))
		return IL005_PARAMETER_ERROR;

  // 2. Make Packet
 if((source&0x0002) == 0x0002)
  {
    if(mode == 0x00)
    {
      counter = 0;
      memset(nonce, 0x00, 32);
      memcpy(nonce, data, 8);
      memcpy(nonce+12, &counter, 4);
    }
    else
    {
      counter = counter + 1;
      memcpy(&nonce[12], &counter, 4);
    }
    
    ret_code = il005_GenMAC(rx_buffer, 0x00, (source >> 8), nonce);
    if(ret_code != IL005_OK)
      return ret_code;
            
    if(mode == 0x00)
      j = 8;
    
    for(i = 0; i < 32; i++)      
      temp[i] = rx_buffer[i+1] ^ data[i+j];
      
    rx_buffer[0] = 1 + 32 + 2;
    memcpy(rx_buffer+1, temp, 32);
    il005_calculate_crc16(rx_buffer[0]-2, rx_buffer, &rx_buffer[rx_buffer[0]-2]);  
    RSP_PRINTF("Decrypt", rx_buffer);      
    
    return IL005_OK;    
  }
  else
  {
    tx_buffer[IL005_INSCODE_INDEX] = IL005_DECRYPT;
    tx_buffer[IL005_P1_INDEX]      = mode;
    tx_buffer[IL005_P2_INDEX]      = (uint8_t)(source);
    tx_buffer[IL005_P2_INDEX + 1]  = (uint8_t)(source >> 8);
    if(mode == 0x00)
    {
      tx_buffer[IL005_LENGTH_INDEX] = DECRYPT_LENGTH_LONG;
      memcpy(&tx_buffer[IL005_OPTIONALDATA_INDEX], data, DECRYPT_IV_CHIPER_SIZE); // Decrpyt command index for data with iv & cipher
    }
    else
    {
      tx_buffer[IL005_LENGTH_INDEX] = DECRYPT_LENGTH_SHORT;  
      memcpy(&tx_buffer[IL005_OPTIONALDATA_INDEX], data, DECRYPT_CHIPER_SIZE);    // Decrpyt command index for data with cipher   
    }
  }
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("Decrypt", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("Decrypt", rx_buffer);  
  
  // 5. Check Reponse Packet Size
  if (rx_buffer[0] != DECRYPT_RSP_SIZE)
    return IL005_INVALID_SIZE;  
  
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);  
}

/**
  * @brief  This function is used to compute the SHA256 digest for general purposes to be used by the master.
  * @param  rx_buffer pointer to receive buffer
  * @param  mode
  * @param  message
  * @return status of the operation
  */
IL005_StatusTypeDef il005_SHA256(uint8_t *rx_buffer, uint8_t mode, uint8_t *message)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[SHA256_LENGTH_LONG];

  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || ((mode & ~SHA256_MODE_MASK) != 0))
		return IL005_PARAMETER_ERROR;
  
  // 2. Make Packet
	tx_buffer[IL005_LENGTH_INDEX]  = SHA256_LENGTH_SHORT;
	tx_buffer[IL005_INSCODE_INDEX] = IL005_SHA256;
	tx_buffer[IL005_P1_INDEX]      = mode;
	tx_buffer[IL005_P2_INDEX]      = 0;
	tx_buffer[IL005_P2_INDEX + 1]  = 0;
	if (mode == SHA256_MODE_CALCULATION)
	{
		memcpy(&tx_buffer[IL005_OPTIONALDATA_INDEX], message, SHA256_MESSAGE_SIZE);
		tx_buffer[IL005_LENGTH_INDEX] = SHA256_LENGTH_LONG;
	}  
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer,
                        tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  CMD_PRINTF("SHA256", tx_buffer);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer);
  if(ret_code != IL005_OK)
    return ret_code;
  RSP_PRINTF("SHA256", rx_buffer);
  
  // 5. Check Reponse Packet Size  
  if (rx_buffer[0] != ((mode != SHA256_MODE_CALCULATION) ? (SHA256_RSP_SIZE_INIT) : (SHA256_RSP_SIZE_CAL)))
    return IL005_INVALID_SIZE;

  if (mode != SHA256_MODE_CALCULATION)
  {
    if (rx_buffer[1] != SHA256_INIT_RSP_SUCCESS)
      return IL005_INVAILD_DATA;
  }
  
  // 6. Check Checksum
	return il005_check_crc16(rx_buffer);
}

/**
  * @brief  This function creates a command packet, sends it, and receives its response.
  * @param  rx_buffer pointer to receive buffer
  * @param  input_buffer pointer to send buffer
  * @return status of the operation
  */
IL005_StatusTypeDef il005_test_data_in_out(uint8_t *rx_buffer, uint8_t *input_buffer)
{
  IL005_StatusTypeDef ret_code;
  uint8_t tx_buffer[512];
  uint8_t rx_buffer2[512];

  memset(tx_buffer, 0x00, sizeof(tx_buffer));
  memset(rx_buffer2, 0x00, sizeof(rx_buffer2));
  
  // 1. Check Parameter 
	if (!tx_buffer || !rx_buffer || !input_buffer)
		return IL005_PARAMETER_ERROR;

  // 2. Make Packet
	tx_buffer[IL005_LENGTH_INDEX] = input_buffer[0]+2;    
	memcpy(&tx_buffer[IL005_INSCODE_INDEX], input_buffer+1, tx_buffer[IL005_LENGTH_INDEX] - 3);        // Write command index for data
  
  // 3. Calculate CRC
  il005_calculate_crc16(tx_buffer[IL005_LENGTH_INDEX] - 2, tx_buffer, tx_buffer + tx_buffer[IL005_LENGTH_INDEX] - 2);
  
  // 4. Command & Response
  ARRAY_PRINTF("TEST  IN", tx_buffer, tx_buffer[0]);  
  ret_code = il005c_send_and_receive(tx_buffer, rx_buffer2);
  if(ret_code != IL005_OK)
    return ret_code;
  else
    ARRAY_PRINTF("TEST OUT", rx_buffer2, rx_buffer2[0]);  
  
  memcpy(rx_buffer, rx_buffer2, rx_buffer2[0]);

  // 5. Check CRC
	return il005_check_crc16(rx_buffer);
}

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/
