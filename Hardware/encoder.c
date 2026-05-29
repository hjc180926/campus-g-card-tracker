#include "encoder.h"

// 全局变量：累计总脉冲
static int32_t total_pulse = 0;

// ===================== 左轮编码器 TIM2 初始化（无需修改，引脚/模式与MG513X兼容） =====================
void Encoder_TIM2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // MG513X编码器自带上拉，浮空输入即可采集
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_TimeBaseInitStruct.TIM_Period = 65535;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);

    // 四倍频编码器模式（MG513X支持正交计数，无需修改）
    TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12,
                               TIM_ICPolarity_Falling, TIM_ICPolarity_Falling);

    TIM_ICStructInit(&TIM_ICInitStruct);
    TIM_ICInitStruct.TIM_ICFilter = 8;  // 滤波消抖（MG513X霍尔编码器无磁抖，可保留）
    TIM_ICInit(TIM2, &TIM_ICInitStruct);
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM2, &TIM_ICInitStruct);

    TIM_SetCounter(TIM2, 0);
    TIM_Cmd(TIM2, ENABLE);
}

// ===================== 右轮编码器 TIM4 初始化（无需修改，引脚/模式与MG513X兼容） =====================
void Encoder_TIM4_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_TimeBaseInitStruct.TIM_Period = 65535;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);

    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12,
                               TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

    TIM_ICStructInit(&TIM_ICInitStruct);
    TIM_ICInitStruct.TIM_ICFilter = 8;
    TIM_ICInit(TIM4, &TIM_ICInitStruct);
    TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM4, &TIM_ICInitStruct);

    TIM_SetCounter(TIM4, 0);
    TIM_Cmd(TIM4, ENABLE);
}

// ===================== 读取编码器脉冲（读后清零，无需修改） =====================
int16_t Read_Encoder(int TIMx)
{
    int16_t val = 0;
    switch(TIMx)
    {
        case 2: val = (int16_t)TIM_GetCounter(TIM2); TIM_SetCounter(TIM2, 0); break;
        case 4: val = (int16_t)TIM_GetCounter(TIM4); TIM_SetCounter(TIM4, 0); break;
    }
    return val;
}

// ===================== 更新里程（仅调用1次，无重复累计，无需修改） =====================
void Update_Distance(int16_t left, int16_t right)
{
    total_pulse += (abs(left) + abs(right)) / 2;
}

// ===================== 获取总里程（cm，核心依赖PULSE_PER_CM，已适配MG513X） =====================
int32_t Get_Total_CM(void)
{
    return total_pulse / PULSE_PER_CM;
}

// ===================== 清零里程（无需修改） =====================
void Reset_Distance(void)
{
    total_pulse = 0;
}

// 空中断服务函数（无需修改）
void TIM2_IRQHandler(void){}
void TIM4_IRQHandler(void){}

	