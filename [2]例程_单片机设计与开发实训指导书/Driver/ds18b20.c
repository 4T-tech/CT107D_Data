sfr  P1 = 0x90;
sbit DQ = P1^4;                  // 单总线接口
// DS18B20延时函数
void DS18B20_Delay(unsigned int t)
{
  while (t--);
}
// DS18B20初始化
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
// DS18B20写字节
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
// DS18B20读字节
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
// 读取温度
unsigned int Temp_Read(void)
{
  unsigned char low, high;

  DS18B20_Init();                // 初始化
  DS18B20_Write(0xCC);           // 跳过ROM
  DS18B20_Write(0x44);           // 启动温度转换

  DS18B20_Init();
  DS18B20_Write(0xCC);
  DS18B20_Write(0xBE);           // 读取温度值
  low = DS18B20_Read();          // 低字节
  high = DS18B20_Read();         // 高字节

  return (high<<8)+low;
}
