#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds18b20.h"
#include "i2c.h"

unsigned char ucState;            	// 系统状态
unsigned int  uiTemp;              	// 温度值(*16)
unsigned int  uiDac;               	// DAC值(*100)
unsigned char ucPara=25, ucPara1=25;	// 温度参数： 在退出参数设置界面时生效

void Seg_Proc(void);
void Key_Proc(void);
void Data_Proc(void);

void main(void)
{
  Close_Peripheral();
  T1_Init();

  while(1)
  {
    Seg_Proc();
    Key_Proc();
    Data_Proc();
  }
}

unsigned char pucSeg_Char[12];    	// 显示字符
unsigned char pucSeg_Code[8];      	// 显示代码
unsigned char ucSeg_Pos;            // 显示位置
unsigned int  uiSeg_Dly;            // 显示刷新延时
unsigned char ucSeg_Dly;            // 显示移位延时
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)             	// 300ms刷新1次
  {
    uiSeg_Dly = 0;

    switch(ucState)
    {
      case 0:                       // 温度显示
        sprintf(pucSeg_Char, "C   %04.2f", uiTemp/16.0);
        break;
      case 1:                       // 参数显示
        sprintf(pucSeg_Char, "P     %02u", (int)ucPara1);
        break;
      case 2:                       // DAC显示
        sprintf(pucSeg_Char, "A    %03.2f", uiDac/100.0);
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
unsigned char ucLed=3;             	// LED值：模式1（L1点亮），温度显示（L2点亮）
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// 按下键值

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
    case 4:                         // S4
      if(++ucState == 3)           	// 循环切换状态
        ucState = 0;
      if(ucState == 2)             	// 设定的温度参数在退出参数设置界面时生效
        ucPara = ucPara1;
      ucLed &= 1;
      ucLed |= 1 << ucState+1;     	// 设置LED指示
      break;
    case 5:                         // S5
      ucLed ^= 1;                   // 切换模式（LED1）
      break;
    case 8:                         // S8
      if(ucState == 1)              // S8按键仅在参数设置界面有效
        if(ucPara1 != 0)
          ucPara1--;
      break;
    case 9:                         // S9
      if(ucState == 1)              // S9按键仅在参数设置界面有效
        ucPara1++;
  }
  Led_Disp(ucLed);
}

unsigned int  uiData_Dly;         	// 数据采集延时
void Data_Proc(void)
{
  unsigned char ucTemp;

  if (uiData_Dly < 400)             // 400ms时间未到
    return;
  uiData_Dly = 0;

  uiTemp = Temp_Read();           	// 读取温度
  ucTemp = uiTemp >> 4;           	// 整数部分

  if((ucLed & 1) == 1)            	// 模式1
  {
    if(ucTemp < ucPara)
      uiDac = 0;                    // 0V
    else
      uiDac = 500;                  // 5V
  }
  else                              // 模式2
  {
    if(ucTemp < 20)
      uiDac = 100;                  // 1V
    else if(ucTemp >= 40)
      uiDac = 400;                  // 4V
    else
      uiDac = (ucTemp * 15) - 200;
  }
  PCF8591_Dac(uiDac * 0.51);      	// 255/500
}
