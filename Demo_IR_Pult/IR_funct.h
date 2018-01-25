unsigned char read_IRsignal(void);
unsigned char send_IR_signal(unsigned char adr,unsigned int time_loop);
void save_scanbuf (unsigned char num_data);
void read_scanbuf (unsigned char num_data);
unsigned char read_myProntobuf(unsigned char num_data);
unsigned char  convert_pronto_short(void);
unsigned char check_data(void);
