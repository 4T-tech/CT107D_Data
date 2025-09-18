#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds1302.h"
#include "i2c.h"

unsigned int  uiSec;               	// 秒值
unsigned char ucState;            	// 系统状态
unsigned int  uiFreq;             	// 频率测量值
  signed int  siFreq;             	// 频率值
unsigned int  uiPF=2000;           	// 超限参数
  signed int  siCL;                	// 校准参数
unsigned int  uiFmax;              	// 频率最大值
unsigned char pucRtc[3];          	// 实时时钟值
unsigned char ucTmax[3];          	// 频率最大实时时钟值

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
unsigned char pucSeg_Code[8];      	// 显示代码
unsigned char ucSeg_Pos;           	// 显示位置
unsigned int  uiSeg_Dly;            // 显示刷新延时
unsigned char ucSeg_Dly;            // 显示移位延时
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)            	// 300ms刷新1次
  {
    uiSeg_Dly = 0;

    switch (ucState)
    {
      case 0:                       // 频率界面
        if (siFreq < 0)
          sprintf(pucSeg_Char, "F     LL");
        else
          sprintf(pucSeg_Char, "F  %5u", siFreq);
        break;
      case 0x10:                    // 超限参数界面
        sprintf(pucSeg_Char, "P1  %4u", uiPF);
        break;
      case 0x11:                    // 校准参数界面
        sprintf(pucSeg_Char, "P2  %4d", siCL);
        break;
      case 0x20:                    // 时间界面
        sprintf(pucSeg_Char, "%02x-%02x-%02x",
          (int)pucRtc[0], (int)pucRtc[1], (int)pucRtc[2]);
        break;
      case 0x30:                    // 频率回显界面
        sprintf(pucSeg_Char, "HF %5u", uiFmax);
        break;
      case 0x31:                    // 时间回显界面
        sprintf(pucSeg_Char, "HA%02x%02x%02x",
          (int)ucTmax[0], (int)ucTmax[1], (int)ucTmax[2]);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)               // 2ms移位1次
  {
    ucSeg_Dly = 0;

    Seg_Disp(pucSeg_Code[ucSeg_Pos], ucSeg_Pos);
    ucSeg_Pos = ++ucSeg_Pos & 7;   	// 数码管循环显示
  }
}

unsigned char ucKey_Old;          	// 旧键值
unsigned char ucKey_Dly;           	// 按键刷新延时
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// 按下键值

  if (ucKey_Dly < 10)             	// 延时10ms消抖
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
    case 4:                         // S4按键按下
      ucState &= 0xf0;              // 清除次状态
      ucState += 0x10;              // 修改主状态
      if (ucState >= 0x40)
        ucState = 0;
      break;
    case 5:                         // S5按键按下
      if ((ucState&0x10) != 0)     	// 参数或回显界面
        ucState ^= 1;               // 修改次状态
      break;
    case 8:                         // S8按键按下
      switch (ucState)
      {
        case 0x10:                  // 超限参数
          if (uiPF < 9000)
            uiPF += 1000;
          break;
        case 0x11:                  // 校准参数
          if (siCL < 900)
            siCL += 100;
      }
      break;
    case 9:                         // S9按键按下
      switch (ucState)
      {
        case 0x10:                  // 超限参数
          if (uiPF > 1000)
            uiPF -= 1000;
          break;
        case 0x11:                  // 校准参数
          if (siCL > -900)
            siCL -= 100;
      }
  }
}

unsigned char ucLed;              	// LED值
unsigned char ucLed_Dly;          	// LED闪烁延时
void Led_Proc(void)
{
  if (ucLed_Dly < 200)             	// 200ms刷新1次
    return;
  ucLed_Dly = 0;

  if (ucState == 0)                 // 频率界面
    ucLed ^= 1;                     // L1翻转
  else
    ucLed &= ~1;                    // L1熄灭

  if (siFreq < 0)                   // 负频率
    ucLed |= 2;                     // L2点亮
  else if (siFreq < uiPF)
    ucLed &= ~2;                    // L2熄灭
  else
    ucLed ^= 2;                     // L2翻转
  Led_Disp(ucLed);                  // LED显示状态
}

unsigned int  uiData_Dly;         	// 数据刷新延时
void Data_Proc(void)
{
  unsigned long ulDac;            	// DAC输出值

  if (uiData_Dly < 100)            	// 100ms刷新1次
    return;
  uiData_Dly = 0;

  DS1302_GetRtc(pucRtc);

  if (uiFreq > 31867)
    return;

  siFreq = uiFreq + siCL;
  if ((siFreq > 0) && (siFreq > uiFmax))
  {
    uiFmax = siFreq;
    ucTmax[0] = pucRtc[0];
    ucTmax[1] = pucRtc[1];
    ucTmax[2] = pucRtc[2];
  }
  if (siFreq < 0)
    ulDac = 0;
  else if (siFreq < 500)
    ulDac = 51;                     // 1V
  else if (siFreq > uiPF)
    ulDac = 255;                    // 5V
  else
  {
    ulDac = siFreq*4+uiPF-2500;
    ulDac *= 51;
    ulDac /= uiPF-500;
  }
  PCF8591_Dac(ulDac);
}
