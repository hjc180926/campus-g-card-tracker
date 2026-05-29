#ifndef _XUNJI_H
#define _XUNJI_H

#include "stm32f10x.h"
#include "motor.h"
#include "delay.h"

// 五路灰度传感器引脚（原有配置不变）
#define L2_PIN     GPIO_Pin_5
#define L2_PORT    GPIOA
#define L1_PIN     GPIO_Pin_6
#define L1_PORT    GPIOA
#define M_PIN      GPIO_Pin_7
#define M_PORT     GPIOA
#define R1_PIN     GPIO_Pin_0
#define R1_PORT    GPIOB
#define R2_PIN     GPIO_Pin_1
#define R2_PORT    GPIOB

// ==================== 红外循迹速度（原有参数，固定不变） ====================
#define STRAIGHT_SPEED      220
#define LIGHT_TURN          160
#define MID_TURN            100

// ==================== 视觉循迹独立速度（可自由调节） ====================
#define VIS_STRAIGHT_SPEED  160
#define VIS_LIGHT_TURN      100
#define VIS_MID_TURN        40

// 函数声明
void Track_Init(void);
void Track_Run(void);        // 原有红外循迹
void Track_Vision_Run(void); // 新增视觉循迹

#endif