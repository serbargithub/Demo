#include <pic18.h>
#include "ctype.h"
#include "usart.h" 
#include "usart_buf.h" 
#include "my_delay.h"
#include "name_konst.h"
#include "name_port.h"
#include "eeprom.h"
#include "IR_funct.h"
#include "EEprom_in_PM.h"

#define MAX_BUF_RX_COM		1536
unsigned char buf_rx_command[MAX_BUF_RX_COM];
extern unsigned char end_bufrx,st_bufrx;
extern unsigned char st_bufrx2,end_bufrx2,SW_uart;
extern unsigned int cnt_secund;
extern  unsigned char led_inform;
extern unsigned char fl_sec_code,ask_geth_byte,clear_bufrx;


extern struct com_map
{
unsigned char type_command;		// 0- usual repeating command, 1- non-repeating sequence of 1 packet, 2 non-repeating sequence of two packets
unsigned char time_burn;        // modulation frequency of the output signal and time slot for the input signal: Calculated by the formula Frequency = 1000000 / (N * .241246)
unsigned int  body_count;		// counter of fronts and falls of the body
unsigned int  tail_count;		// counter of fronts and falls of the tail
unsigned int  len_body[16];		// The length of the gap between fronts, is encoded by references in the database with two bits
unsigned char body[Max_slot_Short];	// The body of the command, is coded by halves of bytes with references to the table of durations
};
extern struct com_map command_data;

extern struct pronto_struct
{
unsigned char time_burn;
unsigned int num_slot;
unsigned int tail_slot;
unsigned int each_slot[4];
unsigned int raw_scan[Max_slot_pronto];
} 
struct  pronto_struct pronto_data;


void detect_command_terminal(unsigned int ukaz);
//--------------------------------------------------------------
// Receiver of commands starting with AT. Result - pointer to the last byte of the string
unsigned int load_command_terminal(void)
{	
unsigned char temp;
static unsigned int count_ukaz; 
static unsigned char start_ld_comm;
	ask_geth_byte=0;
	 CLRWDT();
	if(!SW_uart){if (end_bufrx==st_bufrx){ return 0; }}	// No one command was presented- exit
		else {if (end_bufrx2==st_bufrx2){ return 0;}}
 	temp=getch();
	if(!temp) return 0;
	if (!start_ld_comm) {count_ukaz=0; if(!((temp=='A')||(temp=='a'))) { return 0;}}
		start_ld_comm=1;
  		buf_rx_command[count_ukaz]=temp;
	if ((count_ukaz==1)&&(!((temp=='T')||(temp=='t')))){start_ld_comm=0;return 0;} //ia no AT was recived
		putch(temp);			//echo 
		if (temp=='\b') {putst(" \b");count_ukaz--; return 0;}
		if (temp==0x0d)
			{
			  start_ld_comm=0;
				putch(0x0a);		//supplement with 0x0A
				buf_rx_command[count_ukaz+1]=0;//end mark
			 return	count_ukaz;
			}
	if (count_ukaz<(MAX_BUF_RX_COM-2)) count_ukaz++;
	return 0;
}
//-----------------------------------------
//To compare the command
unsigned char uncode_com(register const char *str)	
{
unsigned int ukaz=0;
	while((*str)!=0)
	{
    if ((*str)!=buf_rx_command[ukaz]) {return 0;}	//the command did not match
		ukaz++;
		str++;
	}
return 1;		// total coincidence
}

//--------------------------------------------------------------
// calculation of the parameter from the string (the argument of the last byte of string 0xOD)
unsigned long calk_parametr(unsigned int ukaz)				
{
unsigned long mnog,work_long;
	ukaz--;			//correction
	work_long=0;
	mnog=1;						// discharge multiplier
while ((buf_rx_command[ukaz]!='=')&&(buf_rx_command[ukaz]!=','))
	{
		work_long+=(buf_rx_command[ukaz]-'0')*mnog;	
		mnog*=10;
		if (!(ukaz--)) {return 0;}		//error protecting
	}
return work_long;
}
//--------------------------------------------------------------
//Calculating the INT HEX parameter from the string of 4 characters (the argument is a pointer to the character separator, or the end of the line)
unsigned int calk_HEX_parametr(unsigned int ukaz)	
{
unsigned int work_int,rezult;
	ukaz--;			//correcting
	rezult=0;
while ((buf_rx_command[ukaz]!='=')&&(buf_rx_command[ukaz]!=',')&&(buf_rx_command[ukaz]!=' '))
	{
	work_int=buf_rx_command[ukaz];
	if(work_int<58) {work_int-=48;}		//convert character to hex
		else   {work_int-=55;}
	work_int<<=12;
	rezult>>=4;
	rezult|=work_int;
	if (!(ukaz--)) {return 0;}		// error protection
	}
return rezult;
}
//----------------------------------
void save_parametr(unsigned long Adres)
{
char ukaz_tek,ukaz;
		ukaz_tek=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM-2);	// найти =
		ukaz_tek++;
//		ukaz=copy_buf(Bufer_SPI,&buf_rx_command[ukaz_tek],0x0D,MAX_BUF_RX_COM);//copy to 0D
//		Bufer_SPI[ukaz]=0;		//character of end
//		Frame_Buf_Write(Adres,ukaz+1);
		putst("\rWrite OK\r");

}
//------------------------
void read_parametr(unsigned long Adres)
{
//		Frame_Buf_Read(Adres,0);					//reading to zero
//		putst("\r");putst_buf(Bufer_SPI);putst("\r");

}
//------------------------
void print_parametr(unsigned long Adres)
{
//		Frame_Buf_Read(Adres,0);					// reading to zero
//		putst_buf(Bufer_SPI);

}
//------------------------
void change_uart_init(unsigned char napravl)
{
if (napravl==SEC_US)
	{
/*	GIE=0;
	T2CON=0b00001000;	//пост дел/2
	TMR2IF=0;
	TMR2ON=1;
	GIE=1;
*/
	TRISB6=0;
	TRISC6=1;
	ch_usart(SEC_US);
	serial_setup(sp9600,SEC_US);		//initialization secondary usart
	}
if (napravl==PRI_US)
	{
/*
	GIE=0;
	T2CON=0b00001000;	//пост дел/2
	TMR2IF=0;
	TMR2ON=1;
	GIE=1;
*/
	TRISB6=1;
	TRISC6=0;
	ch_usart(PRI_US);
	serial_setup(sp9600,PRI_US);		//initialization primary usart
	}
}

//======================================== The command detector
void terminal_start (unsigned char napravl)
{
	change_uart_init(napravl);
	led_inform=7;					// show the computer to the LED
	delay_s(1);
	led_inform=0;
	putst("\rSystem Ready\r");
	clear_buf_RX();
}
//----------

void terminal_task(void)
{
unsigned int i_int,work_int;
	work_int=load_command_terminal();
	if (work_int)for(i_int=0;i_int<=work_int;i_int++) 
			{buf_rx_command[i_int]=(char)toupper(buf_rx_command[i_int]);
			if(buf_rx_command[i_int]=='=')break;	// cast to uppercase
			}
	if(work_int){
			detect_command_terminal(work_int);			//command searching with AT starting
			}

	check_err();
}

//-----------------------------------
void detect_command_terminal(unsigned int ukaz)
{
unsigned char wrk_char,i;
unsigned long wrk_long;
unsigned int wrk_int,burn_count,i_int,end_ukaz,ukaz_tek,ukaz_otnos;
static unsigned long set_adr;
static unsigned int set_adr_eep;
if (uncode_com("AT\r")) 	{putst("OK\r");return;} 
if (uncode_com("ATI1")) 				//version of programm
		{
		putst(Ver_Build);	//version typing 
		putst(" Build: ");putst(time_build);
		putst(" ");putst(date_build);	//buid typing
		putst("\r");
		goto	end_ok;
		}  
if (uncode_com("ATI")) 				//read device ID
		{
		putst(Ver_Signal);	//version typing 
		goto	end_ok;
		}  
if (uncode_com("AT+INIT")) 				
		{
		goto	end_ok;
		} 
//-------------------------------------
if (uncode_com("AT+SETFR="))   //Set frequency 
		{
		wrk_char=calk_parametr(ukaz);		
		if((wrk_char<55)|| (wrk_char>155)) {wrk_char=103;}
		EE_WRITE(freq_mod,wrk_char);
		goto	end_ok;
		}
if (uncode_com("AT+RDFR"))		//Read frequency
		{
		wrk_char=calk_parametr(ukaz);
		wrk_char=EE_READ(freq_mod);
		putchdec(wrk_char);putch(' ');
		goto	end_ok;
		}

if (uncode_com("AT+SCAN=")) 	//Scanning			
		{
		wrk_char=calk_parametr(ukaz);			
        if((!wrk_char)||(wrk_char>Max_memory_Cells))goto end_error;
		if(!read_IRsignal()) goto end_error;
		save_scanbuf(wrk_char);

		if(!read_myProntobuf(wrk_char))goto end_error;
		if (!check_data())goto end_error;


		goto	end_ok;
		} 

if (uncode_com("AT+SEND="))		//Send command		
		{
		ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM-2);	// search ','
		if(ukaz_tek==0xFFFF)goto end_error;
		wrk_char=calk_parametr(ukaz_tek);		// calculating address
        if((!wrk_char)||(wrk_char>Max_memory_Cells))goto end_error;
		wrk_int=calk_parametr(ukaz);			// calculating the value
		if(send_IR_signal(wrk_char,wrk_int)){goto	end_ok;}		//push the bottom
		goto end_error;
		}

if (uncode_com("AT+RDCOM=")) 				//Reading command from memory location in short format
		{									//for testing
		wrk_char=calk_parametr(ukaz);		
        if((!wrk_char)||(wrk_char>Max_memory_Cells))goto end_error;
		read_scanbuf(wrk_char);
		putst("Type command:");putch_long(command_data.type_command);putst(" ");
		putst("Time burn:");putch_long(command_data.time_burn);putst(" ");
		putst("body count:");putch_long(command_data.body_count);putst("\r");
		for(i=0;i<16;i++){putst("lb");putchdec(i);putst(":");putch_long(command_data.len_body[i]);putst(" ");}
		putst("\r");
		putst("BODIE:");
		for(i_int=0;i_int<Max_slot_Short;i_int++){putchhex(command_data.body[i_int]);putst(" "); }
		putst("\r");
		goto	end_ok;
		} 
if (uncode_com("AT+RDPRN=")) 				//Reading command from memory location in PRONTO format
		{
		wrk_char=calk_parametr(ukaz);		
        if((!wrk_char)||(wrk_char>Max_memory_Cells)) goto end_error;
		if(!read_myProntobuf(wrk_char))goto end_error;
// пересчет считаной частоты под real PRONTO
		burn_count	=pronto_data.time_burn;
        wrk_int=(pronto_data.num_slot+pronto_data.tail_slot)*2;		//

		for (i_int=0;i_int<wrk_int;i_int++)			// recalculation of durations for the required period
		{
		wrk_long=pronto_data.raw_scan[i_int];
		wrk_long*=66;
		wrk_long/=burn_count;
		pronto_data.raw_scan[i_int]=wrk_long;
		}

		putinthex(0x0000);putch(' ');                   //0 0-a sign that the code was scanned - always 0
		putinthex(pronto_data.time_burn);putch(' ');    // 103 dec - the scaning rate constant, in this case 41250 Hz 
														// formula Frequency = 1000000/(N * .241246)
		putinthex(pronto_data.num_slot);putch(' '); 					// number of pairs of a single package/2
		putinthex(pronto_data.tail_slot);putch(' '); 					// number of pairs of a repited package/2
		for (i_int=0;i_int<wrk_int;i_int++)
		{putinthex(pronto_data.raw_scan[i_int]);putst(" ");}
		putst("\r");
		goto	end_ok;
		} 

if (uncode_com("AT+WRPRN=")) 				//Writing command from memory location in PRONTO format
		{
		ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM-2);	//  search ','
		if(ukaz_tek==0xFFFF)goto end_error;
		wrk_char=calk_parametr(ukaz_tek);		// вычислить адрес
        if((!wrk_char)||(wrk_char>Max_memory_Cells))goto end_error;
		end_ukaz=find_N_char(buf_rx_command,0x0D,1,MAX_BUF_RX_COM-2);	//  search end of line
		ukaz_tek=find_N_char(buf_rx_command,' ',2,end_ukaz);	// search  second space (20h)	
		if(ukaz_tek==0xFFFF) goto end_error;
		pronto_data.time_burn=calk_HEX_parametr(ukaz_tek);		// time burn
		ukaz_tek=find_N_char(buf_rx_command,' ',3,end_ukaz);	// search  third space (20h) Data begining	
		if(ukaz_tek==0xFFFF) goto end_error;
		pronto_data.num_slot=calk_HEX_parametr(ukaz_tek);  	//$$ // time burn1
		ukaz_tek=find_N_char(buf_rx_command,' ',4,end_ukaz);	// search  fourth space (20h) Data begining	
		if(ukaz_tek==0xFFFF) goto end_error;
		pronto_data.tail_slot=calk_HEX_parametr(ukaz_tek);  	//$$ // time burn2
		
		wrk_int=(pronto_data.num_slot+pronto_data.tail_slot)*2;
		if(wrk_int>=Max_slot_pronto) goto end_error;

//		putch('\r');for (i_int=0;i_int<ukaz;i_int++){putch(buf_rx_command[i_int]);}putch('\r');

		CLRWDT();
		for(i_int=0;i_int<(wrk_int-1);i_int++)
		{
		ukaz_otnos=find_N_char(&buf_rx_command[ukaz_tek+1],' ',1,end_ukaz);	// find the space - beginning of the data	
		if(ukaz_otnos==0xFFFF) goto end_error;
		ukaz_tek+=(ukaz_otnos+1);
		pronto_data.raw_scan[i_int]=calk_HEX_parametr(ukaz_tek);	//Correction for correct counting
		CLRWDT();
		}
		pronto_data.raw_scan[i_int]=calk_HEX_parametr(end_ukaz);

//  recalculation of the read frequency for my PRONTO
		burn_count	=pronto_data.time_burn;
		for (i_int=0;i_int<wrk_int;i_int++)			// recalculation of durations for the required period
		{
		wrk_long=pronto_data.raw_scan[i_int];
		wrk_long*=burn_count;
		wrk_long/=66;
		pronto_data.raw_scan[i_int]=wrk_long+1;
		}


		if(!convert_pronto_short())goto end_error; ;	// converting the signal
		save_scanbuf(wrk_char);							// write to memory			


		goto	end_ok;
		} 



end_error:
putst("#ERROR\r");
CLRWDT();
return;
end_ok:
putst("#OK\r");
}


//------------------------------------------------
//waiting for a lexem within T seconds, is returned- pointer to the last letter (added zero)
char load_lexem(char time_wait)		
{
unsigned char temp;
unsigned int stop_time,count_ukaz;
count_ukaz=0;
stop_time=cnt_secund+time_wait;
				//
 do{
CLRWDT();
if(!SW_uart){if (end_bufrx==st_bufrx){continue;}}//have not command? - continue
	else {if (end_bufrx2==st_bufrx2){continue;}}
 temp=getch();
if (!temp) continue;
if (!count_ukaz) 
		{if ((temp=='\r')||(temp=='\n')||(!temp)) {continue;}
			else {buf_rx_command[0]=temp;count_ukaz++;}
		}
	else{
		buf_rx_command[count_ukaz]=temp;
		if((temp=='\r')||(temp=='\n')){buf_rx_command[count_ukaz]=0;break;}		// end recieving of lexem
		if(count_ukaz>=(MAX_BUF_RX_COM-2)){count_ukaz=MAX_BUF_RX_COM-1;buf_rx_command[count_ukaz]=0;continue;}
		++count_ukaz;
		}
  }while(cnt_secund<=stop_time);
if (cnt_secund<=stop_time) {return count_ukaz;}
return 0;		// если время вышло- возврват с ошибкой

}
//---------------------------------------
// wait for a lexem within T seconds, n lexems. Is returned a pointer to the last letter (added zero)
char read_lexem	(char n_lex,char time)			// ожидание лексемы в течении т секунд, n лексем возвращается указатель на последнюю букву(дописаный ноль)
{
char temp=0;
clear_buf_RX();
while (n_lex)
{
temp=load_lexem(time);
n_lex--;
if(!temp) break;	// if not read first, why to wait
}
return temp;
}
//-----------------------------------
// copy the lexem from buffer 2 to buffer 1
void copy_lexem(char *buf1,char *buf2)		
{
	while((*buf2)!=0)
	{
		*buf1=*buf2;
		buf1++;
		buf2++;
	}
		*buf1=0;				// add 0
}

//=================================================================================
//Comparing the string with command
unsigned char check_str(register const char *str,unsigned int ukaz)		
{
	while((*str)!=0)
	{
    if ((*str)!=buf_rx_command[ukaz]) {return 0;}	//the command did not match
		ukaz++;
		str++;
	}
return 1;		// total coincidence
}

//**********************************************************************
