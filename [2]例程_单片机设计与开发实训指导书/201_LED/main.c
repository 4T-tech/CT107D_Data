// 使用程序前，将J13调整为IO模式（2-3脚短接）
sfr P0 = 0x80;
sfr P2 = 0xA0;
// P0输出: ucData——数据，ucAddr——地址（4~7）
void P0_Out(unsigned char ucData, unsigned char ucAddr)
{
  P0 = ucData;                      	// P0输出数据
  P2 |= ucAddr << 5;                	// 置位P27~P25
  P2 &= 0x1f;                       	// 复位P27~P25
//XBYTE[ucAddr << 13] = ucData;     	// MM模式（J13-2和J13-1相连）
}
// 关闭外设
void Close_Peripheral(void)
{
  P2 &= 0x1f;                       	// 复位P27~P25
  P0_Out(0xff, 4);                  	// 熄灭LED
  P0_Out(~0x50, 5);                 	// 关闭继电器和蜂鸣器
}
// LED显示
void Led_Disp(unsigned char ucLed)
{
  P0_Out(~ucLed, 4);                	// LED显示
}
// 延时函数（最小约1ms@12MHz）
void Delay(unsigned int uiNum)
{
  unsigned int i;

  while (uiNum--)
    for (i=0; i<628; i++);
}
// 主函数
void main(void)
{
  unsigned char i, j;
  Close_Peripheral();

  while (1)
  { // 4个亮度等级
    for (i=0; i<4; i++)
      for (j=0; j<64; j++)
      {
        Led_Disp(0xff);             	// L1~L8点亮
        Delay(i+1);                  	// 延时时间依次是1ms、2ms、3ms和4ms
        Led_Disp(0);                 	// L1~L8熄灭
        Delay(4-i);                  	// 延时时间依次是4ms、3ms、2ms和1ms
      }
  }
}
