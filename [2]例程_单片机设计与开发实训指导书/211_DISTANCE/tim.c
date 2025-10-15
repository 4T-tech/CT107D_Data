sfr  TCON = 0x88;
sfr  TMOD = 0x89;
sfr  AUXR = 0x8E;
sfr  TL1  = 0x8B;
sfr  TH1  = 0x8D;
sbit TR1  = TCON^6;
sbit TF1  = TCON^7;

sfr  IE   = 0xA8;
sbit ET1  = IE^3;
sbit EA   = IE^7;

void T1_Init(void)                  	// 1毫秒@12.000MHz
{
  AUXR &= 0xBF;                     	// 定时器时钟12T模式
  TMOD &= 0x0F;                     	// 设置定时器模式
  TL1 = 0x18;                       	// 设置定时初值（24）
  TH1 = 0xFC;                       	// 设置定时初值（252）
  TF1 = 0;                          	// 清除TF1标志
  TR1 = 1;                           	// 定时器1开始计时
  ET1 = 1;                           	// 允许T1中断
  EA  = 1;                           	// 允许系统中断
}

extern unsigned int  uiTms;         	// 毫秒值
extern unsigned int  uiSec;         	// 秒值
extern unsigned int  uiSeg_Dly;     	// 显示刷新延时
extern unsigned char ucSeg_Dly;     	// 显示移位延时
extern unsigned char ucKey_Dly;     	// 按键刷新延时
extern unsigned int  uiKey_Dly;     	// 双击或长按延时
void T1_Proc(void) interrupt 3
{
  if (++uiTms == 1000)              	// 1s时间到
  {
    uiTms = 0;
    uiSec++;
  }
  uiSeg_Dly++;
  ucSeg_Dly++;
  ucKey_Dly++;
  uiKey_Dly++;
}

sfr  CCON   = 0xD8;                   	// PCA控制寄存器
sfr  CL      = 0xE9;                 	// PCA低8位定时值
sfr  CH      = 0xF9;                 	// PCA高8位定时值
sbit CR     = CCON^6;               	// PCA运行控制位
sbit CF     = CCON^7;               	// PCA溢出标志位

sfr  P1     = 0x90;
sbit TX     = P1^0;
sbit RX     = P1^1;

unsigned char Dist_Meas(void)
{
  unsigned char ucNum = 10;

  // TX引脚发送40KHz方波信号驱动超声波发送探头
  TX = 0;
  CL = 0xf4;                          	// 设置PCA低8位定时值
  CH = 0xff;                          	// 设置PCA高8位定时值
  CR = 1;                             	// 启动PCA
  while (ucNum--)
  {
    while (!CF);                      	// 等待PCA定时时间到
    TX ^= 1;                           	// 翻转TX
    CL = 0xf4;                        	// 重置PCA低8位定时值
    CH = 0xff;                        	// 重置PCA高8位定时值
    CF = 0;
  }
  CR = 0;                             	// 关闭PCA
  // 接收计时
  CL = 0;                              	// 设置PCA低8位定时值
  CH = 0;                             	// 设置PCA高8位定时值
  CR = 1;
  while(RX && !CF);                 	// 等待下降沿
  CR = 0;
  return ((CH<<8)+CL)*0.017;        	// 计算距离：340*100/1000000/2
}
