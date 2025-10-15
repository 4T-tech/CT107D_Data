#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds1302.h"
#include "ds18B20.h"

unsigned char ucState;            	// 系统状态
unsigned char ucLed=2;            	// LED值, 温度控制模式
unsigned char pucRtc[3] = {0x23, 0x59, 0x30};
unsigned char ucRtc;              	// RTC标志
unsigned int  uiTemp;             	// 温度值
unsigned char ucTemp=23;          	// 温度参数

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
// 主函数
void main(void)
{
  Close_Peripheral();
  T1_Init();
  DS1302_SetRtc(pucRtc);

  while (1)
  {
    Seg_Proc();
    Key_Proc();
    Led_Proc();
  }
}

unsigned char pucSeg_Char[12];    	// 显示字符
unsigned char pucSeg_Code[8];     	// 显示代码
unsigned char ucSeg_Pos;           	// 显示位置
unsigned int  uiSeg_Dly;          	// 显示刷新延时
unsigned char ucSeg_Dly;          	// 显示移位延时
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)            	// 300ms刷新1次
  {
    uiSeg_Dly = 0;
 
    switch (ucState)
    {
      case 0:                       // 显示温度
        sprintf(pucSeg_Char, "U1   %03.1f", uiTemp/16.0);
        break;
      case 1:                       // 显示RTC时钟
        if (ucRtc == 0)            	// 显示时分
          sprintf(pucSeg_Char, "U2 %02x-%02x",\
            (unsigned int)pucRtc[0], (int)pucRtc[1]);
        else                        // 显示分秒
          sprintf(pucSeg_Char, "U2 %02x-%02x",\
            (unsigned int)pucRtc[1], (int)pucRtc[2]);
        break;
      case 2:                       // 显示参数
        sprintf(pucSeg_Char, "U3    %02u", (int)ucTemp);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)             	// 2ms移位1次
  {
    ucSeg_Dly = 0;

    Seg_Disp(pucSeg_Code[ucSeg_Pos], ucSeg_Pos);
    ucSeg_Pos = ++ucSeg_Pos & 7;
  }
}

unsigned char ucKey_Old;          	// 旧键值
unsigned char ucKey_Dly;          	// 按键延时
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// 按下键值

  if (ucKey_Dly < 10)             	// 延时10ms消抖
    return;
  ucKey_Dly = 0;

  ucKey_Dn = Key_Read();           	// 键值读取
  if (ucKey_Dn != ucKey_Old)      	// 键值变化
  {
    ucKey_Old = ucKey_Dn;
  } else {
    ucKey_Dn = 0;
  }

  switch (ucKey_Dn)
  {
    case 12:
      if (++ucState == 3)          	// S12按键切换状态
        ucState = 0;
      break;
    case 13:
      ucLed ^= 2;                   // S13按键切换模式
      break;
    case 16:
      if (ucState == 2)           	// 设置界面参数加1
        if (++ucTemp > 99)
          ucTemp = 10;
      break;
    case 17:
      if (ucState == 2)           	// 设置界面参数减1
        if (--ucTemp < 10)
          ucTemp = 99;
  }
  if (ucState == 1)
    if (ucKey_Old == 17)          	// 时间界面S17键切换时间显示
      ucRtc = 1;                    // 显示分秒
    else
      ucRtc = 0;                    // 显示时分
}

unsigned char ucUln;              	// ULN值
unsigned char ucLed_Dly;          	// LED延时
void Led_Proc(void)
{
  if (ucLed_Dly < 100)             	// 100ms时间未到
    return;
  ucLed_Dly = 0;

  DS1302_GetRtc(pucRtc);          	// 获取RTC时钟
  if ((pucRtc[1] == 0) && (pucRtc[2] < 5))
    ucLed |= 1;                     // L1点亮
  else
    ucLed &= ~1;                    // L1熄灭
  if ((ucLed & 2) == 2)            	// 温度控制模式
  {
    uiTemp = Temp_Read();          	// 读取温度
    if (uiTemp > (ucTemp<<4))
      ucUln = 0x10;                 // 继电器闭合
    else
      ucUln = 0;                    // 继电器断开
  } else {                          // 时间控制模式
    if ((pucRtc[1] == 0) && (pucRtc[2] < 5))
      ucUln = 0x10;                 // 继电器闭合
    else
      ucUln = 0;                    // 继电器断开
  }
  if (ucUln & 0x10)                	// 继电器闭合
    ucLed ^= 4;                     // L3闪烁
  else
    ucLed &= ~4;
  Led_Disp(ucLed);                 	// LED显示状态
  Uln_Ctrl(ucUln);                 	// ULN控制
}
