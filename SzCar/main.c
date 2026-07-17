#include <REGX51.H>
#include "oled.h"
#include "motor.h"
#include "track.h"
#include "gate.h"

// IAP15F2K61S2 IO口模式寄存器
sfr P1M1 = 0x91;
sfr P1M0 = 0x92;
sfr P2M1 = 0x95;
sfr P2M0 = 0x96;
sfr P3M1 = 0xB1;
sfr P3M0 = 0xB2;
sfr AUXR = 0x8E; // STC15辅助寄存器，用于设置定时器1T模式

// 引脚定义
sbit KEY_START = P3^3;

// 全局变量
unsigned int gate_count = 0;
bit run_flag = 0;
volatile unsigned int time_count = 0; // 加入 volatile，防止硬件中断修改此变量时被编译器优化

// 函数声明
void Delay(unsigned int ms);
void System_Init(void);
void Display_Init(void);
void Display_Update(void);
void Timer0_Init(void); // 定时器初始化声明

void main(void)
{
    unsigned char i;
    unsigned char blink;
    unsigned int time_temp;         // 用于原子读取，防止16位变量读写撕裂
    static bit key_lock = 0;        // 按键自锁标志，实现非阻塞检测
    
    System_Init();
    Delay(500); // 等待OLED上电稳定
    Display_Init();
    
    while(1)
    {
        // 非阻塞式按键消抖扫描
        if(KEY_START == 0)
        {
            Delay(20); // 软件消抖
            if(KEY_START == 0 && key_lock == 0)
            {
                key_lock = 1; // 加锁，防止长按时重复触发起停逻辑
                
                if(run_flag == 0)
                {
                    run_flag = 1;
                    gate_count = 0;
                    time_count = 0;
                    OLED_Clear();
                    
                    OLED_ShowString(0, 0, "RUNNING...");
                    OLED_ShowString(0, 2, "GATES:");
                    OLED_ShowString(0, 4, "TIME:   S"); 
                }
                else
                {
                    run_flag = 0;
                    Motor_Stop();
                    
                    OLED_Clear();
                    OLED_ShowString(28, 0, "FINAL RESULT");
                    OLED_ShowString(0, 2, "TOTAL GATES:");
                    OLED_ShowNum(72, 2, gate_count, 2);
                    
                    blink = (gate_count > 20) ? 20 : (unsigned char)gate_count;
                    for(i = 0; i < blink; i++)
                    {
                        LED_GATE = 0;
                        Delay(400);
                        LED_GATE = 1;
                        Delay(600);
                    }
                }
            }
        }
        else
        {
            key_lock = 0; // 松手释放锁，等待下一次按下
        }
        
        if(run_flag)
        {
            Track_Control();
            Gate_Detect();
            Display_Update();
            
            // 原子读取 time_count，防止主循环与中断冲突导致的数据脏读
            EA = 0;
            time_temp = time_count;
            EA = 1;
            
            if(time_temp >= 120000)
            {
                run_flag = 0;
                Motor_Stop();
                OLED_Clear();
                OLED_ShowString(37, 0, "TIME OUT!");
                OLED_ShowString(0, 2, "GATES:");
                OLED_ShowNum(36, 2, gate_count, 2);
            }
        }
        
        Delay(1);
    }
}

void Delay(unsigned int ms)  //@12.000MHz
{
    unsigned char data i, j;

    while(ms--)
    {
        i = 12;
        j = 169;
        do
        {
            while (--j);
        } while (--i);
    }
}


void System_Init(void)
{
    // P1口配置暂留原样（注意：当前配置为推挽输出模式）
    P1M1 = 0x00; P1M0 = 0xFF;
    P2M1 = 0x00; P2M0 = 0xFF;
    P3M1 = 0x00; P3M0 = 0x00;
    
    Motor_Init();
    OLED_Init();
    Gate_Init();
    Timer0_Init(); // 启动系统定时器，开始 1ms 心跳
    
    BEEP = 0;
    LED_GATE = 1;
}

void Display_Init(void)
{
    OLED_Clear();
    OLED_ShowString(28, 0, "SUZHOU INST."); 
    OLED_ShowString(37, 2, "CAR READY");    
    OLED_ShowString(37, 4, "PRESS KEY");    
    OLED_ShowString(40, 6, "TO START");     
}

void Display_Update(void)
{
    unsigned int sec;
    unsigned int time_temp;
    
    OLED_ShowNum(36, 2, gate_count, 2); 
    
    EA = 0;
    time_temp = time_count;
    EA = 1;
    
    sec = time_temp / 1000;
    if(sec > 480) sec = 480;
    OLED_ShowNum(30, 4, sec, 3);
}

// 定时器0初始化 (12MHz, 1T模式, 1ms中断)
void Timer0_Init(void)
{
    AUXR |= 0x80;    
    TMOD &= 0xF0;    
    TL0 = 0x20;      
    TH0 = 0xD1;      
    TF0 = 0;         
    TR0 = 1;         
    ET0 = 1;         
    EA = 1;          
}

// 定时器0中断服务
void Timer0_ISR(void) interrupt 1
{
    // 全局运行时间统计
    if(run_flag)
    {
        time_count++; 
    }

    Motor_Tick();    // 驱动电机PWM
    Gate_Tick();     // 处理蜂鸣器与LED
}