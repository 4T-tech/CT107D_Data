#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds1302.h"
#include "ds18B20.h"

unsigned char ucState;            	// ϵͳ״̬
unsigned char ucLed=2;            	// LEDֵ, �¶ȿ���ģʽ
unsigned char pucRtc[3] = {0x23, 0x59, 0x30};
unsigned char ucRtc;              	// RTC��־
unsigned int  uiTemp;             	// �¶�ֵ
unsigned char ucTemp=23;          	// �¶Ȳ���

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
// ������
void main(void)
{
  Close_Peripheral();
  T1_Init();
  DS1302_SetRtc(pucRtc);

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
unsigned char ucSeg_Dly;          	// ��ʾ��λ��ʱ
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)            	// 300msˢ��1��
  {
    uiSeg_Dly = 0;
 
    switch (ucState)
    {
      case 0:                       // ��ʾ�¶�
        sprintf(pucSeg_Char, "U1   %03.1f", uiTemp/16.0);
        break;
      case 1:                       // ��ʾRTCʱ��
        if (ucRtc == 0)            	// ��ʾʱ��
          sprintf(pucSeg_Char, "U2 %02x-%02x",\
            (unsigned int)pucRtc[0], (int)pucRtc[1]);
        else                        // ��ʾ����
          sprintf(pucSeg_Char, "U2 %02x-%02x",\
            (unsigned int)pucRtc[1], (int)pucRtc[2]);
        break;
      case 2:                       // ��ʾ����
        sprintf(pucSeg_Char, "U3    %02u", (int)ucTemp);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)             	// 2ms��λ1��
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

  if (ucKey_Dly < 10)             	// ��ʱ10ms����
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
    case 12:
      if (++ucState == 3)          	// S12�����л�״̬
        ucState = 0;
      break;
    case 13:
      ucLed ^= 2;                   // S13�����л�ģʽ
      break;
    case 16:
      if (ucState == 2)           	// ���ý��������1
        if (++ucTemp > 99)
          ucTemp = 10;
      break;
    case 17:
      if (ucState == 2)           	// ���ý��������1
        if (--ucTemp < 10)
          ucTemp = 99;
  }
  if (ucState == 1)
    if (ucKey_Old == 17)          	// ʱ�����S17���л�ʱ����ʾ
      ucRtc = 1;                    // ��ʾ����
    else
      ucRtc = 0;                    // ��ʾʱ��
}

unsigned char ucUln;              	// ULNֵ
unsigned char ucLed_Dly;          	// LED��ʱ
void Led_Proc(void)
{
  if (ucLed_Dly < 100)             	// 100msʱ��δ��
    return;
  ucLed_Dly = 0;

  DS1302_GetRtc(pucRtc);          	// ��ȡRTCʱ��
  if ((pucRtc[1] == 0) && (pucRtc[2] < 5))
    ucLed |= 1;                     // L1����
  else
    ucLed &= ~1;                    // L1Ϩ��
  if ((ucLed & 2) == 2)            	// �¶ȿ���ģʽ
  {
    uiTemp = Temp_Read();          	// ��ȡ�¶�
    if (uiTemp > (ucTemp<<4))
      ucUln = 0x10;                 // �̵����պ�
    else
      ucUln = 0;                    // �̵����Ͽ�
  } else {                          // ʱ�����ģʽ
    if ((pucRtc[1] == 0) && (pucRtc[2] < 5))
      ucUln = 0x10;                 // �̵����պ�
    else
      ucUln = 0;                    // �̵����Ͽ�
  }
  if (ucUln & 0x10)                	// �̵����պ�
    ucLed ^= 4;                     // L3��˸
  else
    ucLed &= ~4;
  Led_Disp(ucLed);                 	// LED��ʾ״̬
  Uln_Ctrl(ucUln);                 	// ULN����
}
