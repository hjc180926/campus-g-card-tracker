#ifndef __FMQ_H
#define __FMQ_H

void FMQ_Init(void);   // 蜂鸣器初始化
void FMQ_ON(void);     // 蜂鸣器响（高电平）
void FMQ_OFF(void);    // 蜂鸣器停（低电平）
void FMQ_Turn(void);   // 蜂鸣器状态翻转

#endif

