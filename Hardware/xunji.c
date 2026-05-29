#include "xunji.h"

// 红外模式：记录上一次电机速度，丢线保持
static int16_t last_left  = -STRAIGHT_SPEED;
static int16_t last_right = -STRAIGHT_SPEED;

void Track_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IPU;  // 上拉输入
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStruct.GPIO_Pin = L2_PIN | L1_PIN | M_PIN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = R1_PIN | R2_PIN;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Track_Run(void)
{
    // 读取五路灰度传感器
    uint8_t L2 = !GPIO_ReadInputDataBit(L2_PORT, L2_PIN);
    uint8_t L1 = !GPIO_ReadInputDataBit(L1_PORT, L1_PIN);
    uint8_t M  = !GPIO_ReadInputDataBit(M_PORT,  M_PIN);
    uint8_t R1 = !GPIO_ReadInputDataBit(R1_PORT, R1_PIN);
    uint8_t R2 = !GPIO_ReadInputDataBit(R2_PORT, R2_PIN);

    // 1. 全黑 → 停车
    if(L2 == 0 && L1 == 0 && M == 0 && R1 == 0 && R2 == 0)
    {
        Motor_SetSpeed(0, 0);
    }
    // 2. 全白(丢线) → 保持上一次状态
    else if(L2 == 1 && L1 == 1 && M == 1 && R1 == 1 && R2 == 1)
    {
        Motor_SetSpeed(last_left, last_right);
    }
    // 3. 中间探头压线 → 直行
    else if(M == 0)
    {
        last_left  = -STRAIGHT_SPEED;
        last_right = -STRAIGHT_SPEED;
        Motor_SetSpeed(last_left, last_right);
    }
    // 4. 轻微左偏
    else if(L1 == 0)
    {
        last_left  = -LIGHT_TURN;
        last_right = -STRAIGHT_SPEED;
        Motor_SetSpeed(last_left, last_right);
    }
    // 5. 轻微右偏
    else if(R1 == 0)
    {
        last_left  = -STRAIGHT_SPEED;
        last_right = -LIGHT_TURN;
        Motor_SetSpeed(last_left, last_right);
    }
    // 6. 严重左偏
    else if(L2 == 0)
    {
        last_left  = -MID_TURN;
        last_right = -STRAIGHT_SPEED;
        Motor_SetSpeed(last_left, last_right);
    }
    // 7. 严重右偏
    else if(R2 == 0)
    {
        last_left  = -STRAIGHT_SPEED;
        last_right = -MID_TURN;
        Motor_SetSpeed(last_left, last_right);
    }
}

// ===================== 视觉/减速板循迹（完整灰度逻辑，低速参数） =====================
// 视觉模式专属：记录上一次电机速度，实现丢线保持（独立变量）
static int16_t vis_last_left  = -VIS_STRAIGHT_SPEED;
static int16_t vis_last_right = -VIS_STRAIGHT_SPEED;

void Track_Vision_Run(void)
{
    // 读取五路灰度传感器
    uint8_t L2 = !GPIO_ReadInputDataBit(L2_PORT, L2_PIN);
    uint8_t L1 = !GPIO_ReadInputDataBit(L1_PORT, L1_PIN);
    uint8_t M  = !GPIO_ReadInputDataBit(M_PORT,  M_PIN);
    uint8_t R1 = !GPIO_ReadInputDataBit(R1_PORT, R1_PIN);
    uint8_t R2 = !GPIO_ReadInputDataBit(R2_PORT, R2_PIN);

    // 1. 全黑 → 停车
    if(L2 == 0 && L1 == 0 && M == 0 && R1 == 0 && R2 == 0)
    {
        Motor_SetSpeed(0, 0);
    }
    // 2. 全白丢线 → 保持上一次运动状态
    else if(L2 == 1 && L1 == 1 && M == 1 && R1 == 1 && R2 == 1)
    {
        Motor_SetSpeed(vis_last_left, vis_last_right);
    }
    // 3. 中间压线 → 直行（使用视觉低速参数）
    else if(M == 0)
    {
        vis_last_left  = -VIS_STRAIGHT_SPEED;
        vis_last_right = -VIS_STRAIGHT_SPEED;
        Motor_SetSpeed(vis_last_left, vis_last_right);
    }
    // 4. 轻微左偏
    else if(L1 == 0)
    {
        vis_last_left  = -VIS_LIGHT_TURN;
        vis_last_right = -VIS_STRAIGHT_SPEED;
        Motor_SetSpeed(vis_last_left, vis_last_right);
    }
    // 5. 轻微右偏
    else if(R1 == 0)
    {
        vis_last_left  = -VIS_STRAIGHT_SPEED;
        vis_last_right = -VIS_LIGHT_TURN;
        Motor_SetSpeed(vis_last_left, vis_last_right);
    }
    // 6. 严重左偏
    else if(L2 == 0)
    {
        vis_last_left  = -VIS_MID_TURN;
        vis_last_right = -VIS_STRAIGHT_SPEED;
        Motor_SetSpeed(vis_last_left, vis_last_right);
    }
    // 7. 严重右偏
    else if(R2 == 0)
    {
        vis_last_left  = -VIS_STRAIGHT_SPEED;
        vis_last_right = -VIS_MID_TURN;
        Motor_SetSpeed(vis_last_left, vis_last_right);
    }
}
