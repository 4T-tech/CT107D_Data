// ʹ�ó���ǰ����J13����ΪIOģʽ��2-3�Ŷ̽ӣ�
#include "tim.h"
#include "seg.h"
#include <stdio.h>

unsigned int  uiTms;                	// ����ֵ
unsigned int  uiSec;                	// ��ֵ

void Led_Proc(void);
void Seg_Proc(void);
// ������
void main(void)
{
  Close_Peripheral();
  T1_Init();
	
  while (1)
  {
    T1_Proc();
    Seg_Proc();
    Led_Proc();
  }
}

unsigned char pucSeg_Char[12];      	// ��ʾ�ַ�
unsigned char pucSeg_Code[8];       	// ��ʾ����
unsigned char ucSeg_Pos;            	// ��ʾλ��
unsigned int  uiSeg_Dly;            	// ��ʾˢ����ʱ
unsigned char ucSeg_Dly;            	// ��ʾ��λ��ʱ
// ע�⣺sprintf()�����ַ���������ӡ�\0��������pucSeg_Buf[]�ĳ�������ӦΪ9��
// ����ַ����а���С���㣬pucSeg_Buf[]�ĳ�������ӦΪ10��
void Seg_Proc(void)
{
  if (uiSeg_Dly > 300)              	// 300msˢ��1��
  {
    uiSeg_Dly = 0;

    sprintf(pucSeg_Char, "1.   %04u", uiSec);
    Seg_Tran(pucSeg_Char, pucSeg_Code);
  }
  if (ucSeg_Dly >= 2)               	// 2ms��λ1��
  {
    ucSeg_Dly = 0;

    Seg_Disp(pucSeg_Code[ucSeg_Pos], ucSeg_Pos);
    ucSeg_Pos = ++ucSeg_Pos & 7;    	// �����ѭ����ʾ
  }
}

void Led_Proc(void)
{
  Led_Disp(uiSec);
 }
