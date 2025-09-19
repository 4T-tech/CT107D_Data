// 运行程序时，将J5调整为KBD模式（1-2脚短接）
sfr  P3  = 0xB0;
sfr  P4  = 0xC0;
sbit P34 = P3^4;
sbit P35 = P3^5;
sbit P42 = P4^2;
sbit P44 = P4^4;

unsigned char Key_Read(void)
{
  unsigned int  uiKey_Code;

  P44 = 0; P42 = 1; P35 = 1; P34 = 1;	// 第1列（S4~S7）
  uiKey_Code = P3;
  P44 = 1; P42 = 0;                     	// 第2列（S8~S11）
  uiKey_Code = (uiKey_Code<<4) | (P3&0x0f);
  P42 = 1; P35 = 0;                     	// 第3列（S12~S15）
  uiKey_Code = (uiKey_Code<<4) | (P3&0x0f);
  P35 = 1; P34 = 1;                     	// 第4列（S16~S19）
  uiKey_Code = (uiKey_Code<<4) | (P3&0x0f);

  switch(~uiKey_Code)
  {
    case 0x8000: return 4;        	// S4
    case 0x4000: return 5;        	// S5
    case 0x2000: return 6;        	// S6
    case 0x1000: return 7;        	// S7
    case 0x0800: return 8;        	// S8
    case 0x0400: return 9;        	// S9
    case 0x0200: return 10;        	// S10
    case 0x0100: return 11;        	// S11
    case 0x0080: return 12;        	// S12
    case 0x0040: return 13;        	// S13
    case 0x0020: return 14;        	// S14
    case 0x0010: return 15;        	// S15
    case 0x0008: return 16;        	// S16
    case 0x0004: return 17;        	// S17
    case 0x0002: return 18;        	// S18
    case 0x0001: return 19;        	// S19
    case 0x0c00: return 20;        	// S8+S9
    default: return 0;
  }
// 独立按键程序代码，将J5调整为BTN模式（2-3脚短接）
/*switch(~P3 & 0xf)
  {
    case 0x8: return 4;            	// S4
    case 0x4: return 5;            	// S5
    case 0x2: return 6;            	// S6
    case 0x1: return 7;            	// S7
    default: return 0;
  }*/
}
