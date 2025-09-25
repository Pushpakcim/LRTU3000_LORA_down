/*
 * Ethernet_MQTT.c
 *
 *  Created on: Aug 10, 2023
 *      Author: SanketP
 */


/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "Ethernet_MQTT.h"
#include "string.h"
#include "pcbplccomm.h"
#include "mqtt.h"
#include "mqtt_priv.h"

/**************************************************************************//**
 * Macro
 *****************************************************************************/
#define PUBLISH_TOPIC_TB_TELEMETRY 			"v1/devices/me/telemetry"
#define PUBLISH_TOPIC_TB_ATTRIBUTES 		"v1/devices/me/attributes"
#define SUBSCRIBE_TOPIC_TB_ATTRIBUTES 		"v1/devices/me/attributes"

#define LWIP_APP_MQTT_KEEPALIVE_INTERVAL	60
#define OTA_FIRST_MEMORY_SECTOR				1
#define LWIP_APP_MQTT_WILL_QOS				1
#define ETHERNET_OTA_RESET_TIME				120

/**************************************************************************//**
 * Variable
 *****************************************************************************/
osThreadId Ethernet_MQTT_TaskHandle;
SemaphoreHandle_t mqttConnectSemaphore;

unsigned int count_Ethernet_MQTT, Ethernet_MQTT_counter,flag_MQTT_disconnect_count,ethernet_MQTT_publish_success_timer;
mqtt_client_t *mqtt_client = NULL;
struct mqtt_connect_client_info_t mqtt_client_info;
int MQTT_Port;
//char MQTT_SUB_TOPIC[30] = SUBSCRIBE_TOPIC_TB_ATTRIBUTES;//"MQTT_Chunk";
unsigned int LiveDatapubCounter_Ethernet;
uint32_t cim_test_MQTT_send_cnt,cim_test_MQTT_rec_cnt;
mqtt_connection_status_t cim_mqtt_status = MQTT_CONNECT_DISCONNECTED;
unsigned char cim_mqtt_data_received,cim_topic_subscribed;
char client_ID[40] = SUBSCRIBE_TOPIC_TB_ATTRIBUTES;
unsigned char sub_buff[CIM_MAX_SIZE_OF_MQTT_PAYLOAD];
//unsigned char pub_buff[CIM_MAX_SIZE_OF_MQTT_PAYLOAD];
extern unsigned int Modem_MQTT_check_sec, Modem_MQTT_reconnectCount;
extern uint8_t *ACK_string;
extern unsigned char senddata;
extern unsigned int modem_MQTT_Sub_receive_success_count;
extern int MQTT_send_OTA_status;

/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/
void mqtt_subscribe_cb (void *arg, err_t err);
void mqtt_publish_cb (void *arg, err_t err);
void mqtt_incoming_publish_cb (void *arg,const char *topic,uint32_t tot_len);
void mqtt_incoming_data_cb (void *arg,const uint8_t *data,uint16_t len,uint8_t flags);
void mqtt_connection_cb (mqtt_client_t *client,void *arg,mqtt_connection_status_t status);
int lwip_app_mqtt_initialization (void);
int lwip_app_mqtt_connection (void);
int lwip_app_mqtt_publish (mqtt_client_t *client,char *publish_topic,char *message,u8_t qos,u8_t retain,mqtt_request_cb_t publish_cb);
int lwip_app_mqtt_subscribe (mqtt_client_t *client,char *subscribe_topic,u8_t qos,mqtt_request_cb_t subscribe_cb,mqtt_incoming_publish_cb_t incoming_pub_cb,mqtt_incoming_data_cb_t incoming_data_cb);
void Ethernet_MQTT_PUB_Routine(void);

extern void WriteLog(uint8_t LogEnable,const char *pData,uint8_t logType);
/**************************************************************************//**
 * Function name 	: Ethernet_MQTT_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/
void Ethernet_MQTT_start()
{
	osThreadDef(Ethernet_MQTTTask, StartEthernet_MQTTTask, osPriorityNormal, 0, 1024); //512
	Ethernet_MQTT_TaskHandle = osThreadCreate(osThread(Ethernet_MQTTTask), NULL);
}

/**************************************************************************//**
 * Function name 	: StartEthernet_MQTTTask
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void StartEthernet_MQTTTask(void const * argument)
{
	osDelay(3000);

	int ret = 0;
	JSON_ERROR_RESPONSE JSON_ret;
	ACK_string = &MqttPubBuf[0];

	//unsigned char sub_buff[10];
	//unsigned char pub_buff[10] = "Test";

#ifdef LWIP_IPERF_SERVER
	lwiperf_start_tcp_server_default(lwip_iperf_results,0);
#endif

	  /*Create semaphore to handle mqtt*/
	mqttConnectSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(mqttConnectSemaphore);

	lwip_app_mqtt_initialization ();

	for( ;; )
	{
#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
		HAL_IWDG_Refresh(&hiwdg1);
#endif
		count_Ethernet_MQTT = 0;
		Ethernet_MQTT_counter++;
		LiveDatapubCounter_Ethernet++;
		OTA_resetCouner++;

		if(flag_modem_MQTT_Reconnect == 1)
		{
			flag_modem_MQTT_Reconnect = 0;

			LOCK_TCPIP_CORE();
			mqtt_disconnect(mqtt_client);
			UNLOCK_TCPIP_CORE();

			sprintf((char *)print,"flag_Ethernet_MQTT_Reconnect:%d\r\n",flag_modem_MQTT_Reconnect);
			WriteLog(1, (const char *)print, 1);
			osDelay(1000);
		}

		LOCK_TCPIP_CORE();
		ret = mqtt_client_is_connected(mqtt_client);
		UNLOCK_TCPIP_CORE();

		if(ret == 0)
		{
			ret = lwip_app_mqtt_connection();

			sprintf((char *)print,"lwip_app_mqtt_connection ret=%d\r\n",ret);
			WriteLog(1,(const char *)print,1);

			if(ret == 0)
			{
				memset(MqttSubTopic,0,sizeof(MqttSubTopic));
				sprintf((char *)MqttSubTopic, "v1/devices/%d/%d/%d", EPROM_General.Cust_Detail.Client_Id, EPROM_General.Cust_Detail.Reader_Id, EPROM_General.Rtu_Detail.RTUId);
		        ret = lwip_app_mqtt_subscribe(mqtt_client, (char *)&MqttSubTopic, 1, mqtt_subscribe_cb, mqtt_incoming_publish_cb, mqtt_incoming_data_cb);
			}
			else
			{
				flag_modem_MQTT_Reconnect = 1;
			}
			Modem_MQTT_check_sec = 120;
			Modem_MQTT_reconnectCount++;
		}
		else
		{
			if(cim_mqtt_data_received==1)
			{
				modem_MQTT_Sub_receive_success_count++;
				cim_mqtt_data_received=0;
				JSON_ret = parse_JSON_frame(MQTT,(char*)&sub_buff,(char *)ACK_string);
				if(JSON_ret!=JSON_SUCCESS)
				{
					JSON_ret = parse_JSON_frame(MQTT,(char*)&sub_buff,(char *)ACK_string);
				}
				// mqtt[0].mqtt_rx_state = MQTT_RX_STATE_FREE;
				senddata=1;
			}
			if(OTA_START == 1)
			{
				flagMqttPubLiveData = 0;
			}
			Ethernet_MQTT_PUB_Routine();

				//lwip_app_mqtt_publish(mqtt_client, PUBLISH_TOPIC_TB_TELEMETRY_test, (char *)&MqttPubBuf_ethernet, 1 , 0 ,'\0');
		}
		if((OTA_START==1)&&(OTA_resetCouner>ETHERNET_OTA_RESET_TIME))
		{
			OTA_START=0;
		}
		if(OTA_START==0)
		{
			if(LiveDatapubCounter_Ethernet>EPROM_General.Mo_Comm.MQTT_LiveFreq)
			{
				LiveDatapubCounter_Ethernet = 0;
				flagMqttPubLiveData = 1;
			}
		}
		if(flag_MQTT_disconnect_count > (int)(120/EPROM_General.Mo_Comm.MQTT_LiveFreq))
		{
			flag_MQTT_disconnect_count = 0;
			flag_modem_MQTT_Reconnect = 1;
		}
		gFinalAnaValF[ETHER_MQTT_CONNECTION_STATUS_gFinalAnaValF]= mqtt_client->conn_state;
		osDelay(1000);
	}
}
/**************************************************************************//**
 * Initialize the MQTT resources.
 *****************************************************************************/
int lwip_app_mqtt_initialization (void)
{
  int ret = 0;

  // Create a new MQTT client
  LOCK_TCPIP_CORE();
  mqtt_client = mqtt_client_new();
  UNLOCK_TCPIP_CORE();

  if (mqtt_client == NULL) {
    // Error
    ret = -1;
  }
  return ret;
}

/**************************************************************************//**
 * Connect to a MQTT broker.
 *****************************************************************************/
int lwip_app_mqtt_connection (void)
{
	int ret = 0;
	ip_addr_t mqtt_broker_ip;
	lwgsm_ip_t ip_t;
	unsigned char *buf_t;

	buf_t = (unsigned char*)&EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP;
    lwgsmi_parse_ip((const char**)&buf_t, &ip_t);

	IP4_ADDR(&mqtt_broker_ip, ip_t.ip[0], ip_t.ip[1], ip_t.ip[2],ip_t.ip[3]);
	sprintf((char *)print,"Connecting to MQTT broker (%s)...\r\n", ipaddr_ntoa(&mqtt_broker_ip));
	WriteLog(1,(const char *)print,1);

	MQTT_Port = EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_Port;


	//strcpy((char *)MqttBorker,(const char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP);
	//MqttBrokerPort = EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_Port;
	memset(MqttClientId,0,sizeof(MqttClientId));
	sprintf((char *)MqttClientId, "v1/devices/%d/%d/%d", EPROM_General.Cust_Detail.Client_Id, EPROM_General.Cust_Detail.Reader_Id, EPROM_General.Rtu_Detail.RTUId);
	strcpy((char *)MqttClientUser,(const char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Name);
	strcpy((char *)MqttClientPass,(const char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Pass);

	mqtt_client_info.client_id= (const char*)&MqttClientId;
	mqtt_client_info.client_user = (const char*)&MqttClientUser;
	mqtt_client_info.client_pass = (const char*)&MqttClientPass;
	//mqtt_client_info.keep_alive = 120;
	//_mqtt->connect.ret_code_temp = MQTT_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT;
	//_mqtt->sent_result = MQTT_PACKET_SENT_RESULT_DEFAULT;
	mqtt_client_info.will_qos=LWIP_APP_MQTT_WILL_QOS;                  //will_qos =1 (Improvement)

	LOCK_TCPIP_CORE();
	ret = mqtt_client_connect(mqtt_client,
							  &mqtt_broker_ip,
							  MQTT_Port,
							  mqtt_connection_cb,
							  NULL,
							  &mqtt_client_info);
	UNLOCK_TCPIP_CORE();
	return ret;
}
/**************************************************************************//**
 * Subscribe to MQTT topic.
 *****************************************************************************/
int lwip_app_mqtt_subscribe (mqtt_client_t *client,
									char *subscribe_topic,
                                    u8_t qos,
                                    mqtt_request_cb_t subscribe_cb,
                                    mqtt_incoming_publish_cb_t incoming_pub_cb,
                                    mqtt_incoming_data_cb_t incoming_data_cb)
{
  err_t err = -1;
  (void)client;
  if ((subscribe_topic != NULL)
      && (strlen(subscribe_topic) > 0)) {

    LOCK_TCPIP_CORE();
    err=mqtt_sub_unsub(client,
                   subscribe_topic,
                   qos,
                   subscribe_cb,
                   NULL,
                   1);
    UNLOCK_TCPIP_CORE();

    LOCK_TCPIP_CORE();
    mqtt_set_inpub_callback(mqtt_client,
                            incoming_pub_cb,
                            incoming_data_cb,
                            NULL);
    UNLOCK_TCPIP_CORE();
  }
 // mqtt_error_status=((-1)*err)+8000;  //maulin : subscribe time error //MAU_EXCEPTION
  return err;
}
/**************************************************************************//**
 * Publish a MQTT message.
 *****************************************************************************/
int lwip_app_mqtt_publish (mqtt_client_t *client,
								  char *publish_topic,
                                  char *message,
                                  u8_t qos,
                                  u8_t retain,
                                  mqtt_request_cb_t publish_cb)
{
  uint16_t len = 0;
  err_t err;
  (void)client;

  if (message != NULL) {
    len = strlen(message);
  }

  LOCK_TCPIP_CORE();
  err = mqtt_publish(client,
                     publish_topic,
                     (void *)message,
                     len,
                     qos,
                     retain,
					 mqtt_publish_cb,
                     NULL);
  UNLOCK_TCPIP_CORE();
 // mqtt_error_status=((-1)*err)+9000;  //maulin : publish time error //MAU_EXCEPTION
  return err;
}

/**************************************************************************//**
 * Manage a MQTT connection change of state.
 *****************************************************************************/

void mqtt_connection_cb (mqtt_client_t *client,
                                void *arg,
                                mqtt_connection_status_t status)
{

  (void)client;
  (void)arg;
//  cim_mqtt_status=status;
 // mqtt_error_status=cim_mqtt_status+5000; //MAU_EXCEPTION
  if (status == MQTT_CONNECT_ACCEPTED) {
    sprintf((char *)print,"Connection success\r\n");
    WriteLog(1,print,1);
  } else {
    sprintf((char *)print,"Disconnection(%d)\r\n", status);
    WriteLog(1,print,1);
  }
  //xSemaphoreGive(mqttConnectSemaphore);

}

/**************************************************************************//**
 * Manage a MQTT subscription result.
 *****************************************************************************/
void mqtt_subscribe_cb (void *arg, err_t err)
{
  (void)arg;

  if (err == 0) {
    sprintf((char *)print,"Subscribe success\r\n");
    WriteLog(1,(const char *)print,1);
  } else {
    sprintf((char *)print,"Subscribe error: %d\r\n", err);
    WriteLog(1,(const char *)print,1);
  }
}


/**************************************************************************//**
 * Manage a MQTT publishing result.
 *****************************************************************************/
void mqtt_publish_cb (void *arg, err_t err)
{
  (void)arg;

  if (err != 0) {
	// mqtt_error_status=((-1)*err)+10000; //MAU_EXCEPTION
    sprintf((char *)print,"Publish error: %d\r\n", err);
    WriteLog(1,(const char *)print,1);
  }
  else
  {
	  //cim_test_MQTT_send_cnt++;
	  ethernet_MQTT_publish_success_timer=0;
	  sprintf((char *)print,"Publish OK\r\n");
	  WriteLog(1,(const char *)print,1);
  }
 // Publish_Finish = 1;
}

/**************************************************************************//**
 *  * Manage a MQTT incoming data.
 *****************************************************************************/
static int inpub_id;
static unsigned int cim_mqtt_rec_data_len;

void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
	sprintf((char *)print,"Incoming publish at topic %s with total length %u\r\n", topic, (unsigned int)tot_len);
	WriteLog(1, (const char *)print, 1);
	  /* Decode topic string into a user defined reference */
	//  if(strcmp(topic, &MQTT_SUB_TOPIC) == 0) //I_EDGE
	//  {
	    inpub_id = 0;
	    cim_mqtt_rec_data_len=tot_len;
	//  }
	 // else
//	if(strcmp(topic, &client_ID) == 0)
//	{
//		inpub_id = 0;
//		cim_mqtt_rec_data_len=tot_len;
//	}
}
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
	//	unsigned int length_t=0;
	  sprintf((char *)print,"Incoming publish payload with length %d, flags %u, inpub_id %d\r\n", len, (unsigned int)flags,inpub_id);
	  WriteLog(1, (char *)print, 1);

	  if(flags & MQTT_DATA_FLAG_LAST) {
	    /* Last fragment of payload received (or whole part if payload fits receive buffer
	       See MQTT_VAR_HEADER_BUFFER_LEN)  */

	    /* Call function or do action depending on reference, in this case inpub_id */
	    if(inpub_id == 0) {
	      /* Don't trust the publisher, check zero termination */
			cim_test_MQTT_rec_cnt++;
			cim_mqtt_data_received=1;

			//sprintf((char *)print,"data:%s\r\n", data);
			//WriteLog(1, (const char *)print, 1);

			memset(sub_buff, 0, CIM_MAX_SIZE_OF_MQTT_PAYLOAD);
			strncpy((char *)sub_buff, (const char *)data, len);

			//sprintf((char *)print,"cim_mqtt_rec_data_len:%d, data:%s, mqtt_incoming_data_cb: %s\r\n", len, data,(const char *)sub_buff);
			//WriteLog(1, (const char *)print, 1);
	     // }
	    }
	    else {
	      sprintf((char *)print,"mqtt_incoming_data_cb: Ignoring payload...\r\n");
	      WriteLog(1, (const char *)print, 1);
	    }
	  } else {
	    /* Handle fragmented payload, store in buffer, write to file or whatever */
	  }
}

void Ethernet_MQTT_PUB_Routine(void)
{
	unsigned char pubTopic=0; // 0 = "v1/devices/Response" 1 = "v1/devices/me/log"  2 = "v1/devices/me/live"
	err_t err = -1;
	if(MQTT_send_OTA_status)
	{
		MQTT_send_OTA_status = 0;
		pubTopic = 3;
	}
	else if(flagMQTTPubSchedule)
	{
		flagMQTTPubSchedule = 0;
		buildGetScheduleJson(1,pubScheduleBlock);
		pubTopic = 0;
	}
	else if(flagMQTTPubGetMode)
	{
		flagMQTTPubGetMode = 0;
		buildGetModeResponseJson(1);
		pubTopic = 0;
	}
	else if(flagMqttPubLogData)
	{
		flagMqttPubLogData = 0;
		buildLograteDataJson(1);
		pubTopic = 1;
	}
	else if(flagMqttPubLiveData)
	{
		flagMqttPubLiveData = 0;
		buildLograteDataJson(1);
		pubTopic = 2;
	}
	else
	{
		return;
	}

	if(pubTopic == 0)
	{
		//modem_MQTT_publish(&mqtt[0],"v1/devices/Response",(const char*)MqttPubBuf,strlen((const char *)&MqttPubBuf),0,0);
		lwip_app_mqtt_publish(mqtt_client, "v1/devices/Response", (char *)&MqttPubBuf, 1 , 0 ,'\0');
		osDelay(100);
		sprintf((char *)print,"MQTT modem_MQTT_publish \"v1/devices/Response\"\r\n");
		WriteLog(1, print, 1);
	}
	else if(pubTopic == 1)
	{
		//modem_MQTT_publish(&mqtt[0],"v1/devices/me/log",(const char*)MqttPubBuf,strlen((const char *)&MqttPubBuf),0,0);
		lwip_app_mqtt_publish(mqtt_client, "v1/devices/me/log", (char *)&MqttPubBuf, 1 , 0 ,'\0');
		osDelay(100);
		sprintf((char *)print,"MQTT modem_MQTT_publish \"v1/devices/me/log\"\r\n");
		WriteLog(1, print, 1);
	}
	else if(pubTopic == 2)
	{
		//modem_MQTT_publish(&mqtt[0],"v1/devices/me/live",(const char*)MqttPubBuf,strlen((const char *)&MqttPubBuf),0,0);
		err = lwip_app_mqtt_publish(mqtt_client, "v1/devices/me/live", (char *)&MqttPubBuf, 1 , 0 ,'\0');
		osDelay(100);
		if(err != 0)
		{
			flag_MQTT_disconnect_count++;
		}
		else
		{
			flag_MQTT_disconnect_count = 0;
		}
		sprintf((char *)print,"MQTT modem_MQTT_publish \"v1/devices/me/live\" , err:%d\r\n",err);
		WriteLog(1, print, 1);
	}
	else if(pubTopic == 3)
	{
		unsigned char ret;
		extern char crcMatch;
		extern CMD_TYPE current_cmd;
		JSON_ERROR_RESPONSE JSON_ret;
//		ACK_string =&MqttPubBuf;


		//ret = modem_MQTT_publish(&mqtt[0],"v1/device/sub",(const char*)MqttPubBuf,strlen((const char *)MqttPubBuf),0,0);
		//ret = modem_MQTT_publish(&mqtt[0],"v1/devices/Response",(const char*)MqttPubBuf,strlen((const char *)MqttPubBuf),0,0);
		ret = lwip_app_mqtt_publish(mqtt_client, "v1/devices/Response", (char *)&MqttPubBuf, 1 , 0 ,'\0');
		osDelay(100);
		if( (1 == crcMatch) && (OTA_ACK_Data.CMDState == 2) && (current_cmd == CMD_OTA ) )   // send Ack message after file received done
		{
			// this is only used for plc and rec file
			if(OTA_ACK_Data.FileType == HEX)
			{
				JSON_ret = response_ACK_JSON_frame1(MQTT , CMD_OTA, OTA_ACK_Data.OtaACK,(char *)ACK_string);
				if(JSON_ret!=JSON_SUCCESS)
				{
					JSON_ret = response_ACK_JSON_frame1(MQTT , CMD_OTA, OTA_ACK_Data.OtaACK,(char *)ACK_string);
				}
	//			len = strlen((const char *)MqttPubBuf);
				//ret = modem_MQTT_publish(&mqtt[0],"v1/devices/Response",(const char*)MqttPubBuf,strlen((const char *)MqttPubBuf),0,0);
				ret = lwip_app_mqtt_publish(mqtt_client, "v1/devices/Response", (char *)&MqttPubBuf, 1 , 0 ,'\0');
				osDelay(100);
				//modem_MQTT_publish(&mqtt[0],"v1/device/sub",(const char*)MqttPubBuf,strlen((const char *)MqttPubBuf),0,0);
				crcMatch =0;
				reboot_device_func();
				osDelay(10000);
				//HAL_NVIC_SystemReset();
			}
			else
			{
				JSON_ret = response_ACK_JSON_frame1(MQTT , CMD_OTA, OTA_ACK_Data.OtaACK,(char *)ACK_string);
				if(JSON_ret!=JSON_SUCCESS)
				{
					JSON_ret = response_ACK_JSON_frame1(MQTT , CMD_OTA, OTA_ACK_Data.OtaACK,(char *)ACK_string);
				}
	//			len = strlen((const char *)MqttPubBuf);
				//ret = modem_MQTT_publish(&mqtt[0],"v1/devices/Response",(const char*)MqttPubBuf,strlen((const char *)MqttPubBuf),0,0);
				ret = lwip_app_mqtt_publish(mqtt_client, "v1/devices/Response", (char *)&MqttPubBuf, 1 , 0 ,'\0');
				osDelay(100);
				//modem_MQTT_publish(&mqtt[0],"v1/device/sub",(const char*)MqttPubBuf,strlen((const char *)MqttPubBuf),0,0);
				crcMatch =0;
			}
		}
		sprintf((char *)print,"MQTT modem_MQTT_publish \"v1/devices/Response\"\r\n");
		WriteLog(1, print, 1);
	}
}

