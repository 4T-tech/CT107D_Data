#ifndef __I2C_H
#define __I2C_H
void AT24C02_WriteBuffer(unsigned char *pucBuf,
  unsigned char ucAddr, unsigned char ucNum);
void AT24C02_ReadBuffer(unsigned char *pucBuf,
  unsigned char ucAddr, unsigned char ucNum);

unsigned char PCF8591_Adc(unsigned char ucAin);
void PCF8591_Dac(unsigned char ucDac);
#endif
