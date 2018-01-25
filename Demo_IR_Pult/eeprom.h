unsigned char EE_READ (unsigned int addr);
void EE_WRITE (unsigned int addr,unsigned char var);
unsigned int EE_READ_Int (unsigned int addr);
void EE_WRITE_Int (unsigned int addr,unsigned int var);
void EE_WRITE_Long (unsigned int addr,unsigned long var);
unsigned long EE_READ_Long (unsigned int addr);
unsigned char read_eep_frame(unsigned int set_adr_eep,unsigned int cnt_byte_eep);
unsigned char  write_eep_frame(unsigned int set_adr_eep,unsigned int cnt_byte_eep);


