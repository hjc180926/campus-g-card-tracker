#include "motor.h"

static void Motor_LimitSpeed(int16_t *left, int16_t *right);

void Motor_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = AIN1_PIN | AIN2_PIN | BIN1_PIN | BIN2_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOB, AIN1_PIN | AIN2_PIN | BIN1_PIN | BIN2_PIN);

    GPIO_InitStructure.GPIO_Pin   = PWMA_PIN | PWMB_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_TimeBaseStructure.TIM_Prescaler     = 71;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period        = 999;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = 0;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;

    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

// 新增：电机彻底停止函数
void Motor_Stop(void)
{
    Motor_SetSpeed(0, 0);
}

void Motor_SetSpeed(int16_t left, int16_t right)
{
    Motor_LimitSpeed(&left, &right);

    if(left > 0)
    {
        GPIO_ResetBits(AIN1_PORT, AIN1_PIN);
        GPIO_SetBits(AIN2_PORT, AIN2_PIN);
    }
    else if(left < 0)
    {
        GPIO_SetBits(AIN1_PORT, AIN1_PIN);
        GPIO_ResetBits(AIN2_PORT, AIN2_PIN);
    }
    else
    {
        GPIO_ResetBits(AIN1_PORT, AIN1_PIN);
        GPIO_ResetBits(AIN2_PORT, AIN2_PIN);
    }
    TIM_SetCompare4(TIM1, GFP_abs(left));

    if(right > 0)
    {
        GPIO_ResetBits(BIN1_PORT, BIN1_PIN);
        GPIO_SetBits(BIN2_PORT, BIN2_PIN);
    }
    else if(right < 0)
    {
        GPIO_SetBits(BIN1_PORT, BIN1_PIN);
        GPIO_ResetBits(BIN2_PORT, BIN2_PIN);
    }
    else
    {
        GPIO_ResetBits(BIN1_PORT, BIN1_PIN);
        GPIO_ResetBits(BIN2_PORT, BIN2_PIN);
    }
    TIM_SetCompare1(TIM1, GFP_abs(right));
}

static void Motor_LimitSpeed(int16_t *left, int16_t *right)
{
    if(*left > PWM_MAX)      *left = PWM_MAX;
    if(*left < PWM_MIN)      *left = PWM_MIN;
    if(*right > PWM_MAX)     *right = PWM_MAX;
    if(*right < PWM_MIN)     *right = PWM_MIN;
}

int GFP_abs(int p)
{
    return p > 0 ? p : -p;
}

