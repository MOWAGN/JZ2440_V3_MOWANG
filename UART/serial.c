#include "s3c24xx.h"
#include "serial.h"
#include "interrupt.h"

#define TXD0READY   (1<<2)
#define RXD0READY   (1)

#define PCLK            50000000    // init.c�е�clock_init��������PCLKΪ50MHz
#define UART_CLK        PCLK        //  UART0��ʱ��Դ��ΪPCLK
#define UART_BAUD_RATE  115200      // ������
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)


/*
 * ��ʼ��UART0
 * 115200,8N1,������
 */
void uart0_init(void)
{
    GPHCON  |= 0xa0;    // GPH2,GPH3����TXD0,RXD0     1010,0000
    GPHUP   = 0x0c;     // GPH2,GPH3�ڲ�����          1100

    ULCON0  = 0x03;     // 8N1(8������λ���޽��飬1��ֹͣλ)  11
    UCON0   = 0x05;     // �жϷ�ʽ��UARTʱ��ԴΪPCLK       0101
    UFCON0  = 0x00;     // ��ʹ��FIFO
    UMCON0  = 0x00;     // ��ʹ������
    UBRDIV0 = UART_BRD; // ������Ϊ115200

    SRCPND |= 0x1<<28;		//��������жϹ���
    SUBSRCPND |= 0x3;		//����շ��ж�
	INTPND |= 0x1<<28;		//��������ж�����
    INTSUBMSK &= ~(0x1);             //��UART0�����жϣ�ʹ���ж�
    INTSUBMSK |= (0x1<<1);          //�ر�UART0�����жϣ���ֹ�ж�
    INTMSK &= ~(0x1<<28);            //��UART0�ж����Σ����ж�
           
}

/*
 * ����һ���ַ�
 */
void putc(unsigned char c)
{
    /* �ȴ���ֱ�����ͻ������е������Ѿ�ȫ�����ͳ�ȥ */
    while (!(UTRSTAT0 & TXD0READY));
    
    /* ��UTXH0�Ĵ�����д�����ݣ�UART���Զ��������ͳ�ȥ */
    UTXH0 = c;
}

/*
 * �����ַ�
 */
unsigned char getc(void)
{
    /* �ȴ���ֱ�����ջ������е������� */
    while (!(UTRSTAT0 & RXD0READY));   //bit=0,�����ݣ�ֵΪ�棬ѭ���ȴ�
    
    /* ֱ�Ӷ�ȡURXH0�Ĵ��������ɻ�ý��յ������� */
    return URXH0;
}

/*
 * �ж�һ���ַ��Ƿ�����
 */
int isDigit(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return 1;
    else
        return 0;       
}

/*
 * �ж�һ���ַ��Ƿ�Ӣ����ĸ
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