#include "serial.h"
#include "interrupt.h"

int main()
{
    uart0_init();   // ������115200��8N1(8������λ����У��λ��1��ֹͣλ)
    while(1){}
    return 0;
}


