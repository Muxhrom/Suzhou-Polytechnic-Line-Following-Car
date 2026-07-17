#ifndef __GATE_H__
#define __GATE_H__

#include <REGX51.H>

sbit GATE_SENSOR = P3^2;

extern unsigned int gate_count;

void Gate_Init(void);
void Gate_Detect(void);
void Gate_Tick(void); //新增

#endif