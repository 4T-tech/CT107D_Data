#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds1302.h"
#include "i2c.h"

unsigned char ucState;            	// 系统状态
unsigned char pucRtc[3] = {0x23, 0x59, 0x50};
unsigned char ucMode;             	// 模式: 0-触发，1-定时
unsigned char ucDist;             	// 距离值
unsigned char ucAdc;               	// ADC值
unsigned char ucType;             	// 数据类型
unsigned char ucMax;              	// 最大值
unsigned char ucMin=255;          	// 最小值
unsigned int  uiSum;              	// 数据累加
unsigned char ucNum;              	// 数据数量
unsigned char ucCnt;              	// 连续计数
unsigned char ucTime[5]={2, 3, 5, 7, 9};
unsigned char ucTime1, ucTime2;   	// 时间参数
unsigned char ucDist1=20, ucDist2=20; // 距离参数

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
void Data_Proc(void);
// 主函数
void main(void)
{
  Close_Peripheral();
  T1_Init();
  DS1302_SetRtc(pucRtc);           	// 设置RTC时钟

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
  if (uiSeg_Dly > 200)            	// 200ms时间到
  {
    uiSeg_Dly = 0;

    switch (ucState)
    {
      case 0:                       // 显示时间
        sprintf(pucSeg_Char, "%02x-%02x-%02x",\
          (int)pucRtc[0], (int)pucRtc[1], (int)pucRtc[2]);
        break;
      case 1:                       // 显示距离
        if (ucMode == 0)
          sprintf(pucSeg_Char, "LC   %3u", (int)ucDist);
        else
          sprintf(pucSeg_Char, "LF   %3u", (int)ucDist);
        break;
      case 2:                       // 显示数据
        switch (ucType)
        {
          case 0:
            sprintf(pucSeg_Char, "H^  %4u", (int)ucMax);
            break;
          case 1:
            if (ucNum == 0)
              sprintf(pucSeg_Char, "H-  127.5");
            else
              sprintf(pucSeg_Char, "H-  %3u.%1u", uiSum/ucNum, uiSum%ucNum);
            break;
          case 2:
            sprintf(pucSeg_Char, "H_  %4u", (int)ucMin);
        }
        break;
      case 0x10:                    // 显示时间参数
        sprintf(pucSeg_Char, "P1    %02u",
          (unsigned int)ucTime[ucTime2]);
        break;
      case 0x11:                    // 显示距离参数
        sprintf(pucSeg_Char, "P2    %02u", (int)ucDist2);
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
unsigned char ucKey_Dly;           	// 按键刷新延时
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// 按下键值

  if (ucKey_Dly < 10)               // 延时10ms消抖
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
    case 4:                         // S4按键
      ucState ^= 0x10;
      ucState &= ~3;
      if ((ucState & 0x10) == 0)
        ucState |= 2;
      else
        ucState |= 1;
    case 5:                         // S5按键
      ucState++;
      if ((ucState & 0x10) == 0)
      {
        if (ucState == 2)         	// 记录显示
          ucType = 0;               // 最大值
        if (ucState == 3)         	// 3个数据界面
          ucState = 0;
      }
      else
      {
        if (ucState == 0x12)      	// 2个设置界面
          ucState = 0x10;
      }
      ucTime1 = ucTime2;
      ucDist1 = ucDist2;
      break;
    case 8:                         // S8按键
      if (ucState == 1)
        ucMode ^= 1;               	// 切换触发和定时模式
      if (ucState == 2)
        if (++ucType == 3)        	// 切换数据类型
          ucType = 0;
      break;
    case 9:                         // S9按键
      if (ucState == 0x10)
        if (++ucTime2 == 5)       	// 修改时间参数
          ucTime2 = 0;
      if (ucState == 0x11)
      {
        ucDist2 += 10;              // 修改距离参数
        if (ucDist2 == 90)
          ucDist2 = 10;
      }
  }
}

unsigned char ucLed;              	// LED值
void Led_Proc(void)
{
  if (ucState < 3)
    ucLed = 1<<ucState;
  else
    ucLed = 0;

  if (ucMode == 0)
    ucLed |= 8;                     // L4点亮
  else
    ucLed &= ~8;                   	// L4熄灭

  if (ucCnt >= 3)
    ucLed |= 0x10;                 	// L5点亮
  else
    ucLed &= ~0x10;                	// L5熄灭

  if (ucAdc > 50)
    ucLed |= 0x20;                 	// L6点亮
  else
    ucLed &= ~0x20;                	// L6熄灭

  Led_Disp(ucLed);                 	// LED显示
}

unsigned int  uiData_Dly;         	// 数据采集延时
unsigned char ucFlag, ucFlag1;    	// 距离测量条件
void Data_Proc(void)
{
  unsigned int  uiDac;             	// DAC值(*100)

  if (uiData_Dly < 300)            	// 300ms时间未到
     return;
  uiData_Dly = 0;

  DS1302_GetRtc(pucRtc);           	// 获取RTC时钟
  if (ucMode == 1)
  {
    if ((pucRtc[2] % ucTime[ucTime1]) == 0)
      ucFlag = 1;                   // 整除
    else
      ucFlag = 0;
  }
  else
  {
    ucAdc = PCF8591_Adc(1);
    if (ucAdc < 50)
      ucFlag = 1;                   // 暗状态
    else
      ucFlag = 0;                   // 亮状态
  }

  ucDist = Dist_Meas();
  if (ucFlag != ucFlag1)          	// 状态变化
  {
    ucFlag1 = ucFlag;

    if (ucFlag == 1)               	// 满足距离测量条件
    {
      if (ucDist > ucMax)
        ucMax = ucDist;            	// 保存最大值
      if (ucDist < ucMin)
        ucMin = ucDist;            	// 保存最小值
      if (++ucNum != 0)
        uiSum += ucDist;           	// 数据累加
      else
      {
        uiSum = ucDist;            	// 重新累加
        ucNum =1;
      }
      if (ucDist < 10)
        uiDac = 100;                // 1V 
      else if(ucDist > 60) 
        uiDac = 500;                // 5V
      else
        uiDac = (ucDist * 8) + 20;
      PCF8591_Dac(uiDac * 0.51);   	// 255/500

      if (ucMode == 1)             	// 定时模式
      {
        if (((ucDist>ucDist1) && (ucDist-ucDist1)<5)
          || ((ucDist<ucDist1) && (ucDist1-ucDist)<5))
          ucCnt++;
        else
          ucCnt = 0;
      }
    }
  }
}
