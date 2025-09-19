#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "ds18b20.h"
#include "i2c.h"

unsigned char ucState;            	// ϵͳ״̬
unsigned int  uiTemp;              	// �¶�ֵ(*16)
unsigned int  uiDac;               	// DACֵ(*100)
unsigned char ucPara=25, ucPara1=25;	// �¶Ȳ����� ���˳��������ý���ʱ��Ч

void Seg_Proc(void);
void Key_Proc(void);
void Data_Proc(void);

void main(void)
{
  Close_Peripheral();
  T1_Init();

  while(1)
  {
    Seg_Proc();
    Key_Proc();
    Data_Proc();
  }
}

unsigned char pucSeg_Char[12];    	// ��ʾ�ַ�
unsigned char pucSeg_Code[8];      	// ��ʾ����
unsigned char ucSeg_Pos;            // ��ʾλ��
unsigned int  uiSeg_Dly;            // ��ʾˢ����ʱ
unsigned char ucSeg_Dly;            // ��ʾ��λ��ʱ
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)             	// 300msˢ��1��
  {
    uiSeg_Dly = 0;

    switch(ucState)
    {
      case 0:                       // �¶���ʾ
        sprintf(pucSeg_Char, "C   %04.2f", uiTemp/16.0);
        break;
      case 1:                       // ������ʾ
        sprintf(pucSeg_Char, "P     %02u", (int)ucPara1);
        break;
      case 2:                       // DAC��ʾ
        sprintf(pucSeg_Char, "A    %03.2f", uiDac/100.0);
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
unsigned char ucLed=3;             	// LEDֵ��ģʽ1��L1���������¶���ʾ��L2������
void Key_Proc(void)
{
  unsigned char ucKey_Dn;          	// ���¼�ֵ

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
    case 4:                         // S4
      if(++ucState == 3)           	// ѭ���л�״̬
        ucState = 0;
      if(ucState == 2)             	// �趨���¶Ȳ������˳��������ý���ʱ��Ч
        ucPara = ucPara1;
      ucLed &= 1;
      ucLed |= 1 << ucState+1;     	// ����LEDָʾ
      break;
    case 5:                         // S5
      ucLed ^= 1;                   // �л�ģʽ��LED1��
      break;
    case 8:                         // S8
      if(ucState == 1)              // S8�������ڲ������ý�����Ч
        if(ucPara1 != 0)
          ucPara1--;
      break;
    case 9:                         // S9
      if(ucState == 1)              // S9�������ڲ������ý�����Ч
        ucPara1++;
  }
  Led_Disp(ucLed);
}

unsigned int  uiData_Dly;         	// ���ݲɼ���ʱ
void Data_Proc(void)
{
  unsigned char ucTemp;

  if (uiData_Dly < 400)             // 400msʱ��δ��
    return;
  uiData_Dly = 0;

  uiTemp = Temp_Read();           	// ��ȡ�¶�
  ucTemp = uiTemp >> 4;           	// ��������

  if((ucLed & 1) == 1)            	// ģʽ1
  {
    if(ucTemp < ucPara)
      uiDac = 0;                    // 0V
    else
      uiDac = 500;                  // 5V
  }
  else                              // ģʽ2
  {
    if(ucTemp < 20)
      uiDac = 100;                  // 1V
    else if(ucTemp >= 40)
      uiDac = 400;                  // 4V
    else
      uiDac = (ucTemp * 15) - 200;
  }
  PCF8591_Dac(uiDac * 0.51);      	// 255/500
}
