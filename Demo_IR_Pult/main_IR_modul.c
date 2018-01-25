#include <pic18.h>
//#include "ctype.h"
#include "name_port.h"
#include "my_delay.h"
#include "usart.h" 
#include "flash_const.h"
#include "name_konst.h"
#include "eeprom.h"
#include "terminal.h"
#include "EEprom_in_PM.h"


//18F26K22
__CONFIG (1, PLLCFG_ON & IESO_OFF & FCMEN_OFF & PRICLKEN_ON & FOSC_INTIO67);   
__CONFIG (2, PWRTEN_ON & WDTEN_ON &WDTPS_64& BOREN_ON & BORV_250);
__CONFIG (3, MCLRE_INTMCLR & PBADEN_OFF);
__CONFIG (4, DEBUG_OFF & STVREN_ON & LVP_OFF & XINST_OFF);
__CONFIG (5, CP0_ON & CP1_ON & CP2_ON & CP3_ON & CPB_ON & CPD_ON);
//__CONFIG (6, 0xFFFF);
//__CONFIG (7, 0xFFFF);


#define MAX_BUF		128    //MAX of round roll buffer

//extern variables
unsigned char cnt_clk,synhr,cnt_wt,SW_uart,fl_cary,extra_count,stop_scan,start_scan,fl_send_read;
unsigned char cnt_secund1,led_inform;
unsigned int  cnt_secund, dlin_pause,dlin_sig,cnt_millisecund,time_send;
unsigned char fl_start_send,fl_start_wait,fl_find_end,fl_read_error,fl_overfull;
unsigned char buffer_rx[MAX_BUF];  //round roll buffer
unsigned char buffer_rx2[MAX_BUF];  //round roll buffer

unsigned char clear_bufrx,ask_geth_byte,st_bufrx,end_bufrx,geth_byte;
unsigned char clear_bufrx2,ask_geth_byte2,st_bufrx2,end_bufrx2,geth_byte2;
//-----------------------
struct com_map
{							
unsigned char type_command;		// 0- usual repeating command, 1- non-repeating sequence of 1 packet, 2 non-repeating sequence of two packets
unsigned char time_burn;        // modulation frequency of the output signal and time slot for the input signal: Calculated by the formula Frequency = 1000000 / (N * .241246)
unsigned int  body_count;		// counter of fronts and falls of the body
unsigned int  tail_count;		// counter of fronts and falls of the tail
unsigned int  len_body[16];		// The length of the gap between fronts, is encoded by references in the database with two bits
unsigned char body[Max_slot_Short];	// The body of the command, is coded by halves of bytes with references to the table of durations
};
struct com_map command_data;

struct pronto_struct
{
unsigned char time_burn;
unsigned int num_slot;
unsigned int tail_slot;
unsigned int each_slot[4];
unsigned int raw_scan[Max_slot_pronto];
};
struct  pronto_struct pronto_data;

//=============================================================
static void interrupt
isr(void)			
{
static unsigned char work_int,work_int2,count_per,svetik_synhr,num_data,cnt_ms;	
static unsigned int  count_data,tek_slot;  
static unsigned long work_long;
if(TMR2IF)
 {
	TMR2IF =0;
	cnt_clk--;
	synhr++;
	if(!(cnt_wt--)) {fl_cary=1;} 	//internal counter

	if(!synhr)		
		{
			if(!(--cnt_secund1))
				{cnt_secund1=31;++cnt_secund;}				//seconds counter
				svetik_synhr++;
		}
	cnt_ms++;
	if(cnt_ms>8) {cnt_ms=0;cnt_millisecund++;}


//--UART1
	if(clear_bufrx){end_bufrx=0;st_bufrx=0;clear_bufrx=0;}		//buffer clearing
	if (RC1IF){													//if a byte is recived
				work_int=RC1REG;
				if((++end_bufrx)>=MAX_BUF) {end_bufrx=0;}		//buffer full - round roll buffer
				buffer_rx[end_bufrx]=work_int;
			}
							
	if (ask_geth_byte) 
				{
				 if (end_bufrx!=st_bufrx)
					{
					if((++st_bufrx)>=MAX_BUF) st_bufrx=0;
					geth_byte=buffer_rx[st_bufrx];
					ask_geth_byte=0;
					}
					else  geth_byte=0;
				}

//--UART2
	if(clear_bufrx2){end_bufrx2=0;st_bufrx2=0;clear_bufrx2=0;}		//buffer clearing
	if (RC2IF){														//if a byte is recived
 				work_int2=RCREG2;
				if((++end_bufrx2)>=MAX_BUF) {end_bufrx2=0;}			//buffer full - round roll buffer
				buffer_rx2[end_bufrx2]=work_int2;
			  }	
	if (ask_geth_byte2) 
				{
				 if (end_bufrx2!=st_bufrx2)
					{++st_bufrx2;
					if(st_bufrx2>=MAX_BUF) {st_bufrx2=0;}
					geth_byte2=buffer_rx2[st_bufrx2];
					ask_geth_byte2=0;
					}
					else  geth_byte2=0;
				}
//-------------------------
//-Led blynker				
			led_sost=0;
		switch(led_inform)	
				{

					case 0:
						led_sost=0;									//swich off
					break;
					case 2:
						if(svetik_synhr&0x10) {led_sost=1;}			// fast
					break;
					case 3:
						 led_sost=1;								//swich on
					break;
					case 4:
						if((svetik_synhr&0xA0)==0xA0) {led_sost=1;}	// slow
						break;
					case 5:
						if((svetik_synhr&0xA4)==0xA4) {led_sost=1;}	// slow/fast
					break;
					case 7:
							if(svetik_synhr&0x04) {led_sost=1;}		// slow/fast-fast
					break;
					case 8:
							if(svetik_synhr&0x02) {led_sost=1;}		//IR transceiver
					break;
					default:
							led_sost=0; 
						break;
				}
}
//----------------------------------------------
if(TMR1IF)
	{
	 TMR1IF=0;
	if(!fl_send_read)
		{
		 extra_count++;PSTRCON=0b00000000;		//add one more counter
			if ((!fl_find_end)&&extra_count>1){fl_find_end=0;fl_overfull=1;INT2IF=1;}  //if we find a long pause and there is a scan of the signal then simulate the interruption and stop reception
		}		
		else 
			{								// отправка пакета через IR
				if(extra_count){extra_count--;}   // если задержка длинная 
					else
						{
					 		work_long=time_send;
							work_long<<=3;
							extra_count=*(((unsigned char*)&work_long)+2);
							TMR1=0-(unsigned int)(work_long&0x0000FFFF);
							if(fl_start_send){fl_start_send=0;STR1A=1;}	// 
							if(fl_start_wait){fl_start_wait=0;STR1A=0;}	// 
						}
			}
	}

if(INT2IF)       //scanner duration. the reference freq is 62500 Hz
	{
		work_long=TMR1;
		*(((unsigned char*)&work_long)+2)=extra_count;
		work_long>>=3;								// get the length value
		if(INTEDG2){INTEDG2=0;dlin_sig=work_long;}	// new value
			else {INTEDG2=1;dlin_pause=work_long;}
		if(!fl_send_read){TMR1=0;extra_count=0;}
		INT2IF=0;

		if(start_scan&&(dlin_sig>15)) {start_scan=0;stop_scan=0;pronto_data.num_slot=0;tek_slot=0;num_data=0;fl_find_end=0;fl_overfull=0; 		//  start command scanning to memory if the signal is more than 0.4 ms
									  pronto_data.each_slot[0]=pronto_data.each_slot[1]=pronto_data.each_slot[2]=pronto_data.each_slot[3]=0;
									  }							  

		if((!stop_scan)&&(!start_scan))							    //to record command's body
			{
				count_data=pronto_data.num_slot;
			 	if((num_data<4)&&(count_data<(Max_slot_pronto-1))){pronto_data.raw_scan[count_data]=work_long;pronto_data.num_slot++;}   // save only the first 4 packages
				tek_slot++;
				if(dlin_pause>Time_check)									// if the pause is more than 15 ms then stop scanning	
						{
							pronto_data.each_slot[num_data]=tek_slot;
							tek_slot=0;
							if((num_data>=3)||fl_find_end||fl_overfull){stop_scan=1;}
							 else 
								{
									num_data++;						//switch to second base for tail recording
									stop_scan=0;
								}
						}
			}	
		dlin_sig=dlin_pause=0;
	}

//---------------------------------------------
	asm("nop");				  			
}// конец прерывания



//==========================
//СТАРТ
void
main(void)
{
unsigned char save_det,i;
//--INIT
	INTCON=0b01000000;
	OSCCON= 0b11111100;    //16*1=16М Mhz freq
	OSCCON2=0b10000000;
    OSCTUNE=0b00000000;		// PLL disable
	PORTA=0;
	LATA=0;
	PORTB=0;
	LATB=0;
	PORTC=0;
	LATC=0;
	TMR0=0;
	TXREG=0;

//18F2520
	ADCON0=0b00000001;		//Seting ADC
	ADCON1=0b00000000;		//
	ADCON2=0b01000010;		//t/32 left alignment

	ANSELA=0b00000000;
	ANSELB=0b00000000;
	ANSELC=0;
	PR2=127;		 //+1   31.25 khz + postdevider into  4 = 8 khz
	PIE1=0b00000010; //allowing intterupt from TMR2
	TRISA = mask_a;
	TRISB = mask_b;	
	TRISC = mask_c;
	TRISE = mask_e;

	//swich on interrupt from RB2 
	INTEDG2=0;	// interrupt to falling edge
	INT2IE=1;

	T1CON=0b00110001;
	TMR1IF=0;
	TMR1=0;
	TMR1IE=1;		//start on Timer11 

	serial_setup(sp9600,PRI_US);		//Init usart
	serial_setup(sp9600,SEC_US);
	
	CCP1CON=0b00001100;	
	CCPR1L=0x40;	
	PSTRCON=0b00000000;	// off PWM
	T2CON=0b00011000;	//post devider/4
	TMR2IF=0;
	TMR2ON=1;
	GIE=1	;			//start interrupt
//---------------------------------
//reset:
   init_PM();
	led_inform=3;
	delay_s(2); 
	led_inform=0;
start:
for(;;)
 {
//=========RESET==============
	led_inform=0;
	fl_send_read=0;
	delay_ms(500);
	save_det=det_USB;
	if(det_USB) terminal_start(PRI_US);				// Connect primery usart to programmer
		else
			    terminal_start(SEC_US);				// Connect secondary usart to programmer
	while(1)
	{
		terminal_task();
		if (save_det!=det_USB) break;               // If connectoin was changed then swich UART
		CLRWDT();
	}
 }


}		//end of programm
//**********************************************************************
