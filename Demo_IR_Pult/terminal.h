void terminal_start (unsigned char napravl);
void terminal_task(void);
unsigned char load_lexem(unsigned char time_wait);
unsigned char read_lexem	(unsigned char n_lex,unsigned char time);
unsigned char uncode_com(register const unsigned char *str);
void copy_lexem(unsigned char *buf1,unsigned char *buf2);
unsigned long calk_parametr(unsigned char ukaz);
void print_parametr(unsigned long Adres);
void read_parametr(unsigned long Adres);


