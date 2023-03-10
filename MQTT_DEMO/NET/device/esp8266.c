/**
 ************************************************************
 ************************************************************
 ************************************************************
 *	�ļ����� 	esp8266.c
 *
 *	���ߣ� 		�ż���
 *
 *	���ڣ� 		2017-05-08
 *
 *	�汾�� 		V1.0
 *
 *	˵���� 		ESP8266�ļ�����
 *
 *	�޸ļ�¼��
 ************************************************************
 ************************************************************
 ************************************************************
 **/

// ��Ƭ��ͷ�ļ�
#include "stm32f1xx.h"

// �����豸����
#include "esp8266.h"

// Ӳ������
#include "usart.h"

// C��
#include <string.h>
#include <stdio.h>

#if HOME
#define ESP8266_WIFI_INFO "AT+CWJAP=\"ChinaNet-8952\",\"fd4v3rd6\"\r\n"
#else
#define ESP8266_WIFI_INFO "AT+CWJAP=\"TP-LINK_2.4G\",\"hrstekR&D\"\r\n"
#endif

#define ESP8266_ONENET_INFO "AT+CIPSTART=\"TCP\",\"39.98.92.2\",1883\r\n"

uint8_t esp01_rx_buf[1];
unsigned char esp8266_buf[128];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

//==========================================================
//	�������ƣ�	ESP8266_Clear
//
//	�������ܣ�	��ջ���
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

//==========================================================
//	�������ƣ�	ESP8266_WaitRecive
//
//	�������ܣ�	�ȴ��������
//
//	��ڲ�����	��
//
//	���ز�����	REV_OK-�������		REV_WAIT-���ճ�ʱδ���
//
//	˵����		ѭ�����ü���Ƿ�������
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if (esp8266_cnt == 0) // ������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return REV_WAIT;

	if (esp8266_cnt == esp8266_cntPre) // �����һ�ε�ֵ�������ͬ����˵���������
	{
		esp8266_cnt = 0; // ��0���ռ���

		return REV_OK; // ���ؽ�����ɱ�־
	}

	esp8266_cntPre = esp8266_cnt; // ��Ϊ��ͬ

	return REV_WAIT; // ���ؽ���δ��ɱ�־
}

//==========================================================
//	�������ƣ�	ESP8266_SendCmd
//
//	�������ܣ�	��������
//
//	��ڲ�����	cmd������
//				res����Ҫ���ķ���ָ��
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����
//==========================================================
_Bool ESP8266_SendCmd(uint8_t *cmd, char *res)
{

	unsigned char timeOut = 200;

	Usart_SendString(huart2, cmd, strlen((const char *)cmd));

	while (timeOut--)
	{
		if (ESP8266_WaitRecive() == REV_OK) // ����յ�����
		{
			if (strstr((const char *)esp8266_buf, res) != NULL) // ����������ؼ���
			{
				ESP8266_Clear(); // ��ջ���

				return 0;
			}
		}

		HAL_Delay(10);
	}

	return 1;
}

//==========================================================
//	�������ƣ�	ESP8266_SendData
//
//	�������ܣ�	��������
//
//	��ڲ�����	data������
//				len������
//
//	���ز�����	��
//
//	˵����
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

	char cmdBuf[32];

	ESP8266_Clear();						   // ��ս��ջ���
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len); // ��������
	if (!ESP8266_SendCmd(cmdBuf, ">"))		   // �յ���>��ʱ���Է�������
	{
		Usart_SendString(huart2, data, len); // �����豸������������
	}
}

//==========================================================
//	�������ƣ�	ESP8266_GetIPD
//
//	�������ܣ�	��ȡƽ̨���ص�����
//
//	��ڲ�����	�ȴ���ʱ��(����10ms)
//
//	���ز�����	ƽ̨���ص�ԭʼ����
//
//	˵����		��ͬ�����豸���صĸ�ʽ��ͬ����Ҫȥ����
//				��ESP8266�ķ��ظ�ʽΪ	"+IPD,x:yyy"	x�������ݳ��ȣ�yyy����������
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

	char *ptrIPD = NULL;

	do
	{
		if (ESP8266_WaitRecive() == REV_OK) // ����������
		{
			ptrIPD = strstr((char *)esp8266_buf, "IPD,"); // ������IPD��ͷ
			if (ptrIPD == NULL)							  // ���û�ҵ���������IPDͷ���ӳ٣�������Ҫ�ȴ�һ�ᣬ�����ᳬ���趨��ʱ��
			{
				// printf(USART_DEBUG, "\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':'); // �ҵ�':'
				if (ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
			}
		}

		HAL_Delay(5); // ��ʱ�ȴ�
	} while (timeOut--);

	return NULL; // ��ʱ��δ�ҵ������ؿ�ָ��
}

//==========================================================
//	�������ƣ�	ESP8266_Init
//
//	�������ܣ�	��ʼ��ESP8266
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����
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
//	�������ƣ�	USART2_IRQHandler
//
//	�������ܣ�	����2�շ��ж�
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����
//==========================================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
		if (esp8266_cnt >= sizeof(esp8266_buf))
			esp8266_cnt = 0; // ��ֹ���ڱ�ˢ��
		esp8266_buf[esp8266_cnt++] = esp01_rx_buf[0];
	}
	HAL_UART_Receive_IT(&huart2, esp01_rx_buf, 1);
}
