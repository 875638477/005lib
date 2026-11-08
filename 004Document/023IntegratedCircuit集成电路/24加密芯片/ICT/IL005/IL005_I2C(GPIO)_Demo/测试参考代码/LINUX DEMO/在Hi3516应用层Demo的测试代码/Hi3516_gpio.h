#ifndef __HI3516_GPIO_H
#define __HI3516_GPIO_H

/* Includes ------------------------------------------------------------------*/
#include "il005define.h"

/* Defines -------------------------------------------------------------------*/


/* Variables -----------------------------------------------------------------*/
extern void *sda_addr, *scl_addr;

/* Exported functions --------------------------------------------------------*/   
unsigned char hi3516a_pin_read(void);
void pinMode(unsigned char pin, unsigned char dir);

#endif /*__HI3516_GPIO_H*/

/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/


