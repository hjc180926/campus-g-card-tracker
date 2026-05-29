#ifndef __KEY_H
#define __KEY_H

void Key_Init(void);
uint8_t Key_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#endif
