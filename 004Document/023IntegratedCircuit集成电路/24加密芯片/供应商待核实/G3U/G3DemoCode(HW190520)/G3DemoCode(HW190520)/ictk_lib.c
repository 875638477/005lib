/** 
  *****************************************************************************
  * @file    		    : ictk_lib.c
  * @author         : Department 1, R&D Center, Security SoC Division
  * @version        : V1.0.0
  * @date           : 14-June-2016
  * @test processor : STM32F405RGT
  * @test compiler  : IAR ARM 7.7
  * @brief          : ICTK Utility Functions
  *****************************************************************************
  * Copyright (c) 2016 ICTK Co., LTD. All rights reserved.
  */

#include <stdio.h>
#include <stdarg.h>

#include "ictk_lib.h"
#include "usbd_cdc_if.h"
#include "cqueue.h"
#include "g3_define.h"

void iprintf(char *fmt,...) 
{
	va_list ap;				                          //define char* type and pointert to read valiable value
	char buf[128];
  int len;
  
  va_start(ap,fmt);		                        //calculate start address for fmt
	len = vsprintf(buf,fmt,ap);                 //save data form fmt to buf buffer
	va_end(ap);				                          //end
  cdcSend2Host((uint8_t *)buf,(uint16_t)len); //send	
}

void cmd_printf(char *fmt, uint8_t *cmd) 
{  
  uint8_t i;

  if(cmd[0] < G3_CMD_SIZE_MIN)
  {
    iprintf("\r\n COMMAND Length Error : ");
    for(i=0;i<cmd[0];i++)
       iprintf("%02X ", cmd[i]);
    iprintf("\r\n");
    return;
  }

//  if(cmd[0] > 128)
  if(cmd[0] > 256)  
  {
    iprintf("\r\n COMMAND Length Error : ");
    for(i=0;i<16;i++)
       iprintf("%02X ", cmd[i]);
    iprintf("\r\n");
    return;
  }  

  iprintf("\r\n > %s", fmt);  
  iprintf(" COMMAND  : \r\n");
  iprintf("  Count    : %02X \r\n", cmd[0]);
  iprintf("  Opcode   : %02X \r\n", cmd[1]);  
  iprintf("  Param1   : %02X \r\n", cmd[2]);  
  iprintf("  Param2   : %02X %02X\r\n", cmd[3], cmd[4]);  
  iprintf("  Data(%2d) : ", cmd[0]-7);  
  for(i=5;i<cmd[0]-2;i++)
  {
     iprintf("%02X ", cmd[i]);
     if(((i-4)%16 == 0) && (i < cmd[0] - 2 - 1))       
        iprintf("\r\n             ");         
  }
  iprintf("\r\n"); 
  iprintf("  Checksum : %02X %02X \r\n", cmd[cmd[0]-2], cmd[cmd[0]-1] );  
}

void rsp_printf(char *fmt, uint8_t *rsp) 
{  
  uint8_t i;

  
  if(rsp[0] < G3_RSP_SIZE_MIN)
  {
    iprintf("\r\n Response Length Error : ");
    for(i=0;i<rsp[0];i++)
       iprintf("%02X ", rsp[i]);
    iprintf("\r\n");
    return;
  }  
  
//  if(rsp[0] > 128)
  if(rsp[0] > 256)
  {
    iprintf("\r\n Response Length Error : ");
    for(i=0;i<16;i++)
       iprintf("%02X ", rsp[i]);
    iprintf("\r\n");
    return;
  }    
  
  iprintf("\r\n > %s", fmt);  
  iprintf(" RESPONSE : \r\n");
  iprintf("  Count    : %02X \r\n", rsp[0]);
  iprintf("  Data(%2d) : ", rsp[0]-3);
  for(i=1;i<rsp[0]-2;i++)
  {
     iprintf("%02X ", rsp[i]);
     if((i%16 == 0) && (i < rsp[0] - 2 - 1))
        iprintf("\r\n             ");         
  }
  iprintf("\r\n"); 
  iprintf("  Checksum : %02X %02X \r\n", rsp[rsp[0]-2], rsp[rsp[0]-1] );
}

uint8_t igetc(uint8_t *rxch)
{
  if(cdc_fifo_pop2(rxch))
    return  1;
  return 0;
}

void array_printf(char *fmt, uint8_t *array, uint16_t array_cnt) 
{
  uint16_t i,j;
  uint16_t buf_cnt;

  buf_cnt = (uint8_t)strlen(fmt);

  iprintf("\r\n > %s : ", fmt);
  for(i=0;i<array_cnt;i++)
  {
     iprintf("%02X ", array[i]);
     if(((i+1)%16 == 0) && (i < array_cnt - 1))
     {
       iprintf("\r\n");
       for(j=0;j<buf_cnt+6;j++)
         iprintf(" ");
     }
  }
  iprintf("\r\n"); 
}

void err_printf(char *fmt,...) 
{
	va_list ap;				                          //define char* type and pointert to read valiable value
	char buf[128];
  int len;

	va_start(ap,fmt);		                        //calculate start address for fmt
	len = vsprintf(buf,fmt,ap);                 //save data form fmt to buf buffer
	va_end(ap);				                          //end
	cdcSend2Host((uint8_t *)buf,(uint16_t)len); //send	 
}

uint32_t mEnterInput(uint8_t length)
{
	uint8_t	i, len;
	uint8_t	rch;
	uint8_t	outb[128]={0};
	uint8_t	tempb[128]={0};
	uint32_t	temp = 0;

	len=0;
	while(1)
	{
		if(!GETC(&rch))
			continue;
		if((rch>='0' && rch<='9') && len<length*2)
		{
			iprintf("%c",rch);
			outb[len++] = rch-'0';
		}
		else if((rch>='A' && rch<='F') && len<length*2)
		{
			iprintf("%c",rch);
			outb[len++] = rch-'A'+10;
		}
		else if((rch>='a' && rch<='f') && len<length*2)
		{
			iprintf("%c",rch);
			outb[len++] = rch-'a'+10;
		}
		else if(rch==0x0D) // Enter
		{
			if(len==0) return 0;
			break;
		}
		else if(rch==0x08)   // Back Space
		{
			if(len==0);
			else
			{
				iprintf("%c",0x08);				
				iprintf(" ");								
				iprintf("%c",0x08);	
				len--;
				outb[len] = 0;
			}
		}
		else;
	}

	if(len%2)
	{
		for(i=0; i<len; i++)
		{
			tempb[i+1] = outb[i];
		}
		tempb[0] = 0;
		len++;
	}
	else
	{
		for(i=0; i<len; i++)
		{
			tempb[i] = outb[i];
		}
	}

	for(i=0; i<(len/2); i++)
	{
		temp |= (((tempb[i*2]&0x0F)<<4) + ((tempb[i*2+1]&0x0F)<<0)) << (len/2 - i - 1)*8;
	}
	
//	iprintf("\r\nInput Data : 0x%04X\r\n",temp);

	return temp;
}

uint8_t mgets(uint8_t *pBuf, uint16_t length)
{
	uint8_t	i, len;
	uint8_t	rch;
	uint8_t	outb[500]={0};
 
  if(length == NULL)
    length = 500;
  else if(length > 250)
    length = 500;
  else
    length = length*2;

	len=0;
	while(1)
	{
		if(!GETC(&rch))
			continue;
		if((rch>='0' && rch<='9') && len<length)
		{
			iprintf("%c",rch);
			outb[len++] = rch-'0';
		}
		else if((rch>='A' && rch<='F') && len<length)
		{
			iprintf("%c",rch);
			outb[len++] = rch-'A'+10;
		}
		else if((rch>='a' && rch<='f') && len<length)
		{
			iprintf("%c",rch);
			outb[len++] = rch-'a'+10;
		}
		else if(rch==0x0D) // Enter
		{
      if(len%2 != 0)  continue;
			if(len==0)  return 0;
			break;
		}
		else if(rch==0x08)   // Back Space
		{
			if(len==0);
			else
			{
				iprintf("%c",0x08);				
				iprintf(" ");								
				iprintf("%c",0x08);	
				len--;
				outb[len] = 0;
			}
		}
		else;
	}

	for(i=0; i<(len>>1); i++)
	{
		pBuf[i] = ((outb[i*2]&0x0F) << 4) | ((outb[i*2+1]&0x0F) << 0);
	}

	return (len>>1);
}
/************************ (c) COPYRIGHT 2016 ICTK Co., LTD. *****END OF FILE*****/