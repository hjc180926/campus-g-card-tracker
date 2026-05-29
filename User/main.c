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

// ===================== 赛事参数（不变） =====================
#define ONE_CIRCLE_CM    314    
#define MAX_SET_CIRCLE   2      

// ===================== 全局标志位 =====================
uint8_t run_flag = 0;           // 运行标志：0停止 1运行
uint8_t set_circle = 2;         
uint8_t now_circle = 0;
uint8_t last_circle = 0;        
uint8_t send_lock = 0;          // 串口发送锁：保证只发一次'1'
uint8_t track_mode = 0;         // 循迹模式：0=红外循迹  1=视觉循迹

// 函数声明
void Show_Track_Level(void);
void Alarm_Circle(void);

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

    TIM3_Init();  
    time_run_flag = 0;   

    while(1)
    {
        Show_Track_Level(); 

        // ========== PB4 启动循迹（核心修改：动态设置目标圈数） ==========
        if(Key_Scan(GPIOB, GPIO_Pin_4))
        {
            if(run_flag == 0)
            {
                run_flag = 1;
                time_run_flag = 1;
                
                now_circle = 0;
                last_circle = 0;
                // ===== 动态分配圈数 =====
                if(track_mode == 0)
                {
                    set_circle = 2;   // 红外模式：跑2圈
                }
                else
                {
                    set_circle = 1;   // 视觉模式：只跑1圈
                }
                send_lock = 0;  // 解锁发送，下次可正常发指令
                Reset_Distance();
            }
        }

        // ========== PB3：切换循迹模式 ==========
        if(Key_Scan(GPIOB, GPIO_Pin_3))
        {
            track_mode = !track_mode;
            OLED_Clear();   // 切换模式刷新屏幕
            Delay_ms(300);
        }

        if(run_flag)
        {
            // 根据当前模式，选择对应循迹函数
            if(track_mode == 0)
            {
                Track_Run();        // 原有红外循迹
            }
            else
            {
                Track_Vision_Run(); // 新增视觉循迹
            }

            // 跑完单圈，声光提示
            if(now_circle != last_circle)
            {
                Alarm_Circle();
                last_circle = now_circle;
            }

            // 跑满设定圈数，单次发送'1'给K210
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
                
                USART3_SendChar('1');
                send_lock = 1; // 上锁，禁止重复发送
            }
        }
        else
        {
            Motor_Stop();
        }

        Delay_ms(10);
    }
}

// 跑完一圈声光提示（原有代码不变）
void Alarm_Circle(void)
{
    LED_ON();
    FMQ_ON();
    Delay_ms(500);
    LED_OFF();
	FMQ_OFF();
}

// OLED固定显示（删除页面切换，新增模式显示）
void Show_Track_Level(void)
{
    int16_t left_pulse  = Read_Encoder(2);
    int16_t right_pulse = Read_Encoder(4);

    Update_Distance(left_pulse, right_pulse);
    int32_t dist = Get_Total_CM();

    now_circle = dist / ONE_CIRCLE_CM;
    if(now_circle > MAX_SET_CIRCLE) now_circle = MAX_SET_CIRCLE;

    // 运行状态
    if(run_flag)
    {
        OLED_ShowString(1, 1, "State:Run ");
    }
    else
    {
        OLED_ShowString(1, 1, "State:Stop");
    }
    // 圈数
    OLED_ShowNum(1, 11, now_circle, 1);
    OLED_ShowString(1, 12, "/");
    OLED_ShowNum(1, 13, set_circle, 2);

    // 计时
    uint32_t total_sec = run_time_ms / 1000;
    uint8_t min = total_sec / 60;
    uint8_t sec = total_sec % 60;
    OLED_ShowString(2, 1, "Time:");
    OLED_ShowNum(2, 6, min, 2);
    OLED_ShowString(2, 8, ":");
    OLED_ShowNum(2, 9, sec, 2);

    // 路程
    OLED_ShowString(3, 1, "Dist:");
    OLED_ShowNum(3, 6, dist, 4);
    OLED_ShowString(3, 11, "cm");
    
    // 显示当前循迹模式
    if(track_mode == 0)
    {
        OLED_ShowString(4, 1, "Mode:IR Track");
    }
    else
    {
        OLED_ShowString(4, 1, "Mode:Vision");
    }
}
