/*****************************************
 Copyright 2001-2003	
 Sigma Designs, Inc. All Rights Reserved
 Proprietary and Confidential
 *****************************************/
 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "fb.h"

#define ALLOW_OS_CODE 1
/*#include "../rua/include/rua.h"*/

#if 0
#define DEB(f) (f)
#else
#define DEB(f)
#endif

typedef unsigned char RMuint8;
typedef unsigned short RMuint16;
typedef unsigned int RMuint32;

struct fb_var_screeninfo fb_var;
struct fb_fix_screeninfo fb_fix;
char * fb_base_addr = NULL;

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
static void set_pixel(RMuint32 x, RMuint32 y, RMuint32 color)
{
	/*static RMuint32 i=0;*/
	/* TODO We assume for now we have contigus regions */
	RMuint8 red, green, blue;

	switch (fb_var.bits_per_pixel){
		case 16:
			{
				RMuint16 *addr = (RMuint16 *) fb_base_addr+(y*fb_var.xres+x);
				red   = (color >> 19) & 0x1f;
            	green = (color >> 10) & 0x3f;
            	blue  = (color >>  3) & 0x1f;
            	color = (red << 11) | (green << 5) | blue; // 格式5:6:5	
				*addr = (RMuint16) color;
			}
			break;
		case 8:
			{
				RMuint8 *addr = (RMuint8 *)fb_base_addr+(y*fb_var.xres+x);
				*addr = (RMuint8) color;
			}
			break;
		default:
			fprintf(stderr,"Unknown bpp : %d\n",fb_var.bits_per_pixel);
			break;
	}
	/*if (i<10){
		DEB(fprintf(stderr,"(%ld,%ld) [%p] <- %lX\n",x,y,addr,*addr));
		i++;
	}*/
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
static void DrawLine(RMuint32 x1,RMuint32 y1,RMuint32 x2,RMuint32 y2,RMuint32 color)
{
    RMuint32 dx,dy,e;
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
                    set_pixel(x1,y1,color);
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
                    set_pixel(x1,y1,color);
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
                    set_pixel(x1,y1,color);
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
                    set_pixel(x1,y1,color);
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
                    set_pixel(x1,y1,color);
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
                    set_pixel(x1,y1,color);
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
                    set_pixel(x1,y1,color);
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
                    set_pixel(x1,y1,color);
                    if(e>0){x1-=1;e-=dy;}   
                    y1-=1;
                    e+=dx;
                }
            }
        }   
    }
}

/* 
 * 绘制同心圆
 */
static void Mire(void)
{
 	RMuint32 x,y;
	RMuint32 color;
	RMuint8 red,green,blue,alpha;

	DEB(fprintf(stderr,"begin mire\n"));
	for (y=0;y<fb_var.yres;y++)
		for (x=0;x<fb_var.xres;x++){
			color = ((x-fb_var.xres/2)*(x-fb_var.xres/2) + (y-fb_var.yres/2)*(y-fb_var.yres/2))/64;
			red   = (color/8) % 256;
			green = (color/4) % 256;
			blue  = (color/2) % 256;
			alpha = (color*2) % 256;
			/*alpha = 0xFF;*/

			color |= ((RMuint32)alpha << 24);
			color |= ((RMuint32)red   << 16);
			color |= ((RMuint32)green << 8 );
			color |= ((RMuint32)blue       );

			set_pixel(x,y,color);
		}

	DEB(fprintf(stderr,"end mire\n"));
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
static void ClearScr(RMuint32 color)
{   
    RMuint32 x,y;
    
    for (y = 0; y < fb_var.yres; y++)
        for (x = 0; x < fb_var.xres; x++)
            set_pixel(x, y, color);
}

/*******   测试文件命令提示    *******/
void printusage(char *name)
{
		fprintf(stderr,"Usage (example): %s /dev/fb0\n", name);
		fprintf(stderr,"                 %s /dev/fb0 <DrawLine|Mire>\n",name);
		fprintf(stderr,"                 eg.  \n");
		fprintf(stderr,"                 %s /dev/fb0 DrawLine\n",name);
		fprintf(stderr,"                 %s /dev/fb0 Mire\n",name);
}

int main(int argc, char **argv)
{
    int fd=0;
    char* filename;
    char val;

    printf("%dx%d, %dbpp\n", fb_var.xres, fb_var.yres, fb_var.bits_per_pixel );

    if (argc != 3)
    {
        printusage(argv[0]);
        return 0;
    }

    filename = argv[1];

    fd = open(filename, O_RDWR);


    if (fd < 0)
    {
        printf("error, can't open %s\n", filename);
        return 0;
    }

	/* Get fixed screen information */		//获取fb_fix结构体参数
	if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix)) {
		printf("Error reading fb fixed information.\n");
		exit(1);
	}

	/* Get variable screen information 	*/	//获取fb_var结构体参数
	if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_var)) {
		printf("Error reading fb variable information.\n");
		exit(1);
	}

    fb_var.xres = fb_var.xres_virtual = 480;
    fb_var.yres = fb_var.yres_virtual = 272;
    fb_var.bits_per_pixel = 16;
    printf("%dx%d, %dbpp\n", fb_var.xres, fb_var.yres, fb_var.bits_per_pixel );
    
    if (!strcmp("DrawLine", argv[2]))
    {
        //画线
        ClearScr(0x0);
        DrawLine(0  , 0  , 479, 0  , 0xff0000);    // 红色
        DrawLine(0  , 0  , 0  , 271, 0x00ff00);    // 绿色
        DrawLine(479, 0  , 479, 271, 0x0000ff);    // 蓝色
        DrawLine(0  , 271, 479, 271, 0xffffff);    // 白色
        DrawLine(0  , 0  , 479, 271, 0xffff00);    // 黄色
        DrawLine(479, 0  , 0  , 271, 0x8000ff);    // 紫色
        DrawLine(240, 0  , 240, 271, 0xe6e8fa);    // 银色
        DrawLine(0  , 136, 479, 136, 0xcd7f32);    // 金色
    }
    else if (!strcmp("Mire", argv[2]))
    {
        // 画圈
        Mire();
    }
    else
    {
        ClearScr(0xff0000);             // 红色
        printusage(argv[0]);
        return 0;
    }   

    close(fd);
    return 0;
}
