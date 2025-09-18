// ʹ�ó���ǰ����J13����ΪIOģʽ��2-3�Ŷ̽ӣ�
sfr P0 = 0x80;
sfr P2 = 0xA0;
// P0���: ucData�������ݣ�ucAddr������ַ��4~7��
void P0_Out(unsigned char ucData, unsigned char ucAddr)
{
  P0 = ucData;                      	// P0�������
  P2 |= ucAddr << 5;                	// ��λP27~P25
  P2 &= 0x1f;                       	// ��λP27~P25
//XBYTE[ucAddr << 13] = ucData;     	// MMģʽ��J13-2��J13-1������
}
// �ر�����
void Close_Peripheral(void)
{
  P2 &= 0x1f;                       	// ��λP27~P25
  P0_Out(0xff, 4);                  	// Ϩ��LED
  P0_Out(~0x50, 5);                 	// �رռ̵����ͷ�����
}
// LED��ʾ
void Led_Disp(unsigned char ucLed)
{
  P0_Out(~ucLed, 4);                	// LED��ʾ
}
// ��ʱ��������СԼ1ms@12MHz��
void Delay(unsigned int uiNum)
{
  unsigned int i;

  while (uiNum--)
    for (i=0; i<628; i++);
}
// ������
void main(void)
{
  unsigned char i, j;
  Close_Peripheral();

  while (1)
  { // 4�����ȵȼ�
    for (i=0; i<4; i++)
      for (j=0; j<64; j++)
      {
        Led_Disp(0xff);             	// L1~L8����
        Delay(i+1);                  	// ��ʱʱ��������1ms��2ms��3ms��4ms
        Led_Disp(0);                 	// L1~L8Ϩ��
        Delay(4-i);                  	// ��ʱʱ��������4ms��3ms��2ms��1ms
      }
  }
}
