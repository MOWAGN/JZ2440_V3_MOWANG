#include "s3c24xx.h"
#include "serial.h"
#include "interrupt.h"

#define TXD0READY   (1<<2)
#define RXD0READY   (1)

#define PCLK            50000000    // init.c中的clock_init函数设置PCLK为50MHz
#define UART_CLK        PCLK        //  UART0的时钟源设为PCLK
#define UART_BAUD_RATE  115200      // 波特率
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)


/*
 * 初始化UART0
 * 115200,8N1,无流控
 */
void uart0_init(void)
{
    GPHCON  |= 0xa0;    // GPH2,GPH3用作TXD0,RXD0     1010,0000
    GPHUP   = 0x0c;     // GPH2,GPH3内部上拉          1100

    ULCON0  = 0x03;     // 8N1(8个数据位，无较验，1个停止位)  11
    UCON0   = 0x05;     // 中断方式，UART时钟源为PCLK       0101
    UFCON0  = 0x00;     // 不使用FIFO
    UMCON0  = 0x00;     // 不使用流控
    UBRDIV0 = UART_BRD; // 波特率为115200

    SRCPND |= 0x1<<28;		//清除串口中断挂起
    SUBSRCPND |= 0x3;		//清除收发中断
	INTPND |= 0x1<<28;		//清除串口中断请求
    INTSUBMSK &= ~(0x1);             //打开UART0接收中断，使能中断
    INTSUBMSK |= (0x1<<1);          //关闭UART0发送中断，禁止中断
    INTMSK &= ~(0x1<<28);            //打开UART0中断屏蔽，总中断
           
}

/*
 * 发送一个字符
 */
void putc(unsigned char c)
{
    /* 等待，直到发送缓冲区中的数据已经全部发送出去 */
    while (!(UTRSTAT0 & TXD0READY));
    
    /* 向UTXH0寄存器中写入数据，UART即自动将它发送出去 */
    UTXH0 = c;
}

/*
 * 接收字符
 */
unsigned char getc(void)
{
    /* 等待，直到接收缓冲区中的有数据 */
    while (!(UTRSTAT0 & RXD0READY));   //bit=0,无数据，值为真，循环等待
    
    /* 直接读取URXH0寄存器，即可获得接收到的数据 */
    return URXH0;
}

/*
 * 判断一个字符是否数字
 */
int isDigit(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return 1;
    else
        return 0;       
}

/*
 * 判断一个字符是否英文字母
 */
int isLetter(unsigned char c)
{
    if (c >= 'a' && c <= 'z')
        return 1;
    else if (c >= 'A' && c <= 'Z')
        return 1;       
    else
        return 0;
}
