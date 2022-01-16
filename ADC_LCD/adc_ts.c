/*
 * FILE: adc_ts.c
 * ADC和触摸屏、LCD显示和扩展功能的测试函数
 */

#include <stdio.h>
#include "adc_ts.h"
#include "s3c24xx.h"
#include "serial.h"
#include "lcddrv.h"
#include "framebuffer.h"
#include "font.h"

#include "home_key.h"
#include "play_key.h"
#include "note_key.h"
#include "photo_key.h"
#include "left_key.h"
#include "right_key.h"
#include "pic1.h"
#include "pic2.h"
#include "pic3.h"
#include "note.h"
#include "playfun_key.h"

// ADCCON寄存器
#define PRESCALE_DIS        (0 << 14)
#define PRESCALE_EN         (1 << 14)
#define PRSCVL(x)           ((x) << 6)
#define ADC_INPUT(x)        ((x) << 3)
#define ADC_START           (1 << 0)
#define ADC_ENDCVT          (1 << 15)

// ADCTSC寄存器
#define UD_SEN          (1 << 8)
#define DOWN_INT        (UD_SEN*0)
#define UP_INT          (UD_SEN*1)
#define YM_SEN          (1 << 7)
#define YM_HIZ          (YM_SEN*0)
#define YM_GND          (YM_SEN*1)
#define YP_SEN          (1 << 6)
#define YP_EXTVLT       (YP_SEN*0)
#define YP_AIN          (YP_SEN*1)
#define XM_SEN          (1 << 5)
#define XM_HIZ          (XM_SEN*0)
#define XM_GND          (XM_SEN*1)
#define XP_SEN          (1 << 4)
#define XP_EXTVLT       (XP_SEN*0)
#define XP_AIN          (XP_SEN*1)
#define XP_PULL_UP      (1 << 3)
#define XP_PULL_UP_EN   (XP_PULL_UP*0)
#define XP_PULL_UP_DIS  (XP_PULL_UP*1)
#define AUTO_PST        (1 << 2)
#define CONVERT_MAN     (AUTO_PST*0)
#define CONVERT_AUTO    (AUTO_PST*1)
#define XP_PST(x)       (x << 0)

#define NOP_MODE        0
#define X_AXIS_MODE     1
#define Y_AXIS_MODE     2
#define WAIT_INT_MODE   3

// GPIO寄存器
#define	GPFCON		(*(volatile unsigned long *)0x56000050)
#define	GPFDAT		(*(volatile unsigned long *)0x56000054)

#define	GPF4_out	(1<<(4*2))
#define	GPF5_out	(1<<(5*2))
#define	GPF6_out	(1<<(6*2))

/* 设置进入等待中断模式，XP_PU,XP_Dis,XM_Dis,YP_Dis,YM_En
 * (1)对于S3C2410，位[8]只能为0，所以只能使用下面的wait_down_int，
 *    它既等待Pen Down中断，也等待Pen Up中断
 * (2)对于S3C2440，位[8]为0、1时分别表示等待Pen Down中断或Pen Up中断
 */
/* 进入"等待中断模式"，等待触摸屏被按下 */
#define wait_down_int() { ADCTSC = DOWN_INT | XP_PULL_UP_EN | \
                          XP_AIN | XM_HIZ | YP_AIN | YM_GND | \
                          XP_PST(WAIT_INT_MODE); }
/* 进入"等待中断模式"，等待触摸屏被松开 */
#define wait_up_int()   { ADCTSC = UP_INT | XP_PULL_UP_EN | XP_AIN | XM_HIZ | \
                          YP_AIN | YM_GND | XP_PST(WAIT_INT_MODE); }

/* 进入自动(连续) X/Y轴坐标转换模式 */
#define mode_auto_xy()  { ADCTSC = CONVERT_AUTO | XP_PULL_UP_DIS | XP_PST(NOP_MODE); }

extern int home_sign;      
extern int pic_sign;
extern int pic_index;
extern int note_sign;
extern int note_write;
extern int play_sign;
extern int play_index;
extern char Note[40];

extern void (*isr_handle_array[50])(void);

/* 
 * 以480x272,16bpp的显示模式测试TFT LCD
 */
void Test_Lcd_Tft_16Bit_480272(void)
{
    Lcd_Port_Init();                     // 设置LCD引脚
    Tft_Lcd_Init(MODE_TFT_16BIT_480272); // 初始化LCD控制器
    Lcd_PowerEnable(0, 1);               // 设置LCD_PWREN有效，它用于打开LCD的电源
    Lcd_EnvidOnOff(1);                   // 使能LCD控制器输出信号
    InitFont();                          //初始化字符输出

    ClearScr(0xffffff);             // 清屏，白色
    Output_picture(50, 100, 72, 72, gImage_home_key);      //输出HOME键
}

/* 
 * 使用查询方式读取A/D转换值
 * 输入参数：
 *     ch: 模拟信号通道，取值为0~7
 */       
static int ReadAdc(int ch)
{
    // 选择模拟通道，使能预分频功能，设置A/D转换器的时钟 = PCLK/(49+1)
    ADCCON = PRESCALE_EN | PRSCVL(49) | ADC_INPUT(ch);

    // 清除位[2]，设为普通转换模式
    ADCTSC &= ~(1<<2);

    // 设置位[0]为1，启动A/D转换
    ADCCON |= ADC_START;

    // 当A/D转换真正开始时，位[0]会自动清0
    while (ADCCON & ADC_START);

    // 检测位[15]，当它为1时表示转换结束
    while (!(ADCCON & ADC_ENDCVT));

    // 读取数据    
    return (ADCDAT0 & 0x3ff);
}

/* 
 * 测试ADC
 * 通过A/D转换，测量可变电阻器的电压值
 */       
void Test_Adc(void)
{
    float vol0, vol1;
    int t0, t1;

    printf("Measuring the voltage of AIN0 and AIN1, press any key to exit\n\r");
    while (!awaitkey(0))    // 串口无输入，则不断测试
    {
        vol0 = ((float)ReadAdc(0)*3.3)/1024.0;  // 计算电压值
        vol1 = ((float)ReadAdc(1)*3.3)/1024.0;  // 计算电压值
        t0   = (vol0 - (int)vol0) * 1000;   // 计算小数部分, 本代码中的printf无法打印浮点数
        t1   = (vol1 - (int)vol1) * 1000;   // 计算小数部分,  本代码中的printf无法打印浮点数
        printf("AIN0 = %d.%-3dV    AIN1 = %d.%-3dV\r", (int)vol0, t0, (int)vol1, t1);
    }
    printf("\n");
}

/* 
 * INT_TC的中断服务程序
 * 当触摸屏被按下时，进入自动(连续) X/Y轴坐标转换模式；
 * 当触摸屏被松开时，进入等待中断模式，再次等待INT_TC中断
 */       
static void Isr_Tc(void)
{
    if (ADCDAT0 & 0x8000)
    {
        printf("Stylus Up!!\n\r");
        wait_down_int();    /* 进入"等待中断模式"，等待触摸屏被按下 */
    }
    else 
    {
        printf("Stylus Down: ");
        printf("home_sign = %d\n\r", home_sign);

        mode_auto_xy();     /* 进入自动(连续) X/Y轴坐标转换模式 */
    
        /* 设置位[0]为1，启动A/D转换
         * 注意：ADCDLY为50000，PCLK = 50MHz，
         *       要经过(1/50MHz)*50000=1ms之后才开始转换X坐标
         *       再经过1ms之后才开始转换Y坐标
         */
        ADCCON |= ADC_START;
    }
    
    // 清INT_TC中断
    SUBSRCPND |= BIT_SUB_TC;
    SRCPND    |= BIT_ADC;
    INTPND    |= BIT_ADC;
}




/* 
 * INT_ADC的中断服务程序
 * A/D转换结束时发生此中断
 * 先读取X、Y坐标值，再进入等待中断模式
 */       
static void Isr_Adc(void)
{
    // 打印X、Y坐标值    
    printf("xdata = %4d, ydata = %4d\r\n", (int)(ADCDAT0 & 0x3ff), (int)(ADCDAT1 & 0x3ff));

    /* 判断是S3C2410还是S3C2440 */
    if ((GSTATUS1 == 0x32410000) || (GSTATUS1 == 0x32410002))
    {   // S3C2410
        wait_down_int();    /* 进入"等待中断模式"，等待触摸屏被松开 */
    }
    else
    {   // S3C2440
        wait_up_int();      /* 进入"等待中断模式"，等待触摸屏被松开 */
    }

    // 清INT_ADC中断
    SUBSRCPND |= BIT_SUB_ADC;
    SRCPND    |= BIT_ADC;
    INTPND    |= BIT_ADC;
}

/* 
 * INT_ADC的中断服务程序
 * A/D转换结束时发生此中断
 * 先读取X、Y坐标值，进行对应的lcd显示操作，再进入等待中断模式
 */       
void Function_Adc(void)
{
    int x_adc=0, y_adc=0;   //获取触控位置
    x_adc = (int)(ADCDAT0 & 0x3ff);
    y_adc = (int)(ADCDAT1 & 0x3ff);
    int pic_init_sign=0;

    // 打印X、Y坐标值    
    printf("xdata = %4d, ydata = %4d\r\n", (int)(ADCDAT0 & 0x3ff), (int)(ADCDAT1 & 0x3ff));

    if(home_sign == 0)
    {
        if((x_adc>460 && x_adc<560)&&(y_adc>200 && y_adc<260))
        {
            Output_picture(62, 196, 48, 48, gImage_photo_key);      //输出相册键
            delay(68000);											//获得动态展开效果
            Output_picture(136, 112, 48, 48, gImage_note_key);       //输出备忘录功能键
            delay(68000);
            Output_picture(62, 28, 48, 48, gImage_play_key);         //输出娱乐功能键
        }
        home_sign = (!home_sign);
    }
    else 
    {       
        //娱乐功能键判断
        if((x_adc>230 && x_adc<340)&&(y_adc>180 && y_adc<270))   
        {
            pic_sign = 1;		//关闭其他功能
            note_sign = 1;
            Section_clr(230, 38, 230, 220, 0xffffff);      //界面清屏
            printf("This key is Play_Key\n\r");			   //调试输出
            //Section_clr(230, 38, 230, 220, 0xffffff);      //界面关闭，清屏
            Output_picture(418, 222, 36, 35, gImage_play_right_key);      //娱乐右键停止
            Output_picture(264, 222, 35, 36, gImage_play_left_key);      //娱乐左键开始
            Output_picture(319, 83, 80, 80, gImage_playpic1);            //输出娱乐封面图
            play_sign = (!play_sign);
            printf("note_sign = %d\t", note_sign);			//调试输出
            printf("pic_sign = %d\t", pic_sign);
            printf("play_sign = %d\n",play_sign);
        }
        //图片功能键判断
        if((x_adc>680 && x_adc<800)&&(y_adc>180 && y_adc<270))   
        {
            play_sign = 1;      
            note_sign = 1;
            Section_clr(230, 38, 230, 220, 0xffffff);      //界面清屏
            printf("This key is Pic_Key\n\r");				//调试输出
            Output_picture(418, 222, 36, 35, gImage_right_key);      //左功能键
            Output_picture(264, 222, 35, 36, gImage_left_key);      //右功能键
            Output_picture(272, 38, 180, 180, gImage_pic1);    //输出初始化图库首图
            pic_sign = (!pic_sign);
            printf("note_sign = %d\t", note_sign);			//调试输出
            printf("pic_sign = %d\t", pic_sign);
            printf("play_sign = %d\n",play_sign);
        }
        //备忘录功能键判断
        if((x_adc>450 && x_adc<580)&&(y_adc>320 && y_adc<410))   
        {
            pic_sign = 1;
            play_sign = 1;
            Section_clr(230, 38, 230, 220, 0xffffff);      //界面部分清屏
            printf("This key is Note_Key\n\r");
            Output_picture(230, 38, 230, 156, gImage_note);    //输出初始化便签界面
            note_sign = (!note_sign);
            printf("note_sign = %d\t", note_sign);			//调试输出
            printf("pic_sign = %d\t", pic_sign);
            printf("play_sign = %d\n",play_sign);
        }
        //收起辅助按钮选项
        if((x_adc>460 && x_adc<560)&&(y_adc>200 && y_adc<260))
        {
            Section_clr(62, 196, 48, 48, 0xffffff);     // 部分清屏，收拢图标，白色
            Section_clr(136, 112, 48, 48, 0xffffff);
            Section_clr(62, 28, 48, 48, 0xffffff);
            home_sign = (!home_sign);            
        }
        else 
        {
            picture_hub(x_adc, y_adc);
            picture_hub_close(x_adc, y_adc);
            note(Note, x_adc, y_adc);
            note_close(x_adc, y_adc, Note);
            play_hub(x_adc, y_adc);
            play_hub_close(x_adc, y_adc);
        }
    }
    /* 判断是S3C2410还是S3C2440 */
    if ((GSTATUS1 == 0x32410000) || (GSTATUS1 == 0x32410002))
    {   // S3C2410
        wait_down_int();    /* 进入"等待中断模式"，等待触摸屏被松开 */
    }
    else
    {   // S3C2440
        wait_up_int();      /* 进入"等待中断模式"，等待触摸屏被松开 */
    }
    // 清INT_ADC中断
    SUBSRCPND |= BIT_SUB_ADC;
    SRCPND    |= BIT_ADC;
    INTPND    |= BIT_ADC;
}

/* 
 * ADC、触摸屏的中断服务程序
 * 对于INT_TC、INT_ADC中断，分别调用它们的处理程序
 */       
void AdcTsIntHandle(void)
{
    if (SUBSRCPND & BIT_SUB_TC)
        Isr_Tc();

    if (SUBSRCPND & BIT_SUB_ADC)
        //Isr_Adc();
        Function_Adc();
}

/* 
 * 测试触摸屏，打印触点坐标
 */       
void Test_Ts(void)
{
    isr_handle_array[ISR_ADC_OFT] = AdcTsIntHandle;    // 设置ADC中断服务程序
    INTMSK &= ~BIT_ADC;          // 开启ADC总中断
    INTSUBMSK &= ~(BIT_SUB_TC);  // 开启INT_TC中断，即触摸屏被按下或松开时产生中断
    INTSUBMSK &= ~(BIT_SUB_ADC); // 开启INT_ADC中断，即A/D转换结束时产生中断
    
    // 使能预分频功能，设置A/D转换器的时钟 = PCLK/(49+1)
    ADCCON = PRESCALE_EN | PRSCVL(49);

    /* 采样延时时间 = (1/3.6864M)*50000 = 13.56ms
     * 即按下触摸屏后，再过13.56ms才采样
     */
    ADCDLY = 50000;

    wait_down_int();    /* 进入"等待中断模式"，等待触摸屏被按下 */

    printf("Touch the screem to test, press any key to exit\n\r");    
    getc();

    // 屏蔽ADC中断
    INTSUBMSK |= BIT_SUB_TC;
    INTSUBMSK |= BIT_SUB_ADC;
    INTMSK |= BIT_ADC;
}

/*
 * 图库功能
 * 参数说明：ADC转换后获得触摸屏的坐标信号
 */
void picture_hub(int x, int y)
{
    if(pic_sign == 0)
    {
        
        if((x>750 && x<850) && (y>560 && y<630))       //左键按下,更新图片索引值
        {
            pic_index--;
            if(pic_index < 1) 
                {pic_index = 3-pic_index;}                   //其中3为图库图片总数
            if(pic_index > 3)
                {pic_index = pic_index-3;}

        }
        if((x>750 && x<850) && (y>855 && y<915))       //右键按下，更新图片索引值
        {
            pic_index++;
            if(pic_index < 1) 
                {pic_index = 3-pic_index;}                   //其中3为图库图片总数
            if(pic_index > 3)
                {pic_index = pic_index-3;}
        }
        switch (pic_index)							//根据图片索引切换图片
        {
            case 1:
            {
                Output_picture(272, 38, 180, 180, gImage_pic1);
                printf("pic_index = %d\n\r", pic_index);
                break;
            }
            case 2:
            {
                Output_picture(272, 38, 180, 180, gImage_pic2);
                printf("pic_index = %d\n\r", pic_index);
                break;
            }
            case 3:
            {
                Output_picture(272, 38, 180, 180, gImage_pic3);
                printf("pic_index = %d\n\r", pic_index);
                break;
            }
            default: 
                break;
        }   
    }
}

/*
 * 关闭图库功能
 * 参数说明：ADC转换后获得触摸屏的坐标信号
 */
void picture_hub_close(int x, int y)
{
    if((x>680 && x<800)&&(y>180 && y<270))				
    {	
        if(pic_sign == 1)		//检测到关闭标志
        {
            Section_clr(230, 38, 230, 220, 0xffffff);      //清除界面
        }
        pic_index = 1;
    }
}

/*
 * 娱乐功能
 *功能说明：亮灯
 * 参数说明：ADC转换后获得触摸屏的坐标信号
 */

void play_hub(int x, int y)
{

    GPFCON = GPF4_out | GPF5_out | GPF6_out;		// 将LED1,2,4对应的GPF4/5/6三个引脚设为输出
    GPFDAT |= ((1 << 4) | (1 << 5) | (1 << 6));

    if (play_sign == 0)
    {
        
        if ((x > 750 && x < 850) && (y > 560 && y < 630))       //左键按下
        {
            play_index = 0;
        }
        if ((x > 750 && x < 850) && (y > 855 && y < 915))       //右键按下
        {
            play_index = 1;
        }
        printf("play_index = %d\n\r", play_index);
        if(play_index == 1)                                     //右键按下开始 
        {
            Output_picture(319, 83, 80, 80, gImage_playpic2);   //灯亮图片
            GPFDAT |= ((1 << 4) | (1 << 5) | (1 << 6));         //LED依次亮起，然后全灭
            delay(50000);
            GPFDAT &= (~(1 << 4));
            delay(50000);
            GPFDAT &= (~(1 << 5));
            delay(50000);
            GPFDAT &= (~(1 << 6));
            delay(50000);
            GPFDAT &= ((~(1 << 4))& (~(1 << 5))& (~(1 << 6)));
        }
        if (play_index == 0)                                     //左键按下结束 
        {
            Output_picture(319, 83, 80, 80, gImage_playpic1);    //灯灭图片
            GPFDAT |= ((1 << 4) | (1 << 5) | (1 << 6));	 	    // LED全灭
        }
    }
}

/*
 * 娱乐关闭功能
 * 参数说明：ADC转换后获得触摸屏的坐标信号
 */
void play_hub_close(int x, int y)
{
    if ((x > 230 && x < 340) && (y > 180 && y < 270))
    {
        if (play_sign == 1)		//检测到关闭标志
        {
            GPFDAT |= ((1 << 4) | (1 << 5) | (1 << 6));
            Section_clr(230, 38, 230, 220, 0xffffff);      //界面关闭
        }
        play_index = 0;
    }
}

/*
 *备忘录功能（通过UART与PC机进行写入）
 *参数说明：备忘录数据存储地址,ADC转换后获得触摸屏的坐标信号
 */
void note(char* book, int x, int y)
{

    if(note_sign == 0)
    {
        
        char* temp = book;

        PrintFbString8x16(240, 78,(char*) book, 0x000000, 0);
    
        if((x>240 && x<280)&&(y>490 && y<520))  //编辑按钮检测    
        {
            note_write = 0;
        }

        if(note_write == 0)
        {
            Output_picture(230, 38, 230, 156, gImage_note);    //清空便签界面
            
            printf("Please input the note:");    
             
            book = gets(temp);  //Uart进行备忘录读写
            
            PrintFbString8x16(240, 78, (char*) book, 0x000000, 0);//刷新
            
            note_write = 1;     //写完则标志位置1
        }
    }
    
}

/*
 *关闭备忘录
 *参数说明：ADC转换后获得触摸屏的坐标信号
 */
void note_close(int x, int y,char *book)
{
    if((x>450 && x<580)&&(y>320 && y<410))
    {
        if(note_sign==1)		//检测到关闭标志
        {
            ClearFbString8x16(240, 78, book, 40, 0xFFBA, 0);
            Section_clr(230, 38, 230, 220, 0xffffff);   //部分清除备忘录界面
        }
    }
}
