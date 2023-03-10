/*
 * @version: V1.0.0
 * @Author: harmsworth
 * @Date: 2023-03-06 15:01:55
 * @LastEditors: harmsworth
 * @LastEditTime: 2023-03-06 15:29:05
 * @company: null
 * @Mailbox: jojoharmsworth@gmail.com
 * @FilePath: \MDK-ARMg:\Program\Stm32\MQTT_DEMO\UI\ui.c
 * @Descripttion:
 */
#include "ui.h"

void draw(u8g2_t *u8g2)
{
    u8g2_SetFontMode(u8g2, 1); // Transparent
    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf);
    u8g2_DrawStr(u8g2, 0, 20, "U");

    u8g2_SetFontDirection(u8g2, 1);
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf);
    u8g2_DrawStr(u8g2, 21, 8, "8");

    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_wqy12_t_chinese1);
    u8g2_DrawStr(u8g2, 51, 30, "g");
    u8g2_DrawUTF8(u8g2, 10, 50, "你好,world");

    u8g2_DrawHLine(u8g2, 2, 35, 47);
    u8g2_DrawHLine(u8g2, 3, 36, 47);
    u8g2_DrawVLine(u8g2, 45, 32, 12);
    u8g2_DrawVLine(u8g2, 46, 33, 12);
}

void testDrawProcess(u8g2_t *u8g2)
{
    for (int i = 10; i <= 80; i = i + 2)
    {
        u8g2_ClearBuffer(u8g2);

        char buff[20];
        sprintf(buff, "%d%%", (int)(i / 80.0 * 100));

        u8g2_SetFont(u8g2, u8g2_font_ncenB12_tf);
        u8g2_DrawStr(u8g2, 16, 32, "STM32 U8g2"); // 字符显示

        u8g2_SetFont(u8g2, u8g2_font_ncenB08_tf);
        u8g2_DrawStr(u8g2, 100, 49, buff); // 当前进度显示

        u8g2_DrawRBox(u8g2, 16, 43, i, 3, 1);    // 圆角填充框矩形框
        u8g2_DrawRFrame(u8g2, 16, 43, 80, 3, 1); // 圆角矩形

        u8g2_SendBuffer(u8g2);
        // HAL_Delay(500);
    }
}

void dataDisplay(u8g2_t *u8g2)
{
    u8g2_ClearBuffer(u8g2);

    char strBuff[128];
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);

    sprintf(strBuff, "tempture: %d.%d °C", DHT11_Data.temp_int, DHT11_Data.temp_deci);
    u8g2_DrawUTF8(u8g2, 1, 15, strBuff);

    sprintf(strBuff, "humidity: %d.%d %%RH", DHT11_Data.humi_int, DHT11_Data.humi_deci);
    u8g2_DrawUTF8(u8g2, 1, 25, strBuff);

    sprintf(strBuff, "illumination: %d lx", Light);
    u8g2_DrawUTF8(u8g2, 1, 35, strBuff);

    u8g2_SendBuffer(u8g2);
}
