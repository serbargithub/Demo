//Версия программы
#define	Ver_Signal "Termostat\r"   //Name of device
#define	Ver_Build "Ver 1.35_daikin" 
#define	time_build  __TIME__    // Time compilled
#define	date_build  __DATE__	// Date compilled



//#define main_UART  0
//#define sec_UART   1

#define	Time_check    938     //469 for 31250 Hz and 938 for 62500 Hz time more than the signal can't be,is used to find pauses between sending = 15ms
#define	Time_check2   16384   // stop time for non-repiting packages 256 мс
#define	Time_stretch  4       // error range for averaging of received durations

#define	Max_slot_pronto  640       				// The lengh for PRONTO
#define	Max_slot_Short   Max_slot_pronto/2      // The lengh for short format
#define	Max_memory_Cells 100       				//The maximum value of memory cells is now defined as 6 packets of 64 per command no more than 256 packets,
												// to increase the cells, you must change the dimension MAX_NUM_PMBUF

// ячейки флеш памяти

#define	empty			0x00	// on-off computer
#define	freq_mod		0x01	// modulation freq 
#define	freq_mod1		0x02	//

#define	data_command    0x10    //start of the command database
	


