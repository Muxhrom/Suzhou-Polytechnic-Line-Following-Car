#ifndef __TRACK_H__
#define __TRACK_H__

#include <REGX51.H>

sbit TRACK_1 = P1^0;
sbit TRACK_2 = P1^1;
sbit TRACK_3 = P1^2;
sbit TRACK_4 = P1^3;
sbit TRACK_5 = P1^4;

void Track_Control(void);

#endif