void uart0_init(void);
void putc(unsigned char c);
unsigned char getc(void);
unsigned char awaitkey(unsigned long timeout);
int isDigit(unsigned char c);
int isLetter(unsigned char c);
/* 
 * 接受一个字符串
 * 输入参数：接收字符数组地址
 * 返回值：对应字符数组地址
 */
char *gets(char s[]);
