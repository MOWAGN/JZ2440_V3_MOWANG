/*
 * FILE: adc_ts.h
 * ADC�ʹ������Ĳ��Ժ����ӿ�
 */

#ifndef __ADC_TS_H__
#define __ADC_TS_H__

/* 
 * ��480x272,16bpp����ʾģʽ����TFT LCD
 */
void Test_Lcd_Tft_16Bit_480272(void);

/* 
 * ����ADC
 * ͨ��A/Dת���������ɱ�������ĵ�ѹֵ
 */       
void Test_Adc(void);

/* 
 * INT_ADC���жϷ������
 * A/Dת������ʱ�������ж�
 * �ȶ�ȡX��Y����ֵ�����ж�Ӧ��lcd��ʾ�������ٽ���ȴ��ж�ģʽ
 */       
void Function_Adc(void);

/* 
 * ���Դ���������ӡ��������
 */       
void Test_Ts(void);

/*
 * ͼ�⹦��
 * ����˵����ADCת�����ô������������ź�
 */
void picture_hub(int x, int y);

/*
 * �ر�ͼ�⹦��
 * ����˵����ADCת�����ô������������ź�
 */
void picture_hub_close(int x, int y);

/*
 * ���ֹ���
 *����˵��������
 * ����˵����ADCת�����ô������������ź�
 */

void play_hub(int x, int y);

/*
 * ���ֹرչ���
 * ����˵����ADCת�����ô������������ź�
 */
void play_hub_close(int x, int y);

/*
 *����¼���ܣ�ͨ��UART��PC������д�룩
 *����˵��������¼���ݴ洢��ַ,ADCת�����ô������������ź�
 */
void note(char* book, int x, int y);

/*
 *�رձ���¼
 *����˵����ADCת�����ô������������ź�
 */
void note_close(int x, int y, char* book);

#endif /*__ADC_TS_H__*/
