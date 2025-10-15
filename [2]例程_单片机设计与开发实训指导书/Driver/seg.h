#ifndef __SEG_H
#define __SEG_H
void Close_Peripheral(void);
void Led_Disp(unsigned char ucLed);
void Uln_Ctrl(unsigned char ucUln);
void Seg_Tran(unsigned char *pucSeg_Buf, unsigned char *pucSeg_Code);
void Seg_Disp(unsigned char ucSeg_Code, unsigned char ucSeg_Pos);
#endif
