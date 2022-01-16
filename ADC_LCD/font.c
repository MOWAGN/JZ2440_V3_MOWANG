/**************************************************************************************************************************
  * @brief      : 	JZ2440v2开发板LCD显示字体代码源文件
  * @version    : 	V0.0
  * @note       : 	无
  * @history    : 	无
***************************************************************************************************************************/
#include "s3c24xx.h"
#include "lcddrv.h"
#include "framebuffer.h"
#include "font.h"
#include "font_8x16.h"


//extern const unsigned char fontdata_8x16[];
//extern const unsigned char asc2_3216[95][128];

/* LCD参数 */
static unsigned int sg_uiFbBase;
static int sg_iXres, sg_iYres, sg_Bpp;

/**************************************************************************************************************************
  * @brief       : 	初始化字体功能，得到lcd显示屏的参数
  * @param[in]   : 	无
  * @param[out]  : 	无
  * @return      : 	无
  * @others      : 	无
***************************************************************************************************************************/
void InitFont(void)
{
	sg_uiFbBase = LCDFRAMEBUFFER;
    sg_Bpp = 16;
    sg_iXres = 480;
    sg_iYres = 272;

}

/**************************************************************************************************************************
  * @brief       : 	在lcd显示屏的指定位置描绘指定颜色的8x16大小的文字
  * @param[in]   : 	x	文字的x坐标
  					y	文字的y坐标
  					c	显示的字符
  					color	显示的字符颜色
  * @param[out]  : 	无
  * @return      : 	无
  * @others      : 	无
***************************************************************************************************************************/
static void PrintFbChar8x16(int x, int y, char c, unsigned int color)
{
	int i, j;
	
	/* 根据c的ascii码在fontdata_8x16中得到点阵数据 */
	unsigned char *pchDots = (unsigned char *)&fontdata_8x16[c * 16];

	unsigned char uchData;
	int iBit;

	/* 根据点阵来设置对应象素的颜色 */
	for (j = y; j < y+16; j++)
	{
		uchData = *pchDots++;
		iBit = 7;
		for (i = x; i < x+8; i++)
		{
			/* 根据点阵的某位决定是否描颜色 */
			if (uchData & (1<<iBit))
				PutPixel(i, j, color);
			iBit--;
		}
	}
}


/**************************************************************************************************************************
  * @brief       : 	在lcd显示屏的指定位置开始描绘指定颜色的8x16大小的字符串
  * @param[in]   : 	x	字符串的x坐标
  					y	字符串的y坐标
  					c	显示的字符串
  					color	显示的字符串颜色
  					isDefer	是否逐个打印字符串中的字符			1，逐个打印	0，不逐个打印
  * @param[out]  : 	无
  * @return      : 	无
  * @others      : 	无
***************************************************************************************************************************/
void PrintFbString8x16(int x, int y, char* str, UINT32 color,int isDefer)
{
	int i = 0;
	
	while (str[i])
	{
		if (str[i] == '\n')
			y = y+16;
		else if (str[i] == '\r')
			x = 0;
		else
		{
			PrintFbChar8x16(x, y, str[i], color);
			if(isDefer)
				delay(8000);
			x = x+8;
			if (x >= sg_iXres) /* 换行 */
			{
				x = 0;
				y = y+16;
			}
		}
		i++;
	}
}

/**************************************************************************************************************************
  * @brief       : 	在lcd显示屏的指定位置开始清除指定颜色的8x16大小的字符串
  * @param[in]   : 	x	字符串的x坐标
  					y	字符串的y坐标
					str  待清除的字符串
  					length	清除的字符串长度
					color 清除后原字符串位置颜色填充
  					isDefer	是否逐个打印字符串中的字符			1，逐个打印	0，不逐个打印
  * @param[out]  : 	无
  * @return      : 	无
  * @others      : 	无
***************************************************************************************************************************/
void ClearFbString8x16(int x, int y, char* str, int length, UINT32 color, int isDefer)
{
	int i = 0;
	int j = length;

	while (j)
	{
		if (str[i] == '\n')
			y = y+16;
		else if (str[i] == '\r')
			x = 0;
		else
		{
			PrintFbChar8x16(x, y, str[i], color);
			if(isDefer)
				delay(8000);
			x = x+8;
			if (x >= sg_iXres) /* 换行 */
			{
				x = 0;
				y = y+16;
			}
		}
		i++;
		j--;
	}
}
