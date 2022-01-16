/*
 * FILE: adc_ts.c
 * ADC�ʹ�������LCD��ʾ����չ���ܵĲ��Ժ���
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

// ADCCON�Ĵ���
#define PRESCALE_DIS        (0 << 14)
#define PRESCALE_EN         (1 << 14)
#define PRSCVL(x)           ((x) << 6)
#define ADC_INPUT(x)        ((x) << 3)
#define ADC_START           (1 << 0)
#define ADC_ENDCVT          (1 << 15)

// ADCTSC�Ĵ���
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

// GPIO�Ĵ���
#define	GPFCON		(*(volatile unsigned long *)0x56000050)
#define	GPFDAT		(*(volatile unsigned long *)0x56000054)

#define	GPF4_out	(1<<(4*2))
#define	GPF5_out	(1<<(5*2))
#define	GPF6_out	(1<<(6*2))

/* ���ý���ȴ��ж�ģʽ��XP_PU,XP_Dis,XM_Dis,YP_Dis,YM_En
 * (1)����S3C2410��λ[8]ֻ��Ϊ0������ֻ��ʹ�������wait_down_int��
 *    ���ȵȴ�Pen Down�жϣ�Ҳ�ȴ�Pen Up�ж�
 * (2)����S3C2440��λ[8]Ϊ0��1ʱ�ֱ��ʾ�ȴ�Pen Down�жϻ�Pen Up�ж�
 */
/* ����"�ȴ��ж�ģʽ"���ȴ������������� */
#define wait_down_int() { ADCTSC = DOWN_INT | XP_PULL_UP_EN | \
                          XP_AIN | XM_HIZ | YP_AIN | YM_GND | \
                          XP_PST(WAIT_INT_MODE); }
/* ����"�ȴ��ж�ģʽ"���ȴ����������ɿ� */
#define wait_up_int()   { ADCTSC = UP_INT | XP_PULL_UP_EN | XP_AIN | XM_HIZ | \
                          YP_AIN | YM_GND | XP_PST(WAIT_INT_MODE); }

/* �����Զ�(����) X/Y������ת��ģʽ */
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
 * ��480x272,16bpp����ʾģʽ����TFT LCD
 */
void Test_Lcd_Tft_16Bit_480272(void)
{
    Lcd_Port_Init();                     // ����LCD����
    Tft_Lcd_Init(MODE_TFT_16BIT_480272); // ��ʼ��LCD������
    Lcd_PowerEnable(0, 1);               // ����LCD_PWREN��Ч�������ڴ�LCD�ĵ�Դ
    Lcd_EnvidOnOff(1);                   // ʹ��LCD����������ź�
    InitFont();                          //��ʼ���ַ����

    ClearScr(0xffffff);             // ��������ɫ
    Output_picture(50, 100, 72, 72, gImage_home_key);      //���HOME��
}

/* 
 * ʹ�ò�ѯ��ʽ��ȡA/Dת��ֵ
 * ���������
 *     ch: ģ���ź�ͨ����ȡֵΪ0~7
 */       
static int ReadAdc(int ch)
{
    // ѡ��ģ��ͨ����ʹ��Ԥ��Ƶ���ܣ�����A/Dת������ʱ�� = PCLK/(49+1)
    ADCCON = PRESCALE_EN | PRSCVL(49) | ADC_INPUT(ch);

    // ���λ[2]����Ϊ��ͨת��ģʽ
    ADCTSC &= ~(1<<2);

    // ����λ[0]Ϊ1������A/Dת��
    ADCCON |= ADC_START;

    // ��A/Dת��������ʼʱ��λ[0]���Զ���0
    while (ADCCON & ADC_START);

    // ���λ[15]������Ϊ1ʱ��ʾת������
    while (!(ADCCON & ADC_ENDCVT));

    // ��ȡ����    
    return (ADCDAT0 & 0x3ff);
}

/* 
 * ����ADC
 * ͨ��A/Dת���������ɱ�������ĵ�ѹֵ
 */       
void Test_Adc(void)
{
    float vol0, vol1;
    int t0, t1;

    printf("Measuring the voltage of AIN0 and AIN1, press any key to exit\n\r");
    while (!awaitkey(0))    // ���������룬�򲻶ϲ���
    {
        vol0 = ((float)ReadAdc(0)*3.3)/1024.0;  // �����ѹֵ
        vol1 = ((float)ReadAdc(1)*3.3)/1024.0;  // �����ѹֵ
        t0   = (vol0 - (int)vol0) * 1000;   // ����С������, �������е�printf�޷���ӡ������
        t1   = (vol1 - (int)vol1) * 1000;   // ����С������,  �������е�printf�޷���ӡ������
        printf("AIN0 = %d.%-3dV    AIN1 = %d.%-3dV\r", (int)vol0, t0, (int)vol1, t1);
    }
    printf("\n");
}

/* 
 * INT_TC���жϷ������
 * ��������������ʱ�������Զ�(����) X/Y������ת��ģʽ��
 * �����������ɿ�ʱ������ȴ��ж�ģʽ���ٴεȴ�INT_TC�ж�
 */       
static void Isr_Tc(void)
{
    if (ADCDAT0 & 0x8000)
    {
        printf("Stylus Up!!\n\r");
        wait_down_int();    /* ����"�ȴ��ж�ģʽ"���ȴ������������� */
    }
    else 
    {
        printf("Stylus Down: ");
        printf("home_sign = %d\n\r", home_sign);

        mode_auto_xy();     /* �����Զ�(����) X/Y������ת��ģʽ */
    
        /* ����λ[0]Ϊ1������A/Dת��
         * ע�⣺ADCDLYΪ50000��PCLK = 50MHz��
         *       Ҫ����(1/50MHz)*50000=1ms֮��ſ�ʼת��X����
         *       �پ���1ms֮��ſ�ʼת��Y����
         */
        ADCCON |= ADC_START;
    }
    
    // ��INT_TC�ж�
    SUBSRCPND |= BIT_SUB_TC;
    SRCPND    |= BIT_ADC;
    INTPND    |= BIT_ADC;
}




/* 
 * INT_ADC���жϷ������
 * A/Dת������ʱ�������ж�
 * �ȶ�ȡX��Y����ֵ���ٽ���ȴ��ж�ģʽ
 */       
static void Isr_Adc(void)
{
    // ��ӡX��Y����ֵ    
    printf("xdata = %4d, ydata = %4d\r\n", (int)(ADCDAT0 & 0x3ff), (int)(ADCDAT1 & 0x3ff));

    /* �ж���S3C2410����S3C2440 */
    if ((GSTATUS1 == 0x32410000) || (GSTATUS1 == 0x32410002))
    {   // S3C2410
        wait_down_int();    /* ����"�ȴ��ж�ģʽ"���ȴ����������ɿ� */
    }
    else
    {   // S3C2440
        wait_up_int();      /* ����"�ȴ��ж�ģʽ"���ȴ����������ɿ� */
    }

    // ��INT_ADC�ж�
    SUBSRCPND |= BIT_SUB_ADC;
    SRCPND    |= BIT_ADC;
    INTPND    |= BIT_ADC;
}

/* 
 * INT_ADC���жϷ������
 * A/Dת������ʱ�������ж�
 * �ȶ�ȡX��Y����ֵ�����ж�Ӧ��lcd��ʾ�������ٽ���ȴ��ж�ģʽ
 */       
void Function_Adc(void)
{
    int x_adc=0, y_adc=0;   //��ȡ����λ��
    x_adc = (int)(ADCDAT0 & 0x3ff);
    y_adc = (int)(ADCDAT1 & 0x3ff);
    int pic_init_sign=0;

    // ��ӡX��Y����ֵ    
    printf("xdata = %4d, ydata = %4d\r\n", (int)(ADCDAT0 & 0x3ff), (int)(ADCDAT1 & 0x3ff));

    if(home_sign == 0)
    {
        if((x_adc>460 && x_adc<560)&&(y_adc>200 && y_adc<260))
        {
            Output_picture(62, 196, 48, 48, gImage_photo_key);      //�������
            delay(68000);											//��ö�̬չ��Ч��
            Output_picture(136, 112, 48, 48, gImage_note_key);       //�������¼���ܼ�
            delay(68000);
            Output_picture(62, 28, 48, 48, gImage_play_key);         //������ֹ��ܼ�
        }
        home_sign = (!home_sign);
    }
    else 
    {       
        //���ֹ��ܼ��ж�
        if((x_adc>230 && x_adc<340)&&(y_adc>180 && y_adc<270))   
        {
            pic_sign = 1;		//�ر���������
            note_sign = 1;
            Section_clr(230, 38, 230, 220, 0xffffff);      //��������
            printf("This key is Play_Key\n\r");			   //�������
            //Section_clr(230, 38, 230, 220, 0xffffff);      //����رգ�����
            Output_picture(418, 222, 36, 35, gImage_play_right_key);      //�����Ҽ�ֹͣ
            Output_picture(264, 222, 35, 36, gImage_play_left_key);      //���������ʼ
            Output_picture(319, 83, 80, 80, gImage_playpic1);            //������ַ���ͼ
            play_sign = (!play_sign);
            printf("note_sign = %d\t", note_sign);			//�������
            printf("pic_sign = %d\t", pic_sign);
            printf("play_sign = %d\n",play_sign);
        }
        //ͼƬ���ܼ��ж�
        if((x_adc>680 && x_adc<800)&&(y_adc>180 && y_adc<270))   
        {
            play_sign = 1;      
            note_sign = 1;
            Section_clr(230, 38, 230, 220, 0xffffff);      //��������
            printf("This key is Pic_Key\n\r");				//�������
            Output_picture(418, 222, 36, 35, gImage_right_key);      //���ܼ�
            Output_picture(264, 222, 35, 36, gImage_left_key);      //�ҹ��ܼ�
            Output_picture(272, 38, 180, 180, gImage_pic1);    //�����ʼ��ͼ����ͼ
            pic_sign = (!pic_sign);
            printf("note_sign = %d\t", note_sign);			//�������
            printf("pic_sign = %d\t", pic_sign);
            printf("play_sign = %d\n",play_sign);
        }
        //����¼���ܼ��ж�
        if((x_adc>450 && x_adc<580)&&(y_adc>320 && y_adc<410))   
        {
            pic_sign = 1;
            play_sign = 1;
            Section_clr(230, 38, 230, 220, 0xffffff);      //���沿������
            printf("This key is Note_Key\n\r");
            Output_picture(230, 38, 230, 156, gImage_note);    //�����ʼ����ǩ����
            note_sign = (!note_sign);
            printf("note_sign = %d\t", note_sign);			//�������
            printf("pic_sign = %d\t", pic_sign);
            printf("play_sign = %d\n",play_sign);
        }
        //��������ťѡ��
        if((x_adc>460 && x_adc<560)&&(y_adc>200 && y_adc<260))
        {
            Section_clr(62, 196, 48, 48, 0xffffff);     // ������������£ͼ�꣬��ɫ
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
    /* �ж���S3C2410����S3C2440 */
    if ((GSTATUS1 == 0x32410000) || (GSTATUS1 == 0x32410002))
    {   // S3C2410
        wait_down_int();    /* ����"�ȴ��ж�ģʽ"���ȴ����������ɿ� */
    }
    else
    {   // S3C2440
        wait_up_int();      /* ����"�ȴ��ж�ģʽ"���ȴ����������ɿ� */
    }
    // ��INT_ADC�ж�
    SUBSRCPND |= BIT_SUB_ADC;
    SRCPND    |= BIT_ADC;
    INTPND    |= BIT_ADC;
}

/* 
 * ADC�����������жϷ������
 * ����INT_TC��INT_ADC�жϣ��ֱ�������ǵĴ������
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
 * ���Դ���������ӡ��������
 */       
void Test_Ts(void)
{
    isr_handle_array[ISR_ADC_OFT] = AdcTsIntHandle;    // ����ADC�жϷ������
    INTMSK &= ~BIT_ADC;          // ����ADC���ж�
    INTSUBMSK &= ~(BIT_SUB_TC);  // ����INT_TC�жϣ��������������»��ɿ�ʱ�����ж�
    INTSUBMSK &= ~(BIT_SUB_ADC); // ����INT_ADC�жϣ���A/Dת������ʱ�����ж�
    
    // ʹ��Ԥ��Ƶ���ܣ�����A/Dת������ʱ�� = PCLK/(49+1)
    ADCCON = PRESCALE_EN | PRSCVL(49);

    /* ������ʱʱ�� = (1/3.6864M)*50000 = 13.56ms
     * �����´��������ٹ�13.56ms�Ų���
     */
    ADCDLY = 50000;

    wait_down_int();    /* ����"�ȴ��ж�ģʽ"���ȴ������������� */

    printf("Touch the screem to test, press any key to exit\n\r");    
    getc();

    // ����ADC�ж�
    INTSUBMSK |= BIT_SUB_TC;
    INTSUBMSK |= BIT_SUB_ADC;
    INTMSK |= BIT_ADC;
}

/*
 * ͼ�⹦��
 * ����˵����ADCת�����ô������������ź�
 */
void picture_hub(int x, int y)
{
    if(pic_sign == 0)
    {
        
        if((x>750 && x<850) && (y>560 && y<630))       //�������,����ͼƬ����ֵ
        {
            pic_index--;
            if(pic_index < 1) 
                {pic_index = 3-pic_index;}                   //����3Ϊͼ��ͼƬ����
            if(pic_index > 3)
                {pic_index = pic_index-3;}

        }
        if((x>750 && x<850) && (y>855 && y<915))       //�Ҽ����£�����ͼƬ����ֵ
        {
            pic_index++;
            if(pic_index < 1) 
                {pic_index = 3-pic_index;}                   //����3Ϊͼ��ͼƬ����
            if(pic_index > 3)
                {pic_index = pic_index-3;}
        }
        switch (pic_index)							//����ͼƬ�����л�ͼƬ
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
 * �ر�ͼ�⹦��
 * ����˵����ADCת�����ô������������ź�
 */
void picture_hub_close(int x, int y)
{
    if((x>680 && x<800)&&(y>180 && y<270))				
    {	
        if(pic_sign == 1)		//��⵽�رձ�־
        {
            Section_clr(230, 38, 230, 220, 0xffffff);      //�������
        }
        pic_index = 1;
    }
}

/*
 * ���ֹ���
 *����˵��������
 * ����˵����ADCת�����ô������������ź�
 */

void play_hub(int x, int y)
{

    GPFCON = GPF4_out | GPF5_out | GPF6_out;		// ��LED1,2,4��Ӧ��GPF4/5/6����������Ϊ���
    GPFDAT |= ((1 << 4) | (1 << 5) | (1 << 6));

    if (play_sign == 0)
    {
        
        if ((x > 750 && x < 850) && (y > 560 && y < 630))       //�������
        {
            play_index = 0;
        }
        if ((x > 750 && x < 850) && (y > 855 && y < 915))       //�Ҽ�����
        {
            play_index = 1;
        }
        printf("play_index = %d\n\r", play_index);
        if(play_index == 1)                                     //�Ҽ����¿�ʼ 
        {
            Output_picture(319, 83, 80, 80, gImage_playpic2);   //����ͼƬ
            GPFDAT |= ((1 << 4) | (1 << 5) | (1 << 6));         //LED��������Ȼ��ȫ��
            delay(50000);
            GPFDAT &= (~(1 << 4));
            delay(50000);
            GPFDAT &= (~(1 << 5));
            delay(50000);
            GPFDAT &= (~(1 << 6));
            delay(50000);
            GPFDAT &= ((~(1 << 4))& (~(1 << 5))& (~(1 << 6)));
        }
        if (play_index == 0)                                     //������½��� 
        {
            Output_picture(319, 83, 80, 80, gImage_playpic1);    //����ͼƬ
            GPFDAT |= ((1 << 4) | (1 << 5) | (1 << 6));	 	    // LEDȫ��
        }
    }
}

/*
 * ���ֹرչ���
 * ����˵����ADCת�����ô������������ź�
 */
void play_hub_close(int x, int y)
{
    if ((x > 230 && x < 340) && (y > 180 && y < 270))
    {
        if (play_sign == 1)		//��⵽�رձ�־
        {
            GPFDAT |= ((1 << 4) | (1 << 5) | (1 << 6));
            Section_clr(230, 38, 230, 220, 0xffffff);      //����ر�
        }
        play_index = 0;
    }
}

/*
 *����¼���ܣ�ͨ��UART��PC������д�룩
 *����˵��������¼���ݴ洢��ַ,ADCת�����ô������������ź�
 */
void note(char* book, int x, int y)
{

    if(note_sign == 0)
    {
        
        char* temp = book;

        PrintFbString8x16(240, 78,(char*) book, 0x000000, 0);
    
        if((x>240 && x<280)&&(y>490 && y<520))  //�༭��ť���    
        {
            note_write = 0;
        }

        if(note_write == 0)
        {
            Output_picture(230, 38, 230, 156, gImage_note);    //��ձ�ǩ����
            
            printf("Please input the note:");    
             
            book = gets(temp);  //Uart���б���¼��д
            
            PrintFbString8x16(240, 78, (char*) book, 0x000000, 0);//ˢ��
            
            note_write = 1;     //д�����־λ��1
        }
    }
    
}

/*
 *�رձ���¼
 *����˵����ADCת�����ô������������ź�
 */
void note_close(int x, int y,char *book)
{
    if((x>450 && x<580)&&(y>320 && y<410))
    {
        if(note_sign==1)		//��⵽�رձ�־
        {
            ClearFbString8x16(240, 78, book, 40, 0xFFBA, 0);
            Section_clr(230, 38, 230, 220, 0xffffff);   //�����������¼����
        }
    }
}
