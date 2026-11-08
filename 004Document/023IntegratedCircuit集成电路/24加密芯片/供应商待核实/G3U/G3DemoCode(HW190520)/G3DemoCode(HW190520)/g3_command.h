/** 
  *****************************************************************************
  * @file    		    : G3command.h
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 7.7
  * @brief          : Header of G3command.c file.
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#ifndef __G3_COMMAND_H
#define __G3_COMMAND_H

/* Includes ------------------------------------------------------------------*/
#include "g3_define.h"

/* Exported functions --------------------------------------------------------*/   



G3_StatusTypeDef g3api_certification(uint8_t key_index, EN_CERTIFICATION_WRITE_MODE certification_write_mode, uint8_t * cert, int16_t cert_size);
G3_StatusTypeDef g3api_change_password(uint8_t key_index, uint8_t * passwd, uint8_t passwd_size);
G3_StatusTypeDef g3api_decryption(uint8_t key_index, EN_KEY_TYPE key_type, EN_BLOCK_MODE block_mode, uint8_t* iv, uint8_t* cipher, uint8_t cipher_size, uint8_t* data);
G3_StatusTypeDef g3api_decryption_ecies(uint8_t key_index, uint8_t *r,uint8_t *s);
G3_StatusTypeDef g3api_diversify(uint8_t key_index,EN_DIVERSIFY_MODE diversify_mode,uint8_t* data,uint8_t data_size);
G3_StatusTypeDef g3api_dynamic_auth(uint8_t key_index, EN_DYNAMIC_AUTH dauth_option, uint8_t pos_pub_dynamic, uint8_t * msg, uint8_t msg_size, void * sign_structure, uint8_t structure_size);
G3_StatusTypeDef g3api_ecdh(EN_ECDH_MODE en_ecdh_mode,  void* Q_b, uint8_t Q_b_struct_size, uint8_t* st_ecdh_random, uint8_t* Q_chip, void* ecdh_value, uint8_t ecdh_value_struct_size);
G3_StatusTypeDef g3api_encryption(uint8_t key_index, EN_KEY_TYPE key_type, EN_BLOCK_MODE block_mode, uint8_t* iv, uint8_t* data, uint8_t data_size, uint8_t* cipher);
G3_StatusTypeDef g3api_encryption_ecies(uint8_t key_index, uint8_t *rs);
G3_StatusTypeDef g3api_get_challenge(uint8_t chall_size, uint8_t * challenge);
G3_StatusTypeDef g3api_get_public_key(uint8_t key_index, EN_PUB_TYPE pub_type, void* pub_key, uint8_t structure_size);
G3_StatusTypeDef g3api_init_puf(uint8_t key_index, uint8_t *initial);
G3_StatusTypeDef g3api_issue_certification(uint8_t key_index, uint16_t public_key_pos, EN_ISSUE_CERT_AREA_TYPE issue_cert_area_type, uint8_t sector_num_to_store, uint16_t key_id, uint8_t* encrypted_key, uint8_t * cert, uint16_t cert_size);G3_StatusTypeDef g3api_read_key_value(uint8_t key_index, EN_AREA_TYPE area_type, EN_RW_INST_OPTION rw_type,void* data,uint8_t data_size,void* data_structure);
G3_StatusTypeDef g3api_reset();
G3_StatusTypeDef g3api_session(uint8_t key_index,EN_SESSION_MODE en_session_mode, uint8_t* indata, uint8_t indata_size, uint8_t* outdata);
G3_StatusTypeDef g3api_set_extern_public_key(void* pub_key, uint8_t structure_size, uint8_t* puk_hash);
G3_StatusTypeDef g3api_hash(EN_SEL_HASH_ALG alg, EN_HASH_MODE hash_mode, uint8_t* data, uint16_t data_size, uint8_t* outdata);
G3_StatusTypeDef g3api_sign(uint8_t key_index, EN_SIGN_OPTION sign_option, uint8_t * msg, uint8_t msg_size, void * sign_structure, uint8_t structure_size);
G3_StatusTypeDef g3api_verify(uint8_t key_index, EN_VERIFY_OPTION verify_option, uint8_t * msg, uint8_t msg_size, void * sign_structure, uint8_t structure_size);
G3_StatusTypeDef g3api_verify_passwd(uint8_t key_index, uint8_t * passwd,uint8_t passwd_size);
G3_StatusTypeDef g3api_write_key_value(uint8_t key_index, EN_AREA_TYPE area_type, EN_RW_INST_OPTION rw_type, void* data_structure, uint8_t structure_size);

//G3_StatusTypeDef g3_normal_process(uint8_t inst, uint8_t p1, uint16_t p2, uint8_t *data, uint8_t length);
G3_StatusTypeDef g3_normal_process(uint8_t inst, uint8_t p1, uint16_t p2, uint8_t *data, uint8_t length,uint8_t **recv_buff);
G3_StatusTypeDef g3_normal_process_ok(uint8_t inst, uint8_t p1, uint16_t p2,uint8_t **recv_buff);
#endif /*__G3COMMAND_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/