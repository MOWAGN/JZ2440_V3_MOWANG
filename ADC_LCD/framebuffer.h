/*
 * FILE: framebuffer.h
 * 在framebuffer上画点、画线、画同心圆、清屏的函数接口
 */

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <types.h>

/*
 *软件延时
 */
void delay(volatile unsigned long dly);

/* 
 * 画点
 * 输入参数：
 *     x、y : 象素坐标
 *     color: 颜色值
 *         对于16BPP: color的格式为0xAARRGGBB (AA = 透明度),
 *     需要转换为5:6:5格式
 *         对于8BPP: color为调色板中的索引值，
 *     其颜色取决于调色板中的数值
 */
void PutPixel(UINT32 x, UINT32 y, UINT32 color);

/* 
 * 画线
 * 输入参数：
 *     x1、y1 : 起点坐标
 *     x2、y2 : 终点坐标
 *     color  : 颜色值
 *         对于16BPP: color的格式为0xAARRGGBB (AA = 透明度),
 *     需要转换为5:6:5格式
 *         对于8BPP: color为调色板中的索引值，
 *     其颜色取决于调色板中的数值
 */
void DrawLine(int x1,int y1,int x2,int y2,int color);

/*
 * 输出图片
 * 参数说明：X,Y分别为图片起始点坐标
            W,H分别为图片宽，高，可通过Image2LCD查看得到
            gImage为图像数组
 */
void Output_picture(UINT32 X, UINT32 Y, UINT32 W, UINT32 H, const unsigned char* gImage);

/* 
 * 将屏幕部分区域清成单色
 * 输入参数：
 *     color: 颜色值
 *              对于16BPP: color的格式为0xAARRGGBB (AA = 透明度),
 *          需要转换为5:6:5格式
 *               对于8BPP: color为调色板中的索引值，
 *          其颜色取决于调色板中的数值
 *     X,Y：分别为图片起始点坐标
 *     W,H：分别为图片宽，高，可通过Image2LCD查看得到
 */
void Section_clr(UINT32 X, UINT32 Y, UINT32 W, UINT32 H, UINT32 color);

/* 
 * 将屏幕清成单色
 * 输入参数：
 *     color: 颜色值
 *         对于16BPP: color的格式为0xAARRGGBB (AA = 透明度),
 *     需要转换为5:6:5格式
 *         对于8BPP: color为调色板中的索引值，
 *     其颜色取决于调色板中的数值
 */
void ClearScr(UINT32 color);

#endif /*__FRAMEBUFFER_H__*/
