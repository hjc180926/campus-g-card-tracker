#include "tim.h"
#include "encoder.h"
#include "motor.h"

uint32_t run_time_ms = 0;   // 运行总时间（毫秒）
uint8_t  time_run_flag = 0; // 计时启动标志
uint8_t  oled_refresh_flag = 0; // OLED刷新标志
uint16_t encoder_tick = 0;  // 编码器读取计时

// TIM3 1ms中断初始化（72MHz）
void TIM3_Init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef         NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = 999;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// ===================== TIM3 1ms 中断服务函数（G题核心） =====================
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        // 1. 累计行驶时间（小车启动后开始计时）
        if(time_run_flag)
        {
            run_time_ms++;
        }

        // 2. 5ms 读取一次编码器（计算速度/里程）
        encoder_tick++;
        if(encoder_tick >= 5)
        {
            int16_t left_pulse = Read_Encoder(2);
            int16_t right_pulse = Read_Encoder(4);
            Update_Distance(left_pulse, right_pulse);
            encoder_tick = 0;
        }

        // 3. 50ms 刷新一次OLED（防止闪烁）
        if(run_time_ms % 50 == 0)
        {
            oled_refresh_flag = 1;
        }

        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}
