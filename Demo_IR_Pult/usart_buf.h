
void putst_buf(unsigned char *str);
void wr_strbuf(register const unsigned char *str, unsigned char point_n);
void wr_ptbuf(unsigned char c, unsigned char point_n);
void wr_ptbuf_dec(unsigned char c, unsigned char point_n);
unsigned int find_N_char(unsigned char *massive,unsigned char simvol,unsigned char n_cnt,unsigned int MAX_buf);
unsigned char compare_buf(unsigned char *bufer1,unsigned char *bufer2,unsigned char end_symb,unsigned char max_buf2);
unsigned char copy_buf(unsigned char *bufer1,unsigned char *bufer2,unsigned char end_symb,unsigned char max_buf2);
unsigned long calk_parametr_buf(unsigned char  *bufer,unsigned char end_symb);

extern unsigned char STR_Buf[];
