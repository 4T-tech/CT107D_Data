#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "i2c.h"

unsigned int  uiSec;               	// ��ֵ
unsigned char ucState;            	// ϵͳ״̬
unsigned int  uiFreq;              	// Ƶ��ֵ
unsigned char ucFreq=90;          	// Ƶ�ʲ���(/100)
unsigned char ucHumi;             	// ʪ��ֵ
unsigned char ucHumi1=40;         	// ʪ�Ȳ���
unsigned char ucDist;              	// ����ֵ
unsigned char ucDist1=60;         	// �������
unsigned char ucNum;               	// �̵������ش���

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
// ������
void main(void)
{
  Close_Peripheral();
  T1_Init();
  T0_Init();
  T2_Init();

  while (1)
  {
    Seg_Proc();
    Key_Proc();
    Led_Proc();
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

    switch(ucState)
    {
      case 0:
        sprintf(pucSeg_Char, "F %6u", uiFreq);
        break;
      case 1:
        sprintf(pucSeg_Char, "F %5u.%1u", uiFreq/1000, uiFreq%1000);
        break;
      case 0x10:
//      sprintf(pucSeg_Char, "H    %3u", (int)ucHumi);
        sprintf(pucSeg_Char, "H %2u %3u", (int)ucNum, (int)ucHumi);
        break;                      // ���Ӽ̵������ش�����ʾ
      case 0x20:
//      sprintf(pucSeg_Char, "A    %3u", (int)ucDist);
        sprintf(pucSeg_Char, "A %2u %3u", (int)ucNum, (int)ucDist);
        break;                      // ���Ӽ̵������ش�����ʾ
      case 0x21:
        sprintf(pucSeg_Char, "A    %3.2f", ucDist/100.0);
        break;
      case 0x30:
        sprintf(pucSeg_Char, "P1   %2u.%1u", ucFreq/10, ucFreq%10);
        break;
      case 0x31:
        sprintf(pucSeg_Char, "P2    %02u", (int)ucHumi1);
        break;
      case 0x32:
        sprintf(pucSeg_Char, "P3    %2.1f", ucDist1/100.0);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
}

unsigned char ucKey_Old;          	// �ɼ�ֵ
unsigned char ucKey_Dly;          	// ������ʱ
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// ���¼�ֵ

  if(ucKey_Dly < 10)                // ��ʱ10ms����
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
    case 4:                         // S4����
      ucState &= 0x30;
      ucState += 0x10;              // �л�����
      if (ucState >= 0x40)
        ucState = 0;
      break;
    case 5:                         // S5����
      if (ucState >= 0x30)
        if (++ucState >= 0x33)      // �л�����
          ucState = 0x30;
      break;
    case 8:                         // S8����
      switch (ucState)
      {
        case 0x20:
        case 0x21:
          ucState ^= 1;             // �л����뵥λ
          break;
        case 0x30:
          ucFreq += 5;
          if (ucFreq > 120)
            ucFreq = 10;
          break;
        case 0x31:
          ucHumi1 += 10;
          if (ucHumi1 > 60)
            ucHumi1 = 10;
          break;
        case 0x32:
          ucDist1 += 10;
          if (ucDist1 > 120)
            ucDist1 = 10;
      }
      break;
    case 9:                         // S9����
      switch (ucState)
      {
        case 0:
        case 1:
          ucState ^= 1;             // �л�Ƶ�ʵ�λ
          break;
        case 0x30:
          ucFreq -= 5;
          if (ucFreq < 10)
            ucFreq = 120;
          break;
        case 0x31:
          ucHumi1 -= 10;
          if (ucHumi1 < 10)
            ucHumi1 = 60;
          break;
        case 0x32:
          ucDist1 -= 10;
          if (ucDist1 < 10)
            ucDist1 = 120;
      }
      uiSec = 0;                    // ������ʱ
  }
  if ((ucState == 0x10) && (ucKey_Old == 9) && (uiSec > 1))
  {
    ucNum = 0;                      // ����̵������ش���
    AT24C02_WriteBuffer((unsigned char*)&ucNum, 0, 1);
  }
}

unsigned char ucLed;               	// LEDֵ
unsigned char ucUln;              	// ULNֵ
unsigned char ucLed_Dly;          	// LED��ʱ
unsigned char ucDuty=2;           	// ռ�ձ�(/10)
void Led_Proc(void)
{
  if (ucLed_Dly < 100)
    return;
  ucLed_Dly = 0;

  if ((ucState&0xf0) == 0)          // Ƶ�ʽ���
    ucLed |= 1;                     // L1����
  else if (ucState == 0x30)         // Ƶ�ʲ�������
    ucLed ^= 1;                     // L1��˸
  else
    ucLed &= ~1;                    // L1Ϩ��

  if (ucState == 0x10)              // ʪ�Ƚ���
    ucLed |= 2;                     // L2����
  else if (ucState == 0x31)        	// ʪ�Ȳ�������
    ucLed ^= 2;                     // L2��˸
  else
    ucLed &= ~2;                    // L2Ϩ��

  if ((ucState&0xf0) == 0x20)      	// �������
    ucLed |= 4;                     // L3����
  else if (ucState == 0x32)        	// �����������
    ucLed ^= 4;                     // L3��˸
  else
    ucLed &= ~4;                    // L3Ϩ��

  if (uiFreq > ucFreq*100)
  {
    ucLed |= 8;                     // L4����
    ucDuty = 8;                     // ռ�ձ�80%
  } else {
    ucLed &= ~8;                    // L4Ϩ��
    ucDuty = 2;                     // ռ�ձ�20%
  }

  ucHumi = PCF8591_Adc(3)*100/255;
  if (ucHumi > ucHumi1)
  {
    ucLed |= 0x10;                 	// L5����
    if (ucHumi > 80)
      PCF8591_Dac(255);            	// 5V
    else
      PCF8591_Dac(255-204*(80-ucHumi)/(80-ucHumi1));
  } else {
    ucLed &= ~0x10;                	// L5Ϩ��
    PCF8591_Dac(51);               	// 1V
  }

  ucDist = Dist_Meas();
  if (ucDist > ucDist1)
  {
    ucLed |= 0x20;                  // L6����
    if ((ucUln&0x10) == 0)         	// �̵����Ͽ�
    {
      ucUln |= 0x10;                // �̵����պ�
      ucNum++;                      // �̵������ش���
      AT24C02_WriteBuffer((unsigned char*)&ucNum, 0, 1);
    }
  } else {
    ucLed &= ~0x20;                 // L6Ϩ��
    if ((ucUln&0x10) != 0)          // �̵����պ�
    {
      ucUln &= ~0x10;               // �̵����Ͽ�
      ucNum++;                      // �̵������ش���
      AT24C02_WriteBuffer((unsigned char*)&ucNum, 0, 1);
    }
  }

  Led_Disp(ucLed);
  Uln_Ctrl(ucUln);
}
