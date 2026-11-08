#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ictk_util.h"

int set_buffer_from_hexstr(void *dest, char *src)
{
	unsigned char *byte;
	char *pvalue;

	int pos = 0;

	int length = 0;
	
	pvalue = src;

	while (*pvalue != NULL)
	{
		pvalue++;
		length++;
	}
	//printf("len : %d\n", length);
	 
	byte = (unsigned char*)malloc(length);
	
#if 1
//	printf("set : \r\n");
	for (int i = 0; i < length / 2; i++)
	{
		sscanf(src + pos, "%2hhX", (byte + i));
		pos += 2;

//		printf("%02X", *(byte + i));
	}
//	printf("\n");

	memcpy((unsigned char*)dest, byte, length);
	free(byte);
#endif

	return length/2;
}