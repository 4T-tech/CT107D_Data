#define DELAY_TIME 5
// I2C���Ŷ���
sfr  P2 = 0xA0;
sbit SCL = P2^0;              	// ʱ����
sbit SDA = P2^1;              	// ������

void I2C_Delay(unsigned char i)
{
  while(i--);
}
// I2C��ʼ����
void I2C_Start(void)
{
  SDA = 1;
  SCL = 1;
  I2C_Delay(DELAY_TIME);
  SDA = 0;
  I2C_Delay(DELAY_TIME);
}
// I2Cֹͣ����
void I2C_Stop(void)
{
  SDA = 0;
  SCL = 1;
  I2C_Delay(DELAY_TIME);
  SDA = 1;
  I2C_Delay(DELAY_TIME);
}
// I2C����Ӧ��0-Ӧ��1-��Ӧ��
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
// I2C�ȴ�Ӧ��
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
// I2C��������
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
// I2C��������
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
// AT24C02������д��pucBuf-���ݣ�ucAddr-��ַ��ucNum-����
void AT24C02_WriteBuffer(unsigned char *pucBuf,
  unsigned char ucAddr, unsigned char ucNum)
{
  I2C_Start();
  I2C_SendByte(0xa0);          	// ����������ַ������λ��д��
  I2C_WaitAck();

  I2C_SendByte(ucAddr);       	// �������ݵ�ַ
  I2C_WaitAck();

  while (ucNum--)
  {
    I2C_SendByte(*pucBuf++);  	// ��������
    I2C_WaitAck();
    I2C_Delay(200);
  }
  I2C_Stop();
}
// AT24C02����������pucBuf-���ݣ�ucAddr-��ַ��ucNum-����
void AT24C02_ReadBuffer(unsigned char *pucBuf,
  unsigned char ucAddr, unsigned char ucNum)
{
  I2C_Start();
  I2C_SendByte(0xa0);         	// ����������ַ������λ��д��
  I2C_WaitAck();

  I2C_SendByte(ucAddr);       	// �������ݵ�ַ
  I2C_WaitAck();

  I2C_Start();
  I2C_SendByte(0xa1);        	// ����������ַ������λ������
  I2C_WaitAck();

  while (ucNum--)
  {
    *pucBuf++ = I2C_RecvByte();   	// ��������
    if (ucNum)
      I2C_SendAck(0);
    else
      I2C_SendAck(1);
  }
  I2C_Stop();
}
#endif
#ifdef PCF8591
// PCF8591 ADC��ucAin-ADCͨ����0~3��������ֵ-ADCֵ
unsigned char PCF8591_Adc(unsigned char ucAin)
{
  unsigned char ucAdc;

  I2C_Start();
  I2C_SendByte(0x90);         	// ����������ַ������λ��д��
  I2C_WaitAck();

  I2C_SendByte(ucAin + 0x40);	// ���Ϳ����֣�ADCͨ��������DAC��
  I2C_WaitAck();

  I2C_Start();
  I2C_SendByte(0x91);         	// ����������ַ������λ������
  I2C_WaitAck();

  ucAdc = I2C_RecvByte();     	// ����ADCֵ
  I2C_SendAck(1);
  I2C_Stop();

  return ucAdc;
}
// PCF8591 DAC: ucDac-DACֵ
void PCF8591_Dac(unsigned char ucDac)
{
  I2C_Start();
  I2C_SendByte(0x90);            // ����������ַ������λ��д��
  I2C_WaitAck();

  I2C_SendByte(0x40);            // ���Ϳ����֣�����DAC��
  I2C_WaitAck();

  I2C_SendByte(ucDac);           // ����DACֵ
  I2C_WaitAck();
  I2C_Stop();
}
#endif
