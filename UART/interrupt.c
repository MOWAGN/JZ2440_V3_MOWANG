#include "s3c24xx.h"
#include "serial.h"

void UartRx_Handle()
{
       char ch;
      
       SUBSRCPND |= 0x3;	//清除rx，tx中断请求
       SRCPND |= 0x1<<28;	//清除串口源挂起,SRCPND第28位为INT_UART0
       //如果 SRCPND 寄存器的指定位为1，其通常被认作一个有效中断请求正在等待服务
       INTPND |= 0x1<<28;	//清除串口子挂起	
       //SUBSRCPND和SRCPND、INTPND想清除某位，均需往此位写1
      
       if(UTRSTAT0 & 1)             //接收缓冲区有数据
       {
            //ch = URXH0;                     //接收字节数据，接受处理
            ch = getc();
		    //从串口接收数据后，判断其是否数字或子母，若是则加1后输出
            if (isDigit(ch) || isLetter(ch))
              putc(ch+1);
       }  
}
