#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "i2c.h"

unsigned int  uiSec;               	// 秒值
unsigned char ucState;            	// 系统状态
unsigned int  uiFreq;              	// 频率值
unsigned char ucFreq=90;          	// 频率参数(/100)
unsigned char ucHumi;             	// 湿度值
unsigned char ucHumi1=40;         	// 湿度参数
unsigned char ucDist;              	// 距离值
unsigned char ucDist1=60;         	// 距离参数
unsigned char ucNum;               	// 继电器开关次数

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
// 主函数
void main(void)
{
  Close_Peripheral();
  T1_Init();
  T0_Init();
  T2_Init();

  while (1)
  {
    Seg_Proc();
    Key_Proc();
    Led_Proc();
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

    switch(ucState)
    {
      case 0:
        sprintf(pucSeg_Char, "F %6u", uiFreq);
        break;
      case 1:
        sprintf(pucSeg_Char, "F %5u.%1u", uiFreq/1000, uiFreq%1000);
        break;
      case 0x10:
//      sprintf(pucSeg_Char, "H    %3u", (int)ucHumi);
        sprintf(pucSeg_Char, "H %2u %3u", (int)ucNum, (int)ucHumi);
        break;                      // 增加继电器开关次数显示
      case 0x20:
//      sprintf(pucSeg_Char, "A    %3u", (int)ucDist);
        sprintf(pucSeg_Char, "A %2u %3u", (int)ucNum, (int)ucDist);
        break;                      // 增加继电器开关次数显示
      case 0x21:
        sprintf(pucSeg_Char, "A    %3.2f", ucDist/100.0);
        break;
      case 0x30:
        sprintf(pucSeg_Char, "P1   %2u.%1u", ucFreq/10, ucFreq%10);
        break;
      case 0x31:
        sprintf(pucSeg_Char, "P2    %02u", (int)ucHumi1);
        break;
      case 0x32:
        sprintf(pucSeg_Char, "P3    %2.1f", ucDist1/100.0);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
}

unsigned char ucKey_Old;          	// 旧键值
unsigned char ucKey_Dly;          	// 按键延时
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// 按下键值

  if(ucKey_Dly < 10)                // 延时10ms消抖
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
    case 4:                         // S4按键
      ucState &= 0x30;
      ucState += 0x10;              // 切换界面
      if (ucState >= 0x40)
        ucState = 0;
      break;
    case 5:                         // S5按键
      if (ucState >= 0x30)
        if (++ucState >= 0x33)      // 切换参数
          ucState = 0x30;
      break;
    case 8:                         // S8按键
      switch (ucState)
      {
        case 0x20:
        case 0x21:
          ucState ^= 1;             // 切换距离单位
          break;
        case 0x30:
          ucFreq += 5;
          if (ucFreq > 120)
            ucFreq = 10;
          break;
        case 0x31:
          ucHumi1 += 10;
          if (ucHumi1 > 60)
            ucHumi1 = 10;
          break;
        case 0x32:
          ucDist1 += 10;
          if (ucDist1 > 120)
            ucDist1 = 10;
      }
      break;
    case 9:                         // S9按键
      switch (ucState)
      {
        case 0:
        case 1:
          ucState ^= 1;             // 切换频率单位
          break;
        case 0x30:
          ucFreq -= 5;
          if (ucFreq < 10)
            ucFreq = 120;
          break;
        case 0x31:
          ucHumi1 -= 10;
          if (ucHumi1 < 10)
            ucHumi1 = 60;
          break;
        case 0x32:
          ucDist1 -= 10;
          if (ucDist1 < 10)
            ucDist1 = 120;
      }
      uiSec = 0;                    // 长按计时
  }
  if ((ucState == 0x10) && (ucKey_Old == 9) && (uiSec > 1))
  {
    ucNum = 0;                      // 清零继电器开关次数
    AT24C02_WriteBuffer((unsigned char*)&ucNum, 0, 1);
  }
}

unsigned char ucLed;               	// LED值
unsigned char ucUln;              	// ULN值
unsigned char ucLed_Dly;          	// LED延时
unsigned char ucDuty=2;           	// 占空比(/10)
void Led_Proc(void)
{
  if (ucLed_Dly < 100)
    return;
  ucLed_Dly = 0;

  if ((ucState&0xf0) == 0)          // 频率界面
    ucLed |= 1;                     // L1点亮
  else if (ucState == 0x30)         // 频率参数界面
    ucLed ^= 1;                     // L1闪烁
  else
    ucLed &= ~1;                    // L1熄灭

  if (ucState == 0x10)              // 湿度界面
    ucLed |= 2;                     // L2点亮
  else if (ucState == 0x31)        	// 湿度参数界面
    ucLed ^= 2;                     // L2闪烁
  else
    ucLed &= ~2;                    // L2熄灭

  if ((ucState&0xf0) == 0x20)      	// 距离界面
    ucLed |= 4;                     // L3点亮
  else if (ucState == 0x32)        	// 距离参数界面
    ucLed ^= 4;                     // L3闪烁
  else
    ucLed &= ~4;                    // L3熄灭

  if (uiFreq > ucFreq*100)
  {
    ucLed |= 8;                     // L4点亮
    ucDuty = 8;                     // 占空比80%
  } else {
    ucLed &= ~8;                    // L4熄灭
    ucDuty = 2;                     // 占空比20%
  }

  ucHumi = PCF8591_Adc(3)*100/255;
  if (ucHumi > ucHumi1)
  {
    ucLed |= 0x10;                 	// L5点亮
    if (ucHumi > 80)
      PCF8591_Dac(255);            	// 5V
    else
      PCF8591_Dac(255-204*(80-ucHumi)/(80-ucHumi1));
  } else {
    ucLed &= ~0x10;                	// L5熄灭
    PCF8591_Dac(51);               	// 1V
  }

  ucDist = Dist_Meas();
  if (ucDist > ucDist1)
  {
    ucLed |= 0x20;                  // L6点亮
    if ((ucUln&0x10) == 0)         	// 继电器断开
    {
      ucUln |= 0x10;                // 继电器闭合
      ucNum++;                      // 继电器开关次数
      AT24C02_WriteBuffer((unsigned char*)&ucNum, 0, 1);
    }
  } else {
    ucLed &= ~0x20;                 // L6熄灭
    if ((ucUln&0x10) != 0)          // 继电器闭合
    {
      ucUln &= ~0x10;               // 继电器断开
      ucNum++;                      // 继电器开关次数
      AT24C02_WriteBuffer((unsigned char*)&ucNum, 0, 1);
    }
  }

  Led_Disp(ucLed);
  Uln_Ctrl(ucUln);
}
