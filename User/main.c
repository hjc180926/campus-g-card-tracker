#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "OLED_Data.h"
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

#define MAX_CARDS  5
uint8_t card_count = 0;
char    card_list[MAX_CARDS][20];

/* 滚动参数 */
#define LINE_HEIGHT     16
#define SCROLL_SPEED    6
uint16_t scroll_offset = 0;
uint16_t scroll_tick = 0;

void Alarm_Circle(void);
void OLED_ShowUTF8(uint16_t x, uint16_t y, char *str);
void Display_Page(void);

int main(void)
{
    Key_Init();
    USART3_K210_Init();

    LED_Init();
    LED_OFF();

    FMQ_Init();
    FMQ_OFF();

    /* 电机先初始化，再初始化OLED */
    Motor_Init();
    Track_Init();
    Encoder_TIM2_Init();
    Encoder_TIM4_Init();
    Motor_Stop();

    OLED_Init();

    TIM3_Init();

    uart_rx_len = 0;
    card_count = 0;

    while(1)
    {
        Display_Page();

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
            scroll_offset = 0;
            scroll_tick = 0;
        }

        Delay_ms(10);

        if(track_mode == 2 && card_count > 4)
        {
            scroll_tick++;
            if(scroll_tick >= SCROLL_SPEED)
            {
                scroll_tick = 0;
                scroll_offset++;
                if(scroll_offset > LINE_HEIGHT)
                    scroll_offset = 0;
            }
        }
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

void OLED_ShowUTF8(uint16_t x, uint16_t y, char *str)
{
    while(*str != '\0')
    {
        if((*str & 0x80) != 0)
        {
            char hz[4] = {0};
            hz[0] = str[0];
            hz[1] = str[1];
            hz[2] = str[2];
            OLED_ShowChinese(x, y, hz);
            x += 16;
            str += 3;
        }
        else
        {
            OLED_ShowChar(x, y, *str, OLED_8X16);
            x += 8;
            str += 1;
        }
    }
}

void Display_Page(void)
{
    static uint8_t idle_cnt = 0;
    uint8_t do_update = oled_refresh_flag;
    if(!do_update)
    {
        idle_cnt++;
        if(idle_cnt >= 5) { do_update = 1; idle_cnt = 0; }
    }
    else
    {
        oled_refresh_flag = 0;
        idle_cnt = 0;
    }

    if(track_mode == 0 || track_mode == 1)
    {
        int16_t left_pulse  = Read_Encoder(2);
        int16_t right_pulse = Read_Encoder(4);
        Update_Distance(left_pulse, right_pulse);
        int32_t dist = Get_Total_CM();

        now_circle = dist / ONE_CIRCLE_CM;
        if(now_circle > MAX_SET_CIRCLE) now_circle = MAX_SET_CIRCLE;

        if(run_flag)
            OLED_ShowString(0, 0, "State:Run ", OLED_8X16);
        else
            OLED_ShowString(0, 0, "State:Stop", OLED_8X16);
        OLED_ShowNum(80, 0, now_circle, 1, OLED_8X16);
        OLED_ShowChar(88, 0, '/', OLED_8X16);
        OLED_ShowNum(96, 0, set_circle, 2, OLED_8X16);

        uint32_t total_sec = run_time_ms / 1000;
        uint8_t min = total_sec / 60;
        uint8_t sec = total_sec % 60;
        OLED_ShowString(0, 16, "Time:", OLED_8X16);
        OLED_ShowNum(40, 16, min, 2, OLED_8X16);
        OLED_ShowChar(56, 16, ':', OLED_8X16);
        OLED_ShowNum(64, 16, sec, 2, OLED_8X16);

        OLED_ShowString(0, 32, "Dist:", OLED_8X16);
        OLED_ShowNum(40, 32, dist, 4, OLED_8X16);
        OLED_ShowString(80, 32, "cm", OLED_8X16);

        if(track_mode == 0)
            OLED_ShowString(0, 48, "Mode:IR Track", OLED_8X16);
        else
            OLED_ShowString(0, 48, "Mode:Vision  ", OLED_8X16);

        if(do_update) OLED_Update();
    }
    else
    {
        OLED_Clear();

        if(card_count == 0)
        {
            OLED_ShowString(0, 0, "Waiting Card...", OLED_8X16);
        }
        else
        {
            uint8_t i;
            for(i = 0; i < card_count; i++)
            {
                int16_t y = i * LINE_HEIGHT - scroll_offset;
                if(y >= -LINE_HEIGHT && y < 64)
                {
                    OLED_ShowUTF8(0, y, card_list[i]);
                }
            }
        }

        if(do_update) OLED_Update();
    }
}
