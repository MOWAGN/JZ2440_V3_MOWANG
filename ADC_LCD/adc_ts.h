/*
 * FILE: adc_ts.h
 * ADC和触摸屏的测试函数接口
 */

#ifndef __ADC_TS_H__
#define __ADC_TS_H__

/* 
 * 以480x272,16bpp的显示模式测试TFT LCD
 */
void Test_Lcd_Tft_16Bit_480272(void);

/* 
 * 测试ADC
 * 通过A/D转换，测量可变电阻器的电压值
 */       
void Test_Adc(void);

/* 
 * INT_ADC的中断服务程序
 * A/D转换结束时发生此中断
 * 先读取X、Y坐标值，进行对应的lcd显示操作，再进入等待中断模式
 */       
void Function_Adc(void);

/* 
 * 测试触摸屏，打印触点坐标
 */       
void Test_Ts(void);

/*
 * 图库功能
 * 参数说明：ADC转换后获得触摸屏的坐标信号
 */
void picture_hub(int x, int y);

/*
 * 关闭图库功能
 * 参数说明：ADC转换后获得触摸屏的坐标信号
 */
void picture_hub_close(int x, int y);

/*
 * 娱乐功能
 *功能说明：亮灯
 * 参数说明：ADC转换后获得触摸屏的坐标信号
 */

void play_hub(int x, int y);

/*
 * 娱乐关闭功能
 * 参数说明：ADC转换后获得触摸屏的坐标信号
 */
void play_hub_close(int x, int y);

/*
 *备忘录功能（通过UART与PC机进行写入）
 *参数说明：备忘录数据存储地址,ADC转换后获得触摸屏的坐标信号
 */
void note(char* book, int x, int y);

/*
 *关闭备忘录
 *参数说明：ADC转换后获得触摸屏的坐标信号
 */
void note_close(int x, int y, char* book);

#endif /*__ADC_TS_H__*/
