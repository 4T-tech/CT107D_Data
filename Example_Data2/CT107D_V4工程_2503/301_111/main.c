#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "i2c.h"

unsigned int  uiTms;              	// ����ֵ
unsigned int  uiSec;              	// ��ֵ
unsigned char ucState;            	// ϵͳ״̬
unsigned int  uiAdc;              	// ADCֵ��*100��
unsigned char ucVp_Val;            	// ��ѹ����ֵ��*10��
unsigned char ucVp_Old;            	// ��ѹ������ֵ��*10��
unsigned char ucVp_Cnt;           	// ����ֵ
unsigned int  uiVp_Sec;           	// ������ʱֵ

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
void Data_Proc(void);

void main(void)
{ 
  Close_Peripheral();
  T1_Init();

  AT24C02_ReadBuffer((unsigned char*)&ucVp_Val, 0, 1);
  if ((ucVp_Val > 50) || ((ucVp_Val % 5) != 0))
    ucVp_Val = 25;                 	// ����Խ�紦��

  uiAdc = PCF8591_Adc(3) / 0.51;  	// 500/255
  if(uiAdc > (ucVp_Val*10))       	// ��ʼ״̬����
    ucVp_Old = 0;
  else
    ucVp_Old = 1;

  while(1)
  {
    Seg_Proc();
    Key_Proc();
    Led_Proc();
    Data_Proc();
  }
}

unsigned char pucSeg_Char[12];    	// ��ʾ�ַ�
unsigned char pucSeg_Code[8];     	// ��ʾ����
unsigned char ucSeg_Pos;          	// ��ʾλ��
unsigned int  uiSeg_Dly;          	// ��ʾˢ����ʱ
unsigned char ucSeg_Dly;          	// ��ʾ��λ��ʱ
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)             	// 300msˢ��1��
  {
    uiSeg_Dly = 0;

    switch (ucState)
    {
      case 0:                       // ���ݽ���
        sprintf(pucSeg_Char, "U    %3.2f", uiAdc/100.0);
        break;
      case 1:                       // ��������
        sprintf(pucSeg_Char, "P    %3.2f", ucVp_Val/10.0);
        break;
      case 2:                       // ��������
        sprintf(pucSeg_Char, "N%7u", (int)ucVp_Cnt);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)             	// 2ms��λ1��
  {
    ucSeg_Dly = 0;

    Seg_Disp(pucSeg_Code[ucSeg_Pos], ucSeg_Pos);
    ucSeg_Pos = ++ucSeg_Pos & 7;  	// �����ѭ����ʾ
  }
}

unsigned char ucKey_Old;          	// �ɼ�ֵ
unsigned char ucKey_Dly;          	// ����ˢ����ʱ
unsigned char ucKey_Cnt;          	// ��������
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// ���¼�ֵ

  if(ucKey_Dly < 10)              	// ��ʱ10ms����
    return;
  ucKey_Dly = 0;

  ucKey_Dn = Key_Read();          	// ��ֵ��ȡ
  if (ucKey_Dn != ucKey_Old)      	// ��ֵ�仯
  {
    ucKey_Old = ucKey_Dn;
  } else {
    ucKey_Dn = 0;
  }

  switch (ucKey_Dn)
  {
    case 12:                        // S12����
      if(++ucState == 3)          	// �л���ʾ
        ucState = 0;
      if(ucState == 2)              // �뿪�������棬�������
        AT24C02_WriteBuffer((unsigned char*)&ucVp_Val, 0, 1);
      ucKey_Cnt = 0;
      break;
    case 13:                        // S13����
      if(ucState == 2)              // ��������
      {
        ucVp_Cnt = 0;               // �������
        ucKey_Cnt = 0;
      }
      else
        ucKey_Cnt++;
      break;
    case 16:                        // S16����
      if(ucState == 1)              // ��������
      {
        ucVp_Val += 5;              // ��������
        if(ucVp_Val >= 55)
          ucVp_Val = 0;
        ucKey_Cnt = 0;
      }
      else
        ucKey_Cnt++;
      break;
    case 17:                        // S17����
      if(ucState == 1)
      {
        if(ucVp_Val == 0)           // ������Χ��0~5V
          ucVp_Val = 55;
        ucVp_Val -= 5;              // ��������
        ucKey_Cnt = 0;
      }
      else
        ucKey_Cnt++;
  }
}

unsigned char ucLed;              	// LEDֵ
void Led_Proc(void)
{
  if(((uiSec - uiVp_Sec) > 5) && (ucVp_Old == 1))
    ucLed |= 1;                     // L1����
  else
    ucLed &= 0xfe;                 	// L1Ϩ��

  if((ucVp_Cnt & 1) == 1)
    ucLed |= 2;                     // L2����
  else
    ucLed &= 0xfd;                 	// L2Ϩ��

  if(ucKey_Cnt >= 3)
    ucLed |= 4;                     // L3����
  else
    ucLed &= 0xfb;                  // L3Ϩ��

  Led_Disp(ucLed);
}

unsigned char ucData_Dly;         	// ���ݲɼ���ʱ
void Data_Proc(void)
{
  unsigned char ucVp_Key;

  if(ucData_Dly < 100)
    return;
  ucData_Dly = 0;

  uiAdc = PCF8591_Adc(3) / 0.51;  	// 500/255
  if((uiAdc / 10) > ucVp_Val)
    ucVp_Key = 0;
  else
    ucVp_Key = 1;

  if(ucVp_Key != ucVp_Old)
  {
    ucVp_Old = ucVp_Key;
    if(ucVp_Key == 1)
    {
      ucVp_Cnt++;
      uiVp_Sec = uiSec; 
    }
  }
}
