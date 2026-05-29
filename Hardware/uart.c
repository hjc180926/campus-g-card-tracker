#include "uart.h"

// USART3 对接K210 115200 仅发送，无接收、无中断
void USART3_K210_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStruct;
    USART_InitTypeDef       USART_InitStruct;

    // 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // PB11 TX 复用推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // PB10 闲置，配置上拉输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 串口参数：115200 8N1 仅发送模式
    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx;

    USART_Init(USART3, &USART_InitStruct);
    USART_Cmd(USART3, ENABLE);
}

// 串口发送单个字符
void USART3_SendChar(uint8_t ch)
{
    USART_SendData(USART3, ch);
    // 等待发送数据寄存器为空
    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}
