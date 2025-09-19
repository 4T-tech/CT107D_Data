sfr  P1 = 0x90;
sfr  P2 = 0xA0;
sbit RST = P1^3;                    	// DS1302��λ
sbit SCK = P1^7;                    	// DS1302ʱ��
sbit SDA = P2^3;                    	// DS1302����
// DS1302д
void DS1302_Write(unsigned  char ucData)
{
  unsigned char i;

  for (i=0; i<8; i++)
  {
    SCK = 0;
    SDA = ucData & 1;
    ucData >>= 1;
    SCK = 1;
  }
}
// DS1302д�ֽڣ�ucAddr�����λΪ0
void DS1302_WriteByte(unsigned char ucAddr, unsigned char ucData)
{
  RST = 0;
  SCK = 0;
  RST = 1;
  DS1302_Write(ucAddr);
  DS1302_Write(ucData);
  RST = 0;
}
// DS1302���ֽڣ�ucAddr�����λΪ1
unsigned char DS1302_ReadByte(unsigned char ucAddr)
{
  unsigned char i, temp=0;

  RST = 0;
  SCK = 0;
  RST = 1;
  DS1302_Write(ucAddr);
  for (i=0; i<8; i++)
  {
    SCK = 0;
    temp >>= 1;
    if (SDA)
      temp |= 0x80;
    SCK = 1;
  }
  RST = 0;
  SCK = 0;
  SDA = 0;
  return(temp);
}
// ����RTCʱ�ӣ�pucRtc-ʱ��ֵ��ʱ���룺BCD�룩
void DS1302_SetRtc(unsigned char* pucRtc)
{
  DS1302_WriteByte(0x8E, 0);          // WP=0������д����
  DS1302_WriteByte(0x84, pucRtc[0]);  // ����ʱ
  DS1302_WriteByte(0x82, pucRtc[1]);  // ���÷�
  DS1302_WriteByte(0x80, pucRtc[2]);  // ������
  DS1302_WriteByte(0x8E, 0x80);       // WP=1����ֹд����
}
// ��ȡRTCʱ�ӣ�pucRtc-ʱ��ֵ��ʱ���룺BCD�룩
void DS1302_GetRtc(unsigned char* pucRtc)
{
  pucRtc[0] = DS1302_ReadByte(0x85);  // ��ȡʱ
  pucRtc[1] = DS1302_ReadByte(0x83);  // ��ȡ��
  pucRtc[2] = DS1302_ReadByte(0x81);  // ��ȡ��
}
