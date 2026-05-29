#include "stm32f10x.h"

// PA2 高电平点亮LED
void LED_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    // 仅配置 PA2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 初始状态：低电平 → 熄灭
    GPIO_ResetBits(GPIOA, GPIO_Pin_2);
}

// 高电平点亮
void LED_ON(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_2);
}

// 低电平熄灭
void LED_OFF(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_2);
}

// 电平翻转
void LED_Turn(void)
{
    if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2) == 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_2);
    }
    else
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_2);
    }
}
