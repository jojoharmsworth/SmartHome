/**
 ************************************************************
 ************************************************************
 ************************************************************
 *	文件名： 	esp8266.c
 *
 *	作者： 		张继瑞
 *
 *	日期： 		2017-05-08
 *
 *	版本： 		V1.0
 *
 *	说明： 		ESP8266的简单驱动
 *
 *	修改记录：
 ************************************************************
 ************************************************************
 ************************************************************
 **/

// 单片机头文件
#include "stm32f1xx.h"

// 网络设备驱动
#include "esp8266.h"

// 硬件驱动
#include "usart.h"

// C库
#include <string.h>
#include <stdio.h>

#include "MqttKit.h"

#if HOME
#define ESP8266_WIFI_INFO "AT+CWJAP=\"ChinaNet-8952\",\"fd4v3rd6\"\r\n"
#else
#define ESP8266_WIFI_INFO "AT+CWJAP=\"TP-LINK_2.4G\",\"hrstekR&D\"\r\n"
#endif

#define ESP8266_ONENET_INFO "AT+CIPSTART=\"TCP\",\"39.98.92.2\",1883\r\n"

uint8_t esp01_rx_buf[1];
unsigned char esp8266_buf[128];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;
extern char *devSubTopic[];

//==========================================================
//	函数名称：	ESP8266_Clear
//
//	函数功能：	清空缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

//==========================================================
//	函数名称：	ESP8266_WaitRecive
//
//	函数功能：	等待接收完成
//
//	入口参数：	无
//
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if (esp8266_cnt == 0) // 如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;

	if (esp8266_cnt == esp8266_cntPre) // 如果上一次的值和这次相同，则说明接收完毕
	{
		esp8266_cnt = 0; // 清0接收计数

		return REV_OK; // 返回接收完成标志
	}

	esp8266_cntPre = esp8266_cnt; // 置为相同

	return REV_WAIT; // 返回接收未完成标志
}

//==========================================================
//	函数名称：	ESP8266_SendCmd
//
//	函数功能：	发送命令
//
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//
//	返回参数：	0-成功	1-失败
//
//	说明：
//==========================================================
_Bool ESP8266_SendCmd(uint8_t *cmd, char *res)
{

	unsigned char timeOut = 200;

	Usart_SendString(huart2, cmd, strlen((const char *)cmd));

	while (timeOut--)
	{
		if (ESP8266_WaitRecive() == REV_OK) // 如果收到数据
		{
			if (strstr((const char *)esp8266_buf, res) != NULL) // 如果检索到关键词
			{
				ESP8266_Clear(); // 清空缓存

				return 0;
			}
		}

		HAL_Delay(10);
	}

	return 1;
}

//==========================================================
//	函数名称：	ESP8266_SendData
//
//	函数功能：	发送数据
//
//	入口参数：	data：数据
//				len：长度
//
//	返回参数：	无
//
//	说明：
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];

	ESP8266_Clear();						   // 清空接收缓存
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len); // 发送命令
	if (!ESP8266_SendCmd(cmdBuf, ">"))		   // 收到‘>’时可以发送数据
	{
		Usart_SendString(huart2, data, len); // 发送设备连接请求数据
	}
}

//==========================================================
//	函数名称：	ESP8266_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;

	do
	{
		if (ESP8266_WaitRecive() == REV_OK) // 如果接收完成
		{
			ptrIPD = strstr((char *)esp8266_buf, "IPD,"); // 搜索“IPD”头
			if (ptrIPD == NULL)							  // 如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				// printf(USART_DEBUG, "\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':'); // 找到':'
				if (ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
			}
		}

		HAL_Delay(5); // 延时等待
	} while (timeOut--);

	return NULL; // 超时还未找到，返回空指针
}

//==========================================================
//	函数名称：	ESP8266_Init
//
//	函数功能：	初始化ESP8266
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：
//==========================================================
void ESP8266_Init(void)
{

	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_Delay(250);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_SET);
	HAL_Delay(500);

	ESP8266_Clear();

	printf("0. AT\r\n");
	while (ESP8266_SendCmd("AT\r\n", "OK"))
		HAL_Delay(250);

	printf("1. RST\r\n");
	ESP8266_SendCmd("AT+RST\r\n", "OK");
	HAL_Delay(250);

	printf("2. CWMODE\r\n");
	while (ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
		HAL_Delay(250);

	printf("3. AT+CWDHCP\r\n");
	while (ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
		HAL_Delay(250);

	printf("4. CWJAP\r\n");
	while (ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
		HAL_Delay(250);

	printf("5. CIPMUX\r\n");
	while (ESP8266_SendCmd("AT+CIPMUX=0\r\n", "OK"))
		HAL_Delay(250);

	printf("6. CIPSTART\r\n");
	while (ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
		HAL_Delay(250);

	printf("7. ESP8266 Init OK\r\n");
}

//==========================================================
//	函数名称：	USART2_IRQHandler
//
//	函数功能：	串口2收发中断
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：
//==========================================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
		if (esp8266_cnt >= sizeof(esp8266_buf))
			esp8266_cnt = 0; // 防止串口被刷爆
		esp8266_buf[esp8266_cnt++] = esp01_rx_buf[0];
	}
	HAL_UART_Receive_IT(&huart2, esp01_rx_buf, 1);
}

void MQTT_Ping(void)
{
	unsigned char *dataPtr;
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};
	if (MQTT_PacketPing(&mqttPacket) == 0) // 心跳包组包
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);
		printf("发送的心跳数据:%#X", mqttPacket._data);
		printf("Ping data:%x\r\n", mqttPacket._data);
		MQTT_DeleteBuffer(&mqttPacket); // 删除包释放内存
		dataPtr = ESP8266_GetIPD(100);	// 等待响应
		if (dataPtr != NULL)
		{
			if (MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_PINGRESP)
			{
				// 确定是心跳响应
				printf("接收的心跳数据:%#X", MQTT_PKT_PINGRESP);
				printf("Ping succeed\r\n");
			}
		}
		else
		{
			printf("Ping fail\r\n"); // 响应失败/重新连接平台
			while (OneNet_DevLink())
			{
				HAL_Delay(500);
				printf("加载ESP8266\r\n");
				ESP8266_Init();
			}

			printf("重连成功!\r\n");
			OneNet_Subscribe(devSubTopic, 1);
		}
	}
}
