/****************************(C) COPYRIGHT 2016 ICTK Co., LTD.*************************************/
/* program		  	: ictk_lib.c 													                                        	*/
/* processor 	  	: STM32F405RGT																	                               	*/
/* compiler		  	: IAR ARM 6.6 															                               			*/
/* program BY	  	: bckang																		                                    */
/* date			  	  : 2016.02.02																		                                */
/* copyright	  	: ICTK Co., LTD																                              		*/
/**************************************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ictk_lib.h"
//#include "usbd_cdc_if.h"
//#include "cqueue.h"

#include <termios.h>
#include <unistd.h>

int mygetch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO);
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

void iprintf(char *fmt,...) 
{
	va_list ap;				                        //define char* type and pointert to read valiable value
	char buf[128];

	va_start(ap,fmt);		                    //calculate start address for fmt
	vsprintf(buf,fmt,ap);                   	//save data form fmt to buf buffer
	va_end(ap);				                        //end
//	cdcSend2Host((uint8_t *)buf,strlen(buf)); //send	
	printf("%s", (uint8_t *)buf);
}

void cmd_printf(char *fmt, uint8_t *cmd) 
{  
	va_list ap;				                        //define char* type and pointert to read valiable value
	char buf[128];
  uint8_t i;

  iprintf("\r\n > ");
//	va_start(ap,fmt);		                    //calculate start address for fmt
	vsprintf(buf,fmt,ap);                   	//save data form fmt to buf buffer
//	va_end(ap);				                        //end
//	cdcSend2Host((uint8_t *)buf,strlen(buf)); //send	  
	printf("%s", (uint8_t *)buf);
  
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
	va_list ap;				                        //define char* type and pointert to read valiable value
	char buf[128];
  uint8_t i;

  iprintf("\r\n > ");
//	va_start(ap,fmt);		                    //calculate start address for fmt
	vsprintf(buf,fmt,ap);                   	//save data form fmt to buf buffer
//	va_end(ap);				                        //end
//	cdcSend2Host((uint8_t *)buf,strlen(buf)); //send	  
	printf("%s", (uint8_t *)buf);
  
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
	int temp;
//  if(cdc_fifo_pop2(rxch))
  temp= getchar();
//  iprintf("111");   
  if(temp != EOF)
  {
  	if(temp != 0x0A)
  	{
//  iprintf("222");     		
  		*rxch = (uint8_t)temp;
    while(getchar()!='\n');	  		
  	}
	else
		{
//  iprintf("333");   			
		*rxch = 0x0D;
	}
  	return  1;
  }
  else 
{
//  iprintf("444");   	
	return 0;
}
}

void array_printf(char *fmt, uint8_t *array, uint8_t array_cnt) 
{  
	va_list ap;				                        //define char* type and pointert to read valiable value
	char buf[128];
  uint8_t i,j;
  uint8_t buf_cnt;

  iprintf("\r\n > ");
//	va_start(ap,fmt);		                    //calculate start address for fmt
	vsprintf(buf,fmt,ap);                   	//save data form fmt to buf buffer
//	va_end(ap);				                        //end
//	cdcSend2Host((uint8_t *)buf,strlen(buf)); //send	  
	printf("%s", (uint8_t *)buf);
  buf_cnt = (uint8_t)strlen(buf);
  
  iprintf(" : ");
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

uint32_t mEnterInput(uint8_t length)
{
	uint8_t	i, len;
	int rint;
	uint8_t	rch;
	uint8_t	outb[128]={0};
	uint8_t	tempb[128]={0};
	uint32_t	temp = 0;

	len=0;
	while(1)
	{
		rint = mygetch();
		rch = (uint8_t)rint;
//iprintf("%X",rch);		
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
		else if(rch==0x0A) // Enter
		{
//iprintf("ENTER");			
			if(len==0) return 0;
			break;
		}
		else if(rch==0x7F)   // Back Space
		{
//iprintf("BS");			
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
	int rint;
	uint8_t	rch;
	uint8_t	outb[500]={0};
 
  if(length == 0)
    length = 500;
  else if(length > 250)
    length = 500;
  else
    length = length*2;

	len=0;
	while(1)
	{
		rint = mygetch();
		rch = (uint8_t)rint;		
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
		else if(rch==0x0A) // Enter
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

/****************************(C) COPYRIGHT 2016 ICTK Co., LTD.*************************************/