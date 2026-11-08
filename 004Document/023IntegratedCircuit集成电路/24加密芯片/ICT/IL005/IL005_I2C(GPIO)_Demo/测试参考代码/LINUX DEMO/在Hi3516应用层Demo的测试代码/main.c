#include "il005define.h"
#include "il005test.h"
#include "il005communication.h"
#include "i2c_sw.h"
#include "fw_protection.h"
int main(void)
{
//  if(wiringPiSetup() == -1)
//  	return 1;	

  i2c_init();  

  while(1)
  {
    fw_protection();
  }
  
  return 0;
}
