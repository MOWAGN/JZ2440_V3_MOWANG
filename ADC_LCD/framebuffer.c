/*
 * FILE: framebuffer.c
 * 实现在framebuffer上画点、画线、画同心圆、清屏的函数
 */
#include <stdio.h>
#include "serial.h"
#include "framebuffer.h"

extern unsigned int fb_base_addr;
extern unsigned int bpp;
extern unsigned int xsize;
extern unsigned int ysize;

/*
 *软件延时
 */
void delay(volatile unsigned long dly)
{
    for(; dly > 0; dly--);
}

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
void PutPixel(UINT32 x, UINT32 y, UINT32 color)
{
    UINT8 red,green,blue;

    switch (bpp){
        case 16:
        {
            UINT16 *addr = (UINT16 *)fb_base_addr + (y * xsize + x);
            red   = (color >> 19) & 0x1f;
            green = (color >> 10) & 0x3f;
            blue  = (color >>  3) & 0x1f;
            color = (red << 11) | (green << 5) | blue; // 格式5:6:5
            *addr = (UINT16) color;
            break;
        }
        
        case 8:
        {
            UINT8 *addr = (UINT8 *)fb_base_addr + (y * xsize + x);
            *addr = (UINT8) color;
            break;
        }

        default:
            break;
    }
}

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
void DrawLine(int x1,int y1,int x2,int y2,int color)
{
    int dx,dy,e;
    dx=x2-x1; 
    dy=y2-y1;
    
    if(dx>=0)
    {
        if(dy >= 0) // dy>=0
        {
            if(dx>=dy) // 1/8 octant
            {
                e=dy-dx/2;
                while(x1<=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1+=1;e-=dx;}   
                    x1+=1;
                    e+=dy;
                }
            }
            else        // 2/8 octant
            {
                e=dx-dy/2;
                while(y1<=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1+=1;e-=dy;}   
                    y1+=1;
                    e+=dx;
                }
            }
        }
        else           // dy<0
        {
            dy=-dy;   // dy=abs(dy)

            if(dx>=dy) // 8/8 octant
            {
                e=dy-dx/2;
                while(x1<=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1-=1;e-=dx;}   
                    x1+=1;
                    e+=dy;
                }
            }
            else        // 7/8 octant
            {
                e=dx-dy/2;
                while(y1>=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1+=1;e-=dy;}   
                    y1-=1;
                    e+=dx;
                }
            }
        }   
    }
    else //dx<0
    {
        dx=-dx;     //dx=abs(dx)
        if(dy >= 0) // dy>=0
        {
            if(dx>=dy) // 4/8 octant
            {
                e=dy-dx/2;
                while(x1>=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1+=1;e-=dx;}   
                    x1-=1;
                    e+=dy;
                }
            }
            else        // 3/8 octant
            {
                e=dx-dy/2;
                while(y1<=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1-=1;e-=dy;}   
                    y1+=1;
                    e+=dx;
                }
            }
        }
        else           // dy<0
        {
            dy=-dy;   // dy=abs(dy)

            if(dx>=dy) // 5/8 octant
            {
                e=dy-dx/2;
                while(x1>=x2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){y1-=1;e-=dx;}   
                    x1-=1;
                    e+=dy;
                }
            }
            else        // 6/8 octant
            {
                e=dx-dy/2;
                while(y1>=y2)
                {
                    PutPixel(x1,y1,color);
                    if(e>0){x1-=1;e-=dy;}   
                    y1-=1;
                    e+=dx;
                }
            }
        }   
    }
}


/*
 * 输出图片
 * 参数说明：X,Y分别为图片起始点坐标
 *          W,H分别为图片宽，高，可通过Image2LCD查看得到
 *          gImage为图像数组
 */
void Output_picture(UINT32 X, UINT32 Y, UINT32 W, UINT32 H, const unsigned char* gImage)
{
    UINT32 x,y;
    UINT16 color;
    UINT16 color_index=0;

    for (y=Y; y < Y+H; y++)
        for (x=X; x < X+W; x++){
            color = (gImage[color_index]<<8)|gImage[color_index+1];
            color_index = color_index + 2;
            UINT16 *addr = (UINT16 *)fb_base_addr + (y * xsize + x);
            *addr = (UINT16) color;
        }

}

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
void Section_clr(UINT32 X, UINT32 Y, UINT32 W, UINT32 H, UINT32 color)
{
    UINT32 x,y;
    
    for (y = Y; y < Y+H; y++)
        for (x = X; x < X+W; x++)
            PutPixel(x, y, color);
}

/* 
 * 将屏幕清成单色
 * 输入参数：
 *     color: 颜色值
 *         对于16BPP: color的格式为0xAARRGGBB (AA = 透明度),
 *     需要转换为5:6:5格式
 *         对于8BPP: color为调色板中的索引值，
 *     其颜色取决于调色板中的数值
 */
void ClearScr(UINT32 color)
{   
    UINT32 x,y;
    
    for (y = 0; y < ysize; y++)
        for (x = 0; x < xsize; x++)
            PutPixel(x, y, color);
}

