#include "gate.h"
#include "motor.h"

// === 蜂鸣器与指示灯硬件电平适配宏 ===
#define BEEP_ON()   { BEEP = 1; }
#define BEEP_OFF()  { BEEP = 0; }
#define LED_ON()    { LED_GATE = 0; }
#define LED_OFF()   { LED_GATE = 1; }

// === 红外传感器业务逻辑宏 ===
#define GATE_IDLE_STATE    1  
#define GATE_TRIGGER_STATE 0     

extern unsigned int gate_count;
static bit processing = 0;

volatile unsigned int gate_beep_timer = 0;
volatile unsigned char gate_beep_count = 0;
volatile bit is_gate_beeping = 0;
// 新增：200ms 屏蔽死区倒计时变量
volatile unsigned int gate_deadzone_timer = 0; 

void Gate_Init(void)
{
    processing = 0;
    is_gate_beeping = 0;
    gate_deadzone_timer = 0;
    BEEP_OFF(); 
    LED_OFF();
}

void Gate_Tick(void)
{
    // 1. 死区倒计时逻辑 (每1ms递减)
    if(gate_deadzone_timer > 0)
    {
        gate_deadzone_timer--;
    }

    // 2. 蜂鸣器异步报警逻辑
    if(is_gate_beeping)
    {
        if(gate_beep_timer > 0)
        {
            gate_beep_timer--;
        }
        else
        {
            if(gate_beep_count > 0)
            {
                gate_beep_count--;
                BEEP = !BEEP;           
                LED_GATE = !LED_GATE;   
                gate_beep_timer = 300;  
            }
            else
            {
                is_gate_beeping = 0;
                BEEP_OFF();
                LED_OFF();
            }
        }
    }
}

void Gate_Detect(void)
{
    static unsigned char debounce_timer = 0;
    static bit last_sensor_state = GATE_IDLE_STATE; 
    bit current_state = GATE_SENSOR;

    if(current_state != last_sensor_state)
    {
        debounce_timer++;
        // 维持较低的消抖阈值，确保极速过门不漏检
        if(debounce_timer >= 2) 
        {
            last_sensor_state = current_state;
            debounce_timer = 0;

            // 核心修改：触发条件必须同时满足不在屏蔽死区内 (gate_deadzone_timer == 0)
            if(last_sensor_state == GATE_TRIGGER_STATE && processing == 0 && gate_deadzone_timer == 0)
            {
                processing = 1; 
                gate_count++;

                EA = 0; 
                gate_beep_count = 5;    
                gate_beep_timer = 300;  
                is_gate_beeping = 1;  
                gate_deadzone_timer = 200; // 设定 200ms 屏蔽期，期间不再响应任何过门信号
                BEEP_ON();
                LED_ON();
                EA = 1; 
            }
        }
    }
    else
    {
        debounce_timer = 0; 
    }

    // 释放锁逻辑保持不变，确保车身离开大门后解锁
    if(last_sensor_state == GATE_IDLE_STATE && processing == 1)
    {
        processing = 0;
    }
}