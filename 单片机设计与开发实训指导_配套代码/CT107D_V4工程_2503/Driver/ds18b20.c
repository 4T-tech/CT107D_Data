sfr  P1 = 0x90;
sbit DQ = P1^4;                  // �����߽ӿ�
// DS18B20��ʱ����
void DS18B20_Delay(unsigned int t)
{
  while (t--);
}
// DS18B20��ʼ��
bit DS18B20_Init(void)
{
  bit flag = 0;

  DQ = 1;
  DQ = 0;
  DS18B20_Delay(200);
  DQ = 1;
  DS18B20_Delay(100);
  flag = DQ;
  DS18B20_Delay(25);

  return flag;
}
// DS18B20д�ֽ�
void DS18B20_Write(unsigned char dat)
{
  unsigned char i;
  for (i=0; i<8; i++)
  {
    DQ = 0;
    DQ = dat & 1;
    DS18B20_Delay(25);
    DQ = 1;
    dat >>= 1;
  }
  DS18B20_Delay(5);
}
// DS18B20���ֽ�
unsigned char DS18B20_Read(void)
{
  unsigned char i, dat;

  for (i=0; i<8; i++)
  {
    DQ = 0;
    dat >>= 1;
    DQ = 1;
    if (DQ) dat |= 0x80;
    DS18B20_Delay(25);
  }
  return dat;
}
// ��ȡ�¶�
unsigned int Temp_Read(void)
{
  unsigned char low, high;

  DS18B20_Init();                // ��ʼ��
  DS18B20_Write(0xCC);           // ����ROM
  DS18B20_Write(0x44);           // �����¶�ת��

  DS18B20_Init();
  DS18B20_Write(0xCC);
  DS18B20_Write(0xBE);           // ��ȡ�¶�ֵ
  low = DS18B20_Read();          // ���ֽ�
  high = DS18B20_Read();         // ���ֽ�

  return (high<<8)+low;
}
