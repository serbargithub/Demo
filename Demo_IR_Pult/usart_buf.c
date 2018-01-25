//Utilities for working with UART using a buffer (create and output a string)

#include <pic18.h>
#include "usart.h" 


#define MAX_STR_Buf 1024

unsigned char STR_Buf[MAX_STR_Buf];
unsigned int point_buf;					//position counter in the buffer

//---------------------------
//Send a string from the buffer
void putst_buf(unsigned char *str)	
{
	check_err();
	while((*str)!=0)
	{
		putch(*str);
		str++;
	}
}
//--------------------------------------
//Writing a string to the buffer. If point_n=0 - writing from the begin of buffer
void wr_strbuf(register const unsigned char *str, unsigned char point_n)				
{
	if (!point_n) {point_buf=0;}
	while((*str)!=0)
	{
		if (!(point_buf<MAX_STR_Buf)){STR_Buf[point_buf-1]=0;return;}   //if there is the end of the buffer then return
		STR_Buf[point_buf]=*str;
		point_buf++;
		str++;
	}
		STR_Buf[point_buf]=0;							//add zero for ending of string 
}
//----------------------------------------
//Writing a byte to the buffer. If point_n=0 - writing from the begin of buffer
void wr_ptbuf(unsigned char c, unsigned char point_n)				
{
	if (!point_n) {point_buf=0;}
		if (!(point_buf<MAX_STR_Buf)){STR_Buf[point_buf-1]=0;return;}  
		STR_Buf[point_buf]=c;
		point_buf++;
		STR_Buf[point_buf]=0;							//add zero for ending of string 
}
//----------------------------------------
//Char to decimal and writing to the buffer. If point_n=0 - writing from the begin of buffer
void wr_ptbuf_dec(unsigned char c, unsigned char point_n)
{
	unsigned char temp;
	if (!point_n) point_buf=0;
	temp=c;
	//hundreds
	if ((c/100)>0) wr_ptbuf((c/100)+'0',1);
	c-=(c/100)*100;

	//tens
	if (((temp/10)>0) || ((temp/100)>0)) wr_ptbuf((c/10)+'0',1);
	c-=(c/10)*10;

	//ones
	wr_ptbuf((c/1)+'0',1);
}

//-------------------------------------------------
//finding N sybol into data
unsigned int find_N_char(unsigned char *massive,unsigned char simvol,unsigned char n_cnt,unsigned int MAX_buf) 
{
unsigned int ukaz;
ukaz=0;
	if (*massive==simvol){ --n_cnt;}
	while(n_cnt)
	{
	ukaz++;	
	massive++;
	if((ukaz>=MAX_buf)||(!(*massive))) {return 0xFFFF;}
	if (*massive==simvol){ --n_cnt;continue;}
	}
return ukaz;
}
//--------------------------------------------
//comparing buffer2 with buffer1 from beginning to zero or end symbol 
unsigned char compare_buf(unsigned char *bufer1,unsigned char *bufer2,unsigned char end_symb,unsigned char max_buf2)
{
unsigned char i;
for (i=0;i<max_buf2;i++)
	{
		if ((!*bufer2)&&(!*bufer1)) {return 1;}  
		if  (*bufer2==end_symb)		{return 1;}	 
		if (*bufer2!=*bufer1) {return 0;}
		bufer1++;
		bufer2++;	
	}
return 0;
}
//--------------------------------------------
//Copy buffer2 with buffer1 from beginning to zero or end symbol(at the and is written 0) 
//in result - pointer to last byte
unsigned char copy_buf(unsigned char *bufer1,unsigned char *bufer2,unsigned char end_symb,unsigned char max_buf2)	
{
unsigned char i;
for (i=0;i<max_buf2;i++)
	{
		*bufer1=*bufer2;
		if (!*bufer2) {return i;} 
		if  (*bufer2==end_symb)	{return i;}	// признак конца
		bufer1++;
		bufer2++;	
	}
return 0;
}
//---------------------------------
//Calculating a parameter from a string the value is calculated from this pointer, left to right to end_symb
unsigned long calk_parametr_buf(unsigned char  *bufer,unsigned char end_symb)	
{
unsigned char *bufer_st,i;
unsigned long mnog,work_long;
bufer_st=bufer-1;	
for(i=0;i<11;i++)	// Max of digitals into buffer
	{
		if (*bufer==end_symb) break;		//  end_symb
		if (*bufer==0x00) break;			//  end_symb
		bufer++;
	}
	if (i==11) {return 0;}		//error prottecting 
	bufer--;  					//correcting
	work_long=0;
	mnog=1;						
while (bufer!=bufer_st)
	{
	work_long+=(*bufer-'0')*mnog;	
	mnog*=10;
	bufer--;
	}
return work_long;
}
