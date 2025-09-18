#define test

#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "uart.h"
#include "i2c.h"
#include "math.h"

unsigned int  uiSec;                	// 秒值
unsigned char ucState;              	// 系统状态
unsigned char ucSdev;               	// 设备状态：
// 0-空闲状态, 1-运行准备, 2-运行处理, 3-运行结束, 4-等待状态
unsigned int  idata uiCstaX;        	// 起始X坐标
unsigned int  idata uiCstaY;        	// 起始Y坐标
unsigned int  idata uiCtarX;        	// 目的X坐标
unsigned int  idata uiCtarY;        	// 目的Y坐标
unsigned int  idata uiCdevX;        	// 设备X位置
unsigned int  idata uiCdevY;        	// 设备Y位置
unsigned int  idata uiSpeed;        	// 行进速度*10
unsigned char ucScen;               	// 场景
unsigned char ucR=10;               	// 参数R*10
  signed char scB;                   	// 参数B
unsigned char ucDist;               	// 障碍物距离
            bit bRun;                  	// 运行标志

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
void Uart_Proc(void);
void Data_Proc(void);
// 主函数
void main(void)
{
  Close_Peripheral();
  T1_Init();
  Uart_Init();
  T0_Init();

  while (1)
  {
    Seg_Proc();
    Key_Proc();
    Led_Proc();
    Uart_Proc();
    Data_Proc();
  }
}

unsigned char pucSeg_Char[12];      	// 显示字符
unsigned char pucSeg_Code[8];       	// 显示代码
unsigned char ucSeg_Pos;            	// 显示位置
unsigned int  uiSeg_Dly;             	// 显示刷新延时
unsigned char ucSeg_Dly;            	// 显示移位延时
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)              	// 300ms刷新1次
  {
    uiSeg_Dly = 0;

    switch (ucState)
    {
      case 0:                         // 坐标界面
#ifndef test
        if (ucSdev == 0)            	// 空闲状态显示设备坐标
#else
        if (ucSdev == 2)            	// 运行状态显示设备坐标
#endif
          sprintf(pucSeg_Char, "L%3u-%3u", uiCdevX, uiCdevY);
        else                          // 其他状态显示目的坐标
          sprintf(pucSeg_Char, "L%3u-%3u", uiCtarX, uiCtarY);
        break;
      case 1:                         // 速度界面
        switch (ucSdev)
        {
          case 2:                     // 运行处理显示速度
            sprintf(pucSeg_Char, "E1 %4u.%1u", uiSpeed/10, uiSpeed%10);
            break;
          case 0:                     // 空闲状态
#ifndef test
            sprintf(pucSeg_Char, "E2 -----");
#else
            sprintf(pucSeg_Char, "E2 %4u.%1u", uiSpeed/10, uiSpeed%10);
#endif
            break;
          case 4:                     // 等待状态显示距离
#ifndef test
            sprintf(pucSeg_Char, "E3 %5u", (int)ucDist);
#else
            sprintf(pucSeg_Char, "E3 %1u %3u", (int)ucScen, (int)ucDist);
#endif
        }
        break;
      case 2:                         // 参数界面（R参数有效）
      case 3:                         // 参数界面（B参数有效）
        sprintf(pucSeg_Char, "P %2.1f %3d", ucR/10.0, (int)scB);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
}

unsigned char ucKey_Old;            	// 旧键值
unsigned char ucKey_Dly;            	// 按键刷新延时
void Key_Proc(void)
{
  unsigned char ucKey_Dn;           	// 按下键值

  if(ucKey_Dly < 10)                 	// 延时10ms消抖
    return;
  ucKey_Dly = 0;

  ucKey_Dn = Key_Read();            	// 键值读取
  if (ucKey_Dn != ucKey_Old)        	// 键值变化
  {
    ucKey_Old = ucKey_Dn;
  } else {
    ucKey_Dn = 0;
  }

  switch (ucKey_Dn)
  {
    case 4:                           // S4按键按下
      switch (ucSdev)
      {
        case 0:                       // 空闲状态
          if (bRun)                   // 有坐标数据
            ucSdev = 1;              	// 运行准备
          break;
        case 2:                       // 运行处理
          ucSdev = 4;                	// 等待状态
          break;
        case 4:                       // 等待状态
          if (ucDist > 30)
            ucSdev = 2;               // 运行处理
      }
      break;
    case 5:                           // S5按键按下
      if (ucSdev == 0)               	// 空闲状态
      {
        uiCstaX = uiCstaY = 0;      	// 重置起始坐标
        uiCdevX = uiCdevY = 0;      	// 重置设备坐标
      }
      break;
    case 8:                          	// S8按键按下
      if (++ucState >= 3)           	// 切换界面
        ucState = 0;
      break;
    case 9:                          	// S9按键按下
      if ((ucState == 2) || (ucState == 3))	// 参数界面
        ucState ^= 1;
      if (ucState == 1)             	// 速度界面
        ucSdev ^= 4;                 	// 切换设备状态（测试用）
      break;
    case 12:                         	// S9按键按下
      if (ucState == 2)             	// R参数
      {
        if (ucR < 20)
          ucR++;                      // 调整R参数
      }
      else if (ucState == 3)        	// B参数
        if (scB < 90)
          scB += 5;                   // 调整B参数
      break;
    case 13:                          // S9按键按下
      if (ucState == 2)              	// R参数
      {
        if (ucR > 10)
          ucR--;                      // 调整R参数
      }
      else if (ucState == 3)        	// B参数
        if (scB > -90)
          scB -= 5;                   // 调整B参数
  }
}

unsigned char ucLed;                	// LED值
unsigned char ucLed_Dly;            	// LED延时
void Led_Proc(void)
{
  if(ucLed_Dly < 100)               	// 延时100ms
    return;
  ucLed_Dly = 0;

  switch (ucSdev)
  {
    case 0:                           // 空闲状态
    case 3:
      ucLed &= ~1;                   	// L1熄灭
      break;
    case 2:                           // 运行状态
      ucLed |= 1;                     // L1点亮
      break;
    case 4:                           // 等待状态
      ucLed ^= 1;                     // L1闪烁
  }

  if ((ucSdev == 2) && (ucScen == 0)) // 夜间行进
    ucLed |= 2;                      	// L2点亮
  else
    ucLed &= ~2;                     	// L2熄灭

  if (ucSdev == 3)                  	// 运行结束
  {
    if (uiSec < 3)
      ucLed |= 4;                    	// L3点亮
    else
    {
      ucLed &= ~4;                   	// L3熄灭
      ucSdev = 0;
      bRun = 0;
    }
  }
  Led_Disp(ucLed);

  if (ucSdev == 2)                   	// 运行状态
    Uln_Ctrl(0x10);                  	// 继电器吸合
  else
    Uln_Ctrl(0);                    	// 继电器断开
}

unsigned int  uiUsec;               	// UART发送延时
unsigned char pucUart_Buf[10];      	// UART接收值
unsigned char ucUart_Num;           	// 串行口接收字符计数
//unsigned char pucUart_Buf[10] = {"(130,420)"};	// UART接收值
//unsigned char ucUart_Num=0x89;    	// 串行口接收字符计数（9个字符）
void Uart_Proc(void)
{
  unsigned char i, m, n;
  unsigned int  c[2];

  if (uiUsec != uiSec)              	// 测试用
  {
    uiUsec = uiSec;
#ifdef test
    printf("%03u\r\n", uiSec);
#endif
  }

  Uart_RecvString();
  if (ucUart_Num <= 0x80)           	// 接收未完成
    return;
  if (ucUart_Num == 0x81)           	// ?或#
  {
    if (pucUart_Buf[0] == '?')      	// 查询设备状态
      switch (ucSdev)
      {
        case 0:
          printf("Idle");
          break;
        case 2:
          printf("Busy");
          break;
        case 4:
          printf("Wait");
      }
    else if (pucUart_Buf[0] == '#') 	// 查询设备位置
      printf("(%u,%u)", uiCdevX, uiCdevY);
    else
      printf("Error");
  } else if (pucUart_Buf[0] == '(') 	// 设置目的地坐标
  {
    m = 0;
    n = 0;
    ucUart_Num -= 0x81;
    for (i=1; i<=ucUart_Num; i++)   	// 解析数据
    {
      if ((pucUart_Buf[i] == ',') || (pucUart_Buf[i] == ')'))
      {
        c[n] = 0;
        switch (i-m-1)                // 数据位数
        {
          case 3:
            c[n] += (pucUart_Buf[i-3]-'0')*100;
          case 2:
            c[n] += (pucUart_Buf[i-2]-'0')*10;
          case 1:
            c[n] += pucUart_Buf[i-1]-'0';
        }
        m = i;
        n++;
      }
    }
    if ((n == 2) && (ucSdev == 0))
    {
      uiCtarX = c[0];                	// 目的X坐标
      uiCtarY = c[1];                	// 目的Y坐标
      printf("Got it");
      bRun = 1;
    } else
      printf("Busy");
  }
  else
    printf("Error");
  ucUart_Num = 0;
}

char putchar(char c)
{
  Uart_SendChar(c);
  return c;
}

unsigned char ucData_Dly;           	// 数据采集计时
unsigned int  uiFreq;               	// 频率值
  signed int  siDistX;              	// 距离X
  signed int  siDistY;              	// 距离Y
unsigned int  uiDist;               	// 距离*10
unsigned int  uiDcur;               	// 当前距离*10
void Data_Proc(void)                 	// 数据处理
{
  float fProg;

  if (ucData_Dly < 100)               // 100ms未到
    return;
  ucData_Dly = 0;

  if (ucSdev == 1)                  	// 运行准备
  {
    siDistX = uiCtarX-uiCstaX;
    siDistY = uiCtarY-uiCstaY;
    uiDist = sqrt(pow(siDistX, 2)+pow(siDistY, 2)) * 10;
    uiDcur = 0;
    ucSdev = 2;                      	// 运行处理
  }

  uiSpeed = 3.14*ucR*uiFreq/100+scB*10;  	// 速度*10
  if (ucSdev == 2)                  	// 运行处理
  {
    if (uiSpeed > 0)
    {
      uiDcur += uiSpeed / 10;        	// 100ms运行距离: (usSpeed/10)*0.1s*10
      if (uiDcur < uiDist)
      {                               // 按比例计算当前位置
        fProg = (float)uiDist/uiDcur;
        uiCdevX = siDistX/fProg+uiCstaX;
        uiCdevY = siDistY/fProg+uiCstaY;
      } else {
        uiCstaX = uiCtarX;
        uiCstaY = uiCtarY;
        uiCdevX = uiCtarX;
        uiCdevY = uiCtarY;
        ucSdev = 3;                 	// 运行结束
        uiSec = 0;
      }
    }
  }

  ucDist = Dist_Meas();
  if ((ucDist < 30) && (ucSdev == 2))
    ucSdev = 4;                     	// 等待状态

  if (PCF8591_Adc(1)*10/51 > 12)
    ucScen = 1;                     	// 日间场景
  else
    ucScen = 0;                     	// 夜间场景
}
