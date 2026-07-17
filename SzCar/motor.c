#include "motor.h"

static unsigned char motor_dir_L = 0; // 左轮方向 0:停, 1:进, 2:退
static unsigned char motor_dir_R = 0; // 右轮方向 0:停, 1:进, 2:退

// 占空比变量 (取值范围 0~20)，对应 0% ~ 100%
// 默认值设为 8，代表 40% 初始启动速度
static unsigned char motor_pwm_L = 5; 
static unsigned char motor_pwm_R = 5; 

void Motor_Init(void)
{
    M_L_IA = 0; M_L_IB = 0;
    M_R_IA = 0; M_R_IB = 0;
}

// 改变速度 (参数范围调整为 0~20，最小精度 5%)
void Motor_SetSpeed(unsigned char left_speed, unsigned char right_speed)
{
    if(left_speed > 20) left_speed = 20;
    if(right_speed > 20) right_speed = 20;
    motor_pwm_L = left_speed;
    motor_pwm_R = right_speed;
}

void Motor_Stop(void)
{
    motor_dir_L = 0;
    motor_dir_R = 0;
}

void Motor_Forward(void)
{
    motor_dir_L = 1;
    motor_dir_R = 1;
}

void Motor_Backward(void)
{
    motor_dir_L = 2;
    motor_dir_R = 2;
}

void Motor_Left(void)
{
    motor_dir_L = 0;
    motor_dir_R = 1;
}

void Motor_Right(void)
{
    motor_dir_L = 1;
    motor_dir_R = 0;
}

// 由 Timer0 中断每 1ms 调用一次，负责引脚的高低电平翻转
void Motor_Tick(void)
{
    static unsigned char pwm_count = 0;
    
    pwm_count++;
    // 将周期上限设为 20，即 20ms 完成一次 PWM 周期 (频率 50Hz)
    if(pwm_count >= 20) pwm_count = 0; 
    
    // ================= 左电机 PWM 控制 =================
    // 当前计数值小于设定占空比时，输出高电平驱动
    if(pwm_count < motor_pwm_L)
    {
        if(motor_dir_L == 1)      { M_L_IA = 1; M_L_IB = 0; } // 进
        else if(motor_dir_L == 2) { M_L_IA = 0; M_L_IB = 1; } // 退
        else                      { M_L_IA = 0; M_L_IB = 0; } // 停
    }
    // 达到占空比阈值后，输出低电平（滑行模式）
    else
    {
        M_L_IA = 0; M_L_IB = 0;
    }
    
    // ================= 右电机 PWM 控制 =================
    if(pwm_count < motor_pwm_R)
    {
        if(motor_dir_R == 1)      { M_R_IA = 0; M_R_IB = 1; } // 进 (硬件反接纠偏)
        else if(motor_dir_R == 2) { M_R_IA = 1; M_R_IB = 0; } // 退
        else                      { M_R_IA = 0; M_R_IB = 0; } // 停
    }
    else
    {
        M_R_IA = 0; M_R_IB = 0;
    }
}