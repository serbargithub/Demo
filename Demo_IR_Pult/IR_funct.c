#include <pic18.h>  
#include "my_delay.h"
#include "usart.h"
#include "name_konst.h"
#include "eeprom.h"
#include "EEprom_in_PM.h"

extern unsigned char led_inform,stop_scan,start_scan,fl_send_read,fl_start_send,fl_start_wait,fl_find_end,fl_overfull,extra_count,global_switch_protect;
extern unsigned int cnt_secund,dlin_pause,dlin_sig,cnt_millisecund,time_send;
extern unsigned char Buffer_PM[];
//----------------------------------------------
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

unsigned char check_lenght (unsigned int *mass_len_body,unsigned int lenght_raw);
unsigned char read_myProntobuf(unsigned char num_data);
unsigned char convert_mypronto_short(void);
unsigned char convert_pronto_short(void);
unsigned char check_data(void);
//----------------------------------------------
//Reading IR command
unsigned char read_IRsignal(void)
 {
	unsigned char burn_count;

	led_inform=3;
	burn_count=EE_READ(freq_mod);
	if((burn_count<55)|| (burn_count>155)) {burn_count=103;}
	pronto_data.time_burn=burn_count;

	fl_send_read=0;				//Switch Timer1 to receive packet mode 
	INTEDG2=0;					// interrupt to falling edge
	fl_find_end=1;
	fl_overfull=0;
	dlin_sig=dlin_pause=0;
	stop_scan=0;
	start_scan=1;					// Start reading command
	cnt_secund=0;
	while(cnt_secund<30) {CLRWDT();if(stop_scan) break;}			//wait for start 30 sec
	if((cnt_secund>=30)) {led_inform=8;delay_ms(700);led_inform=0;return 0;}   //if time out or we've got a byte from UART - exit 
	pronto_data.num_slot/=2;		//correction of the received amount to pairs (i.e. divide by 2)
//Analise and packing received command
	if(!convert_mypronto_short()){led_inform=8;delay_ms(700);led_inform=0; return 0;}   // if has got error during analise - return with error marker
	led_inform=7;
	delay_ms(200);
	led_inform=0;
return 1;
}

//--------------------------------------------
unsigned char check_lenght (unsigned int *mass_len_body,unsigned int lenght_raw)
{
unsigned int work; 
unsigned char i;
    lenght_raw*=2;
for (i=0;i<16;i++)
	{
	work=*(mass_len_body+i);
	if(!work) {*(mass_len_body+i)=lenght_raw; return i;};			
	if((work>lenght_raw)&&(work-lenght_raw)>Time_stretch) continue;
	if((work<lenght_raw)&&(lenght_raw-work)>Time_stretch) continue;	// If the length differs by more than 2 points, then go to the next gap
	work+=(lenght_raw+1);
	work/=2;											//calculation of the arithmetic mean
	*(mass_len_body+i)=work;							//save new value
	return i;
	}
return 0xFF;   //if the overflow then return with  error
}
//-------------------------------------------------
//Sending command thought IR out
unsigned char send_IR_signal(unsigned char adr,unsigned int time_loop)
{
unsigned char timeburn,type_command;
unsigned int first_stop,second_stop,k_int,work_uint;

	if(!read_myProntobuf(adr))return 0;

	if (!check_data())return 0;
	type_command=command_data.type_command;
	second_stop=(pronto_data.num_slot+pronto_data.tail_slot)*2;			//$$
	first_stop=pronto_data.num_slot*2;
	CLRWDT();
	fl_start_send=fl_start_wait=0;
	time_send=0;
	extra_count=0;
	PSTRCON=0b00000000;

	work_uint=pronto_data.time_burn;
	if((work_uint<55)|| (work_uint>155)) {work_uint=103;}
//  Frequency = 1000000/(N * .241246)
	work_uint*=96;
	work_uint/=100;	//уmultiply by 0.96
	timeburn=(unsigned char) work_uint;
	PR2=timeburn+1;		 //+1   31.25 kHz +post divide by 4 = 8 kHz
	CCPR1L=timeburn/2;

	led_inform=8;		//transmit
	fl_send_read=1;		//switch to transmit
	cnt_millisecund=0;

if(pronto_data.num_slot)							//if transmit only one command
	{
		for (k_int=0;k_int<first_stop;k_int++)      //body of command
			{
			while(fl_start_send||fl_start_wait){}    // ожидание передачи параметров в прерывании
			time_send=pronto_data.raw_scan[k_int];
			if(k_int&0x0001) {fl_start_wait=1;}
				else {fl_start_send=1;}
   			CLRWDT();
			}
	}

if(pronto_data.tail_slot)								   //if transmit only one command
  {
	while (cnt_millisecund<time_loop)
	{
	for (k_int=first_stop;k_int<second_stop;k_int++)      //tail of command
		{
		while(fl_start_send||fl_start_wait){}    // waiting for parameter passing in violation
		time_send=pronto_data.raw_scan[k_int];
		work_uint=k_int;
		work_uint&=0x0001;
		if(work_uint) {fl_start_wait=1;}
			else  {fl_start_send=1;}
		}
     CLRWDT();
	}
 } 
end_send:
	while(fl_start_send||fl_start_wait){}    // waiting for parameter transmit in interrupt
	time_send=0xF000;
	PSTRCON=0b00000000;
	PR2=127;		 //+1   31.25 кгц +postdivide by  4 = 8 кгц
	CCPR1L=0x40;
	fl_send_read=0;		//switch on reicive
	led_inform=0;
return 1;
}
//------------------------------
//Save command which was scanned before 
void save_scanbuf (unsigned char num_data)//++
{
unsigned char  *ukaz_buf;
unsigned int i_cnt,k_cnt;
	ukaz_buf=&command_data.type_command;
	for (i_cnt=0;i_cnt<64;i_cnt++){Buffer_PM[i_cnt]=*ukaz_buf;ukaz_buf++;}	//copy the first part of scan
	global_switch_protect=0xAA;
	Frame_Buf_Write_PM(num_data*6);
	global_switch_protect=0X55;
	for(k_cnt=0;k_cnt<5;k_cnt++)
		{
		ukaz_buf=&command_data.body[k_cnt*64];
		for (i_cnt=0;i_cnt<64;i_cnt++){Buffer_PM[i_cnt]=*ukaz_buf;ukaz_buf++;}	//copy 5 parts of scan
		global_switch_protect=0xAA;
		Frame_Buf_Write_PM(num_data*6+k_cnt+1);
		global_switch_protect=0X55;
		}
}
//-------------------------------
//read command from memory to scanbuf
void read_scanbuf (unsigned char num_data)//++
{
	unsigned char  *ukaz_buf,i_cnt,k_cnt;
	ukaz_buf=&command_data.type_command;
	Frame_Buf_Read_PM(num_data*6);
	for (i_cnt=0;i_cnt<64;i_cnt++){*ukaz_buf=Buffer_PM[i_cnt];ukaz_buf++;}	//copy 1 part of scan
	for(k_cnt=0;k_cnt<5;k_cnt++)
		{
		ukaz_buf=&command_data.body[k_cnt*64];
		Frame_Buf_Read_PM(num_data*6+k_cnt+1);
		for (i_cnt=0;i_cnt<64;i_cnt++){*ukaz_buf=Buffer_PM[i_cnt];ukaz_buf++;}	//copy 2 part of scan
		}
}
//-------------------------------
// read command in PRONTO format
unsigned char read_myProntobuf(unsigned char num_data)//+
{
	unsigned char work_byte,i;
	unsigned int end_body,end_tail,i_int;
	read_scanbuf(num_data);				// прочитать в сокращенном формате

	for (i_int=0;i_int<Max_slot_pronto;i_int++){pronto_data.raw_scan[i_int]=0;}			// clear old data

	pronto_data.time_burn=command_data.time_burn;
	pronto_data.num_slot=(command_data.body_count)/2;	
	pronto_data.tail_slot=(command_data.tail_count)/2;	

//decoding command's body

	end_body=command_data.body_count+command_data.tail_count;				//get a pointer to the intersignal interval
	if(end_body>Max_slot_pronto) {pronto_data.num_slot=pronto_data.tail_slot=0;return 0;} //control of parameters

	i=0;
	for (i_int=0;i_int<end_body;i_int++)      //unpacking the body of the command
	{
    	work_byte=command_data.body[i_int/2];
		work_byte>>=(i*4);  
		work_byte&=0x0F;
		pronto_data.raw_scan[i_int]=(command_data.len_body[work_byte]+1)/2;		// add the calculated duration to the body		  
    	i++;if(i>1){i=0;}    // внутрибайтовый счетчик               
 	}
return 1;
}
//------------------------------
// Convert PRONTO to short format
unsigned char convert_mypronto_short(void)						
{
	unsigned int i_int,count_data,first_stop,second_stop;
	unsigned char i,marker_lenght,type_command;

	count_data=pronto_data.num_slot*2;  //$$
	second_stop=count_data-1;
	type_command=0;
//definition of the command type 0 - single command from 1 packet; 1 repeating sequence of 1 large packet; 2-repeated command from the single command and tail
//3- type DENON tail package first after him body then tail
//4- type DAIKIN 4 commands 12 measurements at first
	while(1)
		{
		if((pronto_data.each_slot[0]==12)&&pronto_data.each_slot[3]){type_command=4;break;}  //if the first time slot is 12 and all 4 are accepted then this is Daikin
		if(pronto_data.each_slot[3]==0){type_command=0;break;}
		if(pronto_data.each_slot[0]==pronto_data.each_slot[1])			// if 1 and the second packages are the same
			{
				type_command=1;break;
			}
				else	// if 1 and the second packages aren't the same
				{
					if(pronto_data.each_slot[1]==pronto_data.each_slot[2]) {type_command=2;break;}      //2-repeated command from the single command and tail
					if((pronto_data.each_slot[0]==pronto_data.each_slot[2])&&(pronto_data.each_slot[1]==pronto_data.each_slot[3])) {type_command=1;break;}  
				}
		break;
		}
//-----saving the command
	command_data.type_command=type_command;
	switch (type_command)
	{
		case 0: command_data.body_count=count_data; command_data.tail_count=0;break;
		case 1: command_data.body_count=0; command_data.tail_count=count_data;break;
		case 2: command_data.body_count=pronto_data.each_slot[0]; command_data.tail_count=pronto_data.each_slot[1];break;
		case 4: command_data.body_count=count_data; command_data.tail_count=0;break;
	default:
		break;
	}


	command_data.time_burn=pronto_data.time_burn;
	for (i=0;i<16;i++){command_data.len_body[i]=0;}   //clear length cells

	for(i_int=0;i_int<Max_slot_Short;i_int++){command_data.body[i_int]=0;} //clear command's body
	for (i_int=0;i_int<count_data;i_int+=2)      //packing command's body
	{
	for(i=0;i<2;i++)
		{
		if((i_int+i)>=count_data) break;
		marker_lenght=check_lenght(&command_data.len_body[0],pronto_data.raw_scan[i_int+i]);      //get a marker for the length of the gap 
		if(marker_lenght==0xFF) return 0;				//if the scan with error-return  with  error
		marker_lenght<<=(i*4);   					    // set the bits to the required position
		command_data.body[i_int/2]|=marker_lenght;      // add the calculated duration to the body		  
		}
	}
	return 1;
}
//----------------------
// Convert to short pronto
unsigned char convert_pronto_short(void)				
 {
	unsigned int i_int,count_data,first_stop,second_stop;
	unsigned char i,marker_lenght,type_command;

	count_data=(pronto_data.num_slot+pronto_data.tail_slot)*2;  //$$
	if(!count_data) return 0;  // if there en error - return
	second_stop=count_data-1;
	type_command=0;
// definition of the command type 0 - single command from 1 packet; 1 repeating sequence of 1 large packet; 2-repeated command from the single command and tail
//3- type DENON tail package first after him body then tail
	while(1)
	{
		if(pronto_data.tail_slot==0){type_command=0;break;}
		if(pronto_data.num_slot== 0){type_command=1;break;}
		type_command=2;
		break;
	}
//-----save the command
	command_data.type_command=type_command;
	command_data.body_count=pronto_data.num_slot*2;
	command_data.tail_count=pronto_data.tail_slot*2;

	command_data.time_burn=pronto_data.time_burn;
	for (i=0;i<16;i++){command_data.len_body[i]=0;}   //clear  length cells

	for(i_int=0;i_int<Max_slot_Short;i_int++){command_data.body[i_int]=0;} //clear command's body
	for (i_int=0;i_int<count_data;i_int+=2)      //packing command's body
		{
			for(i=0;i<2;i++)
			{
			if((i_int+i)>=count_data) break;
			marker_lenght=check_lenght(&command_data.len_body[0],pronto_data.raw_scan[i_int+i]);      //get a marker for the length of the gap 
			if(marker_lenght==0xFF) return 0;				// if the scan with error-return  with  error
			marker_lenght<<=(i*4);   						// set the bits to the required position
			command_data.body[i_int/2]|=marker_lenght;     	// add the calculated duration to the body		  
			}
	}
	return 1;
 }

//-----------------------------------------------------
// Error's checking
unsigned char check_data(void)		
{
	unsigned int work_int,i_int,work_int2;
	work_int=(pronto_data.num_slot+pronto_data.tail_slot)*2;
	if((!work_int)||(work_int>Max_slot_pronto)) return 0; 
	for (i_int=0;i_int<work_int;i_int++)
		{
		work_int2=pronto_data.raw_scan[i_int];
		if((!work_int2)||(work_int2>Time_check2)){ return 0;}
		}
	return 1;	
 }