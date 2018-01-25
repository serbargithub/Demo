#include <pic18.h> 
#include "name_port.h"

extern unsigned char cnt_clk,fl_dtmf_yes,dtmf_read;

//==============Delays=================
void delay_ms(unsigned int wait)		//milliseconds
{
	do
	{
	cnt_clk=8;			//1ms
	while (cnt_clk){CLRWDT();}
	}while (--wait);
}			
	
void delay_s(unsigned char wait)	// seconds
	{
	unsigned char i;
	while (wait)
	{
	for (i=0;i<=125;i++)
		{
		cnt_clk=64;			//1s
		while (cnt_clk){CLRWDT();}
		}
	wait--;
	}
	}
void delay_mks(unsigned char wait)		//microseconds
{
while(wait--){asm("nop");}
}
//=====================
