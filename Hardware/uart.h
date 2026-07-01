#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"

#define UART_RX_BUF_SIZE  32

extern uint8_t  uart_rx_buf[UART_RX_BUF_SIZE];
extern uint8_t  uart_rx_len;
extern uint8_t  uart_rx_complete;

void USART3_K210_Init(void);
void USART3_SendChar(uint8_t ch);

#endif
