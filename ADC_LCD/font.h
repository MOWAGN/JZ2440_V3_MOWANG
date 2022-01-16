/**************************************************************************************************************************
  * @brief      : 	JZ2440v2开发板LCD显示字体代码头文件
  * @version    : 	V0.0
  * @note       : 	无
  * @history    : 	无
***************************************************************************************************************************/

#ifndef _FONT_H
#define _FONT_H

/**************************************************************************************************************************
  * @brief       : 	初始化字体功能，得到lcd显示屏的参数
  * @param[in]   : 	无
  * @param[out]  : 	无
  * @return      : 	无
  * @others      : 	无
***************************************************************************************************************************/
void InitFont(void);

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
void PrintFbString8x16(int x, int y, char* str, unsigned int color,int isDefer);

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
void ClearFbString8x16(int x, int y, char* str, int length, UINT32 color, int isDefer);
#endif /* _FONT_H */