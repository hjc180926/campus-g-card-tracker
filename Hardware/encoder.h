#ifndef  _ENCODER_H
#define  _ENCODER_H
#include "stm32f10x.h"
#include <stdlib.h>

// ========== MG513X 霍尔电机 精准参数 ==========
// 13PPR霍尔编码器 + 1:28减速 + 四倍频计数 + 65mm直径轮
#define PULSE_PER_CM    71     // MG513X：1厘米≈71脉冲（已按65mm轮校准）
#define SPEED_MUL       5      // 速度计算乘数（可保留原参数，无需修改）
#define SPEED_DIV       6      // 速度计算除数（可保留原参数，无需修改）

// 函数声明
void Encoder_TIM2_Init(void);  // 左轮（PA0/PA1，TIM2）
void Encoder_TIM4_Init(void);  // 右轮（PB6/PB7，TIM4）
int16_t Read_Encoder(int TIMx);
int32_t Get_Total_CM(void);    // 获取总里程(cm)
void Reset_Distance(void);    // 清零里程

#endif

