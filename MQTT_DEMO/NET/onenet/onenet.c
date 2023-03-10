/**
 ************************************************************
 ************************************************************
 ************************************************************
 *	�ļ����� 	onenet.c
 *
 *	���ߣ� 		�ż���
 *
 *	���ڣ� 		2017-05-08
 *
 *	�汾�� 		V1.1
 *
 *	˵���� 		��onenetƽ̨�����ݽ����ӿڲ�
 *
 *	�޸ļ�¼��	V1.0��Э���װ�������ж϶���ͬһ���ļ������Ҳ�ͬЭ��ӿڲ�ͬ��
 *				V1.1���ṩͳһ�ӿڹ�Ӧ�ò�ʹ�ã����ݲ�ͬЭ���ļ�����װЭ����ص����ݡ�
 ************************************************************
 ************************************************************
 ************************************************************
 **/

// ��Ƭ��ͷ�ļ�
#include "stm32f1xx.h"

// �����豸
#include "esp8266.h"

// Э���ļ�
#include "onenet.h"
#include "mqttkit.h"

// Ӳ������
#include "usart.h"

// C��
#include <string.h>
#include <stdio.h>

// cJson
#include "cJSON.h"

// LED
#include "led.h"

#define PROID "77247"

#define AUTH_INFO "test"

#define DEVID "5616839"

extern unsigned char esp8266_buf[128];

void Handle(void (*CallBack)());

//==========================================================
//	�������ƣ�	OneNet_DevLink
//
//	�������ܣ�	��onenet��������
//
//	��ڲ�����	��
//
//	���ز�����	1-�ɹ�	0-ʧ��
//
//	˵����		��onenetƽ̨��������
//==========================================================
_Bool OneNet_DevLink(void)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; // Э���

	unsigned char *dataPtr;

	_Bool status = 1;

	printf("OneNet_DevLink\r\n"
		   "PROID: %s,	AUIF: %s,	DEVID:%s\r\n",
		   PROID, AUTH_INFO, DEVID);

	if (MQTT_PacketConnect(PROID, AUTH_INFO, DEVID, 256, 0, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len); // �ϴ�ƽ̨

		dataPtr = ESP8266_GetIPD(250); // �ȴ�ƽ̨��Ӧ
		if (dataPtr != NULL)
		{
			if (MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch (MQTT_UnPacketConnectAck(dataPtr))
				{
				case 0:
					printf("Tips:	���ӳɹ�\r\n");
					status = 0;
					break;

				case 1:
					printf("WARN:	����ʧ�ܣ�Э�����\r\n");
					break;
				case 2:
					printf("WARN:	����ʧ�ܣ��Ƿ���clientid\r\n");
					break;
				case 3:
					printf("WARN:	����ʧ�ܣ�������ʧ��\r\n");
					break;
				case 4:
					printf("WARN:	����ʧ�ܣ��û������������\r\n");
					break;
				case 5:
					printf("WARN:	����ʧ�ܣ��Ƿ�����(����token�Ƿ�)\r\n");
					break;

				default:
					printf("ERR:	����ʧ�ܣ�δ֪����\r\n");
					break;
				}
			}
		}

		MQTT_DeleteBuffer(&mqttPacket); // ɾ��
	}
	else
		printf("WARN:	MQTT_PacketConnect Failed\r\n");

	return status;
}

//==========================================================
//	�������ƣ�	OneNet_Subscribe
//
//	�������ܣ�	����
//
//	��ڲ�����	topics�����ĵ�topic
//				topic_cnt��topic����
//
//	���ز�����	SEND_TYPE_OK-�ɹ�	SEND_TYPE_SUBSCRIBE-��Ҫ�ط�
//
//	˵����
//==========================================================
void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt)
{

	unsigned char i = 0;

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; // Э���

	for (; i < topic_cnt; i++)
		printf("Subscribe Topic: %s\r\n", topics[i]);

	if (MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL2, topics, topic_cnt, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len); // ��ƽ̨���Ͷ�������

		MQTT_DeleteBuffer(&mqttPacket); // ɾ��
	}
}

//==========================================================
//	�������ƣ�	OneNet_Publish
//
//	�������ܣ�	������Ϣ
//
//	��ڲ�����	topic������������
//				msg����Ϣ����
//
//	���ز�����	SEND_TYPE_OK-�ɹ�	SEND_TYPE_PUBLISH-��Ҫ����
//
//	˵����
//==========================================================
void OneNet_Publish(const char *topic, const char *msg)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; // Э���

	printf("   Publish Topic: %s, Msg: %s             \r\n", topic, msg);
	printf("||\t\t\t\t\t\t\t||\r\n");
	printf("||\t\t\t\t\t\t\t||\r\n");
	printf("====================================================\r\n");

	if (MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL2, 0, 1, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len); // ��ƽ̨���Ͷ�������

		MQTT_DeleteBuffer(&mqttPacket); // ɾ��
	}
}

//==========================================================
//	�������ƣ�	OneNet_RevPro
//
//	�������ܣ�	ƽ̨�������ݼ��
//
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//
//	���ز�����	��
//
//	˵����
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{

	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0}; // Э���

	char *req_payload = NULL;
	char *cmdid_topic = NULL;

	unsigned short topic_len = 0;
	unsigned short req_len = 0;

	unsigned char type = 0;
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;

	short result = 0;

	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;

	cJSON *json, *json_value;

	type = MQTT_UnPacketRecv(cmd);
	switch (type)
	{
	case MQTT_PKT_CMD: // �����·�

		result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len); // ���topic����Ϣ��
		if (result == 0)
		{
			printf("cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);

			if (MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0) // ����ظ����
			{
				printf("Tips:	Send CmdResp\r\n");

				ESP8266_SendData(mqttPacket._data, mqttPacket._len); // �ظ�����
				MQTT_DeleteBuffer(&mqttPacket);						 // ɾ��
			}
		}

		break;

	case MQTT_PKT_PUBLISH: // ���յ�Publish��Ϣ

		result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
		if (result == 0)
		{
			printf("topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
				   cmdid_topic, topic_len, req_payload, req_len);

				   // �����ݰ�req_payload ���� JSON ��ʽ����
			json = cJSON_Parse(req_payload);
			if (!json)
				printf("Error before: [%s]\n", cJSON_GetErrorPtr());
			else
			{
				json_value = cJSON_GetObjectItem(json, "LED");

				if (json_value->valueint)
					// Handle(LED_ON());
					LED_R_ON();
				else
					// Handle(LED_OFF());
					LED_R_OFF();
			}
			cJSON_Delete(json);

		// 	switch (qos)
		// 	{
		// 	case 1: // �յ�publish��qosΪ1���豸��Ҫ�ظ�Ack

		// 		if (MQTT_PacketPublishAck(pkt_id, &mqttPacket) == 0)
		// 		{
		// 			printf("Tips:	Send PublishAck\r\n");
		// 			ESP8266_SendData(mqttPacket._data, mqttPacket._len);
		// 			MQTT_DeleteBuffer(&mqttPacket);
		// 		}

		// 		break;

		// 	case 2: // �յ�publish��qosΪ2���豸�Ȼظ�Rec
		// 			// ƽ̨�ظ�Rel���豸�ٻظ�Comp
		// 		if (MQTT_PacketPublishRec(pkt_id, &mqttPacket) == 0)
		// 		{
		// 			printf("Tips:	Send PublishRec\r\n");
		// 			ESP8266_SendData(mqttPacket._data, mqttPacket._len);
		// 			MQTT_DeleteBuffer(&mqttPacket);
		// 		}

		// 		break;

		// 	default:
		// 		break;
		// 	}
		}

		break;

	case MQTT_PKT_PUBACK: // ����Publish��Ϣ��ƽ̨�ظ���Ack

		if (MQTT_UnPacketPublishAck(cmd) == 0)
			printf("Tips:	MQTT Publish Send OK\r\n");

		break;

	case MQTT_PKT_PUBREC: // ����Publish��Ϣ��ƽ̨�ظ���Rec���豸��ظ�Rel��Ϣ

		if (MQTT_UnPacketPublishRec(cmd) == 0)
		{
			printf("Tips:	Rev PublishRec\r\n");
			if (MQTT_PacketPublishRel(MQTT_PUBLISH_ID, &mqttPacket) == 0)
			{
				printf("Tips:	Send PublishRel\r\n");
				ESP8266_SendData(mqttPacket._data, mqttPacket._len);
				MQTT_DeleteBuffer(&mqttPacket);
			}
		}

		break;

	case MQTT_PKT_PUBREL: // �յ�Publish��Ϣ���豸�ظ�Rec��ƽ̨�ظ���Rel���豸���ٻظ�Comp

		if (MQTT_UnPacketPublishRel(cmd, pkt_id) == 0)
		{
			printf("Tips:	Rev PublishRel\r\n");
			if (MQTT_PacketPublishComp(MQTT_PUBLISH_ID, &mqttPacket) == 0)
			{
				printf("Tips:	Send PublishComp\r\n");
				ESP8266_SendData(mqttPacket._data, mqttPacket._len);
				MQTT_DeleteBuffer(&mqttPacket);
			}
		}

		break;

	case MQTT_PKT_PUBCOMP: // ����Publish��Ϣ��ƽ̨����Rec���豸�ظ�Rel��ƽ̨�ٷ��ص�Comp

		if (MQTT_UnPacketPublishComp(cmd) == 0)
		{
			printf("Tips:	Rev PublishComp\r\n");
		}

		break;

	case MQTT_PKT_SUBACK: // ����Subscribe��Ϣ��Ack

		if (MQTT_UnPacketSubscribe(cmd) == 0)
			printf("Tips:	MQTT Subscribe OK\r\n");
		else
			printf("Tips:	MQTT Subscribe Err\r\n");

		break;

	case MQTT_PKT_UNSUBACK: // ����UnSubscribe��Ϣ��Ack

		if (MQTT_UnPacketUnSubscribe(cmd) == 0)
			printf("Tips:	MQTT UnSubscribe OK\r\n");
		else
			printf("Tips:	MQTT UnSubscribe Err\r\n");

		break;

	default:
		result = -1;
		break;
	}

	ESP8266_Clear(); // ��ջ���

	if (result == -1)
		return;

	dataPtr = strchr(req_payload, '}'); // ����'}'

	if (dataPtr != NULL && result != -1) // ����ҵ���
	{
		dataPtr++;

		while (*dataPtr >= '0' && *dataPtr <= '9') // �ж��Ƿ����·��������������
		{
			numBuf[num++] = *dataPtr++;
		}

		num = atoi((const char *)numBuf); // תΪ��ֵ��ʽ
	}

	if (type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}
}

void Handle(void (*CallBack)())
{
	CallBack();
}
