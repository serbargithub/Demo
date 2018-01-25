#include <pic18.h>
#include "my_delay.h"
#include "name_konst.h"

extern unsigned char SW_uart;	
extern unsigned char clear_bufrx,ask_geth_byte,st_bufrx,end_bufrx,geth_byte;
extern unsigned char clear_bufrx2,ask_geth_byte2,st_bufrx2,end_bufrx2,geth_byte2;

extern unsigned char fl_cary;			
extern unsigned char fl_read_error;

extern unsigned char cnt_wt;	//counter from interrupt
extern  unsigned int cnt_secund;


#define sp2400	0
#define sp4800	1
#define sp9600	2
#define sp19200	3
#define sp38400	4
#define sp57600	5
#define sp115200 6

#define PRI_US	0		//chanell 1 usart
#define SEC_US	1		//chanell 2 usart


void serial_setup(unsigned char ind_speed,unsigned char num_usart)	  // calculated for 32Mhz
{
if(!num_usart)
	{
  	switch (ind_speed)
	{

	case 1:				// 4800 baud	
		SPBRG=51;		//
		SPBRGH=0;
		BRGH1=0;
		BRG161=0;
		break;
	case 2:				//9600 baud
		SPBRG=25;		//
		SPBRGH=0;
		BRGH1=0;
		BRG161=0;
		break;
	case 6:				//115200 baud
		SPBRG=14;
		SPBRGH=0;
		BRGH1=1;
		BRG161=1;
		break;
	default:
		break;
	}	
//	BAUDCON1=0;
	SYNC1=0;						//asynchronous
	SPEN1=1;						//enable serial port pins
	CREN1=1;						//enable reception
	SREN1=0;						//no effect
	TX1IE=0;						//disable tx interrupts
	RC1IE=0;						//disable rx interrupts
	TX91=0;							//9-bit transmission
	RX91=0;							//9-bit reception
	TXEN1=0;						//reset transmitter
	TXEN1=1;						//enable the transmitter
	}
	else
	{
  	switch (ind_speed)
	{

	case 1:				// 4800	naud
		SPBRG2=51;
		SPBRGH2=0;
		BRGH2=0;
		BRG162=0;
		break;
	case 2:				//9600 baud
		SPBRG2=25;
		SPBRGH2=0;
		BRGH2=0;
		BRG162=0;
		break;
	case 6:				//115200 baud
		SPBRG2=14;
		SPBRGH2=0;
		BRGH2=1;
		BRG162=1;
		break;
		break;
	default:
		break;
	}	
//	BAUDCON2=0;
	SYNC2=0;						//asynchronous
	SPEN2=1;						//enable serial port pins
	CREN2=1;						//enable reception
	SREN2=0;						//no effect
	TX2IE=0;						//disable tx interrupts
	RC2IE=0;						//disable rx interrupts
	TX92=0;							//8-bit transmission
	RX92=0;							//8-bit reception
	TXEN2=0;						//reset transmitter
	TXEN2=1;						//enable the transmitter
	}
}
//=====================-==
unsigned char check_err(void)					//control end reset errors 
 {
  unsigned char work;
  static unsigned char fl_error=0;

	if (FERR1)	
	{	
		work=RCREG1;
		TXEN1=0;											
		TXEN1=1;	
		FERR1=0; 
		work=RCREG1;
		fl_error=1;
	}
	if (OERR1)
		{
			TXEN1=0;													
			TXEN1=1;	
			CREN1=0;
			CREN1=1;
			work=RCREG1;
			work=RCREG1;
			work=RCREG1;
		}
	if (FERR2)	
	{	
		work=RCREG2;
		TXEN2=0;											
		TXEN2=1;	
		FERR2=0; 
		work=RCREG2;
		fl_error=1;
	}
	if (OERR2)
		{
			TXEN2=0;													
			TXEN2=1;	
			CREN2=0;
			CREN2=1;
			work=RCREG2;
			work=RCREG2;
			work=RCREG2;
		}
 return 1;
 }
//========================
// Clearing buffers
void clear_buf_RX(void)
{
if(!SW_uart)
	{
		clear_bufrx=1;
		while(clear_bufrx);
	}
	else 
	{
		clear_bufrx2=1;
		while(clear_bufrx2);
	}
}
//---------------------------
//reading chain of bytes. Result -the last readed
unsigned char getch_n(unsigned char cnt_byte) 
 {
	unsigned char prom,work;
	fl_read_error=0;
	do
   {
		prom=200;
		if(!SW_uart){work=ask_geth_byte=1;}
				else {work=ask_geth_byte2=1;}
		while (work)
		{	 
			if(!SW_uart){work=ask_geth_byte;}
				else {work=ask_geth_byte2;}
				CLRWDT();
			if (fl_cary)
				{	cnt_wt=255; 
					fl_cary=0;
 					if (!prom--) {fl_read_error=1; return 0xFF;}
				}
		}
	 }while(--cnt_byte);
if(!SW_uart){work=geth_byte;}
	else {work=geth_byte2;}
 return work;
 }
//========================
//Send one byte (for PRINT)
void putch(unsigned char c)	
{
// unsigned char work;
//	check_err();
if(!SW_uart)
	{
		while(!TX1IF){CLRWDT();}			//waiting for free buffer 
		TXREG1=c;
	}
	else
	{
		while(!TX2IF){CLRWDT();}			//waiting for free buffer 
		TXREG2=c;
	}
//	delay_ms(1);
}
//=====================================
//Get one byte
unsigned char getch(void)
{
if(!SW_uart)
	{
	if (end_bufrx==st_bufrx){ return 0; }
	ask_geth_byte=1;
	while(ask_geth_byte){CLRWDT();}
	return geth_byte;
	}
	else
	{
	if (end_bufrx2==st_bufrx2){ return 0; }
	ask_geth_byte2=1;
	while(ask_geth_byte2){CLRWDT();}
	return geth_byte2;
	}
}

//=====================================
//Check buffer for emptyness
unsigned char check_buf(void)
{
if(!SW_uart)
	{
	if (end_bufrx==st_bufrx){ return 0; }
	return 1;
	}
	else
	{
	if (end_bufrx2==st_bufrx2){ return 0; }
	return 1;
	}
}

//==========================
// Send string
void putst(register const char *str)	
{
	check_err();
	while((*str)!=0)
	{
		putch(*str);
    if (*str==13) {putch(10);}
    if (*str==10) {putch(13);}
		str++;
	}
}

//=====================

unsigned char put_command(register const char *str)
{
unsigned char count=0;
unsigned char wrk=0;
	check_err();
	clear_buf_RX();
	while((*str)!=0)
	{
		putch(*str);
		str++;
		count++;
	}
	count+=1;
	while (count--)	
	{	wrk=getch_n(1);
		if(wrk==0x0d) {return 0;}	
		if(wrk==0xFF) {return 1;}
	}
	return 1;
}
//====================================
//Print char to hex format 
void putchhex(unsigned char c)
{
	unsigned char temp;

	// transmits in hex
	temp=c;

	c=(c >> 4);
	if (c<10) c+=48; else c+=55;
	putch(c);

	c=temp;

	c=(c & 0x0F);
	if (c<10) c+=48; else c+=55;
	putch(c);
}
//====================================
//Print int to hex format 
void putinthex(unsigned int c)
{
	#define ramuint(x)			(*((unsigned int *) (x)))
	#define ramuint_hibyte(x)	(*(((unsigned char *)&x)+1))
	#define ramuint_lobyte(x)	(*(((unsigned char *)&x)+0))
	#define ramuchar(x)			(*((unsigned char *) (x)))

	putchhex(ramuint_hibyte(c));
	putchhex(ramuint_lobyte(c));

	#undef ramuint(x)
	#undef ramuint_hibyte(x)
	#undef ramuint_lobyte(x)
	#undef ramuchar(x)
}
//====================================
//Print int to decimal format 
void putchdec(unsigned char c)
{
	unsigned char temp;

	temp=c;
	//hundreds
	if ((c/100)>0) putch((c/100)+'0');
	c-=(c/100)*100;

	//tens
	if (((temp/10)>0) || ((temp/100)>0)) putch((c/10)+'0');
	c-=(c/10)*10;

	//ones
	putch((c/1)+'0');
}

//====================================
//Print long to decimal format 
void putch_long(unsigned long c)		// отображение long числа
{
	static unsigned char st_razr;
	unsigned char temp;
	unsigned long  delit;
	if (!c) {putch('0');return;}
	st_razr=0;
	for (delit=1000000000L;delit>=1;delit/=10)
		{	
			if(((c/delit)>0)||st_razr) 
					{
					temp=(c/delit)+'0';
					st_razr=1;
					putch(temp);
					}
			c-=(c/delit)*delit;
			CLRWDT();
		}
}

//------------------------------------
//Waiting OK within a preset time(seconds) 
unsigned char check_OK(unsigned int time)					
{
char temp;
	check_err();
	clear_buf_RX();						//очистить буфер
	time+=cnt_secund;
	if(cnt_secund>time){time-=cnt_secund; cnt_secund=0;}
	while(1)
	{
	temp=getch_n(1); 
	if(temp=='O')
		{
		temp=getch_n(1); 
		if (temp=='K') return 1;
		}
	if (cnt_secund>=time) return 0;
	}
	return 1;
}
//----------------------------------
//Setting Urar chanell

void ch_usart(char ch)
{
switch(ch)
{
case SEC_US:
	SW_uart=1;
	break;
case PRI_US:
	SW_uart=0;
	break;

default:
	break;
}
}
//-------------------------------
