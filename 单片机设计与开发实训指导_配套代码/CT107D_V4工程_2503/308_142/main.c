#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds18b20.h"
#include "i2c.h"

unsigned int  uiSec;              	// ��ֵ
unsigned char ucState;            	// ϵͳ״̬
unsigned int  uiTemp;             	// �¶�ֵ
unsigned char ucDrec;             	// ��¼״̬
  signed char scDist;             	// ����ֵ
  signed char scCali=0;           	// У׼ֵ
unsigned char ucPdist=40;         	// �������
unsigned char ucPtemp=30;         	// �¶Ȳ���
unsigned int  uiSpeed=340;        	// �ٶ�ֵ
unsigned char ucVol=10;           	// ����ֵ(*10)

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
void Data_Proc(void);
// ������
void main(void)
{
  Close_Peripheral();
  T1_Init();

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
unsigned int  uiSeg_Dly;          	// ��ʾˢ����ʱ
unsigned char ucSeg_Dly;          	// ��ʾ��λ��ʱ
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)             	// 300msˢ��1��
  {
    uiSeg_Dly = 0;

    switch (ucState)
    {
      case 0:                       // �����棨cm��
//      sprintf(pucSeg_Char, "%3.1f- %3u",
//        uiTemp/16.0, (signed)scDist);
        sprintf(pucSeg_Char, "%3.1f-%1u%3u",
          uiTemp/16.0, (int)ucDrec, (signed)scDist);
        break;                      // ���Ӽ�¼״̬��ʾ
      case 1:                       // �����棨m��
        sprintf(pucSeg_Char, "%3.1f- %3.2f", uiTemp/16.0, scDist/100.0);
        break;
      case 0x10:                    // �������
        sprintf(pucSeg_Char, "P1    %2u", (int)ucPdist);
        break;
      case 0x11:                    // �¶Ȳ���
        sprintf(pucSeg_Char, "P2    %2u", (int)ucPtemp);
        break;
      case 0x20:                    // У׼ֵ����
        sprintf(pucSeg_Char, "F1   %3d", (signed)scCali);
        break;
      case 0x21:                    // �ٶ�����
        sprintf(pucSeg_Char, "F2  %4u", (int)uiSpeed);
        break;
      case 0x22:                    // DAC�������
        sprintf(pucSeg_Char, "F3    %2.1f", (int)ucVol/10.0);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
}

unsigned char ucKey_Old;          	// �ɼ�ֵ
unsigned char ucKey_Dly;          	// ����ˢ����ʱ
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// ���¼�ֵ

  if (ucDrec == 1)                 	// ��¼����״̬
    return;

  if(ucKey_Dly < 10)              	// ��ʱ10ms����
    return;
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
    case 4:                         // S4��������
      ucState &= 0xf0;             	// �����״̬
      ucState += 0x10;             	// �޸���״̬
      if (ucState == 0x30)
        ucState = 0;
      break;
    case 5:                         // S5��������
      switch (ucState & 0xf0)
      {
        case 0:
        case 0x10:
          ucState ^= 1;             // �л���״̬
          break;
        case 0x20:
          ucState++;                // �޸Ĵ�״̬
          if (ucState == 0x23)
            ucState = 0x20;
      }
      break;
    case 8:                         // S8��������
      switch (ucState)
      {
        case 0:
          ucDrec = 1;               // ��¼����
          break;
        case 0x10:
          if (ucPdist < 90)
            ucPdist += 10;         	// �����������
          break;
        case 0x11:
          if (ucPtemp < 80)
            ucPtemp++;              // �¶Ȳ�������
          break;
        case 0x20:
          if (scCali < 90)
            scCali += 5;            // У׼ֵ����
          break;
        case 0x21:
          if (uiSpeed < 9990)
            uiSpeed += 10;          // �ٶ�ֵ����
          break;
        case 0x22:
          if (ucVol < 20)
            ucVol++;                // ����ֵ����
      }
      uiSec = 0;
      break;
    case 9:                         // S9��������
      switch (ucState)
      {
        case 0:
          if (ucDrec == 2)
            ucDrec = 3;             // �������
          break;
        case 0x10:
          if (ucPdist > 10)
            ucPdist -= 10;          // �����������
          break;
        case 0x11:
          if (ucPtemp > 0)
            ucPtemp--;              // �¶Ȳ�������
          break;
        case 0x20:
          if (scCali > -90)
            scCali -= 5;            // У׼ֵ����
          break;
        case 0x21:
          if (uiSpeed > 10)
            uiSpeed -= 10;          // �ٶ�ֵ����
          break;
        case 0x22:
          if (ucVol > 1)
            ucVol--;                // ����ֵ����
      }
      uiSec = 0;
  }
  if ((ucKey_Old == 20) && (uiSec >= 2))
  {                                 // S8+S9����2s
    ucState = 0;                    // ������
    ucPdist = 40;                   // �������
    ucPtemp = 30;                   // �¶Ȳ���
    uiSpeed = 340;                  // �ٶ�ֵ
    ucVol = 10;                     // ����ֵ(*10)
    scCali = 0;                     // У׼ֵ
    ucDrec = 0;                     // ��¼״̬
  }
}

unsigned char ucLed;              	// LEDֵ
unsigned char ucLed_Dly;          	// LED��ʱ
void Led_Proc(void)
{
  if(ucLed_Dly < 100)              	// ��ʱ100ms
    return;
  ucLed_Dly = 0;

  switch (ucState & 0xf0)
  {
    case 0:
      ucLed = scDist;
      break;
    case 0x10:
      ucLed = 0x80;
      break;
    case 0x20:
      ucLed &= 1;
      ucLed ^= 1;
  }
  Led_Disp(ucLed);
}

unsigned int  uiSec1;             	// ��ֵ1
  signed char scData[7];          	// ����ֵ��¼
unsigned char ucUln;              	// ULNֵ
void Data_Proc(void)
{
  if (uiSec1 == uiSec)
    return;
  uiSec1 = uiSec;

  uiTemp = Temp_Read();
  scDist = Dist_Meas()*uiSpeed/340 + scCali;

  if ((scDist >= ucPdist-5) && (scDist <= ucPdist+5)
    && (uiTemp <= (ucPtemp<<4)))
    ucUln = 0x10;                   // �̵����պ�
  else
    ucUln = 0;                      // �̵����Ͽ�
  Uln_Ctrl(ucUln);

  switch (ucDrec)
  {
    case 1:                         // ��¼����
      scData[uiSec] = scDist;
      if (uiSec == 6)
        ucDrec = 2;                 // ��ɼ�¼
      break;
    case 3:                         // �������
      if (scData[uiSec] < 10)
        PCF8591_Dac(ucVol * 5.1);  	// ucVol*255/50
      else if (scData[uiSec] > 90)
        PCF8591_Dac(255);           // 5V
      else
        PCF8591_Dac(255 - (50-ucVol) * (90 - scData[uiSec]) * 0.06375);
      if (uiSec == 6)
        ucDrec = 2;                 // ������
  }
}
