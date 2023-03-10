#ifndef __LED_H
#define __LED_H
#include "stm32f1xx.h"

#define LEDR_GPIO_PORT      GPIOB
#define LEDR_GPIO_PIN       GPIO_PIN_5

void LED_R_ON(void);
void LED_R_OFF(void);

#endif
