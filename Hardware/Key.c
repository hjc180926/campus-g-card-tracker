#include "stm32f10x.h"
#include "Delay.h"

// 按键引脚：
// PA4、PB3、PB4、PC13
// 按键一端接IO，一端接GND，上拉输入，按下低电平
void Key_Init(void)
{
    // 开启时钟：GPIOA GPIOB GPIOC AFIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB 
                            | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    // 关闭JTAG 释放PB3 PB4为普通GPIO，保留SWD下载
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;   // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    // PA4 按键
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PB3 PB4 按键
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // PC13 按键
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

// 按键扫描：按下返回1 松开返回0 带20ms消抖
uint8_t Key_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    if(GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == 0)
    {
        Delay_ms(20);       // 软件消抖
        if(GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == 0)
        {
            return 1;
        }
    }
    return 0;
}
