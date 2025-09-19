sfr  P0 = 0x80;
sfr  P2 = 0xA0;
// P0���: ucData�������ݣ�ucAddr������ַ��4~7��
void P0_Out(unsigned char ucData, unsigned char ucAddr)
{
  P0 = ucData;                       	// P0�������
  P2 |= ucAddr << 5;                	// ��λP27~P25
  P2 &= 0x1f;                       	// ��λP27~P25
}
// LED��ʾ: ucLed����LEDֵ
void Led_Disp(unsigned char ucLed)
{
  P0_Out(~ucLed, 4);
}
// ULN���ƣ�ucUln=0���رռ̵�����ucUln=0x10���򿪼̵���
void Uln_Ctrl(unsigned char ucUln)
{
  P0_Out(ucUln, 5);
}
// �ر�����
void Close_Peripheral(void)
{
  P2 &= 0x1f;                         // ��λP27~P25
  Led_Disp(0);                        // Ϩ��LED
  Uln_Ctrl(0);                        // �رռ̵����ͷ�����
}
// ��ʾ����ת����pucSeg_Char����ʾ�ַ���pucSeg_Code����ʾ����
void Seg_Tran(unsigned char *pucSeg_Buf, unsigned char *pucSeg_Code)
{
  unsigned char i, j=0, temp;
  for (i=0; i<8; i++, j++)
  {
    switch (pucSeg_Buf[j]) 
    { // �͵�ƽ�����Σ�����[MSB...LSB]��Ӧ��˳��Ϊ[dp g f e d c b a]
      case '0': temp = 0xc0; break;        	// 1 1 0 0 0 0 0 0
      case '1': temp = 0xf9; break;        	// 1 1 1 1 1 0 0 1
//    case '1': temp = 0xcf; break;        	// 1 1 0 0 1 1 1 1
      case '2': temp = 0xa4; break;
      case '3': temp = 0xb0; break;
//    case '3': temp = 0x86; break;
      case '4': temp = 0x99; break;
//    case '4': temp = 0x8b; break;
      case '5': temp = 0x92; break;
      case '6': temp = 0x82; break;
//    case '6': temp = 0x90; break;
      case '7': temp = 0xf8; break;
//    case '7': temp = 0xc7; break;
      case '8': temp = 0x80; break;
      case '9': temp = 0x90; break;
//    case '9': temp = 0x82; break;
      case 'A': temp = 0x88; break;
      case 'B': temp = 0x83; break;
      case 'C': temp = 0xc6; break;
      case 'D': temp = 0xA1; break;
      case 'E': temp = 0x86; break;
      case 'F': temp = 0x8E; break;
      case 'H': temp = 0x89; break;
      case 'L': temp = 0xC7; break;
      case 'N': temp = 0xC8; break;
      case 'P': temp = 0x8c; break;
      case 'U': temp = 0xC1; break;
      case '-': temp = 0xbf; break;
      case ' ': temp = 0xff; break;
      case '^': temp = 0xfe; break;         // 1 1 1 1 1 1 1 0
      case '_': temp = 0xf7; break;         // 1 1 1 1 0 1 1 1
	    default: temp = 0xff;
    }
    if (pucSeg_Buf[j+1] == '.')
    {
      temp = temp&0x7f;              	// ����С����
      j++;
    }
    pucSeg_Code[i] = temp;
  }
}
// �������ʾ��ucSeg_Code-��ʾ���룬ucSeg_Pos-��ʾλ��
void Seg_Disp(unsigned char ucSeg_Code, unsigned char ucSeg_Pos)
{
  P0_Out(0xFF, 7);                  	// ����
  P0_Out(1<<ucSeg_Pos, 6);          	// λѡ
  P0_Out(ucSeg_Code, 7);            	// ��ѡ
}
