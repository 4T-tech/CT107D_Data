#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds1302.h"
#include "ds18B20.h"
#include "i2c.h"

unsigned int  uiTms;              	// ����ֵ
unsigned int  uiSec;              	// ��ֵ
unsigned char ucState;            	// ϵͳ״̬
unsigned char pucRtc[3] = {0x16, 0x59, 0x50};
unsigned int  uiTemp;              	// �¶�ֵ
unsigned char ucAdc;               	// ADCֵ
unsigned char ucRds, ucRdt;       	// ����״̬
unsigned char ucHour=17, ucHour1=17;  // ʱ�����
unsigned char ucTemp=25, ucTemp1=25; 	// �¶Ȳ���
unsigned char ucLedp=4,  ucLedp1=4;   // LED����

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
// ������
void main(void)
{
  Close_Peripheral();
  T1_Init();
  DS1302_SetRtc(pucRtc);          	// ����RTCʱ��

  while (1)
  {
    Seg_Proc();
    Key_Proc();
    Led_Proc();
  }
}

unsigned char pucSeg_Char[12];    	// ��ʾ�ַ�
unsigned char pucSeg_Code[8];     	// ��ʾ����
unsigned char ucSeg_Pos;           	// ��ʾλ��
unsigned int  uiSeg_Dly;          	// ��ʾˢ����ʱ
unsigned char ucSeg_Dly;           	// ��ʾ��λ��ʱ
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)              // 300msˢ��1��
  {
    uiSeg_Dly = 0;

    switch (ucState)
    {
      case 0:                       // ��ʾʱ��
        sprintf(pucSeg_Char, "%02u-%02u-%02u",\
          (int)pucRtc[0], (int)pucRtc[1], (int)pucRtc[2]);
        break;
      case 1:                       // ��ʾ�¶�
        sprintf(pucSeg_Char, "C    %03.1f", uiTemp/16.0);
        break;
      case 2:                       // ��ʾ����״̬
        sprintf(pucSeg_Char, "E %3.2f%3u", ucAdc/51.0, (int)ucRds);
        break;
      case 0x10:                    // ��ʾʱ�����
        sprintf(pucSeg_Char, "P1    %02u", (int)ucHour1);
        break;
      case 0x11:                    // ��ʾ�¶Ȳ���
        sprintf(pucSeg_Char, "P2    %02u", (int)ucTemp1);
        break;
      case 0x12:                    // ��ʾLED����
        sprintf(pucSeg_Char, "P3     %1u", (int)ucLedp1);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)              	// 2ms��λ1��
  {
    ucSeg_Dly = 0;

    Seg_Disp(pucSeg_Code[ucSeg_Pos], ucSeg_Pos);
    ucSeg_Pos = ++ucSeg_Pos & 7;  	// �����ѭ����ʾ
  }
}

unsigned char ucKey_Old;          	// �ɼ�ֵ
unsigned char ucKey_Dly;           	// ����ˢ����ʱ
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// ���¼�ֵ

  if (ucKey_Dly < 10)             	// 10msʱ��δ��
    return;                         // ��ʱ����
  ucKey_Dly = 0;

  ucKey_Dn = Key_Read();           	// ��ֵ��ȡ
  if (ucKey_Dn != ucKey_Old)      	// ��ֵ�仯
  {
    ucKey_Old = ucKey_Dn;
  } else {
    ucKey_Dn = 0;
  }

  switch (ucKey_Dn)
  {
    case 4:                         // S4����
      ucState ^= 0x10;             	// �л����ݺͲ�������
      ucState &= ~3;
      ucState |= 2;
    case 5:                         // S5����
      if ((++ucState & 3) == 3)   	// �л����ݻ�����ӽ���״̬
        ucState &= ~3;
      ucHour = ucHour1;           	// �������
      ucTemp = ucTemp1;
      ucLedp = ucLedp1;
      break;
    case 8:                         // S8����
      switch (ucState)             	// ������1
      {
        case 0x10:                  // �޸�ʱ�����
          if (ucHour1 > 0)
            --ucHour1;
          else
            ucHour1 = 23;
          break;
        case 0x11:                  // �޸��¶Ȳ���
          if (ucTemp1 > 0)
            --ucTemp1;
          else
            ucTemp1 = 99;
          break;
        case 0x12:                  // �޸�LED����
          if (ucLedp1 > 4)
            --ucLedp1;
          else
            ucLedp1 = 8;
      }
      break;
    case 9:                         // S9����
      switch (ucState)             	// ������1
      {
        case 0x10:                  // �޸�ʱ�����
          if (ucHour1 < 23)
            ++ucHour1;
          else
            ucHour1 = 0;
          break;
        case 0x11:                  // �޸��¶Ȳ���
          if (ucTemp1 < 99)
            ++ucTemp1;
          else
            ucTemp1 = 0;
          break;
        case 0x12:                  // �޸�LED����
          if (ucLedp1 < 8)
            ++ucLedp1;
          else
            ucLedp1 = 4;
      }
  }
}

unsigned char ucLed;              	// LEDֵ
unsigned int  uiLed_Dly;          	// LED��ʱ
void Led_Proc(void)
{
  unsigned long ulTime;

  if (uiLed_Dly < 500)             	// 500msˢ��1��
    return;
  uiLed_Dly = 0;

  DS1302_GetRtc(pucRtc);
  pucRtc[0] = (pucRtc[0]>>4)*10 + (pucRtc[0]&0xf);
  pucRtc[1] = (pucRtc[1]>>4)*10 + (pucRtc[1]&0xf);
  pucRtc[2] = (pucRtc[2]>>4)*10 + (pucRtc[2]&0xf);
  ulTime = (pucRtc[0]*60+pucRtc[1])*60+pucRtc[2];
  if ((ulTime >= ucHour*3600) || (ulTime <= 8*3600))
    ucLed |= 1;
  else
    ucLed &= ~1;

  uiTemp = Temp_Read();
  if (uiTemp < (ucTemp<<4))
    ucLed |= 2;
  else
    ucLed &= ~2;

  ucAdc = PCF8591_Adc(1);
  if (ucAdc > 70)
  {
    ucRds = 0;                      // ��״̬
    ucLed &= ~(1 << (ucLedp-1));  	// Ϩ��ָ����LED
  }
  else
  {
    ucRds = 1;                      // ��״̬
    ucLed |= 1 << (ucLedp-1);      	// ����ָ����LED
  }
  if (ucRds != ucRdt)
  {
    uiSec = 0;                      // ״̬�仯��ʼ��ʱ
    ucRdt = ucRds;
  }
  if (uiSec >=3)                    // ����ʱ�䳬��3s
    if (ucAdc > 140)
      ucLed &= ~4;
    else
      ucLed |= 4;

  Led_Disp(ucLed);                  // LED��ʾ״̬
}
