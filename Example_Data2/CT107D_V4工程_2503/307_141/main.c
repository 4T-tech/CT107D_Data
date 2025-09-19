#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds1302.h"
#include "ds18B20.h"
#include "i2c.h"

unsigned int  uiTms;              	// ����ֵ
unsigned int  uiSec;               	// ��ֵ
unsigned char ucState;            	// ϵͳ״̬
unsigned char pucRtc[3] = {0x23, 0x59, 0x50};
unsigned char ucAdc;              	// ADCֵ
unsigned int  uiFreq;             	// Ƶ��ֵ
unsigned char ucCnt;              	// ��������
unsigned char ucTemp;             	// �¶Ȳɼ�ֵ
unsigned char ucTmax;             	// �¶����ֵ
unsigned char ucTcur;             	// �¶ȵ�ǰֵ
unsigned int  uiTsum;             	// �¶��ۼ�ֵ
unsigned char ucTpre;             	// �¶���ʷֵ
unsigned char ucHumi;             	// ʪ�Ȳɼ�ֵ
unsigned char ucHmax;             	// ʪ�����ֵ
unsigned int  uiHsum;             	// ʪ���ۼ�ֵ
unsigned char ucHcur;             	// ʪ�ȵ�ǰֵ
unsigned char ucHpre;             	// ʪ����ʷֵ
unsigned char ucPara=30;          	// �¶Ȳ���ֵ
unsigned char ucHour;             	// �ɼ�ʱ
unsigned char ucMinu;             	// �ɼ���

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
unsigned char pucSeg_Code[8];     	// ��ʾ����
unsigned char ucSeg_Pos;           	// ��ʾλ��
unsigned int  uiSeg_Dly;           	// ��ʾˢ����ʱ
unsigned char ucSeg_Dly;           	// ��ʾ��λ��ʱ
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)            	// 300msˢ��1��
  {
    uiSeg_Dly = 0;
 
    switch (ucState)
    {
      case 0:                       // ʱ�����
        sprintf(pucSeg_Char, "%02x-%02x-%02x",\
          (int)pucRtc[0], (int)pucRtc[1], (int)pucRtc[2]);
        break;
      case 1:                       // �¶Ƚ��棨���ӹ��ܣ�
        sprintf(pucSeg_Char, "C     %02u", (int)ucTemp);
        break;
      case 2:                       // �������棨���ӹ��ܣ�
        sprintf(pucSeg_Char, "A    %03u", (int)ucAdc);
        break;
      case 3:                       // Ƶ�ʽ��棨���ӹ��ܣ�
        sprintf(pucSeg_Char, "F  %05u", uiFreq);
        break;
      case 0x10:                    // �¶Ȼ���
        if (ucCnt == 0)
          sprintf(pucSeg_Char, "C       ");
        else
          sprintf(pucSeg_Char, "C %02u-%3.1f", \
            (int)ucTmax, (float)uiTsum/ucCnt);
       break;
      case 0x11:                    // �¶Ȼ���
        if (ucCnt == 0)
          sprintf(pucSeg_Char, "H       ");
        else
          sprintf(pucSeg_Char, "H %02u-%3.1f", \
            (int)ucHmax, (float)uiHsum/ucCnt);
        break;
      case 0x12:                    // ʱ�����
        if (ucCnt == 0)
          sprintf(pucSeg_Char, "F%02u     ", (int)ucCnt);
        else
          sprintf(pucSeg_Char, "F%02u%02x-%02x",
            (int)ucCnt, (int)ucHour, (int)ucMinu);
        break;
      case 0x20:                    // ��������
        sprintf(pucSeg_Char, "P     %02u", (int)ucPara);
        break;
      case 0x50:                    // ��ʪ�Ƚ���
        sprintf(pucSeg_Char, "E  %02u-%02u", (int)ucTcur, (int)ucHcur);
        if (ucHumi == 0)
          pucSeg_Char[6] = pucSeg_Char[7] = 'A';
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)              	// 2ms��λ1��
  {
    ucSeg_Dly = 0;

    Seg_Disp(pucSeg_Code[ucSeg_Pos], ucSeg_Pos);
    ucSeg_Pos = ++ucSeg_Pos & 7;
  }
}

unsigned char ucKey_Old;          	// �ɼ�ֵ
unsigned char ucKey_Dly;          	// ������ʱ
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// ���¼�ֵ

  if (ucState == 0x50)
    return;

  if (ucKey_Dly < 10)             	// 10msʱ��δ��
    return;                        	// ��ʱ����
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
      ucState &= 0xf0;
      ucState += 0x10;              // �л�������
      if (ucState == 0x30)
        ucState = 0;
      break;
    case 5:                         // S5����
      if ((ucState & 0xf0) == 0)
        if (++ucState == 4)        	// �л�ʱ�����
          ucState = 0;
      if ((ucState & 0xf0) == 0x10)
        if (++ucState == 0x13)     	// �л����Խ���
          ucState = 0x10;
      break;
    case 8:                         // S8����
      if (ucState == 0x20)        	// ��������
        if (++ucPara == 100)
          ucPara = 0;
      break;
    case 9:                         // S9����
      if (ucState == 0x20)        	// ��������
        if (ucPara == 0)
          ucPara = 99;
        else
          ucPara--;
      if (ucState == 0x12)        	// ʱ�����
        uiSec = 0;
  }
  if (ucState == 0x12)             	// ʱ�����
    if ((ucKey_Old == 9) && (uiSec >= 2))
    {
      ucTmax = ucHmax = 0;        	// �����¼����
      uiTsum = uiHsum = 0;
      ucCnt = 0;
    }
}

unsigned char ucLed;              	// LEDֵ
unsigned int  uiLed_Dly;          	// LEDˢ����ʱ
void Led_Proc(void)
{
  if (uiLed_Dly < 100)             	// 100msʱ�䵽
    return;
  uiLed_Dly = 0;

  if ((ucState & 0xf0) == 0)
    ucLed |= 1;                     // L1����
  else
    ucLed &= ~1;

  if ((ucState & 0xf0) == 0x10)
    ucLed |= 2;                     // L2����
  else
    ucLed &= ~2;

  if (ucState == 0x50)
    ucLed |= 4;                     // L3����
  else
    ucLed &= ~4;

  if (ucTemp > ucPara)
    ucLed ^= 8;                     // L4����
  else
    ucLed &= ~8;

  if ((uiFreq < 200) || (uiFreq > 2000))
    ucLed |= 0x10;                 	// L5����
  else
    ucLed &= ~0x10;

  if ((ucCnt >= 2) && (ucHcur > ucHpre) && (ucTcur > ucTpre))
    ucLed |= 0x20;                  // L6����
  else
    ucLed &= ~0x20;

  Led_Disp(ucLed);                 	// LED��ʾ״̬
}

bit bLsta;                          // ������ǰ״̬
bit bLold;                          // ������ʷ״̬
unsigned int  uiData_Dly;          	// ����ˢ����ʱ
void Data_Proc(void)
{
  static unsigned char ucSold;    	// ԭϵͳ״̬

  if (uiData_Dly < 300)             // 300msʱ��δ��
    return;
  uiData_Dly = 0;

  DS1302_GetRtc(pucRtc);          	// ��ȡRTCʱ��
  ucTemp = Temp_Read()>>4;
  if ((uiFreq > 200) && (uiFreq < 2000))
    ucHumi = uiFreq * 10 / 225 + 1.1;
  else
    ucHumi = 0;

  ucAdc = PCF8591_Adc(1);
  if (ucAdc > 80)
    bLsta = 1;                      // ��״̬
  else
    bLsta = 0;                      // ��״̬
  if ((bLold != bLsta) && (uiSec > 3))
  {
    bLold = bLsta;                  // ��������״̬
    if (bLsta == 0)
    {
      if (ucHumi != 0)
      {
        ucCnt++;
        ucHour = pucRtc[0];         // ��������
        ucMinu = pucRtc[1];
        if (ucTemp > ucTmax)
          ucTmax = ucTemp;
        if (ucHumi > ucHmax)
          ucHmax = ucHumi;
        uiTsum += ucTemp;           // �ۼ�����
        uiHsum += ucHumi;
        ucTpre = ucTcur;
        ucHpre = ucHcur;
      }
      ucTcur = ucTemp;
      ucHcur = ucHumi;
      ucSold = ucState;             // ����״̬
      ucState = 0x50;
      uiSec = 0;
    }
  }
  if ((ucState == 0x50) && (uiSec > 3))
    ucState = ucSold;               // �ָ�״̬
}
