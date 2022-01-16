/*
 * FILE: framebuffer.h
 * ��framebuffer�ϻ��㡢���ߡ���ͬ��Բ�������ĺ����ӿ�
 */

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <types.h>

/*
 *�����ʱ
 */
void delay(volatile unsigned long dly);

/* 
 * ����
 * ���������
 *     x��y : ��������
 *     color: ��ɫֵ
 *         ����16BPP: color�ĸ�ʽΪ0xAARRGGBB (AA = ͸����),
 *     ��Ҫת��Ϊ5:6:5��ʽ
 *         ����8BPP: colorΪ��ɫ���е�����ֵ��
 *     ����ɫȡ���ڵ�ɫ���е���ֵ
 */
void PutPixel(UINT32 x, UINT32 y, UINT32 color);

/* 
 * ����
 * ���������
 *     x1��y1 : �������
 *     x2��y2 : �յ�����
 *     color  : ��ɫֵ
 *         ����16BPP: color�ĸ�ʽΪ0xAARRGGBB (AA = ͸����),
 *     ��Ҫת��Ϊ5:6:5��ʽ
 *         ����8BPP: colorΪ��ɫ���е�����ֵ��
 *     ����ɫȡ���ڵ�ɫ���е���ֵ
 */
void DrawLine(int x1,int y1,int x2,int y2,int color);

/*
 * ���ͼƬ
 * ����˵����X,Y�ֱ�ΪͼƬ��ʼ������
            W,H�ֱ�ΪͼƬ���ߣ���ͨ��Image2LCD�鿴�õ�
            gImageΪͼ������
 */
void Output_picture(UINT32 X, UINT32 Y, UINT32 W, UINT32 H, const unsigned char* gImage);

/* 
 * ����Ļ����������ɵ�ɫ
 * ���������
 *     color: ��ɫֵ
 *              ����16BPP: color�ĸ�ʽΪ0xAARRGGBB (AA = ͸����),
 *          ��Ҫת��Ϊ5:6:5��ʽ
 *               ����8BPP: colorΪ��ɫ���е�����ֵ��
 *          ����ɫȡ���ڵ�ɫ���е���ֵ
 *     X,Y���ֱ�ΪͼƬ��ʼ������
 *     W,H���ֱ�ΪͼƬ���ߣ���ͨ��Image2LCD�鿴�õ�
 */
void Section_clr(UINT32 X, UINT32 Y, UINT32 W, UINT32 H, UINT32 color);

/* 
 * ����Ļ��ɵ�ɫ
 * ���������
 *     color: ��ɫֵ
 *         ����16BPP: color�ĸ�ʽΪ0xAARRGGBB (AA = ͸����),
 *     ��Ҫת��Ϊ5:6:5��ʽ
 *         ����8BPP: colorΪ��ɫ���е�����ֵ��
 *     ����ɫȡ���ڵ�ɫ���е���ֵ
 */
void ClearScr(UINT32 color);

#endif /*__FRAMEBUFFER_H__*/
