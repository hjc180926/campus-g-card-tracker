#include "stm32f10x.h"
#include "Delay.h"
#include "oled.h"
#include "xunji.h"
#include "motor.h"
#include "encoder.h"
#include "tim.h"
#include "led.h"
#include "fmq.h"
#include "uart.h"
#include "key.h"
#include "string.h"
#include "stdio.h"

#define ONE_CIRCLE_CM    314
#define MAX_SET_CIRCLE   2

uint8_t run_flag = 0;
uint8_t set_circle = 2;
uint8_t now_circle = 0;
uint8_t last_circle = 0;
uint8_t send_lock = 0;
uint8_t track_mode = 0;
uint8_t card_page = 0;

#define MAX_CARDS  4
uint8_t card_count = 0;
char    card_list[MAX_CARDS][20];

void Alarm_Circle(void);
void Display_Page(void);

int main(void)
{
    Key_Init();
    USART3_K210_Init();

    LED_Init();
    LED_OFF();

    FMQ_Init();
    FMQ_OFF();

    OLED_Init();
    Motor_Init();
    Track_Init();
    Encoder_TIM2_Init();
    Encoder_TIM4_Init();
    Motor_Stop();
    OLED_Clear();
    OLED_ShowString(1, 1, "Ready");

    TIM3_Init();
    // time_run_flag starts at 0, only starts on PB4 press

    uart_rx_len = 0;
    card_count = 0;

    while(1)
    {
        Display_Page();

        // PB4 - start only
        if(Key_Scan(GPIOB, GPIO_Pin_4))
        {
            if(run_flag == 0)
            {
                run_flag = 1;
                time_run_flag = 1;
                now_circle = 0;
                last_circle = 0;
                if(track_mode == 0)
                    set_circle = 2;
                else if(track_mode == 1)
                    set_circle = 1;
                else
                    set_circle = 1;
                send_lock = 0;
                Reset_Distance();
                OLED_Clear();
            }
        }

        // PB3 - cycle modes: 0(xunji) -> 1(vision) -> 2(card) -> 0
        if(Key_Scan(GPIOB, GPIO_Pin_3))
        {
            track_mode++;
            if(track_mode > 2) track_mode = 0;
            card_page = 0;
            OLED_Clear();
            Delay_ms(300);
        }

        if(run_flag)
        {
            if(track_mode == 0)
                Track_Run();
            else if(track_mode == 1)
                Track_Vision_Run();

            if(now_circle != last_circle)
            {
                Alarm_Circle();
                last_circle = now_circle;
            }

            if(now_circle >= set_circle && send_lock == 0)
            {
                run_flag = 0;
                Motor_Stop();
                time_run_flag = 0;
                LED_ON();
                FMQ_ON();
                Delay_ms(1000);
                LED_OFF();
                FMQ_OFF();
                send_lock = 1;
                OLED_Clear();
            }
        }
        else
        {
            Motor_Stop();
        }

        if(uart_rx_complete)
        {
            uart_rx_complete = 0;
            if(card_count < MAX_CARDS)
            {
                strcpy(card_list[card_count], (char*)uart_rx_buf);
                card_count++;
            }
            if(track_mode == 2)
            {
                OLED_Clear();
            }
        }

        Delay_ms(10);
    }
}

void Alarm_Circle(void)
{
    LED_ON();
    FMQ_ON();
    Delay_ms(500);
    LED_OFF();
    FMQ_OFF();
}

void Display_Page(void)
{
    if(track_mode == 0 || track_mode == 1)
    {
        int16_t left_pulse  = Read_Encoder(2);
        int16_t right_pulse = Read_Encoder(4);
        Update_Distance(left_pulse, right_pulse);
        int32_t dist = Get_Total_CM();

        now_circle = dist / ONE_CIRCLE_CM;
        if(now_circle > MAX_SET_CIRCLE) now_circle = MAX_SET_CIRCLE;

        if(run_flag)
            OLED_ShowString(1, 1, "State:Run ");
        else
            OLED_ShowString(1, 1, "State:Stop");
        OLED_ShowNum(1, 11, now_circle, 1);
        OLED_ShowString(1, 12, "/");
        OLED_ShowNum(1, 13, set_circle, 2);

        uint32_t total_sec = run_time_ms / 1000;
        uint8_t min = total_sec / 60;
        uint8_t sec = total_sec % 60;
        OLED_ShowString(2, 1, "Time:");
        OLED_ShowNum(2, 6, min, 2);
        OLED_ShowString(2, 8, ":");
        OLED_ShowNum(2, 9, sec, 2);

        OLED_ShowString(3, 1, "Dist:");
        OLED_ShowNum(3, 6, dist, 4);
        OLED_ShowString(3, 11, "cm");

        if(track_mode == 0)
            OLED_ShowString(4, 1, "Mode:IR Track");
        else
            OLED_ShowString(4, 1, "Mode:Vision  ");
    }
    else
    {
        // Card mode: show nothing until cards arrive
        if(card_count == 0)
        {
            OLED_ShowString(1, 1, "                ");
            OLED_ShowString(2, 1, "                ");
            OLED_ShowString(3, 1, "                ");
            OLED_ShowString(4, 1, "                ");
        }
        else
        {
            char buf[17];
            uint8_t i;
            for(i = 0; i < 4; i++)
            {
                if(i < card_count)
                {
                    sprintf(buf, "%-16s", card_list[i]);
                    OLED_ShowString(1 + i, 1, buf);
                }
                else
                {
                    OLED_ShowString(1 + i, 1, "                ");
                }
            }
        }
    }
}