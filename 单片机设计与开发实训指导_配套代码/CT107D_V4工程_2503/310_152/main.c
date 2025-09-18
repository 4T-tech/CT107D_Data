#define test

#include "tim.h"
#include "seg.h"
#include <stdio.h>
#include "key.h"
#include "uart.h"
#include "i2c.h"
#include "math.h"

unsigned int  uiSec;                	// ��ֵ
unsigned char ucState;              	// ϵͳ״̬
unsigned char ucSdev;               	// �豸״̬��
// 0-����״̬, 1-����׼��, 2-���д���, 3-���н���, 4-�ȴ�״̬
unsigned int  idata uiCstaX;        	// ��ʼX����
unsigned int  idata uiCstaY;        	// ��ʼY����
unsigned int  idata uiCtarX;        	// Ŀ��X����
unsigned int  idata uiCtarY;        	// Ŀ��Y����
unsigned int  idata uiCdevX;        	// �豸Xλ��
unsigned int  idata uiCdevY;        	// �豸Yλ��
unsigned int  idata uiSpeed;        	// �н��ٶ�*10
unsigned char ucScen;               	// ����
unsigned char ucR=10;               	// ����R*10
  signed char scB;                   	// ����B
unsigned char ucDist;               	// �ϰ������
            bit bRun;                  	// ���б�־

void Seg_Proc(void);
void Key_Proc(void);
void Led_Proc(void);
void Uart_Proc(void);
void Data_Proc(void);
// ������
void main(void)
{
  Close_Peripheral();
  T1_Init();
  Uart_Init();
  T0_Init();

  while (1)
  {
    Seg_Proc();
    Key_Proc();
    Led_Proc();
    Uart_Proc();
    Data_Proc();
  }
}

unsigned char pucSeg_Char[12];      	// ��ʾ�ַ�
unsigned char pucSeg_Code[8];       	// ��ʾ����
unsigned char ucSeg_Pos;            	// ��ʾλ��
unsigned int  uiSeg_Dly;             	// ��ʾˢ����ʱ
unsigned char ucSeg_Dly;            	// ��ʾ��λ��ʱ
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)              	// 300msˢ��1��
  {
    uiSeg_Dly = 0;

    switch (ucState)
    {
      case 0:                         // �������
#ifndef test
        if (ucSdev == 0)            	// ����״̬��ʾ�豸����
#else
        if (ucSdev == 2)            	// ����״̬��ʾ�豸����
#endif
          sprintf(pucSeg_Char, "L%3u-%3u", uiCdevX, uiCdevY);
        else                          // ����״̬��ʾĿ������
          sprintf(pucSeg_Char, "L%3u-%3u", uiCtarX, uiCtarY);
        break;
      case 1:                         // �ٶȽ���
        switch (ucSdev)
        {
          case 2:                     // ���д�����ʾ�ٶ�
            sprintf(pucSeg_Char, "E1 %4u.%1u", uiSpeed/10, uiSpeed%10);
            break;
          case 0:                     // ����״̬
#ifndef test
            sprintf(pucSeg_Char, "E2 -----");
#else
            sprintf(pucSeg_Char, "E2 %4u.%1u", uiSpeed/10, uiSpeed%10);
#endif
            break;
          case 4:                     // �ȴ�״̬��ʾ����
#ifndef test
            sprintf(pucSeg_Char, "E3 %5u", (int)ucDist);
#else
            sprintf(pucSeg_Char, "E3 %1u %3u", (int)ucScen, (int)ucDist);
#endif
        }
        break;
      case 2:                         // �������棨R������Ч��
      case 3:                         // �������棨B������Ч��
        sprintf(pucSeg_Char, "P %2.1f %3d", ucR/10.0, (int)scB);
    }
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
}

unsigned char ucKey_Old;            	// �ɼ�ֵ
unsigned char ucKey_Dly;            	// ����ˢ����ʱ
void Key_Proc(void)
{
  unsigned char ucKey_Dn;           	// ���¼�ֵ

  if(ucKey_Dly < 10)                 	// ��ʱ10ms����
    return;
  ucKey_Dly = 0;

  ucKey_Dn = Key_Read();            	// ��ֵ��ȡ
  if (ucKey_Dn != ucKey_Old)        	// ��ֵ�仯
  {
    ucKey_Old = ucKey_Dn;
  } else {
    ucKey_Dn = 0;
  }

  switch (ucKey_Dn)
  {
    case 4:                           // S4��������
      switch (ucSdev)
      {
        case 0:                       // ����״̬
          if (bRun)                   // ����������
            ucSdev = 1;              	// ����׼��
          break;
        case 2:                       // ���д���
          ucSdev = 4;                	// �ȴ�״̬
          break;
        case 4:                       // �ȴ�״̬
          if (ucDist > 30)
            ucSdev = 2;               // ���д���
      }
      break;
    case 5:                           // S5��������
      if (ucSdev == 0)               	// ����״̬
      {
        uiCstaX = uiCstaY = 0;      	// ������ʼ����
        uiCdevX = uiCdevY = 0;      	// �����豸����
      }
      break;
    case 8:                          	// S8��������
      if (++ucState >= 3)           	// �л�����
        ucState = 0;
      break;
    case 9:                          	// S9��������
      if ((ucState == 2) || (ucState == 3))	// ��������
        ucState ^= 1;
      if (ucState == 1)             	// �ٶȽ���
        ucSdev ^= 4;                 	// �л��豸״̬�������ã�
      break;
    case 12:                         	// S9��������
      if (ucState == 2)             	// R����
      {
        if (ucR < 20)
          ucR++;                      // ����R����
      }
      else if (ucState == 3)        	// B����
        if (scB < 90)
          scB += 5;                   // ����B����
      break;
    case 13:                          // S9��������
      if (ucState == 2)              	// R����
      {
        if (ucR > 10)
          ucR--;                      // ����R����
      }
      else if (ucState == 3)        	// B����
        if (scB > -90)
          scB -= 5;                   // ����B����
  }
}

unsigned char ucLed;                	// LEDֵ
unsigned char ucLed_Dly;            	// LED��ʱ
void Led_Proc(void)
{
  if(ucLed_Dly < 100)               	// ��ʱ100ms
    return;
  ucLed_Dly = 0;

  switch (ucSdev)
  {
    case 0:                           // ����״̬
    case 3:
      ucLed &= ~1;                   	// L1Ϩ��
      break;
    case 2:                           // ����״̬
      ucLed |= 1;                     // L1����
      break;
    case 4:                           // �ȴ�״̬
      ucLed ^= 1;                     // L1��˸
  }

  if ((ucSdev == 2) && (ucScen == 0)) // ҹ���н�
    ucLed |= 2;                      	// L2����
  else
    ucLed &= ~2;                     	// L2Ϩ��

  if (ucSdev == 3)                  	// ���н���
  {
    if (uiSec < 3)
      ucLed |= 4;                    	// L3����
    else
    {
      ucLed &= ~4;                   	// L3Ϩ��
      ucSdev = 0;
      bRun = 0;
    }
  }
  Led_Disp(ucLed);

  if (ucSdev == 2)                   	// ����״̬
    Uln_Ctrl(0x10);                  	// �̵�������
  else
    Uln_Ctrl(0);                    	// �̵����Ͽ�
}

unsigned int  uiUsec;               	// UART������ʱ
unsigned char pucUart_Buf[10];      	// UART����ֵ
unsigned char ucUart_Num;           	// ���пڽ����ַ�����
//unsigned char pucUart_Buf[10] = {"(130,420)"};	// UART����ֵ
//unsigned char ucUart_Num=0x89;    	// ���пڽ����ַ�������9���ַ���
void Uart_Proc(void)
{
  unsigned char i, m, n;
  unsigned int  c[2];

  if (uiUsec != uiSec)              	// ������
  {
    uiUsec = uiSec;
#ifdef test
    printf("%03u\r\n", uiSec);
#endif
  }

  Uart_RecvString();
  if (ucUart_Num <= 0x80)           	// ����δ���
    return;
  if (ucUart_Num == 0x81)           	// ?��#
  {
    if (pucUart_Buf[0] == '?')      	// ��ѯ�豸״̬
      switch (ucSdev)
      {
        case 0:
          printf("Idle");
          break;
        case 2:
          printf("Busy");
          break;
        case 4:
          printf("Wait");
      }
    else if (pucUart_Buf[0] == '#') 	// ��ѯ�豸λ��
      printf("(%u,%u)", uiCdevX, uiCdevY);
    else
      printf("Error");
  } else if (pucUart_Buf[0] == '(') 	// ����Ŀ�ĵ�����
  {
    m = 0;
    n = 0;
    ucUart_Num -= 0x81;
    for (i=1; i<=ucUart_Num; i++)   	// ��������
    {
      if ((pucUart_Buf[i] == ',') || (pucUart_Buf[i] == ')'))
      {
        c[n] = 0;
        switch (i-m-1)                // ����λ��
        {
          case 3:
            c[n] += (pucUart_Buf[i-3]-'0')*100;
          case 2:
            c[n] += (pucUart_Buf[i-2]-'0')*10;
          case 1:
            c[n] += pucUart_Buf[i-1]-'0';
        }
        m = i;
        n++;
      }
    }
    if ((n == 2) && (ucSdev == 0))
    {
      uiCtarX = c[0];                	// Ŀ��X����
      uiCtarY = c[1];                	// Ŀ��Y����
      printf("Got it");
      bRun = 1;
    } else
      printf("Busy");
  }
  else
    printf("Error");
  ucUart_Num = 0;
}

char putchar(char c)
{
  Uart_SendChar(c);
  return c;
}

unsigned char ucData_Dly;           	// ���ݲɼ���ʱ
unsigned int  uiFreq;               	// Ƶ��ֵ
  signed int  siDistX;              	// ����X
  signed int  siDistY;              	// ����Y
unsigned int  uiDist;               	// ����*10
unsigned int  uiDcur;               	// ��ǰ����*10
void Data_Proc(void)                 	// ���ݴ���
{
  float fProg;

  if (ucData_Dly < 100)               // 100msδ��
    return;
  ucData_Dly = 0;

  if (ucSdev == 1)                  	// ����׼��
  {
    siDistX = uiCtarX-uiCstaX;
    siDistY = uiCtarY-uiCstaY;
    uiDist = sqrt(pow(siDistX, 2)+pow(siDistY, 2)) * 10;
    uiDcur = 0;
    ucSdev = 2;                      	// ���д���
  }

  uiSpeed = 3.14*ucR*uiFreq/100+scB*10;  	// �ٶ�*10
  if (ucSdev == 2)                  	// ���д���
  {
    if (uiSpeed > 0)
    {
      uiDcur += uiSpeed / 10;        	// 100ms���о���: (usSpeed/10)*0.1s*10
      if (uiDcur < uiDist)
      {                               // ���������㵱ǰλ��
        fProg = (float)uiDist/uiDcur;
        uiCdevX = siDistX/fProg+uiCstaX;
        uiCdevY = siDistY/fProg+uiCstaY;
      } else {
        uiCstaX = uiCtarX;
        uiCstaY = uiCtarY;
        uiCdevX = uiCtarX;
        uiCdevY = uiCtarY;
        ucSdev = 3;                 	// ���н���
        uiSec = 0;
      }
    }
  }

  ucDist = Dist_Meas();
  if ((ucDist < 30) && (ucSdev == 2))
    ucSdev = 4;                     	// �ȴ�״̬

  if (PCF8591_Adc(1)*10/51 > 12)
    ucScen = 1;                     	// �ռ䳡��
  else
    ucScen = 0;                     	// ҹ�䳡��
}
