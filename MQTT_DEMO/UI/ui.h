#ifndef __UI_H
#define __UI_H
#include "stm32f1xx.h"

#include "dht11.h"
#include "u8g2.h"
#include "u8x8.h"
#include <stdio.h>

extern DHT11_Data_TypeDef DHT11_Data;
extern uint16_t Light;

typedef struct
{
    uint8_t initFlag;
    uint8_t linkFlag;
}UI_FLAG_TypeDef;

void draw(u8g2_t *u8g2);
void testDrawProcess(u8g2_t *u8g2);
void dataDisplay(u8g2_t *u8g2);

#endif