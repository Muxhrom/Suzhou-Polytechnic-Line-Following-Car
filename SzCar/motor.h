#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <REGX51.H>

sbit M_L_IA = P3^4;
sbit M_L_IB = P3^5;
sbit M_R_IA = P3^6;
sbit M_R_IB = P3^7;

sbit BEEP = P2^3;
sbit LED_GATE = P2^0;

void Motor_Init(void);
void Motor_SetSpeed(unsigned char left_speed, unsigned char right_speed); // 新增：设置速度范围0~10
void Motor_Stop(void);
void Motor_Forward(void);
void Motor_Backward(void);
void Motor_Left(void);
void Motor_Right(void);
void Motor_Tick(void); // 新增：由定时器调用的滴答函数

#endif