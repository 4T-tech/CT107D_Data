#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "i2c.h"

unsigned int  uiTms;              	// 毫秒值
unsigned int  uiSec;              	// 秒值
unsigned char ucState;            	// 系统状态
unsigned int  uiAdc;              	// ADC值（*100）
unsigned char ucVp_Val;            	// 电压参数值（*10）
unsigned char ucVp_Old;            	// 电压参数旧值（*10）
unsigned char ucVp_Cnt;           	// 计数值
unsigned int  uiVp_Sec;           	// 计数计时值

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
void Data_Proc(void);

void main(void)
{ 
  Close_Peripheral();
  T1_Init();

  AT24C02_ReadBuffer((unsigned char*)&ucVp_Val, 0, 1);
  if ((ucVp_Val > 50) || ((ucVp_Val % 5) != 0))
    ucVp_Val = 25;                 	// 参数越界处理

  uiAdc = PCF8591_Adc(3) / 0.51;  	// 500/255
  if(uiAdc > (ucVp_Val*10))       	// 初始状态处理
    ucVp_Old = 0;
  else
    ucVp_Old = 1;

  while(1)
  {
    Seg_Proc();
    Key_Proc();
    Led_Proc();
    Data_Proc();
  }
}

unsigned char pucSeg_Char[12];    	// 显示字符
unsigned char pucSeg_Code[8];     	// 显示代码
unsigned char ucSeg_Pos;          	// 显示位置
unsigned int  uiSeg_Dly;          	// 显示刷新延时
unsigned char ucSeg_Dly;          	// 显示移位延时
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)             	// 300ms刷新1次
  {
    uiSeg_Dly = 0;

    switch (ucState)
    {
      case 0:                       // 数据界面
        sprintf(pucSeg_Char, "U    %3.2f", uiAdc/100.0);
        break;
      case 1:                       // 参数界面
        sprintf(pucSeg_Char, "P    %3.2f", ucVp_Val/10.0);
        break;
      case 2:                       // 计数界面
        sprintf(pucSeg_Char, "N%7u", (int)ucVp_Cnt);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)             	// 2ms移位1次
  {
    ucSeg_Dly = 0;

    Seg_Disp(pucSeg_Code[ucSeg_Pos], ucSeg_Pos);
    ucSeg_Pos = ++ucSeg_Pos & 7;  	// 数码管循环显示
  }
}

unsigned char ucKey_Old;          	// 旧键值
unsigned char ucKey_Dly;          	// 按键刷新延时
unsigned char ucKey_Cnt;          	// 按键计数
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// 按下键值

  if(ucKey_Dly < 10)              	// 延时10ms消抖
    return;
  ucKey_Dly = 0;

  ucKey_Dn = Key_Read();          	// 键值读取
  if (ucKey_Dn != ucKey_Old)      	// 键值变化
  {
    ucKey_Old = ucKey_Dn;
  } else {
    ucKey_Dn = 0;
  }

  switch (ucKey_Dn)
  {
    case 12:                        // S12按键
      if(++ucState == 3)          	// 切换显示
        ucState = 0;
      if(ucState == 2)              // 离开参数界面，保存参数
        AT24C02_WriteBuffer((unsigned char*)&ucVp_Val, 0, 1);
      ucKey_Cnt = 0;
      break;
    case 13:                        // S13按键
      if(ucState == 2)              // 计数界面
      {
        ucVp_Cnt = 0;               // 清零计数
        ucKey_Cnt = 0;
      }
      else
        ucKey_Cnt++;
      break;
    case 16:                        // S16按键
      if(ucState == 1)              // 参数界面
      {
        ucVp_Val += 5;              // 参数增加
        if(ucVp_Val >= 55)
          ucVp_Val = 0;
        ucKey_Cnt = 0;
      }
      else
        ucKey_Cnt++;
      break;
    case 17:                        // S17按键
      if(ucState == 1)
      {
        if(ucVp_Val == 0)           // 参数范围：0~5V
          ucVp_Val = 55;
        ucVp_Val -= 5;              // 参数减少
        ucKey_Cnt = 0;
      }
      else
        ucKey_Cnt++;
  }
}

unsigned char ucLed;              	// LED值
void Led_Proc(void)
{
  if(((uiSec - uiVp_Sec) > 5) && (ucVp_Old == 1))
    ucLed |= 1;                     // L1点亮
  else
    ucLed &= 0xfe;                 	// L1熄灭

  if((ucVp_Cnt & 1) == 1)
    ucLed |= 2;                     // L2点亮
  else
    ucLed &= 0xfd;                 	// L2熄灭

  if(ucKey_Cnt >= 3)
    ucLed |= 4;                     // L3点亮
  else
    ucLed &= 0xfb;                  // L3熄灭

  Led_Disp(ucLed);
}

unsigned char ucData_Dly;         	// 数据采集延时
void Data_Proc(void)
{
  unsigned char ucVp_Key;

  if(ucData_Dly < 100)
    return;
  ucData_Dly = 0;

  uiAdc = PCF8591_Adc(3) / 0.51;  	// 500/255
  if((uiAdc / 10) > ucVp_Val)
    ucVp_Key = 0;
  else
    ucVp_Key = 1;

  if(ucVp_Key != ucVp_Old)
  {
    ucVp_Old = ucVp_Key;
    if(ucVp_Key == 1)
    {
      ucVp_Cnt++;
      uiVp_Sec = uiSec; 
    }
  }
}
