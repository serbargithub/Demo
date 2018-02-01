#include <pic18.h>

//Name of ports

#define led_sost	 LA2			//> Led output


#define	det_USB		(PORTB&0x01)	//<B0 detection of USB controll
#define IR_IN		(PORTB&0x04) 	//<B2 input of IR reciver IOCB4 

#define prog_tx		 LB6			//B6>usart
#define prog_rx		(PORTB&0x80)	//B7<



#define led_IR		LC2				//C2> output of IR Led

#define dat_tel_tx	LC6				//C6>usart
#define dat_tel_rx	(PORTC&0x80)	//C7<



//Port discribing
#define  mask_a 	0b11111011
#define  mask_b 	0b10111111    
#define  mask_c    	0b10111011
#define  mask_e    	0b11111111




