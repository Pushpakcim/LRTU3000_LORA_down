/*
 * udpserver.c
 *
 *  Created on: Mar 31, 2022
 *      Author: controllerstech
 */



/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "udpserver.h"
#include "string.h"

/**************************************************************************//**
 * Variable
 *****************************************************************************/

ip_addr_t *last_UDP_con_IP_addr;
u16_t last_UDP_con_port;

osThreadId udp_TaskHandle;


unsigned char historyDataPacket[700];
unsigned char historyDataPackettemp[100];
unsigned char gStopHistoricalDataStoreCounter=0;
unsigned char gStopHistoricalDataStore=0;
unsigned short int tHistoriDatalength = 0;
flashRunTimeParaSturct g_flashRunTimeParaSturct;
extern char tDebug[500];
void WriteLog(uint8_t LogEnable,const char *pData,uint8_t logType);
/**************************************************************************//**
 * Function name 	: UDPFrameParser
 * arguments		:
 * 		 			:
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/
void udpserver_1_init(void)
{
  osThreadDef(udp_server_1, udp_server_1_thread, osPriorityAboveNormal, 0, 1024);
  osThreadCreate (osThread(udp_server_1), NULL);
}

/**************************************************************************//**
 * Function name 	: UDPFrameParser
 * arguments		:
 * 		 			:
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/
void udp_server_1_thread(void const * argument)
{
	struct netconn *conn;
	struct netbuf *buf, *tx_buf;
	err_t err;
	LWIP_UNUSED_ARG(argument);

	conn = netconn_new(NETCONN_UDP);
	netconn_bind(conn, IP_ADDR_ANY, 3030);

	LWIP_ERROR("udpecho: invalid conn", (conn != NULL), return;);

	while (1)
	{

		//netconn_set_recvtimeout(conn , 100);
		err = netconn_recv(conn, &buf);
		if (err == ERR_OK)
		{

			last_UDP_con_IP_addr = netbuf_fromaddr(buf);	//get the address of client
			last_UDP_con_port = netbuf_fromport(buf);	//get the port of client

			UDPFrameParser((char* )buf->p->payload,buf->p->tot_len);

			tx_buf = netbuf_new();
			netbuf_alloc(tx_buf, tHistoriDatalength);

			pbuf_take(tx_buf->p, &historyDataPacket, tHistoriDatalength);

			err = netconn_sendto(conn, tx_buf, (const ip_addr_t *)&(buf->addr), buf->port);

			if(err != ERR_OK)
			{
				LWIP_DEBUGF(LWIP_DBG_ON, ("netconn_send failed: %d\n", (int)err));
			}
			else
			{
				LWIP_DEBUGF(LWIP_DBG_ON, ("got %s\n", buffer));
			}
			netbuf_delete(tx_buf);
		}
//		else if (err == ERR_TIMEOUT) //No new data
//		{
//			osDelay(10);
//
//		}
		netbuf_delete(buf);
	}
}
/**************************************************************************//**
 * Function name 	: UDPFrameParser
 * arguments		:
 * 		 			:
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/

unsigned char UDPFrameParser(char* data,unsigned int len)
{
	if(FindSubstr(data,"JJMHDP;")!=-1) // JJMHDP = protocol for JJM lograte History Data send to ethernet from external flash storage jal jivan
	{
		memset(historyDataPacket,0,sizeof(historyDataPacket));

		if(data[7] == JJMHDP_HEADER)
		{
			gStopHistoricalDataStoreCounter = 0 ;
			switch (data[8])
			{
					case JJMHDP_GET_RTU_ID:
					{
						//fnDebugMsg("\r\nJJMHDP_GET_RTU_ID\r\n");
						//sprintf(historyDataPackettemp,"====>>%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11]);
						//fnDebugMsg(historyDataPackettemp);
						historyDataPacket[0] = 'H';
						historyDataPacket[1] = JJMHDP_GET_RTU_ID;
						historyDataPacket[2] = (unsigned char)EPROM_General.Rtu_Detail.RTUId;
						tHistoriDatalength = 3;
						gStopHistoricalDataStore = 1;
						//tSendData=1;
						break;
					}
					case JJMHDP_GET_TOTAL_PACKETS:
					{
						//fnDebugMsg("\r\nJJMHDP_GET_TOTAL_PACKETS\r\n");
						//sprintf(historyDataPackettemp,"====>>%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11]);
						//fnDebugMsg(historyDataPackettemp);
						if(data[9] == (unsigned char)EPROM_General.Rtu_Detail.RTUId)
						{
							historyDataPacket[0] = 'H';
							historyDataPacket[1] = JJMHDP_GET_TOTAL_PACKETS;
							historyDataPacket[2] = (unsigned char)EPROM_General.Rtu_Detail.RTUId;
							if(g_flashRunTimeParaSturct.s_ExtDataFlash_IsDataLogOverwritten==1)
							{
								historyDataPacket[3] = (unsigned char)((unsigned short int)((MAX_HISTORY_DATA_PACKETS)>>8));
								historyDataPacket[4] = (unsigned char)(MAX_HISTORY_DATA_PACKETS);
							}
							else
							{
								historyDataPacket[3] = (unsigned char)((unsigned short int)(((g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter))>>8));
								historyDataPacket[4] = (unsigned char)((g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter));
							}
							tHistoriDatalength = 5;
							//tSendData=1;
							break;
						}
						else
						{
							//fnDebugMsg("\r\nRTU ID not matched\r\n");
						}
					}
					case JJMHDP_GET_PACKET_BASED_ON_PACKET_ID:
					{
						//fnDebugMsg("\r\nJJMHDP_GET_PACKET_BASED_ON_PACKET_ID\r\n");
						//sprintf(historyDataPackettemp,"====>>%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11]);
						//fnDebugMsg(historyDataPackettemp);
						if(data[9] == (unsigned char)EPROM_General.Rtu_Detail.RTUId)
						{
							//Packet_no = (data[10]<<8)||(data[11]);
							historyDataPacket[0] = 'H';
							historyDataPacket[1] = JJMHDP_GET_PACKET_BASED_ON_PACKET_ID;
							historyDataPacket[2] = (unsigned char)EPROM_General.Rtu_Detail.RTUId;
							historyDataPacket[3] = (unsigned char)data[10];   // MSB Packet_no
							historyDataPacket[4] = (unsigned char)data[11];		// LSB Packet_no
							historyDataPacket[5] = ':';
							historyDataPacket[6] = ':';
							if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
							{
								if(g_flashRunTimeParaSturct.s_ExtDataFlash_IsDataLogOverwritten==1)
								{
									ExtFlash_ReadHistoricalDataLogFromFlash((((data[10]<<8)|(data[11]))-1),&historyDataPacket[7]);
									//ExtFlash_ReadHistoricalDataLogFromFlash((((data[10]<<8)|(data[11]))-1)*2,&historyDataPacket[7]);// packet stored in 2 pages so multiplied by2
								}
								else
								{
									ExtFlash_ReadHistoricalDataLogFromFlash((((data[10]<<8)|(data[11]))-1),&historyDataPacket[7]);// packet stored in 2 pages so multiplied by2
								}
								xSemaphoreGive(sendExternalFlashSemaphore);
							}

							tHistoriDatalength = 7 + strlen((const char*)&historyDataPacket[7]);
							//tSendData=1;
						}
						else
						{
							//fnDebugMsg("\r\nRTU ID not matched\r\n");
						}
						break;
					}
					case JJMHDP_HISTORY_DATA_DONE:
					{
						//fnDebugMsg("\r\nJJMHDP_HISTORY_DATA_DONE\r\n");
						//sprintf(historyDataPackettemp,"====>>%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11]);
						//fnDebugMsg(historyDataPackettemp);
						if(data[9] == (unsigned char)EPROM_General.Rtu_Detail.RTUId)
						{
							historyDataPacket[0] = 'H';
							historyDataPacket[1] = JJMHDP_HISTORY_DATA_DONE;
							historyDataPacket[2] = (unsigned char)EPROM_General.Rtu_Detail.RTUId;
							historyDataPacket[3] = 1;
							tHistoriDatalength = 4;
							//tSendData=1;
							gStopHistoricalDataStore = 0;
						}
						else
						{
							//fnDebugMsg("\r\nRTU ID not matched\r\n");
						}
						break;
					}
					case JJMHDP_HISTORY_DATA_ERASE:
					{
						//fnDebugMsg("\r\nJJMHDP_HISTORY_DATA_ERASE\r\n");
						//sprintf(historyDataPackettemp,"====>>%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11]);
						//fnDebugMsg(historyDataPackettemp);
						if(data[9] == (unsigned char)EPROM_General.Rtu_Detail.RTUId)
						{
							g_flashRunTimeParaSturct.s_ExtDataFlash_IsDataLogOverwritten=0;
							g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter=0;
							W25Q_Erase_Write_One_Sector((unsigned char *)&g_flashRunTimeParaSturct,sizeof((const char *)&g_flashRunTimeParaSturct), HISTORY_DATA_RUN_TIME_PARA_START_ADDRESS);

							historyDataPacket[0] = 'H';
							historyDataPacket[1] = JJMHDP_HISTORY_DATA_ERASE;
							historyDataPacket[2] = (unsigned char)EPROM_General.Rtu_Detail.RTUId;
							historyDataPacket[3] = 1;
							tHistoriDatalength = 4;
							//tSendData=1;
							gStopHistoricalDataStore = 0;
						}
						else
						{
							//fnDebugMsg("\r\nRTU ID not matched\r\n");
						}
						break;
					}
					default:
					{
						// default statements
					}
			}
		}

//		fnDebugMsg("=========================================\r\n");
//		sprintf(historyDataPackettemp,"\r\nlength:%d",tHistoriDatalength);
//		fnDebugMsg(historyDataPackettemp);
//		if((tHistoriDatalength <= UDP_BUFFER_SIZE)&&(tSendData==1))
//		{
//			fnDebugMsg("=========================================\r\n");
//			sprintf(historyDataPackettemp,"\r\nlength:%d",tHistoriDatalength);
//			fnDebugMsg(historyDataPackettemp);
//			sprintf(historyDataPackettemp,"\r\n====>>%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",historyDataPacket[0],historyDataPacket[1],historyDataPacket[2],historyDataPacket[3],historyDataPacket[4],historyDataPacket[5],historyDataPacket[6],historyDataPacket[7],historyDataPacket[8],historyDataPacket[9],historyDataPacket[10],historyDataPacket[11]);
//			fnDebugMsg(historyDataPackettemp);
//			fnDebugMsg(historyDataPacket);
//			fnDebugMsg("=========================================/r/n");
//			uMemcpy(&ptrUDP_Frame->ucUDP_Message, historyDataPacket, tHistoriDatalength );     // send the received UDP frame back
//			fnSendUDP(MyUDP_Socket, ucIP, usPortNr, (unsigned char*)&ptrUDP_Frame->tUDP_Header, tHistoriDatalength , OWN_TASK);
//		}
	}
	return 0;
}
/**************************************************************************//**
 * Function name 	: ExtFlash_Read_RuntimePara
 * arguments		:
 * 		 			:
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/
void ExtFlash_Read_RuntimePara(unsigned char makeDefault)
{

	W25Q_ReadRaw((u8_t*) &g_flashRunTimeParaSturct, sizeof(g_flashRunTimeParaSturct), HISTORY_DATA_RUN_TIME_PARA_START_ADDRESS);


	if((g_flashRunTimeParaSturct.s_ExtDataFlash_CheckByte != 0xAB)||(makeDefault == 1))
	{
		g_flashRunTimeParaSturct.s_ExtDataFlash_CheckByte = 0xAB;
		g_flashRunTimeParaSturct.s_ExtDataFlash_IsDataLogOverwritten = 0;
		g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter = 0;
		g_flashRunTimeParaSturct.s_temp_Counter = 0 ;
	    sprintf(tDebug, "ExtFlash_Read_RuntimePara\r\n");
	    WriteLog(1, tDebug, 1);
		W25Q_Erase_Write_One_Sector((unsigned char *)&g_flashRunTimeParaSturct,sizeof(g_flashRunTimeParaSturct), HISTORY_DATA_RUN_TIME_PARA_START_ADDRESS);
	}

	if(g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter>=MAX_HISTORY_DATA_PACKETS)
	{
		g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter = 0;
	}
	if(g_flashRunTimeParaSturct.s_ExtDataFlash_IsDataLogOverwritten>1)
	{
		g_flashRunTimeParaSturct.s_ExtDataFlash_IsDataLogOverwritten = 0;
	}
	g_flashRunTimeParaSturct.s_temp_Counter ++;

//	W25Q_Erase_Write_One_Sector((unsigned char *)&g_flashRunTimeParaSturct,sizeof(g_flashRunTimeParaSturct), HISTORY_DATA_RUN_TIME_PARA_START_ADDRESS);
}

/**************************************************************************//**
 * Function name 	: ExtFlash_ReadHistoricalDataLogFromFlash
 * arguments		:
 * 		 			:
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/
void ExtFlash_ReadHistoricalDataLogFromFlash(unsigned int pageCounter,unsigned char *buffer)
{
	//pageCounter means frame number

	unsigned char temp_string[100],tag_no=0;

	unsigned int HistoryDataAddressPointer = pageCounter*512+HISTORY_DATA_FILE_START_ADDRESS;

	flashDataSturct tflashDataSturct;


	W25Q_ReadRaw((u8_t*) &tflashDataSturct, sizeof(tflashDataSturct), HistoryDataAddressPointer);

	sprintf((char*)temp_string,"%1d,%1d,%1d,%1dD,%02d%02d%02d,%02d%02d%02d,",(unsigned int)(pageCounter)+1,tflashDataSturct.sf_client_id,tflashDataSturct.sf_reader_id,1,tflashDataSturct.sf_Date,tflashDataSturct.sf_Month,tflashDataSturct.sf_Year,tflashDataSturct.sf_Hour,tflashDataSturct.sf_Min,tflashDataSturct.sf_Sec);
	strcpy((char*)buffer,(char*)temp_string);

	sprintf((char*)temp_string,"%02d,%02d,%d,",tflashDataSturct.sf_NO_DI_LG,(int)(tflashDataSturct.sf_NoofSMSTag),tflashDataSturct.sf_Address);
	strcat((char*)buffer,(char*)temp_string);

	sprintf((char*)temp_string,"%02X,%02X,",tflashDataSturct.sf_DI[0],tflashDataSturct.sf_DI[1]);			// combine 8 DI in 1Byte sf_DI[0] : 0 to 7 DI and sf_DI[1] have 8 to 15DI channel
	strcat((char*)buffer,(char*)temp_string);

	tag_no = 0;
	do
	{
		sprintf((char*)temp_string,"%3.02f,",tflashDataSturct.sf_AnalogValue[tag_no]);
		strcat((char*)buffer,(char*)temp_string);
		tag_no++;
	}
	while(tag_no < tflashDataSturct.sf_NoofSMSTag);
}

/**************************************************************************//**
 * Function name 	: ExtFlash_WriteHistoricalData
 * arguments		:
 * 		 			:
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/
void ExtFlash_WriteHistoricalData(flashDataSturct tflashDataSturct)
{
	//sprintf((char*)debug_print,"size of dataStruct : %d size of runtimeStruct : %d",sizeof(tFlashDataSturct),sizeof(g_flashRunTimeParaSturct));
	//fnDebugMsg(debug_print);
    unsigned int WriteAddress;

	if(gStopHistoricalDataStore == 0)
	{

		if(g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter<MAX_HISTORY_DATA_PACKETS)
		{
			WriteAddress = g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter*512+HISTORY_DATA_FILE_START_ADDRESS;

			if(g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter%8==0)
			{
				W25Q_EraseSector(WriteAddress);
			}

			W25Q_Write_continous((u8_t *)&tflashDataSturct, sizeof(tflashDataSturct),WriteAddress);

		}
		else
		{
			g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter = 0;
			g_flashRunTimeParaSturct.s_ExtDataFlash_IsDataLogOverwritten = 1;
			WriteAddress = g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter*512+HISTORY_DATA_FILE_START_ADDRESS;
			if(g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter%8==0)
			{
				W25Q_EraseSector(WriteAddress);
			}
			W25Q_Write_continous((u8_t *)&tflashDataSturct, sizeof(tflashDataSturct),WriteAddress);
			//WriteFlashtoflashMemory_new(g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter,(unsigned char *)&tflashDataSturct);
		}

		g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter++;
		W25Q_Erase_Write_One_Sector((unsigned char *)&g_flashRunTimeParaSturct,sizeof(g_flashRunTimeParaSturct), HISTORY_DATA_RUN_TIME_PARA_START_ADDRESS);
	}
}


