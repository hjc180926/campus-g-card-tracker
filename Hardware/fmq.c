#include "stm32f10x.h"

// 蜂鸣器引脚：PB5  高电平触发响
void FMQ_Init(void)
{
    // 开启GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;  // 改为 PB5
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOB, GPIO_Pin_5);  // 初始状态：关闭
}

// 开启蜂鸣器
void FMQ_ON(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_5);
}

// 关闭蜂鸣器
void FMQ_OFF(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
}

// 翻转蜂鸣器状态
void FMQ_Turn(void)
{
    if(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_5) == 0)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_5);
    }
    else
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    }
}
