sfr SCON = 0x98;
sfr SBUF = 0x99;
sfr AUXR = 0x8e;
sfr T2L = 0xd7;
sfr T2H = 0xd6;
sbit TI = SCON^1;
sbit RI = SCON^0;

void Uart_Init(void)                	// 9600bps@12.000MHz
{
  SCON = 0x50;                       	// 8位数据,可变波特率
  AUXR |= 0x01;                      	// 串行口1选择定时器2为波特率发生器
  AUXR |= 0x04;                      	// 定时器2时钟为Fosc, 即1T
  T2L = 0xC7;                        	// 设定定时初值
  T2H = 0xFE;                        	// 设定定时初值
  AUXR |= 0x10;                      	// 启动定时器2
}

       unsigned char ucUart_Dly;    	// 串行口接收字符延时
extern unsigned char pucUart_Buf[10];	// 串行口缓存
extern unsigned char ucUart_Num;    	// 串行口接收字符计数
void Uart_RecvString(void)
{
  if (RI)
  {
    pucUart_Buf[ucUart_Num++] = SBUF;
    RI = 0;
    ucUart_Dly = 0;                 	// 开始计时
  }
  if ((ucUart_Dly > 200) && (ucUart_Num != 0))
  {                                  	// 串行口接收完成
    pucUart_Buf[ucUart_Num] = 0;
    ucUart_Num |= 0x80;             	// 设置接收完成标志
  }
}

void Uart_SendChar(unsigned char c)
{
  SBUF = c;
  while (TI == 0);
  TI = 0;
}
