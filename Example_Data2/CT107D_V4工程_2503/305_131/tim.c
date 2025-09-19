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

void T1_Init(void)                  	// 1����@12.000MHz
{
  AUXR &= 0xBF;                     	// ��ʱ��ʱ��12Tģʽ
  TMOD &= 0x0F;                     	// ���ö�ʱ��ģʽ
  TL1 = 0x18;                       	// ���ö�ʱ��ֵ��24��
  TH1 = 0xFC;                       	// ���ö�ʱ��ֵ��252��
  TF1 = 0;                          	// ���TF1��־
  TR1 = 1;                           	// ��ʱ��1��ʼ��ʱ
  ET1 = 1;                           	// ����T1�ж�
  EA  = 1;                           	// ����ϵͳ�ж�
}

       unsigned int  uiTms;         	// ����ֵ
       unsigned int  uiSec;         	// ��ֵ
extern unsigned int  uiSeg_Dly;     	// ��ʾˢ����ʱ
extern unsigned char ucSeg_Dly;     	// ��ʾ��λ��ʱ
extern unsigned char ucKey_Dly;     	// ����ˢ����ʱ
extern unsigned char ucLed_Dly;       // LED��ʱ
void T1_Proc(void) interrupt 3
{
  if (++uiTms == 1000)              	// 1sʱ�䵽
  {
    uiTms = 0;
    uiSec++;
  }
  uiSeg_Dly++;
  ucSeg_Dly++;
  ucKey_Dly++;
  ucLed_Dly++;
}
