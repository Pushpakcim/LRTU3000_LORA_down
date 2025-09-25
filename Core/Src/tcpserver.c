/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
/* Includes ------------------------------------------------------------------*/
#include "tcpserver.h"
#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#define TCPECHO_THREAD_PRIO  (osPriorityAboveNormal)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
struct netconn *conn, *newconn;
static struct netbuf *buf;
static ip_addr_t *addr;
static unsigned short port;
//char TCP_msg[1400];
//char msg[1400];
//char smsg[1400];
//extern char msg_payload ;

void tcp_server_thread(void const * argument);
void tcp_server_1_thread(void const * argument);
//extern JSON_ERROR_RESPONSE response_ACK_JSON_frame(COM_TYPE com_mode ,CMD_TYPE CMD, RES_ACK ACK,char * ACK_Response);
extern JSON_ERROR_RESPONSE response_ACK_JSON_frame1(COM_TYPE com_mode ,CMD_TYPE CMD, OTA_FILE_ACK iACK,char * ACK_Response);
extern CMD_TYPE current_cmd;
extern char crcMatch;

char tcp_ResponseBuffer[1500];

void tcpserver_init(void)
{
  osThreadDef(tcp_server, tcp_server_thread, osPriorityAboveNormal, 0, 256*32);
  osThreadCreate (osThread(tcp_server), NULL);
}

void tcp_server_thread(void const * argument)
{

	err_t err,accept_err;//,recv_error;
	char * ACK_response;

	//  LWIP_UNUSED_ARG(argument);  // To Do: check why it is used?

	/* Create a new connection identifier. */
	/* Bind connection to well known port number 7. */
	conn = netconn_new(NETCONN_TCP);
	if(conn != NULL)
	{
		err = netconn_bind(conn, IP_ADDR_ANY, 3032);

		if(err == ERR_OK)
		{
			netconn_listen(conn); 	//Set connection in listening mode

			while(1)
			{
				memset(tcp_ResponseBuffer,0,sizeof(tcp_ResponseBuffer));
				ACK_response = tcp_ResponseBuffer;//= calloc(1,1500);
				/* Grab new connection. */
				accept_err = netconn_accept(conn, &newconn);
				// process new connection
				if(accept_err != ERR_OK)
				{
					netconn_close(newconn);
					netconn_delete(newconn);
				    sprintf((char *)print, "TCP_3032_accept_err=%d\r\n", accept_err);
				    WriteLog(1, (char *)print, 1);
//					  if(accept_err == ERR_TIMEOUT)
//					  {
//						  ModbusCloseConnNull(modH);
//						  return xTCPvalid;
//					  }
//					  else
//					  {
//						  ModbusCloseConnNull(modH);
//						  return xTCPvalid;
//					  }
				}
				else if(accept_err == ERR_OK)
				{
//					clientconn->aging=0;

					while ((err = netconn_recv(newconn, &buf)) == ERR_OK)
					{
					//	print_time();		//an
						addr = netbuf_fromaddr(buf);	//get the address of client
						port = netbuf_fromport(buf);	//get the port of client

						do
						{
							strcpy((char *)print,"");
//						    sprintf((char *)print, "TCP_3032_Received_Payload=%s\r\n", (char *)buf->p->payload);
//						    WriteLog(1, (char *)print, 1);

							JSON_ERROR_RESPONSE JSON_ret;

							JSON_ret = parse_JSON_frame(TCP,buf->p->payload ,ACK_response);
							if(JSON_ret!=JSON_SUCCESS)
							{
								JSON_ret = parse_JSON_frame(TCP,buf->p->payload ,ACK_response);
							}


							if((OTA_ACK_Data.CMDState == 3)&&(OTA_ACK_Data.FileType == HEX))
							{
								JSON_ret = response_ACK_JSON_frame1(TCP , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_response);
								if(JSON_ret!=JSON_SUCCESS)
								{
									JSON_ret = response_ACK_JSON_frame1(TCP , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_response);
								}
								int len = strlen((const char *)ACK_response);
								netconn_write(newconn, ACK_response, len, NETCONN_COPY);// send message to client
								crcMatch =0;
								OTA_ACK_Data.CMDState = 0;
								ExtFlash_update_OTA_Data();
							}
							switch(current_cmd)
							{
								case CMD_OTA :
								{
									int len = strlen((const char *)ACK_response);
									netconn_write(newconn, ACK_response, len, NETCONN_COPY);// send message to client

									if( (1 == crcMatch) && (OTA_ACK_Data.CMDState == 2) && (current_cmd == CMD_OTA ) )   // send Ack message after file received done
									{// this is only used for plc and rec file

										if(OTA_ACK_Data.FileType == HEX)
										{
											JSON_ret = response_ACK_JSON_frame1(TCP , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_response);
											if(JSON_ret!=JSON_SUCCESS)
											{
												JSON_ret = response_ACK_JSON_frame1(TCP , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_response);
											}
											len = strlen((const char *)ACK_response);
											netconn_write(newconn, ACK_response, len, NETCONN_COPY);// send message to client
											crcMatch =0;
											reboot_device_func();
											osDelay(10000);
											//HAL_NVIC_SystemReset();
										}
										else
										{
											JSON_ret = response_ACK_JSON_frame1(TCP , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_response);
											if(JSON_ret!=JSON_SUCCESS)
											{
												JSON_ret = response_ACK_JSON_frame1(TCP , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_response);
											}
											len = strlen((const char *)ACK_response);
											netconn_write(newconn, ACK_response, len, NETCONN_COPY);// send message to client
											crcMatch =0;
										}

									}
									//osDelay(200);
								}
								break;
								case CMD_CONF :
								{
									int len = strlen((const char *)ACK_response);
									netconn_write(newconn, ACK_response, len, NETCONN_COPY);// send message to client

//									if( (1 == crcMatch) && (OTA_ACK_Data.CMDState == 2) && (current_cmd == CMD_OTA ) )   // send Ack message after file received done
//									{// this is only used for plc and rec file
//										JSON_ret = response_ACK_JSON_frame1(TCP , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_response);
//										if(JSON_ret!=JSON_SUCCESS)
//										{
//											JSON_ret = response_ACK_JSON_frame1(TCP , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_response);
//										}
//										len = strlen((const char *)ACK_response);
//										netconn_write(newconn, ACK_response, len, NETCONN_COPY);// send message to client
//										crcMatch =0;
//									}
								}
								break;
								case CMD_PRODUCTION :
								{
									if(flagTCP_ID_First == 1)    				 //  "CMD": 2,"CMDState": 1
									{
										flagTCP_ID_First = 0;
										buildProIdFrameJson(2,0);
									}
									else if(flagTCP_ID_afterPowerCycle == 1)     //  "CMD": 2,"CMDState": 4
									{
										flagTCP_ID_afterPowerCycle = 0;
										buildProIdFrameJson(2,1);
									}
									else if(flagTCP_TestMethod_1_ACK == 1)  	//  "CMD": 2,"CMDState": 2
									{
										flagTCP_TestMethod_1_ACK = 0;
										buildTestMethodAckJson(2,1);
									}
									else if(flagTCP_TestMethod_1_Result == 1)	//  "CMD": 2,"CMDState": 3
									{
										flagTCP_TestMethod_1_Result = 0;
										buildTestMethodResultJson(2,1,1);
									}
									else if(flagTCP_TestMethod_2_ACK == 1)	    //  "CMD": 2,"CMDState": 5
									{
										flagTCP_TestMethod_2_ACK = 0;
										buildTestMethodAckJson(2,2);
									}
									else if(flagTCP_TestMethod_2_Result == 1)    //  "CMD": 2,"CMDState": 6
									{
										flagTCP_TestMethod_2_Result = 0;
										buildTestMethodResultJson(2,2,2);

									}
									int len = strlen((const char *)ACK_response);
									netconn_write(newconn, ACK_response, len, NETCONN_COPY);
									if(key_LED_flag)
									{
										pro_checkDIDOState();
									}
								}
								break;
								case CMD_AI_CAL :
								{
									if(flagTCP_ID_AI_CALI_App == 1)
									{
										buildProIdFrameJson(2,2);
										flagTCP_ID_AI_CALI_App = 0;
									}
									else if(flagTCP_AI_Channel_CaliResponse)
									{
										buildAICaliJson(2,0,(flagTCP_AI_Channel_CaliResponse-1));
										flagTCP_AI_Channel_CaliResponse = 0;
									}
									else if(flagTCP_AI_Channel_Test_Result)
									{
										buildAICaliJson(2,1,(flagTCP_AI_Channel_Test_Result-1));
										flagTCP_AI_Channel_Test_Result = 0;
									}
									int len = strlen((const char *)ACK_response);
									netconn_write(newconn, ACK_response, len, NETCONN_COPY);
								}
								break;
								case CMD_SET_RTC :
								{

								}
								break;
								default:
								{

								}
								break;
							}
//							memset(msg,0x00,sizeof(msg));//clear the buffer
						}while(netbuf_next(buf)>0);
						netbuf_delete(buf);
					}
				    sprintf((char *)print, "TCP_3032_2_accept_err=%d\r\n", accept_err);
				    WriteLog(1, (char *)print, 1);
					netconn_close(newconn);
					netconn_delete(newconn);
				//free(ACK_response);
				}
			}
		}
	}
	else
	{
		err_t ret;
		ret =  netconn_delete(conn);
		//TODO Print value
	    sprintf((char *)print, "TCP_3032_conn=%d\r\n", conn);
	    WriteLog(1, (char *)print, 1);
	}
}

void tcpserver_1_init(void)
{
  osThreadDef(tcp_server_1, tcp_server_1_thread, osPriorityAboveNormal, 0, 256);
  osThreadCreate (osThread(tcp_server_1), NULL);
}

void tcp_server_1_thread(void const * argument)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(argument);

  /* Create a new connection identifier. */
  /* Bind connection to well known port number 7. */
  conn = netconn_new(NETCONN_TCP);
  if(conn!=NULL)
  {
	  netconn_bind(conn, IP_ADDR_ANY, 3033);
  }
  LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);

  /* Tell connection to go into listening mode. */
  netconn_listen(conn);

  while (1) {

    /* Grab new connection. */
    err = netconn_accept(conn, &newconn);
    /*printf("accepted new connection %p\n", newconn);*/
    /* Process the new connection. */
    if (err == ERR_OK) {
      struct netbuf *buf;
      void *data;
      u16_t len;

      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
        /*printf("Recved\n");*/
        do {
             netbuf_data(buf, &data, &len);
             err = netconn_write(newconn, data, len, NETCONN_COPY);
        } while (netbuf_next(buf) >= 0);
        netbuf_delete(buf);
      }
      /*printf("Got EOF, looping\n");*/
      /* Close connection and discard connection identifier. */
      netconn_close(newconn);
      netconn_delete(newconn);
    }
  }
}
#endif /* LWIP_NETCONN */
