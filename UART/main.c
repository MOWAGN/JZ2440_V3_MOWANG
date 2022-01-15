#include "serial.h"
#include "interrupt.h"

int main()
{
    uart0_init();   // 波特率115200，8N1(8个数据位，无校验位，1个停止位)
    while(1){}
    return 0;
}


