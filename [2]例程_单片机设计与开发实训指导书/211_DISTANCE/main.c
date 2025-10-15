// 使用程序前，将J13调整为IO模式（2-3脚短接）
#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"

unsigned int  uiTms;                	// 毫秒值
unsigned int  uiSec;                	// 秒值
unsigned char ucState;              	// 系统状态
unsigned char ucDist;               	// 距离值

void Led_Proc(void);
void Seg_Proc(void);
void Key_Proc(void);
// 主函数
void main(void)
{
  Close_Peripheral();
  T1_Init();
	
  while (1)
  {
    Seg_Proc();
    Key_Proc();
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

    switch (ucState & 0xf0)
    {
      case 0:                        	// 显示T1时钟
        sprintf(pucSeg_Char, "1   %04u", (int)uiSec);
        break;
      case 0x10:                     	// 显示距离值
        ucDist = Dist_Meas();
        sprintf(pucSeg_Char, "2   %04u", (int)ucDist);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)               	// 2ms移位1次
  {
    ucSeg_Dly = 0;

    Seg_Disp(pucSeg_Code[ucSeg_Pos], ucSeg_Pos);
    ucSeg_Pos = ++ucSeg_Pos & 7;    	// 数码管循环显示
  }
}

unsigned char ucKey_Old;            	// 旧键值
unsigned char ucKey_Dly;            	// 按键刷新延时
unsigned int  uiKey_Dly=500;        	// 双击或长按延时
void Key_Proc(void)
{
  unsigned char ucKey_Dn;           	// 按下键值
  unsigned char ucKey_Up;           	// 松开键值

  if(ucKey_Dly < 10)                	// 延时10ms消抖
    return;
  ucKey_Dly = 0;

  ucKey_Dn = Key_Read();            	// 键值读取
  if (ucKey_Dn != ucKey_Old)        	// 键值变化
  {
    ucKey_Up = ucKey_Old;           	// 保存键值
    ucKey_Old = ucKey_Dn;
  } else {
    ucKey_Up = 0;                   	// 清除键值
    ucKey_Dn = 0;
  }

  if (ucKey_Dn == 4)                	// S4按下
  {
    ucState ^= 0x10;                	// 修改主状态（L5翻转）
    if (uiKey_Dly < 500)            	// S4双击
      ucState ^= 2;                 	// 修改次状态（L2翻转）
    uiKey_Dly = 0;                  	// 开始计时
  }

  if (ucKey_Up == 4)                	// S4松开
      ucState ^= 1;                 	// 修改次状态（L1翻转）
  if ((ucKey_Old == 4) && (uiKey_Dly > 1000))
  {                                 	// S4长按
    ucState ^= 4;                   	// 修改次状态（L3翻转）
    uiKey_Dly = 0;                  	// 重新计时
  }
}

void Led_Proc(void)
{
  Led_Disp(ucState);
}
