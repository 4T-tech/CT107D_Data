sfr SCON = 0x98;
sfr SBUF = 0x99;
sfr AUXR = 0x8e;
sfr T2L = 0xd7;
sfr T2H = 0xd6;
sbit TI = SCON^1;
sbit RI = SCON^0;

void Uart_Init(void)                	// 9600bps@12.000MHz
{
  SCON = 0x50;                       	// 8λ����,�ɱ䲨����
  AUXR |= 0x01;                      	// ���п�1ѡ��ʱ��2Ϊ�����ʷ�����
  AUXR |= 0x04;                      	// ��ʱ��2ʱ��ΪFosc, ��1T
  T2L = 0xC7;                        	// �趨��ʱ��ֵ
  T2H = 0xFE;                        	// �趨��ʱ��ֵ
  AUXR |= 0x10;                      	// ������ʱ��2
}

       unsigned char ucUart_Dly;    	// ���пڽ����ַ���ʱ
extern unsigned char pucUart_Buf[10];	// ���пڻ���
extern unsigned char ucUart_Num;    	// ���пڽ����ַ�����
void Uart_RecvString(void)
{
  if (RI)
  {
    pucUart_Buf[ucUart_Num++] = SBUF;
    RI = 0;
    ucUart_Dly = 0;                 	// ��ʼ��ʱ
  }
  if ((ucUart_Dly > 200) && (ucUart_Num != 0))
  {                                  	// ���пڽ������
    pucUart_Buf[ucUart_Num] = 0;
    ucUart_Num |= 0x80;             	// ���ý�����ɱ�־
  }
}

void Uart_SendChar(unsigned char c)
{
  SBUF = c;
  while (TI == 0);
  TI = 0;
}
