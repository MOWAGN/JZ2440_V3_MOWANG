#include <stdio.h>
#include "serial.h"
#include "adc_ts.h"

int home_sign = 0;      //�����ж�home���Ƿ��£��򿪻�رղ˵�
                        //0��ʾ�����˵�Ϊչ����1��ʾ�����˵��Ѿ�չ��
int pic_index = 1;      //ͼ�⹦��ͼƬ����ֵ
int pic_sign = 1;       //ͼ�⹦�ܿ����ж�
int note_sign=1;        //��ǩ���ܿ���
int note_write=1;       //��ǩ���¿��أ�0��ʾ���±༭��ǩ
int play_index = 0;     //���ֹ��ܼ�ֵ
int play_sign = 1;      //���ֿ����ж�
char Note[40]={"Hello,world!"};       //��ǩ���ݴ洢�ռ�

int main()
{
    char c;

    uart0_init();   // ������115200��8N1(8������λ����У��λ��1��ֹͣλ)
    
    while (1)
    {
        printf("\r\n##### Test ADC and Touch Screem #####\r\n");
        Test_Lcd_Tft_16Bit_480272();
        Test_Ts();
    }   
    return 0;
}

