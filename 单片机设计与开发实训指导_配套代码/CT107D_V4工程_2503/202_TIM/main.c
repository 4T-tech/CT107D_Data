// ʹ�ó���ǰ����J13����ΪIOģʽ��2-3�Ŷ̽ӣ�
#include "tim.h"

unsigned int  uiTms;                	// ����ֵ
unsigned int  uiSec;                	// ��ֵ
unsigned int  uiDuty;               	// ���ȵȼ���ռ�ձȣ�

sfr P0 = 0x80;
sfr P2 = 0xA0;
// P0���: ucData�������ݣ�ucAddr������ַ��4~7��
void P0_Out(unsigned char ucData, unsigned char ucAddr)
{
  P0 = ucData;                      	// P0�������
  P2 |= ucAddr << 5;                	// ��λP27~P25
  P2 &= 0x1f;                       	// ��λP27~P25
//XBYTE[ucAddr << 13] = ucData;     	// MMģʽ��J13-2��J13-1������
}
// �ر�����
void Close_Peripheral(void)
{
  P2 &= 0x1f;                       	// ��λP27~P25
  P0_Out(0xff, 4);                  	// Ϩ��LED
  P0_Out(~0x50, 5);                 	// �رռ̵����ͷ�����
}
// LED��ʾ
void Led_Disp(unsigned char ucLed)
{
  P0_Out(~ucLed, 4);                	// LED��ʾ
}

void Led_Proc(void);
// ������
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
