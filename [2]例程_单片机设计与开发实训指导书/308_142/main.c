#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds18b20.h"
#include "i2c.h"

unsigned int  uiSec;              	// 秒值
unsigned char ucState;            	// 系统状态
unsigned int  uiTemp;             	// 温度值
unsigned char ucDrec;             	// 记录状态
  signed char scDist;             	// 距离值
  signed char scCali=0;           	// 校准值
unsigned char ucPdist=40;         	// 距离参数
unsigned char ucPtemp=30;         	// 温度参数
unsigned int  uiSpeed=340;        	// 速度值
unsigned char ucVol=10;           	// 下限值(*10)

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
void Data_Proc(void);
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
    Data_Proc();
  }
}

unsigned char pucSeg_Char[12];    	// 显示字符
unsigned char pucSeg_Code[8];     	// 显示代码
unsigned char ucSeg_Pos;           	// 显示位置
unsigned int  uiSeg_Dly;          	// 显示刷新延时
unsigned char ucSeg_Dly;          	// 显示移位延时
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)             	// 300ms刷新1次
  {
    uiSeg_Dly = 0;

    switch (ucState)
    {
      case 0:                       // 测距界面（cm）
//      sprintf(pucSeg_Char, "%3.1f- %3u",
//        uiTemp/16.0, (signed)scDist);
        sprintf(pucSeg_Char, "%3.1f-%1u%3u",
          uiTemp/16.0, (int)ucDrec, (signed)scDist);
        break;                      // 增加记录状态显示
      case 1:                       // 测距界面（m）
        sprintf(pucSeg_Char, "%3.1f- %3.2f", uiTemp/16.0, scDist/100.0);
        break;
      case 0x10:                    // 距离参数
        sprintf(pucSeg_Char, "P1    %2u", (int)ucPdist);
        break;
      case 0x11:                    // 温度参数
        sprintf(pucSeg_Char, "P2    %2u", (int)ucPtemp);
        break;
      case 0x20:                    // 校准值设置
        sprintf(pucSeg_Char, "F1   %3d", (signed)scCali);
        break;
      case 0x21:                    // 速度设置
        sprintf(pucSeg_Char, "F2  %4u", (int)uiSpeed);
        break;
      case 0x22:                    // DAC输出设置
        sprintf(pucSeg_Char, "F3    %2.1f", (int)ucVol/10.0);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
}

unsigned char ucKey_Old;          	// 旧键值
unsigned char ucKey_Dly;          	// 按键刷新延时
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// 按下键值

  if (ucDrec == 1)                 	// 记录数据状态
    return;

  if(ucKey_Dly < 10)              	// 延时10ms消抖
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
    case 4:                         // S4按键按下
      ucState &= 0xf0;             	// 清除次状态
      ucState += 0x10;             	// 修改主状态
      if (ucState == 0x30)
        ucState = 0;
      break;
    case 5:                         // S5按键按下
      switch (ucState & 0xf0)
      {
        case 0:
        case 0x10:
          ucState ^= 1;             // 切换次状态
          break;
        case 0x20:
          ucState++;                // 修改次状态
          if (ucState == 0x23)
            ucState = 0x20;
      }
      break;
    case 8:                         // S8按键按下
      switch (ucState)
      {
        case 0:
          ucDrec = 1;               // 记录数据
          break;
        case 0x10:
          if (ucPdist < 90)
            ucPdist += 10;         	// 距离参数增加
          break;
        case 0x11:
          if (ucPtemp < 80)
            ucPtemp++;              // 温度参数增加
          break;
        case 0x20:
          if (scCali < 90)
            scCali += 5;            // 校准值增加
          break;
        case 0x21:
          if (uiSpeed < 9990)
            uiSpeed += 10;          // 速度值增加
          break;
        case 0x22:
          if (ucVol < 20)
            ucVol++;                // 下限值增加
      }
      uiSec = 0;
      break;
    case 9:                         // S9按键按下
      switch (ucState)
      {
        case 0:
          if (ucDrec == 2)
            ucDrec = 3;             // 输出数据
          break;
        case 0x10:
          if (ucPdist > 10)
            ucPdist -= 10;          // 距离参数减少
          break;
        case 0x11:
          if (ucPtemp > 0)
            ucPtemp--;              // 温度参数减少
          break;
        case 0x20:
          if (scCali > -90)
            scCali -= 5;            // 校准值减少
          break;
        case 0x21:
          if (uiSpeed > 10)
            uiSpeed -= 10;          // 速度值减少
          break;
        case 0x22:
          if (ucVol > 1)
            ucVol--;                // 下限值减少
      }
      uiSec = 0;
  }
  if ((ucKey_Old == 20) && (uiSec >= 2))
  {                                 // S8+S9按下2s
    ucState = 0;                    // 测距界面
    ucPdist = 40;                   // 距离参数
    ucPtemp = 30;                   // 温度参数
    uiSpeed = 340;                  // 速度值
    ucVol = 10;                     // 下限值(*10)
    scCali = 0;                     // 校准值
    ucDrec = 0;                     // 记录状态
  }
}

unsigned char ucLed;              	// LED值
unsigned char ucLed_Dly;          	// LED延时
void Led_Proc(void)
{
  if(ucLed_Dly < 100)              	// 延时100ms
    return;
  ucLed_Dly = 0;

  switch (ucState & 0xf0)
  {
    case 0:
      ucLed = scDist;
      break;
    case 0x10:
      ucLed = 0x80;
      break;
    case 0x20:
      ucLed &= 1;
      ucLed ^= 1;
  }
  Led_Disp(ucLed);
}

unsigned int  uiSec1;             	// 秒值1
  signed char scData[7];          	// 距离值记录
unsigned char ucUln;              	// ULN值
void Data_Proc(void)
{
  if (uiSec1 == uiSec)
    return;
  uiSec1 = uiSec;

  uiTemp = Temp_Read();
  scDist = Dist_Meas()*uiSpeed/340 + scCali;

  if ((scDist >= ucPdist-5) && (scDist <= ucPdist+5)
    && (uiTemp <= (ucPtemp<<4)))
    ucUln = 0x10;                   // 继电器闭合
  else
    ucUln = 0;                      // 继电器断开
  Uln_Ctrl(ucUln);

  switch (ucDrec)
  {
    case 1:                         // 记录数据
      scData[uiSec] = scDist;
      if (uiSec == 6)
        ucDrec = 2;                 // 完成记录
      break;
    case 3:                         // 输出数据
      if (scData[uiSec] < 10)
        PCF8591_Dac(ucVol * 5.1);  	// ucVol*255/50
      else if (scData[uiSec] > 90)
        PCF8591_Dac(255);           // 5V
      else
        PCF8591_Dac(255 - (50-ucVol) * (90 - scData[uiSec]) * 0.06375);
      if (uiSec == 6)
        ucDrec = 2;                 // 完成输出
  }
}
