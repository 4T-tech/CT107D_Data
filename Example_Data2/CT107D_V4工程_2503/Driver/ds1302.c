sfr  P1 = 0x90;
sfr  P2 = 0xA0;
sbit RST = P1^3;                    	// DS1302复位
sbit SCK = P1^7;                    	// DS1302时钟
sbit SDA = P2^3;                    	// DS1302数据
// DS1302写
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
// DS1302写字节，ucAddr的最低位为0
void DS1302_WriteByte(unsigned char ucAddr, unsigned char ucData)
{
  RST = 0;
  SCK = 0;
  RST = 1;
  DS1302_Write(ucAddr);
  DS1302_Write(ucData);
  RST = 0;
}
// DS1302读字节，ucAddr的最低位为1
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
// 设置RTC时钟：pucRtc-时钟值（时分秒：BCD码）
void DS1302_SetRtc(unsigned char* pucRtc)
{
  DS1302_WriteByte(0x8E, 0);          // WP=0：允许写操作
  DS1302_WriteByte(0x84, pucRtc[0]);  // 设置时
  DS1302_WriteByte(0x82, pucRtc[1]);  // 设置分
  DS1302_WriteByte(0x80, pucRtc[2]);  // 设置秒
  DS1302_WriteByte(0x8E, 0x80);       // WP=1：禁止写操作
}
// 获取RTC时钟：pucRtc-时钟值（时分秒：BCD码）
void DS1302_GetRtc(unsigned char* pucRtc)
{
  pucRtc[0] = DS1302_ReadByte(0x85);  // 读取时
  pucRtc[1] = DS1302_ReadByte(0x83);  // 读取分
  pucRtc[2] = DS1302_ReadByte(0x81);  // 读取秒
}
