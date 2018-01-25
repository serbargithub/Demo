
void serial_setup(unsigned char,unsigned char);
unsigned char check_err(void);
void clear_buf_RX(void);
unsigned char getch_n(unsigned char cnt_byte);
void putch(unsigned char c);
unsigned char getch(void);
void putst(register const char *str);
unsigned char put_command(register const char *str);
void putchhex(unsigned char c);
void putinthex(unsigned int c);
void putchdec(unsigned char c);
void putch_long(unsigned long c);
unsigned char check_OK(unsigned int time);
void ch_usart(char ch);
unsigned char check_buf(void);


#define sp2400	0
#define sp4800	1
#define sp9600	2
#define sp19200	3
#define sp38400	4
#define sp57600	5
#define sp115200 6

#define PRI_US	0		//Port 1 usart
#define SEC_US	1		//Port 2 usart



