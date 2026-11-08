#define uchar unsigned char
#define uint  unsigned int
               
                #include    "USER_PROGRAM.H"  
                
typedef union BYTEBITSDEF
{
	unsigned char uc; 
	struct  
	{
		unsigned char BIT0:1; 
		unsigned char BIT1:1; 
		unsigned char BIT2:1; 
		unsigned char BIT3:1; 
		unsigned char BIT4:1; 
		unsigned char BIT5:1; 
		unsigned char BIT6:1; 
		unsigned char BIT7:1; 
	} b; 
} TYPEBYTEBITS;

TYPEBYTEBITS  R_READER_KEY;              
                
#define         _sda_pcu     _pcpu7 
#define         _sda_c       _pcc7 
#define         _sda         _pc7 
#define         _scl_pcu     _pcpu6 
#define         _scl_c       _pcc6 
#define         _scl         _pc6 

volatile  uchar    IIC_READ_DATA1; 
volatile  uchar    IIC_READ_DATA2;   
volatile  uchar    r_data; 
volatile  uchar    r_data1;
volatile  uchar    r_data2;
volatile  uchar    r_add_key;
volatile  uchar    R_key_checksum;

void F_READER_KEY();
void F_Compare_reader_key();
void dealy();            
//==============================================
//**********************************************
//==============================================
DEFINE_ISR (Interrupt_Extemal, 0x04)

{
	
GCC_NOP();	
	
}
//==============================================
//**********************************************
//==============================================
void USER_PROGRAM_INITIAL()
{
	
  _papu=0;
  _pac=0;	
  _pa=0;	
  _pbpu=0;
  _pbc=0;	
  _pb=0;	
  _pcpu=0;
  _pcc=0;	
  _pc=0; 
  _pc7=1;
  _pc6=1;
}

//==============================================
//**********************************************
//==============================================
void dealy()
{
	   GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
	   GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
	   GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
	   GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
       GCC_NOP();
}
//==============================================
//**********************************************
//==============================================
void F_READER_KEY()
{
    uchar i=0;
    _sda_c=1;
  //================================
  if(!_sda)
   { 
   IIC_READ_DATA1=0;
   IIC_READ_DATA2=0;      
      
     //_c=0;
	for( i=0 ; i<16 ; i++)
	  {
	   dealy();
	   _scl_c=0;			
       _scl=0;	

       dealy();
       _scl_c=1;
        _c=0;
       
        //IIC_READ_DATA1 |= 0b00000000;
		if(_sda)
		 {
		 //IIC_READ_DATA1 |= 0b00000001;
		  _c=1;	
		 } //else  
		   //{}
		 
		GCC_RRC(IIC_READ_DATA1);
		GCC_RRC(IIC_READ_DATA2);
		
         dealy();

	  }
	  _sda=1;
	 _sda_c=0;
     _sda=1; 
   }

}
//==============================================
//**********************************************
//==============================================
void F_Compare_reader_key()
{   //uchar     
	r_data=IIC_READ_DATA1;
	r_data1=r_data&0xf0;     // 判断bit12--15是不是A0H
	if(!(r_data1==0XA0))
 	  {
	   IIC_READ_DATA1=0;
 	   IIC_READ_DATA2=0;
 	   return;	
	  }
	 
	 R_READER_KEY.uc=IIC_READ_DATA2;     // 判断bit8--11数据是不是等于按键数据
	 r_add_key=0;
	 if(!R_READER_KEY.b.BIT0)r_add_key++;
	 if(!R_READER_KEY.b.BIT1)r_add_key++;
	 if(!R_READER_KEY.b.BIT2)r_add_key++;
	 if(!R_READER_KEY.b.BIT3)r_add_key++;
	 if(!R_READER_KEY.b.BIT4)r_add_key++;
	 if(!R_READER_KEY.b.BIT5)r_add_key++;
	 if(!R_READER_KEY.b.BIT6)r_add_key++;
	 if(!R_READER_KEY.b.BIT7)r_add_key++;
	R_key_checksum=IIC_READ_DATA1;        
	r_data2=R_key_checksum&0x0f;
	if(!(r_data2==r_add_key))
	 {
	   r_add_key=0;
	   IIC_READ_DATA1=0;
 	   IIC_READ_DATA2=0;	
	 }
}
//==============================================
//**********************************************
//==============================================
void USER_PROGRAM()
{  
  F_READER_KEY();
  F_Compare_reader_key();
}