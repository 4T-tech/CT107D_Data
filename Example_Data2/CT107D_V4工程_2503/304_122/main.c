#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds1302.h"
#include "i2c.h"

unsigned char ucState;            	// ϵͳ״̬
unsigned char pucRtc[3] = {0x23, 0x59, 0x50};
unsigned char ucMode;             	// ģʽ: 0-������1-��ʱ
unsigned char ucDist;             	// ����ֵ
unsigned char ucAdc;               	// ADCֵ
unsigned char ucType;             	// ��������
unsigned char ucMax;              	// ���ֵ
unsigned char ucMin=255;          	// ��Сֵ
unsigned int  uiSum;              	// �����ۼ�
unsigned char ucNum;              	// ��������
unsigned char ucCnt;              	// ��������
unsigned char ucTime[5]={2, 3, 5, 7, 9};
unsigned char ucTime1, ucTime2;   	// ʱ�����
unsigned char ucDist1=20, ucDist2=20; // �������

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
void Data_Proc(void);
// ������
void main(void)
{
  Close_Peripheral();
  T1_Init();
  DS1302_SetRtc(pucRtc);           	// ����RTCʱ��

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
  if (uiSeg_Dly > 200)            	// 200msʱ�䵽
  {
    uiSeg_Dly = 0;

    switch (ucState)
    {
      case 0:                       // ��ʾʱ��
        sprintf(pucSeg_Char, "%02x-%02x-%02x",\
          (int)pucRtc[0], (int)pucRtc[1], (int)pucRtc[2]);
        break;
      case 1:                       // ��ʾ����
        if (ucMode == 0)
          sprintf(pucSeg_Char, "LC   %3u", (int)ucDist);
        else
          sprintf(pucSeg_Char, "LF   %3u", (int)ucDist);
        break;
      case 2:                       // ��ʾ����
        switch (ucType)
        {
          case 0:
            sprintf(pucSeg_Char, "H^  %4u", (int)ucMax);
            break;
          case 1:
            if (ucNum == 0)
              sprintf(pucSeg_Char, "H-  127.5");
            else
              sprintf(pucSeg_Char, "H-  %3u.%1u", uiSum/ucNum, uiSum%ucNum);
            break;
          case 2:
            sprintf(pucSeg_Char, "H_  %4u", (int)ucMin);
        }
        break;
      case 0x10:                    // ��ʾʱ�����
        sprintf(pucSeg_Char, "P1    %02u",
          (unsigned int)ucTime[ucTime2]);
        break;
      case 0x11:                    // ��ʾ�������
        sprintf(pucSeg_Char, "P2    %02u", (int)ucDist2);
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
unsigned char ucKey_Dly;           	// ����ˢ����ʱ
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// ���¼�ֵ

  if (ucKey_Dly < 10)               // ��ʱ10ms����
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
    case 4:                         // S4����
      ucState ^= 0x10;
      ucState &= ~3;
      if ((ucState & 0x10) == 0)
        ucState |= 2;
      else
        ucState |= 1;
    case 5:                         // S5����
      ucState++;
      if ((ucState & 0x10) == 0)
      {
        if (ucState == 2)         	// ��¼��ʾ
          ucType = 0;               // ���ֵ
        if (ucState == 3)         	// 3�����ݽ���
          ucState = 0;
      }
      else
      {
        if (ucState == 0x12)      	// 2�����ý���
          ucState = 0x10;
      }
      ucTime1 = ucTime2;
      ucDist1 = ucDist2;
      break;
    case 8:                         // S8����
      if (ucState == 1)
        ucMode ^= 1;               	// �л������Ͷ�ʱģʽ
      if (ucState == 2)
        if (++ucType == 3)        	// �л���������
          ucType = 0;
      break;
    case 9:                         // S9����
      if (ucState == 0x10)
        if (++ucTime2 == 5)       	// �޸�ʱ�����
          ucTime2 = 0;
      if (ucState == 0x11)
      {
        ucDist2 += 10;              // �޸ľ������
        if (ucDist2 == 90)
          ucDist2 = 10;
      }
  }
}

unsigned char ucLed;              	// LEDֵ
void Led_Proc(void)
{
  if (ucState < 3)
    ucLed = 1<<ucState;
  else
    ucLed = 0;

  if (ucMode == 0)
    ucLed |= 8;                     // L4����
  else
    ucLed &= ~8;                   	// L4Ϩ��

  if (ucCnt >= 3)
    ucLed |= 0x10;                 	// L5����
  else
    ucLed &= ~0x10;                	// L5Ϩ��

  if (ucAdc > 50)
    ucLed |= 0x20;                 	// L6����
  else
    ucLed &= ~0x20;                	// L6Ϩ��

  Led_Disp(ucLed);                 	// LED��ʾ
}

unsigned int  uiData_Dly;         	// ���ݲɼ���ʱ
unsigned char ucFlag, ucFlag1;    	// �����������
void Data_Proc(void)
{
  unsigned int  uiDac;             	// DACֵ(*100)

  if (uiData_Dly < 300)            	// 300msʱ��δ��
     return;
  uiData_Dly = 0;

  DS1302_GetRtc(pucRtc);           	// ��ȡRTCʱ��
  if (ucMode == 1)
  {
    if ((pucRtc[2] % ucTime[ucTime1]) == 0)
      ucFlag = 1;                   // ����
    else
      ucFlag = 0;
  }
  else
  {
    ucAdc = PCF8591_Adc(1);
    if (ucAdc < 50)
      ucFlag = 1;                   // ��״̬
    else
      ucFlag = 0;                   // ��״̬
  }

  ucDist = Dist_Meas();
  if (ucFlag != ucFlag1)          	// ״̬�仯
  {
    ucFlag1 = ucFlag;

    if (ucFlag == 1)               	// ��������������
    {
      if (ucDist > ucMax)
        ucMax = ucDist;            	// �������ֵ
      if (ucDist < ucMin)
        ucMin = ucDist;            	// ������Сֵ
      if (++ucNum != 0)
        uiSum += ucDist;           	// �����ۼ�
      else
      {
        uiSum = ucDist;            	// �����ۼ�
        ucNum =1;
      }
      if (ucDist < 10)
        uiDac = 100;                // 1V 
      else if(ucDist > 60) 
        uiDac = 500;                // 5V
      else
        uiDac = (ucDist * 8) + 20;
      PCF8591_Dac(uiDac * 0.51);   	// 255/500

      if (ucMode == 1)             	// ��ʱģʽ
      {
        if (((ucDist>ucDist1) && (ucDist-ucDist1)<5)
          || ((ucDist<ucDist1) && (ucDist1-ucDist)<5))
          ucCnt++;
        else
          ucCnt = 0;
      }
    }
  }
}
