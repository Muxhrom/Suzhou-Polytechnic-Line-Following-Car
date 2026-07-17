#ifndef __OLED_H__
#define __OLED_H__

#include <REGX51.H>

sbit OLED_SCL = P2^1;
sbit OLED_SDA = P2^2;

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char chr);
void OLED_ShowString(unsigned char x, unsigned char y, unsigned char *str);
void OLED_ShowNum(unsigned char x, unsigned char y, unsigned int num, unsigned char len);

#endif