#include <stdio.h>
#include <sys/mman.h>
//#include <unistd.h>
#include <fcntl.h>
#include <sys/io.h>

#include "Hi3516_gpio.h"
#include "timer_util.h"

#define MAP_SIZE  0x1000
#define MAP_MASK  (MAP_SIZE - 1)

#define GPIO0_2_MUXCTRL 0x200F00DC
#define GPIO0_3_MUXCTRL 0x200F00E0
#define GPIO0_4_MUXCTRL 0x200F00E4

#define GPIO0_DIR  0x20140400
#define GPIO0_DATA 0x201403FC

#define HW_REG(reg)  *((volatile unsigned long *)(reg))
#define Hi3516_gpio_cfgpin(addr,dir) HW_REG(addr) = dir
#define Hi3516_gpio_setpin(addr,value) HW_REG(addr) = value
#define Hi3516_gpio_getpin(addr) HW_REG(addr)

#if 0
  #define PRINTF printf
#else
  #define PRINTF(args...)
#endif


unsigned char writeMem(unsigned long memAddr, unsigned long value)
{
  int fd;
  void *map_base, *virt_addr;
  off_t target = memAddr;
  unsigned char ret;

  // open the mem file
  if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
    return 0;

  // map one page
  map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
  if (map_base == (void *)-1)
    return 0;

  // map virutial space
  virt_addr = map_base + (target & MAP_MASK);

  // write the value
  *((uint32_t *) virt_addr) = value;

  // clean up
  ret = (munmap(map_base, MAP_SIZE) != -1);
  close(fd);

  return ret;
}

unsigned char readMem(unsigned long memAddr, unsigned long value)
{
  int fd;
  void *map_base, *virt_addr;
  off_t target = memAddr;
  unsigned char ret;

  // open the mem file
  if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
    return 0;

  // map one page
  map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
  if (map_base == (void *)-1)
    return 0;

  // map virutial space
  virt_addr = map_base + (target & MAP_MASK);

  // read the value
  value = *((uint32_t *) virt_addr);

  // clean up
  ret = (munmap(map_base, MAP_SIZE) != -1);
  close(fd);

  return ret;
}

#define MUXCTRL_OFFSET  0x200F0000

#define GPIO0_2_MUXCTRL_OFFSET 0xDC
#define GPIO0_3_MUXCTRL_OFFSET 0xE0
#define GPIO0_4_MUXCTRL_OFFSET 0xE4

#define GPIO0_2_MUXCTRL 0x200F00DC
#define GPIO0_3_MUXCTRL 0x200F00E0
#define GPIO0_4_MUXCTRL 0x200F00E4

#define GPIO0_DIR  0x20140400
#define GPIO0_DATA 0x201403FC

#define GPIO_DIR_OFFSET   0x400
#define GPIO0_DATA_OFFSET 0x20140000

#define GPIO0_2_DATA  0x20140010
#define GPIO0_3_DATA  0x20140020
#define GPIO0_4_DATA  0x20140040

#define GPIO_PIN_0    0x004
#define GPIO_PIN_1    0x008
#define GPIO_PIN_2    0x010
#define GPIO_PIN_3    0x020
#define GPIO_PIN_4    0x040
#define GPIO_PIN_5    0x080
#define GPIO_PIN_6    0x100
#define GPIO_PIN_7    0x200
#define GPIO_PIN_ALL  0x3FC


#define INPUT   0
#define OUTOUT  1

void *map_base1, *virt_addr1, *map_base2, *virt_addr2, *map_base3, *virt_addr3;

void *virtualmap(unsigned int physical_address)
{
  int fd;
  void *base_address, *virtual_address;

  // open the mem file
  if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
    return NULL;

  // map one page
  base_address = mmap((void *)0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, physical_address & ~(MAP_MASK));
  if (base_address == (void *)-1)
    return NULL;

  virtual_address = base_address + (physical_address & (MAP_MASK));

  return virtual_address;
}

void *sda_virtual_addr, *scl_virtual_addr, *dir_virtual_addr;


void *sda_addr, *scl_addr, *gpio0_dir_addr;
void hi3516a_i2c_init(void)
{
  void *gpio0_3_muxctrl_addr, *gpio0_4_muxctrl_addr;
  
  // GPIO0_3(SDA) pin muxctrl : GPIO
  gpio0_3_muxctrl_addr = virtualmap(GPIO0_3_MUXCTRL);
  *((unsigned int *)gpio0_3_muxctrl_addr) &= ~(0x3);
  
  // GPIO0_4(SCL) pin muxctrl : GPIO
  gpio0_4_muxctrl_addr = virtualmap(GPIO0_4_MUXCTRL);
  *((unsigned int *)gpio0_4_muxctrl_addr) &= ~(0x4);  

  // GPIO0_2, GPIO0_4 pin DIR : OUTPUT
  gpio0_dir_addr = virtualmap(GPIO0_DIR);
  *((unsigned int *)gpio0_dir_addr) |= 0x18;
  
  // GPIO0_2(SDA) pin data : High
  sda_addr = virtualmap(GPIO0_3_DATA);
  *((unsigned int *)sda_addr) = 1<<3;
  
  // GPIO0_3(SCL) pin data : High
  scl_addr = virtualmap(GPIO0_4_DATA);
  *((unsigned int *)scl_addr) = 1<<4;  
}
//pinMode(SDA_PIN, INPUT);   
void pinMode(unsigned char pin, unsigned char dir)
{
  unsigned int temp;
  
//  temp = *((unsigned int *)gpio0_dir_addr);
  if(dir == OUTPUT)
  {
    *((unsigned int *)gpio0_dir_addr) |= 0x08;
  }
  else
  {
    *((unsigned int *)gpio0_dir_addr) &= 0xF7;
  }
  
}

void hi3516a_pin_write(unsigned int address, unsigned int value)
{
  
}

unsigned char hi3516a_pin_read(void)
{
  unsigned int temp;
  temp = *((unsigned int *)sda_addr);
  if((temp>>3)&0x1 == 0x1)
    return 1;
  else
    return 0;
}
 /* 
int main()
{
  void *addr1;

  hi3516a_i2c_init();

    *((unsigned int *)sda_addr) = 1<<3;

    int k;
  for(k =0 ; k < 1000; k++)
  {  
    *((unsigned int *)sda_addr) = 0<<3;
    
    *((unsigned int *)scl_addr) = 0<<4;   
    delay_ms(1000);
    *((unsigned int *)sda_addr) = 1<<3;
    
    *((unsigned int *)scl_addr) = 1<<4;   
    delay_ms(1000);
  }
 
}
*/





/*


unsigned char hi3516a_i2c_init(void)
{
  int fd;
  int i;

  off_t target1, target2, target3;
  
  printf("1\r\n");
  
  if((fd=open("/dev/mem", O_RDWR | O_SYNC)) == -1)
  {
    printf("error1\r\n");
    return -1;
  }
  
  
  target1 = 0x200F00DC;
  
  map_base1 = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target1 &~(MAP_MASK));
  if(map_base1 == (void*)-1)
  {
    printf("error2\r\n");
  	return -1;
  }
  
  virt_addr1 = map_base1 + (target1 & (MAP_MASK));
  
  *((unsigned int *)virt_addr1) &= ~(0x3);
  
  if((fd=open("/dev/mem", O_RDWR | O_SYNC)) == -1)
  {
    printf("error1\r\n");
    return -1;
  }
  
      
  target2 = 0x20140400;
  
  map_base2 = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target2 &~(MAP_MASK));
  if(map_base2 == (void*)-1)
  {
    printf("error2\r\n");
  	return -1;
  }
  
  virt_addr2 = map_base2 + (target2 & (MAP_MASK));
  
  *((unsigned int *)virt_addr2) |= 0xFF;
 
  if((fd=open("/dev/mem", O_RDWR | O_SYNC)) == -1)
  {
    printf("error1\r\n");
    return -1;
  }  
  
  target3 = 0x201403FC;  
  map_base3 = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target3 &~(MAP_MASK));
  if(map_base3 == (void*)-1)
  {
    printf("error2\r\n");
  	return -1;
  }
  
  virt_addr3 = map_base3 + (target3 & (MAP_MASK));


}*/
