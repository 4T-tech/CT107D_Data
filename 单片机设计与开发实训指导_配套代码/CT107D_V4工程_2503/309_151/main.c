#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds1302.h"
#include "i2c.h"

unsigned int  uiSec;               	// ��ֵ
unsigned char ucState;            	// ϵͳ״̬
unsigned int  uiFreq;             	// Ƶ�ʲ���ֵ
  signed int  siFreq;             	// Ƶ��ֵ
unsigned int  uiPF=2000;           	// ���޲���
  signed int  siCL;                	// У׼����
unsigned int  uiFmax;              	// Ƶ�����ֵ
unsigned char pucRtc[3];          	// ʵʱʱ��ֵ
unsigned char ucTmax[3];          	// Ƶ�����ʵʱʱ��ֵ

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
void Data_Proc(void);
// ������
void main(void)
{
  Close_Peripheral();
  T1_Init();
  T0_Init();
  DS1302_SetRtc(pucRtc);          	// ����RTCʱ��

  while (1)
  {
    Seg_Proc();
    Key_Proc();
    Led_Proc();
    Data_Proc();
  }
}

unsigned char pucSeg_Char[12];    	// ��ʾ�ַ�
unsigned char pucSeg_Code[8];      	// ��ʾ����
unsigned char ucSeg_Pos;           	// ��ʾλ��
unsigned int  uiSeg_Dly;            // ��ʾˢ����ʱ
unsigned char ucSeg_Dly;            // ��ʾ��λ��ʱ
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)            	// 300msˢ��1��
  {
    uiSeg_Dly = 0;

    switch (ucState)
    {
      case 0:                       // Ƶ�ʽ���
        if (siFreq < 0)
          sprintf(pucSeg_Char, "F     LL");
        else
          sprintf(pucSeg_Char, "F  %5u", siFreq);
        break;
      case 0x10:                    // ���޲�������
        sprintf(pucSeg_Char, "P1  %4u", uiPF);
        break;
      case 0x11:                    // У׼��������
        sprintf(pucSeg_Char, "P2  %4d", siCL);
        break;
      case 0x20:                    // ʱ�����
        sprintf(pucSeg_Char, "%02x-%02x-%02x",
          (int)pucRtc[0], (int)pucRtc[1], (int)pucRtc[2]);
        break;
      case 0x30:                    // Ƶ�ʻ��Խ���
        sprintf(pucSeg_Char, "HF %5u", uiFmax);
        break;
      case 0x31:                    // ʱ����Խ���
        sprintf(pucSeg_Char, "HA%02x%02x%02x",
          (int)ucTmax[0], (int)ucTmax[1], (int)ucTmax[2]);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)               // 2ms��λ1��
  {
    ucSeg_Dly = 0;

    Seg_Disp(pucSeg_Code[ucSeg_Pos], ucSeg_Pos);
    ucSeg_Pos = ++ucSeg_Pos & 7;   	// �����ѭ����ʾ
  }
}

unsigned char ucKey_Old;          	// �ɼ�ֵ
unsigned char ucKey_Dly;           	// ����ˢ����ʱ
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// ���¼�ֵ

  if (ucKey_Dly < 10)             	// ��ʱ10ms����
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
    case 4:                         // S4��������
      ucState &= 0xf0;              // �����״̬
      ucState += 0x10;              // �޸���״̬
      if (ucState >= 0x40)
        ucState = 0;
      break;
    case 5:                         // S5��������
      if ((ucState&0x10) != 0)     	// ��������Խ���
        ucState ^= 1;               // �޸Ĵ�״̬
      break;
    case 8:                         // S8��������
      switch (ucState)
      {
        case 0x10:                  // ���޲���
          if (uiPF < 9000)
            uiPF += 1000;
          break;
        case 0x11:                  // У׼����
          if (siCL < 900)
            siCL += 100;
      }
      break;
    case 9:                         // S9��������
      switch (ucState)
      {
        case 0x10:                  // ���޲���
          if (uiPF > 1000)
            uiPF -= 1000;
          break;
        case 0x11:                  // У׼����
          if (siCL > -900)
            siCL -= 100;
      }
  }
}

unsigned char ucLed;              	// LEDֵ
unsigned char ucLed_Dly;          	// LED��˸��ʱ
void Led_Proc(void)
{
  if (ucLed_Dly < 200)             	// 200msˢ��1��
    return;
  ucLed_Dly = 0;

  if (ucState == 0)                 // Ƶ�ʽ���
    ucLed ^= 1;                     // L1��ת
  else
    ucLed &= ~1;                    // L1Ϩ��

  if (siFreq < 0)                   // ��Ƶ��
    ucLed |= 2;                     // L2����
  else if (siFreq < uiPF)
    ucLed &= ~2;                    // L2Ϩ��
  else
    ucLed ^= 2;                     // L2��ת
  Led_Disp(ucLed);                  // LED��ʾ״̬
}

unsigned int  uiData_Dly;         	// ����ˢ����ʱ
void Data_Proc(void)
{
  unsigned long ulDac;            	// DAC���ֵ

  if (uiData_Dly < 100)            	// 100msˢ��1��
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
