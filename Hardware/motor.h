#ifndef _MOTOR_H
#define _MOTOR_H

#include "stm32f10x.h"

// ===================== 电赛G题 小车板电机引脚定义 =====================
// 左电机方向控制
#define AIN1_PORT    GPIOB
#define AIN1_PIN     GPIO_Pin_13
#define AIN2_PORT    GPIOB
#define AIN2_PIN     GPIO_Pin_12

// 右电机方向控制
#define BIN1_PORT    GPIOB
#define BIN1_PIN     GPIO_Pin_14
#define BIN2_PORT    GPIOB
#define BIN2_PIN     GPIO_Pin_15

// 电机PWM控制
#define PWMA_PORT    GPIOA
#define PWMA_PIN     GPIO_Pin_11
#define PWMB_PORT    GPIOA
#define PWMB_PIN     GPIO_Pin_8

#define PWM_MAX   999
#define PWM_MIN  -999

// 函数声明
void Motor_Init(void);
void Motor_SetSpeed(int16_t left, int16_t right);
void Motor_Stop(void);  // 新增：电机停止函数
int GFP_abs(int p);

#endif

