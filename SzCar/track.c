#include "track.h"
#include "motor.h"

// 记录上一次的偏转方向：0为直行，1为向左丢线(需左转找线)，2为向右丢线(需右转找线)
static unsigned char last_state = 0; 

void Track_Control(void)
{
    // 将5个传感器状态合并为一个字节，方便进行状态机判断
    // 布局：T1(左外) T2(左内) T3(中) T4(右内) T5(右外)
    unsigned char sensor = 0;
    
    // 硬件逻辑：黑线=0，白底=1。
    // 这里进行逻辑提取：检测到黑线时，将sensor对应位置1
    if(TRACK_1 == 0) sensor |= 0x10; // 0001 0000 (极左)
    if(TRACK_2 == 0) sensor |= 0x08; // 0000 1000 (微左)
    if(TRACK_3 == 0) sensor |= 0x04; // 0000 0100 (正中)
    if(TRACK_4 == 0) sensor |= 0x02; // 0000 0010 (微右)
    if(TRACK_5 == 0) sensor |= 0x01; // 0000 0001 (极右)
    
    // 此时 sensor 变量完美映射了黑线的真实物理位置
    switch(sensor)
    {
        // ================= 直行区间 =================
        case 0x04: // 00100: 仅T3压线
        case 0x0E: // 01110: T2,T3,T4压线 (遇到十字路口或黑线较宽)
        case 0x1F: // 11111: 全黑 (到达终点或停止线)
            Motor_Forward();
            last_state = 0;
            break;
            
        // ================= 左转纠偏 =================
        // 车体偏右 -> 左侧传感器压线 -> 必须向左转
        case 0x08: // 01000: 仅T2压线
        case 0x0C: // 01100: T2,T3同时压线
            Motor_Left();
            last_state = 1; // 记忆：车身最后一次是靠左侧摸到线的
            break;
            
        case 0x10: // 10000: 仅T1压线 (大幅偏右)
        case 0x18: // 11000: T1,T2同时压线
            Motor_Left();
            last_state = 1;
            break;
            
        // ================= 右转纠偏 =================
        // 车体偏左 -> 右侧传感器压线 -> 必须向右转
        case 0x02: // 00010: 仅T4压线
        case 0x06: // 00110: T3,T4同时压线
            Motor_Right();
            last_state = 2; // 记忆：车身最后一次是靠右侧摸到线的
            break;
            
        case 0x01: // 00001: 仅T5压线 (大幅偏左)
        case 0x03: // 00011: T4,T5同时压线
            Motor_Right();
            last_state = 2;
            break;
            
        // ================= 极速脱轨 / 盲区 =================
        case 0x00: // 00000: 5个传感器全是白底
            if(last_state == 1)
            {
                Motor_Left();  // 之前是偏右导致的脱线，继续左转把线找回来
            }
            else if(last_state == 2)
            {
                Motor_Right(); // 之前是偏左导致的脱线，继续右转把线找回来
            }
            else
            {
                Motor_Forward(); // 没有任何偏转记录，大概率是刚发车时的全白场地，直行寻找
            }
            break;
            
        // ================= 未知噪声 =================
        default:
            // 遇到类似 10101 的不连续跳变噪声，保持之前的运动状态，这里默认直行滤除干扰
            Motor_Forward();
            break;
    }
}