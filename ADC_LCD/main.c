#include <stdio.h>
#include "serial.h"
#include "adc_ts.h"

int home_sign = 0;      //用于判断home键是否按下，打开或关闭菜单
                        //0表示辅助菜单为展开，1表示辅助菜单已经展开
int pic_index = 1;      //图库功能图片索引值
int pic_sign = 1;       //图库功能开关判断
int note_sign=1;        //便签功能开关
int note_write=1;       //便签更新开关，0表示重新编辑便签
int play_index = 0;     //娱乐功能键值
int play_sign = 1;      //娱乐开关判断
char Note[40]={"Hello,world!"};       //便签数据存储空间

int main()
{
    char c;

    uart0_init();   // 波特率115200，8N1(8个数据位，无校验位，1个停止位)
    
    while (1)
    {
        printf("\r\n##### Test ADC and Touch Screem #####\r\n");
        Test_Lcd_Tft_16Bit_480272();
        Test_Ts();
    }   
    return 0;
}

