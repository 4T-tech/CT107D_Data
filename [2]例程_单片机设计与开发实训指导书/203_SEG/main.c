// 使用程序前，将J13调整为IO模式（2-3脚短接）
#include "tim.h"
#include "seg.h"
#include <stdio.h>

unsigned int  uiTms;                	// 毫秒值
unsigned int  uiSec;                	// 秒值

void Led_Proc(void);
void Seg_Proc(void);
// 主函数
void main(void)
{
  Close_Peripheral();
  T1_Init();
	
  while (1)
  {
    T1_Proc();
    Seg_Proc();
    Led_Proc();
  }
}

unsigned char pucSeg_Char[12];      	// 显示字符
unsigned char pucSeg_Code[8];       	// 显示代码
unsigned char ucSeg_Pos;            	// 显示位置
unsigned int  uiSeg_Dly;            	// 显示刷新延时
unsigned char ucSeg_Dly;            	// 显示移位延时
// 注意：sprintf()会在字符串后面添加”\0”，所以pucSeg_Buf[]的长度至少应为9。
// 如果字符串中包含小数点，pucSeg_Buf[]的长度至少应为10。
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)              	// 300ms刷新1次
  {
    uiSeg_Dly = 0;

    sprintf(pucSeg_Char, "1.   %04u", uiSec);
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)               	// 2ms移位1次
  {
    ucSeg_Dly = 0;

    Seg_Disp(pucSeg_Code[ucSeg_Pos], ucSeg_Pos);
    ucSeg_Pos = ++ucSeg_Pos & 7;    	// 数码管循环显示
  }
}

void Led_Proc(void)
{
  Led_Disp(uiSec);
 }
