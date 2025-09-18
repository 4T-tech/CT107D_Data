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

       unsigned int  uiTms;         	// 毫秒值
       unsigned int  uiSec;         	// 秒值
extern unsigned int  uiSeg_Dly;     	// 显示刷新延时
extern unsigned char ucSeg_Dly;     	// 显示移位延时
extern unsigned char ucKey_Dly;     	// 按键刷新延时
extern unsigned char ucLed_Dly;       // LED延时
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
  ucLed_Dly++;
}
