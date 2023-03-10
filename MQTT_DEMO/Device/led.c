/*
 * @version: V1.0.0
 * @Author: harmsworth
 * @Date: 2023-03-07 10:16:46
 * @LastEditors: harmsworth
 * @LastEditTime: 2023-03-07 10:17:54
 * @company: null
 * @Mailbox: jojoharmsworth@gmail.com
 * @FilePath: \MDK-ARMg:\Program\Stm32\MQTT_DEMO\Device\led.c
 * @Descripttion:
 */
#include "led.h"
#include "gpio.h"

void LED_R_ON(void)
{
    HAL_GPIO_WritePin(LEDR_GPIO_PORT, LEDR_GPIO_PIN, GPIO_PIN_RESET);
}

void LED_R_OFF(void)
{
    HAL_GPIO_WritePin(LEDR_GPIO_PORT, LEDR_GPIO_PIN, GPIO_PIN_SET);
}
