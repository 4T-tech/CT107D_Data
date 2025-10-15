// 使用程序前，将J13调整为IO模式（2-3脚短接）
#include "tim.h"

unsigned int  uiTms;                	// 毫秒值
unsigned int  uiSec;                	// 秒值
unsigned int  uiDuty;               	// 亮度等级（占空比）

sfr P0 = 0x80;
sfr P2 = 0xA0;
// P0输出: ucData——数据，ucAddr——地址（4~7）
void P0_Out(unsigned char ucData, unsigned char ucAddr)
{
  P0 = ucData;                      	// P0输出数据
  P2 |= ucAddr << 5;                	// 置位P27~P25
  P2 &= 0x1f;                       	// 复位P27~P25
//XBYTE[ucAddr << 13] = ucData;     	// MM模式（J13-2和J13-1相连）
}
// 关闭外设
void Close_Peripheral(void)
{
  P2 &= 0x1f;                       	// 复位P27~P25
  P0_Out(0xff, 4);                  	// 熄灭LED
  P0_Out(~0x50, 5);                 	// 关闭继电器和蜂鸣器
}
// LED显示
void Led_Disp(unsigned char ucLed)
{
  P0_Out(~ucLed, 4);                	// LED显示
}

void Led_Proc(void);
// 主函数
void main(void)
{
  Close_Peripheral();
  T1_Init();
	
  while (1)
  {
    T1_Proc();
    Led_Proc();
  }
}

void Led_Proc(void)
{
  if(uiTms < uiDuty)
    Led_Disp(0xff);
  else
    Led_Disp(0);
}
