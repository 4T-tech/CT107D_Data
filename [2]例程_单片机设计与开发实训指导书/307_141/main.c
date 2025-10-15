#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds1302.h"
#include "ds18B20.h"
#include "i2c.h"

unsigned int  uiTms;              	// 毫秒值
unsigned int  uiSec;               	// 秒值
unsigned char ucState;            	// 系统状态
unsigned char pucRtc[3] = {0x23, 0x59, 0x50};
unsigned char ucAdc;              	// ADC值
unsigned int  uiFreq;             	// 频率值
unsigned char ucCnt;              	// 触发次数
unsigned char ucTemp;             	// 温度采集值
unsigned char ucTmax;             	// 温度最大值
unsigned char ucTcur;             	// 温度当前值
unsigned int  uiTsum;             	// 温度累加值
unsigned char ucTpre;             	// 温度历史值
unsigned char ucHumi;             	// 湿度采集值
unsigned char ucHmax;             	// 湿度最大值
unsigned int  uiHsum;             	// 湿度累加值
unsigned char ucHcur;             	// 湿度当前值
unsigned char ucHpre;             	// 湿度历史值
unsigned char ucPara=30;          	// 温度参数值
unsigned char ucHour;             	// 采集时
unsigned char ucMinu;             	// 采集分

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
void Data_Proc(void);
// 主函数
void main(void)
{
  Close_Peripheral();
  T1_Init();
  T0_Init();
  DS1302_SetRtc(pucRtc);          	// 设置RTC时钟

  while (1)
  {
    Seg_Proc();
    Key_Proc();
    Led_Proc();
    Data_Proc();
  }
}

unsigned char pucSeg_Char[12];    	// 显示字符
unsigned char pucSeg_Code[8];     	// 显示代码
unsigned char ucSeg_Pos;           	// 显示位置
unsigned int  uiSeg_Dly;           	// 显示刷新延时
unsigned char ucSeg_Dly;           	// 显示移位延时
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)            	// 300ms刷新1次
  {
    uiSeg_Dly = 0;
 
    switch (ucState)
    {
      case 0:                       // 时间界面
        sprintf(pucSeg_Char, "%02x-%02x-%02x",\
          (int)pucRtc[0], (int)pucRtc[1], (int)pucRtc[2]);
        break;
      case 1:                       // 温度界面（增加功能）
        sprintf(pucSeg_Char, "C     %02u", (int)ucTemp);
        break;
      case 2:                       // 亮暗界面（增加功能）
        sprintf(pucSeg_Char, "A    %03u", (int)ucAdc);
        break;
      case 3:                       // 频率界面（增加功能）
        sprintf(pucSeg_Char, "F  %05u", uiFreq);
        break;
      case 0x10:                    // 温度回显
        if (ucCnt == 0)
          sprintf(pucSeg_Char, "C       ");
        else
          sprintf(pucSeg_Char, "C %02u-%3.1f", \
            (int)ucTmax, (float)uiTsum/ucCnt);
       break;
      case 0x11:                    // 温度回显
        if (ucCnt == 0)
          sprintf(pucSeg_Char, "H       ");
        else
          sprintf(pucSeg_Char, "H %02u-%3.1f", \
            (int)ucHmax, (float)uiHsum/ucCnt);
        break;
      case 0x12:                    // 时间回显
        if (ucCnt == 0)
          sprintf(pucSeg_Char, "F%02u     ", (int)ucCnt);
        else
          sprintf(pucSeg_Char, "F%02u%02x-%02x",
            (int)ucCnt, (int)ucHour, (int)ucMinu);
        break;
      case 0x20:                    // 参数界面
        sprintf(pucSeg_Char, "P     %02u", (int)ucPara);
        break;
      case 0x50:                    // 温湿度界面
        sprintf(pucSeg_Char, "E  %02u-%02u", (int)ucTcur, (int)ucHcur);
        if (ucHumi == 0)
          pucSeg_Char[6] = pucSeg_Char[7] = 'A';
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)              	// 2ms移位1次
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

  if (ucState == 0x50)
    return;

  if (ucKey_Dly < 10)             	// 10ms时间未到
    return;                        	// 延时消抖
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
      ucState &= 0xf0;
      ucState += 0x10;              // 切换主界面
      if (ucState == 0x30)
        ucState = 0;
      break;
    case 5:                         // S5按键
      if ((ucState & 0xf0) == 0)
        if (++ucState == 4)        	// 切换时间界面
          ucState = 0;
      if ((ucState & 0xf0) == 0x10)
        if (++ucState == 0x13)     	// 切换回显界面
          ucState = 0x10;
      break;
    case 8:                         // S8按键
      if (ucState == 0x20)        	// 参数界面
        if (++ucPara == 100)
          ucPara = 0;
      break;
    case 9:                         // S9按键
      if (ucState == 0x20)        	// 参数界面
        if (ucPara == 0)
          ucPara = 99;
        else
          ucPara--;
      if (ucState == 0x12)        	// 时间回显
        uiSec = 0;
  }
  if (ucState == 0x12)             	// 时间回显
    if ((ucKey_Old == 9) && (uiSec >= 2))
    {
      ucTmax = ucHmax = 0;        	// 清除记录数据
      uiTsum = uiHsum = 0;
      ucCnt = 0;
    }
}

unsigned char ucLed;              	// LED值
unsigned int  uiLed_Dly;          	// LED刷新延时
void Led_Proc(void)
{
  if (uiLed_Dly < 100)             	// 100ms时间到
    return;
  uiLed_Dly = 0;

  if ((ucState & 0xf0) == 0)
    ucLed |= 1;                     // L1操作
  else
    ucLed &= ~1;

  if ((ucState & 0xf0) == 0x10)
    ucLed |= 2;                     // L2操作
  else
    ucLed &= ~2;

  if (ucState == 0x50)
    ucLed |= 4;                     // L3操作
  else
    ucLed &= ~4;

  if (ucTemp > ucPara)
    ucLed ^= 8;                     // L4操作
  else
    ucLed &= ~8;

  if ((uiFreq < 200) || (uiFreq > 2000))
    ucLed |= 0x10;                 	// L5操作
  else
    ucLed &= ~0x10;

  if ((ucCnt >= 2) && (ucHcur > ucHpre) && (ucTcur > ucTpre))
    ucLed |= 0x20;                  // L6操作
  else
    ucLed &= ~0x20;

  Led_Disp(ucLed);                 	// LED显示状态
}

bit bLsta;                          // 亮暗当前状态
bit bLold;                          // 亮暗历史状态
unsigned int  uiData_Dly;          	// 数据刷新延时
void Data_Proc(void)
{
  static unsigned char ucSold;    	// 原系统状态

  if (uiData_Dly < 300)             // 300ms时间未到
    return;
  uiData_Dly = 0;

  DS1302_GetRtc(pucRtc);          	// 获取RTC时钟
  ucTemp = Temp_Read()>>4;
  if ((uiFreq > 200) && (uiFreq < 2000))
    ucHumi = uiFreq * 10 / 225 + 1.1;
  else
    ucHumi = 0;

  ucAdc = PCF8591_Adc(1);
  if (ucAdc > 80)
    bLsta = 1;                      // 亮状态
  else
    bLsta = 0;                      // 暗状态
  if ((bLold != bLsta) && (uiSec > 3))
  {
    bLold = bLsta;                  // 保存亮暗状态
    if (bLsta == 0)
    {
      if (ucHumi != 0)
      {
        ucCnt++;
        ucHour = pucRtc[0];         // 保存数据
        ucMinu = pucRtc[1];
        if (ucTemp > ucTmax)
          ucTmax = ucTemp;
        if (ucHumi > ucHmax)
          ucHmax = ucHumi;
        uiTsum += ucTemp;           // 累加数据
        uiHsum += ucHumi;
        ucTpre = ucTcur;
        ucHpre = ucHcur;
      }
      ucTcur = ucTemp;
      ucHcur = ucHumi;
      ucSold = ucState;             // 保存状态
      ucState = 0x50;
      uiSec = 0;
    }
  }
  if ((ucState == 0x50) && (uiSec > 3))
    ucState = ucSold;               // 恢复状态
}
