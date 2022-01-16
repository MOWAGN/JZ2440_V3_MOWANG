#include "s3c24xx.h"
#include "serial.h"

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
    GPHCON  |= 0xa0;    // GPH2,GPH3����TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3�ڲ�����

    ULCON0  = 0x03;     // 8N1(8������λ���޽��飬1��ֹͣλ)
    UCON0   = 0x05;     // ��ѯ��ʽ��UARTʱ��ԴΪPCLK
    UFCON0  = 0x00;     // ��ʹ��FIFO
    UMCON0  = 0x00;     // ��ʹ������
    UBRDIV0 = UART_BRD; // ������Ϊ115200
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
    while (!(UTRSTAT0 & RXD0READY));
    
    /* ֱ�Ӷ�ȡURXH0�Ĵ��������ɻ�ý��յ������� */
    return URXH0;
}

int getchar(void)
{
	while (!(UTRSTAT0 & (1<<0)));
	return URXH0;
}

/* 
 * ����һ���ַ���
 * ��������������ַ������ַ
 * ����ֵ����Ӧ�ַ������ַ
 */
char *gets(char s[])
{ 
	int i = 0;
	char c;
	
	while(1)
	{
		c = getchar();
		/* ���� */
		putchar(c);
		if (c == '\n')
			putchar('\r');
		else if (c == '\r')
			putchar('\n');		
		if((c == '\n') || (c == '\r'))
		{
			s[i] = '\0';
			break;
		}
		else
		{
			s[i++] = c;
		}
	}	
	return s;
}



/*
 * �����ַ�����������ֱ�ӷ��أ�����ȴ��涨��ʱ��
 * ���������
 *     timeout: �ȴ������ѭ��������0��ʾ���ȴ�
 * ����ֵ: 
 *    0     : �����ݣ���ʱ�˳�
 *    ����ֵ�����ڽ��յ�������
 */
unsigned char awaitkey(unsigned long timeout)
{
	while (!(UTRSTAT0 & RXD0READY))
    {
        if (timeout > 0)
            timeout--;
        else
            return 0;   // ��ʱ������0
	}

    return URXH0;       // ���ؽ��յ��Ĵ�������
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
