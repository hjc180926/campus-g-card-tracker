#include "stm32f10x_it.h"
#include "uart.h"

void NMI_Handler(void) {}
void HardFault_Handler(void) { while (1) {} }
void MemManage_Handler(void) { while (1) {} }
void BusFault_Handler(void) { while (1) {} }
void UsageFault_Handler(void) { while (1) {} }
void SVC_Handler(void) {}
void DebugMon_Handler(void) {}
void PendSV_Handler(void) {}
void SysTick_Handler(void) {}

// USART3???????K230???????
void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        uint8_t ch = USART_ReceiveData(USART3);

        if(ch == '\n')
        {
            uart_rx_buf[uart_rx_len] = '\0';
            uart_rx_complete = 1;
            uart_rx_len = 0;
        }
        else if(uart_rx_len < UART_RX_BUF_SIZE - 1)
        {
            uart_rx_buf[uart_rx_len++] = ch;
        }

        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}
