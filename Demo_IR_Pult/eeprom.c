#include <pic18.h>  
#include "my_delay.h"
#include "usart.h"

extern unsigned char fl_read_error;

//-------------------------------------------------------------------
//Reading char from EEPROM
unsigned char EE_READ (unsigned int addr)
{
	unsigned char c;
	while (WR){};
	c=EEPROM_READ(addr);
	while (WR){};
	return c;
}
//-------------------------------------------------------------------
//Writing char to EEPROM
void EE_WRITE (unsigned int addr,unsigned char var)
{
	while (WR){};
	EEPROM_WRITE(addr,var);
}
//-------------------------------------------------------------------
//Reading int from EEPROM
unsigned int EE_READ_Int (unsigned int addr)
{
	unsigned int res;
	while (WR){};
	*(unsigned char*)&res=EE_READ(addr);
	while (WR){};
	*(((unsigned char*)&res)+1)=EE_READ(addr+1);
	return res;
}
//-------------------------------------------------------------------
//Writing int to EEPROM
void EE_WRITE_Int (unsigned int addr,unsigned int var)
{
	while (WR){};
	EE_WRITE(addr,*((unsigned char*)&var));
	while (WR){};
	EE_WRITE(addr+1,*(((unsigned char*)&var)+1));
}
//-------------------------------------------------------------------
//Writing long to EEPROM
void EE_WRITE_Long (unsigned int addr,unsigned long var)
{
	unsigned char j;
	for (j=0;j<4;j++)
	 { 
	  while (WR){};
	  EE_WRITE(addr+j,*(((unsigned char*)&var)+j));
	 }
}
//-------------------------------------------------------------------
//Reading long from EEPROM
unsigned long EE_READ_Long (unsigned int addr)
{
	unsigned long res;
	unsigned char j;
	for (j=0;j<4;j++)
   	{
		while (WR){};
		*(((unsigned char*)&res)+j)=EE_READ(addr+j);
	}
	return res;
}
//-------------------------------------------------------------------
//Writing frame of bytes to adres EEPROM
unsigned char write_eep_frame(unsigned int set_adr_eep,unsigned int cnt_byte_eep) 
{
	unsigned char work;
	clear_buf_RX();

      	while(cnt_byte_eep)
		{
      		work=getch_n(1);   //reading one byte  
	  		if(fl_read_error) {putst("\rError Time\r"); return 0;}
      		cnt_byte_eep--;
	  		CLRWDT();
			while (WR){};
      		EEPROM_WRITE(set_adr_eep,work);
	  		delay_ms(10);
      		putch(work);							// ACK
      		set_adr_eep++;
     	}
	return 1;
}
//-------------------------------------------------------------------
//Reading frame of bytes from adres EEPROM
unsigned char read_eep_frame(unsigned int set_adr_eep,unsigned int cnt_byte_eep)
{
	unsigned char work;
	clear_buf_RX();
	while(cnt_byte_eep)
   `	{
		while (WR){};
   		work=EEPROM_READ(set_adr_eep);
		if(getch_n(1)!='+') {return 0;}	// if not get answer then error
		putch(work);					//echo
		set_adr_eep++;
		cnt_byte_eep--;
		CLRWDT();
   		}
	return 1;
}
