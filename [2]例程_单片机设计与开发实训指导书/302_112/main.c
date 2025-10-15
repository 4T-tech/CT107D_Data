#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds1302.h"
#include "ds18B20.h"
#include "i2c.h"

unsigned int  uiTms;              	// 毫秒值
unsigned int  uiSec;              	// 秒值
unsigned char ucState;            	// 系统状态
unsigned char pucRtc[3] = {0x16, 0x59, 0x50};
unsigned int  uiTemp;              	// 温度值
unsigned char ucAdc;               	// ADC值
unsigned char ucRds, ucRdt;       	// 亮暗状态
unsigned char ucHour=17, ucHour1=17;  // 时间参数
unsigned char ucTemp=25, ucTemp1=25; 	// 温度参数
unsigned char ucLedp=4,  ucLedp1=4;   // LED参数

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
// 主函数
void main(void)
{
  Close_Peripheral();
  T1_Init();
  DS1302_SetRtc(pucRtc);          	// 设置RTC时钟

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
unsigned char ucSeg_Dly;           	// 显示移位延时
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)              // 300ms刷新1次
  {
    uiSeg_Dly = 0;

    switch (ucState)
    {
      case 0:                       // 显示时间
        sprintf(pucSeg_Char, "%02u-%02u-%02u",\
          (int)pucRtc[0], (int)pucRtc[1], (int)pucRtc[2]);
        break;
      case 1:                       // 显示温度
        sprintf(pucSeg_Char, "C    %03.1f", uiTemp/16.0);
        break;
      case 2:                       // 显示亮暗状态
        sprintf(pucSeg_Char, "E %3.2f%3u", ucAdc/51.0, (int)ucRds);
        break;
      case 0x10:                    // 显示时间参数
        sprintf(pucSeg_Char, "P1    %02u", (int)ucHour1);
        break;
      case 0x11:                    // 显示温度参数
        sprintf(pucSeg_Char, "P2    %02u", (int)ucTemp1);
        break;
      case 0x12:                    // 显示LED参数
        sprintf(pucSeg_Char, "P3     %1u", (int)ucLedp1);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)              	// 2ms移位1次
  {
    ucSeg_Dly = 0;

    Seg_Disp(pucSeg_Code[ucSeg_Pos], ucSeg_Pos);
    ucSeg_Pos = ++ucSeg_Pos & 7;  	// 数码管循环显示
  }
}

unsigned char ucKey_Old;          	// 旧键值
unsigned char ucKey_Dly;           	// 按键刷新延时
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// 按下键值

  if (ucKey_Dly < 10)             	// 10ms时间未到
    return;                         // 延时消抖
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
    case 4:                         // S4按键
      ucState ^= 0x10;             	// 切换数据和参数界面
      ucState &= ~3;
      ucState |= 2;
    case 5:                         // S5按键
      if ((++ucState & 3) == 3)   	// 切换数据或参数子界面状态
        ucState &= ~3;
      ucHour = ucHour1;           	// 保存参数
      ucTemp = ucTemp1;
      ucLedp = ucLedp1;
      break;
    case 8:                         // S8按键
      switch (ucState)             	// 参数减1
      {
        case 0x10:                  // 修改时间参数
          if (ucHour1 > 0)
            --ucHour1;
          else
            ucHour1 = 23;
          break;
        case 0x11:                  // 修改温度参数
          if (ucTemp1 > 0)
            --ucTemp1;
          else
            ucTemp1 = 99;
          break;
        case 0x12:                  // 修改LED参数
          if (ucLedp1 > 4)
            --ucLedp1;
          else
            ucLedp1 = 8;
      }
      break;
    case 9:                         // S9按键
      switch (ucState)             	// 参数加1
      {
        case 0x10:                  // 修改时间参数
          if (ucHour1 < 23)
            ++ucHour1;
          else
            ucHour1 = 0;
          break;
        case 0x11:                  // 修改温度参数
          if (ucTemp1 < 99)
            ++ucTemp1;
          else
            ucTemp1 = 0;
          break;
        case 0x12:                  // 修改LED参数
          if (ucLedp1 < 8)
            ++ucLedp1;
          else
            ucLedp1 = 4;
      }
  }
}

unsigned char ucLed;              	// LED值
unsigned int  uiLed_Dly;          	// LED延时
void Led_Proc(void)
{
  unsigned long ulTime;

  if (uiLed_Dly < 500)             	// 500ms刷新1次
    return;
  uiLed_Dly = 0;

  DS1302_GetRtc(pucRtc);
  pucRtc[0] = (pucRtc[0]>>4)*10 + (pucRtc[0]&0xf);
  pucRtc[1] = (pucRtc[1]>>4)*10 + (pucRtc[1]&0xf);
  pucRtc[2] = (pucRtc[2]>>4)*10 + (pucRtc[2]&0xf);
  ulTime = (pucRtc[0]*60+pucRtc[1])*60+pucRtc[2];
  if ((ulTime >= ucHour*3600) || (ulTime <= 8*3600))
    ucLed |= 1;
  else
    ucLed &= ~1;

  uiTemp = Temp_Read();
  if (uiTemp < (ucTemp<<4))
    ucLed |= 2;
  else
    ucLed &= ~2;

  ucAdc = PCF8591_Adc(1);
  if (ucAdc > 70)
  {
    ucRds = 0;                      // 亮状态
    ucLed &= ~(1 << (ucLedp-1));  	// 熄灭指定的LED
  }
  else
  {
    ucRds = 1;                      // 暗状态
    ucLed |= 1 << (ucLedp-1);      	// 点亮指定的LED
  }
  if (ucRds != ucRdt)
  {
    uiSec = 0;                      // 状态变化开始计时
    ucRdt = ucRds;
  }
  if (uiSec >=3)                    // 持续时间超过3s
    if (ucAdc > 140)
      ucLed &= ~4;
    else
      ucLed |= 4;

  Led_Disp(ucLed);                  // LED显示状态
}
