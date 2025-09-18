#define DELAY_TIME 5
// I2C引脚定义
sfr  P2 = 0xA0;
sbit SCL = P2^0;              	// 时钟线
sbit SDA = P2^1;              	// 数据线

void I2C_Delay(unsigned char i)
{
  while(i--);
}
// I2C起始条件
void I2C_Start(void)
{
  SDA = 1;
  SCL = 1;
  I2C_Delay(DELAY_TIME);
  SDA = 0;
  I2C_Delay(DELAY_TIME);
}
// I2C停止条件
void I2C_Stop(void)
{
  SDA = 0;
  SCL = 1;
  I2C_Delay(DELAY_TIME);
  SDA = 1;
  I2C_Delay(DELAY_TIME);
}
// I2C发送应答：0-应答，1-非应答
void I2C_SendAck(bit bAck)
{
  SCL = 0;
  SDA = bAck;
  I2C_Delay(DELAY_TIME);
  SCL = 1;
  I2C_Delay(DELAY_TIME);
  SCL = 0; 
  SDA = 1;
  I2C_Delay(DELAY_TIME);
}
// I2C等待应答
bit I2C_WaitAck(void)
{
  bit bAck;

  SCL  = 1;
  I2C_Delay(DELAY_TIME);
  bAck = SDA;
  SCL = 0;
  I2C_Delay(DELAY_TIME);
  return bAck;
}
// I2C发送数据
void I2C_SendByte(unsigned char ucData)
{
  unsigned char i;

  for (i=0; i<8; i++)
  {
    SCL  = 0;
    I2C_Delay(DELAY_TIME);
    if (ucData & 0x80)
      SDA  = 1;
    else
      SDA  = 0;
    I2C_Delay(DELAY_TIME);
    SCL = 1;
    ucData <<= 1;
    I2C_Delay(DELAY_TIME);
  }
  SCL  = 0;
}
// I2C接收数据
unsigned char I2C_RecvByte(void)
{
  unsigned char i, ucData;

  for (i=0; i<8; i++)
  {
    SCL = 1;
    I2C_Delay(DELAY_TIME);
    ucData <<= 1;
    if (SDA)
      ucData |= 1;
    SCL = 0;
    I2C_Delay(DELAY_TIME);
  }
  return ucData;
}
#ifdef AT24C02
// AT24C02缓存器写：pucBuf-数据，ucAddr-地址，ucNum-数量
void AT24C02_WriteBuffer(unsigned char *pucBuf,
  unsigned char ucAddr, unsigned char ucNum)
{
  I2C_Start();
  I2C_SendByte(0xa0);          	// 发送器件地址及控制位（写）
  I2C_WaitAck();

  I2C_SendByte(ucAddr);       	// 发送数据地址
  I2C_WaitAck();

  while (ucNum--)
  {
    I2C_SendByte(*pucBuf++);  	// 发送数据
    I2C_WaitAck();
    I2C_Delay(200);
  }
  I2C_Stop();
}
// AT24C02缓存器读：pucBuf-数据，ucAddr-地址，ucNum-数量
void AT24C02_ReadBuffer(unsigned char *pucBuf,
  unsigned char ucAddr, unsigned char ucNum)
{
  I2C_Start();
  I2C_SendByte(0xa0);         	// 发送器件地址及控制位（写）
  I2C_WaitAck();

  I2C_SendByte(ucAddr);       	// 发送数据地址
  I2C_WaitAck();

  I2C_Start();
  I2C_SendByte(0xa1);        	// 发送器件地址及控制位（读）
  I2C_WaitAck();

  while (ucNum--)
  {
    *pucBuf++ = I2C_RecvByte();   	// 接收数据
    if (ucNum)
      I2C_SendAck(0);
    else
      I2C_SendAck(1);
  }
  I2C_Stop();
}
#endif
#ifdef PCF8591
// PCF8591 ADC：ucAin-ADC通道（0~3），返回值-ADC值
unsigned char PCF8591_Adc(unsigned char ucAin)
{
  unsigned char ucAdc;

  I2C_Start();
  I2C_SendByte(0x90);         	// 发送器件地址及控制位（写）
  I2C_WaitAck();

  I2C_SendByte(ucAin + 0x40);	// 发送控制字（ADC通道，允许DAC）
  I2C_WaitAck();

  I2C_Start();
  I2C_SendByte(0x91);         	// 发送器件地址及控制位（读）
  I2C_WaitAck();

  ucAdc = I2C_RecvByte();     	// 接收ADC值
  I2C_SendAck(1);
  I2C_Stop();

  return ucAdc;
}
// PCF8591 DAC: ucDac-DAC值
void PCF8591_Dac(unsigned char ucDac)
{
  I2C_Start();
  I2C_SendByte(0x90);            // 发送器件地址及控制位（写）
  I2C_WaitAck();

  I2C_SendByte(0x40);            // 发送控制字（允许DAC）
  I2C_WaitAck();

  I2C_SendByte(ucDac);           // 发送DAC值
  I2C_WaitAck();
  I2C_Stop();
}
#endif
