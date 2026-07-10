#ifndef __TIM_H
#define __TIM_H

#include "stm32f10x.h"

extern uint32_t run_time_ms;   // 运行总时间（毫秒）
extern uint8_t  time_run_flag;  // 计时运行标志
extern uint8_t  oled_refresh_flag; // OLED刷新标志

void TIM3_Init(void);

#endif
