#ifndef __SHA256_H
#define __SHA256_H

/* Includes ------------------------------------------------------------------*/
#include "il005define.h"

/* Types ------------------------------------------------------------*/
typedef struct
{
  uint32_t state[8];
  uint64_t count;
  uint8_t buffer[64];
} CSha256;

/* Exported functions --------------------------------------------------------*/   
void sha256_Init(CSha256 *p);
void sha256_Update(CSha256 *p, const uint8_t *data, uint32_t size);
void sha256_Final(CSha256 *p, uint8_t *digest);
void sha256(const uint8_t *data, uint32_t size, uint8_t *digest);


#endif  /*__SHA256_H*/