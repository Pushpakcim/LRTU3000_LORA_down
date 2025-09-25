/*
 * json_parser.c
 *
 *  Created on: Dec 15, 2022
 *      Author: maulin
 */


/*
 * json_parser.c
 *
 *  Created on: Dec 15, 2022
 *      Author: maulin
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "cJSON.h"
#include "json_parser.h"
#include "lwip/api.h"
#include "pcbplcService.h"
/**************************************************************************//**
 * Variable
 *****************************************************************************/
modem_time_t rtc;
//extern struct OTA OTA_Data;
extern struct Configuration Config;
struct OTA OTA_Data;
char msg_payload[1400];
char print1[300];
unsigned int OTA_START=0,OTA_resetCouner=0;
CMD_TYPE current_cmd;
extern void Ota_File_write_ack(COM_TYPE com_mode,int fileType,int chunk_number,int chunk_lenth,char *data ,char * ACK_Response);
JSON_ERROR_RESPONSE response_ACK_JSON_frame(COM_TYPE com_mode ,CMD_TYPE CMD, OTA_FILE_ACK iACK,char * ACK_Response);
JSON_ERROR_RESPONSE Config_response_ACK_JSON_frame(COM_TYPE com_mode , CMD_TYPE CMD, char cmdState, char file_CMD_type, RES_ACK iACK, char * ACK_Response);
extern int send_OTA_status;
int OTAStart =0;
int CRC_value;
extern plcRecFlashInfo_t gPlcRecFlash;
int gPlcFileLength;
int gRecFileLength;
unsigned long int ghexFileLength;

unsigned char flagMQTTPubSchedule=0,flagMQTTPubGetMode = 0,flagMqttPubLogData = 0,flagMqttPubLiveData=0,
		flagMQTT_ID_First=0,flagMQTT_ID_afterPowerCycle = 0,flagMQTT_TestMethod_1_ACK = 0,flagMQTT_TestMethod_1_Result=0,
		flagMQTT_TestMethod_2_ACK = 0,flagMQTT_TestMethod_2_Result=0,
		flagMQTT_ID_AI_CALI_App=0,flagMQTT_AI_Channel_CaliResponse=0,flagMQTT_AI_Channel_Test_Result=0;


unsigned char flagLORAPubLogData,flagLORAPubLogData_fail;

unsigned char ProductionModeIDFrameReceived=0,flagTCP_ID_First=0,flagTCP_ID_afterPowerCycle = 0,flagTCP_TestMethod_1_ACK = 0,flagTCP_TestMethod_1_Result=0,
		flagTCP_TestMethod_2_ACK = 0,flagTCP_TestMethod_2_Result=0, key_LED_flag=0,
		flagTCP_ID_AI_CALI_App=0,flagTCP_AI_Channel_CaliResponse=0,flagTCP_AI_Channel_Test_Result=0;

unsigned char pubScheduleBlock=0;
unsigned char gAI_Point = 0;
int Fream_id;

void WriteLog(uint8_t LogEnable,const char *pData,uint8_t logType);

/**************************************************************************//**
 * Check MQTT Received JSON Frame
 *****************************************************************************/

void Parse_IP_Address(char *input_string ,unsigned char * Ip_Address)
{
	Ip_Address[0]= 0;
	Ip_Address[1]= 0;
	Ip_Address[2]= 0;
	Ip_Address[3]= 0;

	size_t index = 0;
	while (*input_string)
	{
		if (isdigit((unsigned char)*input_string))
		{
			Ip_Address[index] *= 10;
			Ip_Address[index] += *input_string - '0';
		} else
		{
			index++;
		}
		input_string++;
	}
}
JSON_ERROR_RESPONSE parse_JSON_frame(COM_TYPE com_mode,char* jsonString, char * ACK_Response)
{
	JSON_ERROR_RESPONSE ret;
	const cJSON *CMD = NULL;
	cJSON *Received_json = NULL;

	ret = JSON_SUCCESS;

	Received_json = cJSON_Parse(jsonString);

    if (Received_json == NULL)
    {
        ret = FAILED_CREATE_JSON_OBJECT;
        cJSON_Delete(Received_json);
        return ret;
    }

    //const cJSON *TrackiD = NULL;
    //TrackiD = cJSON_GetObjectItemCaseSensitive(Received_json, "Trackid");

    CMD = cJSON_GetObjectItemCaseSensitive(Received_json, "CMD");
    current_cmd = CMD->valueint;
    switch (current_cmd)
    {
		case CMD_OTA:
		{
	       	OTA_START=1; //SH_INT_FLASH
	       	OTA_resetCouner=0;
			const cJSON *CMDState = NULL; //1
			const cJSON *FileType = NULL; //1
			const cJSON *Chunk_no = NULL; //1
			const cJSON *Chunk_Size = NULL; //512
			const cJSON *Chunk_Data = NULL; // hex string 2k or 4k data
			const cJSON *FileSize = NULL;
			const cJSON *CRC_Value = NULL;

			memset(&OTA_Data,0,sizeof(OTA_Data));
			if (cJSON_HasObjectItem(Received_json, "CMDState"))
			{
				CMDState = cJSON_GetObjectItemCaseSensitive(Received_json, "CMDState");
				OTA_Data.CMDState = CMDState->valueint;
				OTAStart=1;
			}

			if (cJSON_HasObjectItem(Received_json, "FileType"))
			{
				FileType = cJSON_GetObjectItemCaseSensitive(Received_json, "FileType");
				OTA_Data.FileType = FileType->valueint;
			}

			if (cJSON_HasObjectItem(Received_json, "Chunk_no"))
			{
				Chunk_no = cJSON_GetObjectItemCaseSensitive(Received_json, "Chunk_no");
				OTA_Data.Chunk_no = Chunk_no->valueint;
			}

			if (cJSON_HasObjectItem(Received_json, "Chunk_Size"))
			{
				Chunk_Size = cJSON_GetObjectItemCaseSensitive(Received_json, "Chunk_Size");
				OTA_Data.Chunk_Size = Chunk_Size->valueint;
			}

			if (cJSON_HasObjectItem(Received_json, "FileSize"))
			{
				FileSize = cJSON_GetObjectItemCaseSensitive(Received_json, "FileSize");
				OTA_Data.FileSize = FileSize->valueint;
				if(OTA_Data.FileType == PLC)
				{
					//gPlcRecFlash.mPlcFileLength = OTA_Data.FileSize;
					gPlcFileLength = OTA_Data.FileSize;
				}
				else if(OTA_Data.FileType == REC)
				{
					//gPlcRecFlash.mRecFileLength = OTA_Data.FileSize;
					gRecFileLength = OTA_Data.FileSize;
				}
				else if(OTA_Data.FileType == HEX)
				{
					ghexFileLength = OTA_Data.FileSize;
				}
			}
			if (cJSON_HasObjectItem(Received_json, "CRC"))
			{
				CRC_Value = cJSON_GetObjectItemCaseSensitive(Received_json, "CRC");
				CRC_value = CRC_Value->valueint;
			}

			if (cJSON_HasObjectItem(Received_json, "Chunk_Data"))
			{
				Chunk_Data = cJSON_GetObjectItemCaseSensitive(Received_json, "Chunk_Data");
//				if(OTA_Data.FileType == PLC)
//				{
//					strcpy(OTA_Data.Chunk_Data,Chunk_Data->valuestring);
//				}
//				else if(OTA_Data.FileType == REC)
//				{
//					strcpy(OTA_Data.Chunk_Data,Chunk_Data->valuestring);
//				}
//				else if(OTA_Data.FileType == HEX)
//				{
//					strcpy(OTA_Data.Chunk_Data,Chunk_Data->valuestring);
//				}
				//if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					Ota_File_write_ack(com_mode,OTA_Data.FileType,OTA_Data.Chunk_no,strlen(Chunk_Data->valuestring),Chunk_Data->valuestring,ACK_Response);
					//xSemaphoreGive(sendExternalFlashSemaphore);
				}
			}
			else
			{
				//if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					Ota_File_write_ack(com_mode,OTA_Data.FileType,0,0,0,(char *)ACK_Response);
					//xSemaphoreGive(sendExternalFlashSemaphore);
				}
			}
		}
		break;
		case CMD_CONF:// 1. Configuration Data
		{
			//{"CMD":1,"CMDState":1,"CMDType":2}
			const cJSON *CMDState = NULL;
			const cJSON *CMDType = NULL;

			if (cJSON_HasObjectItem(Received_json, "CMDState"))
			{
				CMDState = cJSON_GetObjectItemCaseSensitive(Received_json, "CMDState");
				Config.CMD_State = CMDState->valueint;
			}

			if (cJSON_HasObjectItem(Received_json, "CMDType"))
			{
				CMDType = cJSON_GetObjectItemCaseSensitive(Received_json, "CMDType");
				Config.CMD_Type = CMDType->valueint;
			}

			switch(Config.CMD_State)
			{
				case RTU_INFO:
				{
					switch(Config.CMD_Type)
					{
//						case WRITE_CMD:
//						{
//							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
//							//send ACK of write command
//							JSON_ret = response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
//							if(JSON_ret != JSON_SUCCESS)
//							{
//								JSON_ret = response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
//								if(JSON_ret != JSON_SUCCESS)
//								{
//									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
//									WriteLog(1,print,1);
//								}
//							}
//						}
//						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,  ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,  ACK_Response);
								if(JSON_ret )
								{

								}
							}

						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case CUST_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							//{"CMD":1,"CMDState":2,"CMDType":1,
							//"ProjectCode":"fthjf","SiteName":"sdfg","Timezone":"(UTC+05:30) Chennai, Kolkata, Mumbai, New Delhi",
							//"Lattitude":12.212,"Longitude":78.322,"RTUID":1,"ClientID":21,"ReaderID":12}

							if (cJSON_HasObjectItem(Received_json, "ProjectCode"))
							{
								const cJSON *ProjectCode = NULL;
								ProjectCode = cJSON_GetObjectItemCaseSensitive(Received_json, "ProjectCode");
								strcpy(EPROM_General.Cust_Detail.Proj_Code,"");
								strcpy(EPROM_General.Cust_Detail.Proj_Code,ProjectCode->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "SiteName"))
							{
								const cJSON *SiteName = NULL;
								SiteName = cJSON_GetObjectItemCaseSensitive(Received_json, "SiteName");
								strcpy(EPROM_General.Cust_Detail.Site_Name,"");
								strcpy(EPROM_General.Cust_Detail.Site_Name,SiteName->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "Timezone"))
							{
								const cJSON *Timezone = NULL;
								Timezone = cJSON_GetObjectItemCaseSensitive(Received_json, "Timezone");
								strcpy(EPROM_General.Cust_Detail.Time_zone,"");
								strcpy(EPROM_General.Cust_Detail.Time_zone,Timezone->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "Lattitude"))
							{
								const cJSON *Lattitude = NULL;
								Lattitude = cJSON_GetObjectItemCaseSensitive(Received_json, "Lattitude");
								EPROM_General.Cust_Detail.Lattitude = Lattitude->valuedouble;
							}

							if (cJSON_HasObjectItem(Received_json, "Longitude"))
							{
								const cJSON *Longitude = NULL;
								Longitude = cJSON_GetObjectItemCaseSensitive(Received_json, "Longitude");
								EPROM_General.Cust_Detail.Longitude = Longitude->valuedouble;
								Get_Astro_time();
							}

							if (cJSON_HasObjectItem(Received_json, "RTUID"))
							{
								const cJSON *RTUID = NULL;
								RTUID = cJSON_GetObjectItemCaseSensitive(Received_json, "RTUID");
								EPROM_General.Rtu_Detail.RTUId = RTUID->valueint;
								changeRTUIDinSlaveLogic();
							}

							if (cJSON_HasObjectItem(Received_json, "ClientID"))
							{
								const cJSON *ClientID = NULL;
								ClientID = cJSON_GetObjectItemCaseSensitive(Received_json, "ClientID");
								EPROM_General.Cust_Detail.Client_Id = ClientID->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "ReaderID"))
							{
								const cJSON *ReaderID = NULL;
								ReaderID = cJSON_GetObjectItemCaseSensitive(Received_json, "ReaderID");
								EPROM_General.Cust_Detail.Reader_Id = ReaderID->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "Timezone_hours"))
							{
								const cJSON *Timezone_hours = NULL;
								Timezone_hours = cJSON_GetObjectItemCaseSensitive(Received_json, "Timezone_hours");
								EPROM_General.Cust_Detail.Timezone_hours = Timezone_hours->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "Timezone_minutes"))
							{
								const cJSON *Timezone_minutes = NULL;
								Timezone_minutes = cJSON_GetObjectItemCaseSensitive(Received_json, "Timezone_minutes");
								EPROM_General.Cust_Detail.Timezone_minutes = Timezone_minutes->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "Timezone_sign"))
							{
								const cJSON *Timezone_sign = NULL;
								Timezone_sign = cJSON_GetObjectItemCaseSensitive(Received_json, "Timezone_sign");
								EPROM_General.Cust_Detail.Timezone_sign = Timezone_sign->valueint;
							}



							flag_flashUpdateEPROM_General = 1;
							flag_flashUpdateEPROM_General_WaitCounter = 5;
							Config.Res_Ack=1;
//To do Write in flash
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
							//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case AI_DI_DO_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
//{"CMD": 1,"CMDState": 3,"CMDType": 1,
//"TotalDI": 16,"TotalDO": 8,"TotalAI": 6,"AI": [
//{"id": 1,"type": 1,"sLow": 12.3,"sHigh": 15.5,"CalZ": 0,"CalS": 0},
//{"id": 2,"type": 1,"sLow": 12.3,"sHigh": 15.5,"CalZ": 0,"CalS": 0},
//{"id": 3,"type": 1,"sLow": 12.3,"sHigh": 15.5,"CalZ": 0,"CalS": 0}]}

							if (cJSON_HasObjectItem(Received_json, "TotalDI"))
							{
								const cJSON *TotalDI = NULL;
								TotalDI = cJSON_GetObjectItemCaseSensitive(Received_json, "TotalDI");
								EPROM_General.AI_DI_DO_Detail.Total_Di = TotalDI->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "TotalDO"))
							{
								const cJSON *TotalDO = NULL;
								TotalDO = cJSON_GetObjectItemCaseSensitive(Received_json, "TotalDO");
								EPROM_General.AI_DI_DO_Detail.Total_Do= TotalDO->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "TotalAI"))
							{
								const cJSON *TotalAI = NULL;
								TotalAI = cJSON_GetObjectItemCaseSensitive(Received_json, "TotalAI");
								EPROM_General.AI_DI_DO_Detail.Total_Ai = TotalAI->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "AI"))
							{
								const cJSON *Ai = NULL;

								Ai = cJSON_GetObjectItemCaseSensitive(Received_json, "AI");
								if(Ai)
								{
									const cJSON *a_Ai = NULL;
									int  Ai_length_array = 0;
									int index = 0;

									Ai_length_array = cJSON_GetArraySize(Ai);
									sprintf(print1,"Number of AI :%d",Ai_length_array);
									WriteLog(1,(const char*)print,1);

									cJSON_ArrayForEach(a_Ai, Ai)
									{
										if (cJSON_HasObjectItem(a_Ai, "id"))
										{
											const cJSON *id = NULL;
											id = cJSON_GetObjectItemCaseSensitive(a_Ai, "id");
											EPROM_General.AI_DI_DO_Detail.AI_Detail[index].Id = id->valueint;
										}

										if (cJSON_HasObjectItem(a_Ai, "type"))
										{
											const cJSON *type = NULL;
											type = cJSON_GetObjectItemCaseSensitive(a_Ai, "type");
											EPROM_General.AI_DI_DO_Detail.AI_Detail[index].AI_ch_Type = type->valueint;
										}

										if (cJSON_HasObjectItem(a_Ai, "sLow"))
										{
											const cJSON *sLow = NULL;
											sLow = cJSON_GetObjectItemCaseSensitive(a_Ai, "sLow");
											EPROM_General.AI_DI_DO_Detail.AI_Detail[index].scaleLo = sLow->valuedouble;
										}

										if (cJSON_HasObjectItem(a_Ai, "sHigh"))
										{
											const cJSON *sHigh = NULL;
											sHigh = cJSON_GetObjectItemCaseSensitive(a_Ai, "sHigh");
											EPROM_General.AI_DI_DO_Detail.AI_Detail[index].scaleHi = sHigh->valuedouble;
										}

										if (cJSON_HasObjectItem(a_Ai, "CalZ"))
										{
											const cJSON *CalZ = NULL;
											CalZ = cJSON_GetObjectItemCaseSensitive(a_Ai, "CalZ");
											EPROM_General.AI_DI_DO_Detail.AI_Detail[index].calZ = CalZ->valuedouble;
										}

										if (cJSON_HasObjectItem(a_Ai, "CalS"))
										{
											const cJSON *CalS = NULL;
											CalS = cJSON_GetObjectItemCaseSensitive(a_Ai, "CalS");
											EPROM_General.AI_DI_DO_Detail.AI_Detail[index].calS = CalS->valuedouble;
										}
										index++;
									}
								}
							}
							flag_flashUpdateEPROM_General = 1;
							flag_flashUpdateEPROM_General_WaitCounter = 5;
							Config.Res_Ack=1;

//To do Write in flash
//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case SERIAL_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
//{"CMD":1,"CMDState":4,"CMDType":1,
//{"CMD":1,"CMDState":4,"CMDType":1,
//"RS232_1":{"Enable":1,"Protocol":3,"Master":0,"PortID":1,"BaudRate":38400,"PollFreq":2},
//"RS232_2":{"Enable":1,"Protocol":1,"Master":0,"PortID":1,"BaudRate":600,"PollFreq":3},
//"RS485_1":{"Enable":1,"Protocol":2,"Master":0,"PortID":1,"BaudRate":9600,"PollFreq":54},
//"RS485_2":{"Enable":0,"Protocol":3,"Master":0,"PortID":1,"BaudRate":115200,"PollFreq":3}}

							if (cJSON_HasObjectItem(Received_json, "RS232_1"))
							{
								const cJSON *RS232_1 = NULL;

								RS232_1 = cJSON_GetObjectItemCaseSensitive(Received_json, "RS232_1");

								if(RS232_1)
								{
									if (cJSON_HasObjectItem(RS232_1, "Enable"))
									{
										const cJSON *Enable = NULL;
										Enable = cJSON_GetObjectItemCaseSensitive(RS232_1, "Enable");
										EPROM_General.S_Comm.Rs232_1_Info.S_Co_En_Di = Enable->valueint;
									}

									if (cJSON_HasObjectItem(RS232_1, "Protocol"))
									{
										const cJSON *Protocol = NULL;
										Protocol = cJSON_GetObjectItemCaseSensitive(RS232_1, "Protocol");
										EPROM_General.S_Comm.Rs232_1_Info.S_Protocol = Protocol->valueint;
									}

									if (cJSON_HasObjectItem(RS232_1, "Master"))
									{
										const cJSON *Master = NULL;
										Master = cJSON_GetObjectItemCaseSensitive(RS232_1, "Master");
										EPROM_General.S_Comm.Rs232_1_Info.S_Ma_sl_Cu = Master->valueint;
									}

									if (cJSON_HasObjectItem(RS232_1, "PortID"))
									{
										const cJSON *PortID = NULL;
										PortID = cJSON_GetObjectItemCaseSensitive(RS232_1, "PortID");
										EPROM_General.S_Comm.Rs232_1_Info.S_Port_Id = PortID->valueint;
									}

									if (cJSON_HasObjectItem(RS232_1, "BaudRate"))
									{
										const cJSON *BaudRate = NULL;
										BaudRate = cJSON_GetObjectItemCaseSensitive(RS232_1, "BaudRate");
										if((BaudRate->valueint < 1200) || (BaudRate->valueint > 256000)) // for handling || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-26
										{
											EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate = 9600;
										}
										else
										{
											EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate = BaudRate->valueint;
										}
									}

									if (cJSON_HasObjectItem(RS232_1, "PollFreq"))
									{
										const cJSON *PollFreq = NULL;
										PollFreq = cJSON_GetObjectItemCaseSensitive(RS232_1, "PollFreq");
										EPROM_General.S_Comm.Rs232_1_Info.S_Poll_Freq = PollFreq->valueint;
									}

								}

							}

							if (cJSON_HasObjectItem(Received_json, "RS232_2"))
							{
								const cJSON *RS232_2 = NULL;

								RS232_2 = cJSON_GetObjectItemCaseSensitive(Received_json, "RS232_2");

								if(RS232_2)
								{
									if (cJSON_HasObjectItem(RS232_2, "Enable"))
									{
										const cJSON *Enable = NULL;
										Enable = cJSON_GetObjectItemCaseSensitive(RS232_2, "Enable");
										EPROM_General.S_Comm.Rs232_2_Info.S_Co_En_Di = Enable->valueint;
									}

									if (cJSON_HasObjectItem(RS232_2, "Protocol"))
									{
										const cJSON *Protocol = NULL;
										Protocol = cJSON_GetObjectItemCaseSensitive(RS232_2, "Protocol");
										EPROM_General.S_Comm.Rs232_2_Info.S_Protocol  = Protocol->valueint;
									}

									if (cJSON_HasObjectItem(RS232_2, "Master"))
									{
										const cJSON *Master = NULL;
										Master = cJSON_GetObjectItemCaseSensitive(RS232_2, "Master");
										EPROM_General.S_Comm.Rs232_2_Info.S_Ma_sl_Cu = Master->valueint;
									}

									if (cJSON_HasObjectItem(RS232_2, "PortID"))
									{
										const cJSON *PortID = NULL;
										PortID = cJSON_GetObjectItemCaseSensitive(RS232_2, "PortID");
										EPROM_General.S_Comm.Rs232_2_Info.S_Port_Id = PortID->valueint;
									}

									if (cJSON_HasObjectItem(RS232_2, "BaudRate"))
									{
										const cJSON *BaudRate = NULL;
										BaudRate = cJSON_GetObjectItemCaseSensitive(RS232_2, "BaudRate");
										if((BaudRate->valueint < 1200) || (BaudRate->valueint > 256000)) // for handling || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-26
										{
											EPROM_General.S_Comm.Rs232_2_Info.S_Baudrate = 9600;
										}
										else
										{
											EPROM_General.S_Comm.Rs232_2_Info.S_Baudrate = BaudRate->valueint;
										}
									}

									if (cJSON_HasObjectItem(RS232_2, "PollFreq"))
									{
										const cJSON *PollFreq = NULL;
										PollFreq = cJSON_GetObjectItemCaseSensitive(RS232_2, "PollFreq");
										EPROM_General.S_Comm.Rs232_2_Info.S_Poll_Freq = PollFreq->valueint;
									}
								}
							}

							if (cJSON_HasObjectItem(Received_json, "RS485_1"))
							{
								const cJSON *RS485_1 = NULL;

								RS485_1 = cJSON_GetObjectItemCaseSensitive(Received_json, "RS485_1");

								if(RS485_1)
								{
									if (cJSON_HasObjectItem(RS485_1, "Enable"))
									{
										const cJSON *Enable = NULL;
										Enable = cJSON_GetObjectItemCaseSensitive(RS485_1, "Enable");
										EPROM_General.S_Comm.Rs485_1_Info.S_Co_En_Di = Enable->valueint;
									}

									if (cJSON_HasObjectItem(RS485_1, "Protocol"))
									{
										const cJSON *Protocol = NULL;
										Protocol = cJSON_GetObjectItemCaseSensitive(RS485_1, "Protocol");
										EPROM_General.S_Comm.Rs485_1_Info.S_Protocol = Protocol->valueint;
									}

									if (cJSON_HasObjectItem(RS485_1, "Master"))
									{
										const cJSON *Master = NULL;
										Master = cJSON_GetObjectItemCaseSensitive(RS485_1, "Master");
										EPROM_General.S_Comm.Rs485_1_Info.S_Ma_sl_Cu = Master->valueint;
									}

									if (cJSON_HasObjectItem(RS485_1, "PortID"))
									{
										const cJSON *PortID = NULL;
										PortID = cJSON_GetObjectItemCaseSensitive(RS485_1, "PortID");
										EPROM_General.S_Comm.Rs485_1_Info.S_Port_Id = PortID->valueint;
									}

									if (cJSON_HasObjectItem(RS485_1, "BaudRate"))
									{
										const cJSON *BaudRate = NULL;
										BaudRate = cJSON_GetObjectItemCaseSensitive(RS485_1, "BaudRate");
										if((BaudRate->valueint < 1200) || (BaudRate->valueint > 256000)) // for handling || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-26
										{
											EPROM_General.S_Comm.Rs485_1_Info.S_Baudrate = 9600;
										}
										else
										{
											EPROM_General.S_Comm.Rs485_1_Info.S_Baudrate = BaudRate->valueint;
										}
									}

									if (cJSON_HasObjectItem(RS485_1, "PollFreq"))
									{
										const cJSON *PollFreq = NULL;
										PollFreq = cJSON_GetObjectItemCaseSensitive(RS485_1, "PollFreq");
										EPROM_General.S_Comm.Rs485_1_Info.S_Poll_Freq = PollFreq->valueint;
									}
								}
							}

							if (cJSON_HasObjectItem(Received_json, "RS485_2"))
							{
								const cJSON *RS485_2 = NULL;

								RS485_2 = cJSON_GetObjectItemCaseSensitive(Received_json, "RS485_2");

								if(RS485_2)
								{
									if (cJSON_HasObjectItem(RS485_2, "Enable"))
									{
										const cJSON *Enable = NULL;
										Enable = cJSON_GetObjectItemCaseSensitive(RS485_2, "Enable");
										EPROM_General.S_Comm.Rs485_2_Info.S_Co_En_Di = Enable->valueint;
									}

									if (cJSON_HasObjectItem(RS485_2, "Protocol"))
									{
										const cJSON *Protocol = NULL;
										Protocol = cJSON_GetObjectItemCaseSensitive(RS485_2, "Protocol");
										EPROM_General.S_Comm.Rs485_2_Info.S_Protocol = Protocol->valueint;
									}

									if (cJSON_HasObjectItem(RS485_2, "Master"))
									{
										const cJSON *Master = NULL;
										Master = cJSON_GetObjectItemCaseSensitive(RS485_2, "Master");
										EPROM_General.S_Comm.Rs485_2_Info.S_Ma_sl_Cu = Master->valueint;
									}

									if (cJSON_HasObjectItem(RS485_2, "PortID"))
									{
										const cJSON *PortID = NULL;
										PortID = cJSON_GetObjectItemCaseSensitive(RS485_2, "PortID");
										EPROM_General.S_Comm.Rs485_2_Info.S_Port_Id = PortID->valueint;
									}

									if (cJSON_HasObjectItem(RS485_2, "BaudRate"))
									{
										const cJSON *BaudRate = NULL;
										BaudRate = cJSON_GetObjectItemCaseSensitive(RS485_2, "BaudRate");
										if((BaudRate->valueint < 1200) || (BaudRate->valueint > 256000)) // for handling || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-26
										{
											EPROM_General.S_Comm.Rs485_2_Info.S_Baudrate = 9600;
										}
										else
										{
											EPROM_General.S_Comm.Rs485_2_Info.S_Baudrate = BaudRate->valueint;
										}
									}

									if (cJSON_HasObjectItem(RS485_2, "PollFreq"))
									{
										const cJSON *PollFreq = NULL;
										PollFreq = cJSON_GetObjectItemCaseSensitive(RS485_2, "PollFreq");
										EPROM_General.S_Comm.Rs485_2_Info.S_Poll_Freq = PollFreq->valueint;
									}
								}
							}
							flag_flashUpdateEPROM_General = 1;
							flag_flashUpdateEPROM_General_WaitCounter = 5;
							BuildModbusMasterQueryTelegrams();
							Config.Res_Ack=1;
//To do Write in flash
//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case ETHERNET_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;

//{"CMD":1,"CMDState":5,"CMDType":1,
//"Enable":1,"Mode":0,"IPAddress":"123.021.36.23","Subnetmask":"123.021.36.21","Gateway":"123.021.36.26",
//"PreferredDNS":"123.021.36.27","AlternateDNS":"123.021.36.28","ModbusTCPEnable":1,"ModbusTCPServer":2,
//"ModbusTCPPort":502,"PollFreq":3}

							if (cJSON_HasObjectItem(Received_json, "Enable"))
							{
								const cJSON *Enable = NULL;
								Enable = cJSON_GetObjectItemCaseSensitive(Received_json, "Enable");
								EPROM_General.E_Comm.E_Co_En_Di = Enable->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "Mode"))
							{
								const cJSON *Mode = NULL;
								Mode = cJSON_GetObjectItemCaseSensitive(Received_json, "Mode");
								EPROM_General.E_Comm.E_Mode = Mode->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "IPAddress"))
							{
								const cJSON *IPAddress = NULL;
								IPAddress = cJSON_GetObjectItemCaseSensitive(Received_json, "IPAddress");
								Parse_IP_Address(IPAddress->valuestring,(unsigned char * )EPROM_General.E_Comm.E_IP_Add);
//								strcpy(E_Comm.E_IP_Add , IPAddress->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "Subnetmask"))
							{
								const cJSON *Subnetmask = NULL;
								Subnetmask = cJSON_GetObjectItemCaseSensitive(Received_json, "Subnetmask");
//								strcpy(E_Comm.E_Subnet_Add , Subnetmask->valuestring);
								Parse_IP_Address(Subnetmask->valuestring,(unsigned char * )EPROM_General.E_Comm.E_Subnet_Add);
							}

							if (cJSON_HasObjectItem(Received_json, "Gateway"))
							{
								const cJSON *Gateway = NULL;
								Gateway = cJSON_GetObjectItemCaseSensitive(Received_json, "Gateway");
//								strcpy(E_Comm.E_Gateway_Add , Gateway->valuestring);
								Parse_IP_Address(Gateway->valuestring,(unsigned char * )EPROM_General.E_Comm.E_Gateway_Add);
							}

							if (cJSON_HasObjectItem(Received_json, "PreferredDNS"))
							{
								const cJSON *PreferredDNS = NULL;
								PreferredDNS = cJSON_GetObjectItemCaseSensitive(Received_json, "PreferredDNS");
//								strcpy(E_Comm.E_Preferred_DNS , PreferredDNS->valuestring);
								Parse_IP_Address(PreferredDNS->valuestring,(unsigned char * )EPROM_General.E_Comm.E_Preferred_DNS);
							}

							if (cJSON_HasObjectItem(Received_json, "AlternateDNS"))
							{
								const cJSON *AlternateDNS = NULL;
								AlternateDNS = cJSON_GetObjectItemCaseSensitive(Received_json, "AlternateDNS");
//								strcpy(E_Comm.E_Alternate_DNS , AlternateDNS->valuestring);
								Parse_IP_Address(AlternateDNS->valuestring,(unsigned char * )EPROM_General.E_Comm.E_Alternate_DNS);
							}

							if (cJSON_HasObjectItem(Received_json, "ModbusTCPEnable"))
							{
								const cJSON *ModbusTCPEnable = NULL;
								ModbusTCPEnable = cJSON_GetObjectItemCaseSensitive(Received_json, "ModbusTCPEnable");
								EPROM_General.E_Comm.E_Mod_TCP = ModbusTCPEnable->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "ModbusTCPServer"))
							{
								const cJSON *ModbusTCPServer = NULL;
								ModbusTCPServer = cJSON_GetObjectItemCaseSensitive(Received_json, "ModbusTCPServer");
								EPROM_General.E_Comm.E_Ser_cli = ModbusTCPServer->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "ModbusTCPPort"))
							{
								const cJSON *ModbusTCPPort = NULL;
								ModbusTCPPort = cJSON_GetObjectItemCaseSensitive(Received_json, "ModbusTCPPort");
								EPROM_General.E_Comm.E_TCP_Port = ModbusTCPPort->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "PollFreq"))
							{
								const cJSON *PollFreq = NULL;
								PollFreq = cJSON_GetObjectItemCaseSensitive(Received_json, "PollFreq");
								EPROM_General.E_Comm.E_Poll_Freq = PollFreq->valueint;
							}
							flag_flashUpdateEPROM_General = 1;
							flag_flashUpdateEPROM_General_WaitCounter = 5;
							Config.Res_Ack=1;

//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case CELL_MODEM_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
//{"CMD":1,"CMDState":6,"CMDType":1,
//"Enable":1,"CellInterface":"1","APN":"airtelgprs.com","Protocol":0,"MQTTBrokerIP":"32.36.65.54","MQTTBrokerPort":123,
//"MQTTClientId":"12","MQTTUserName":"asd","MQTTPWD":"sdd","MQTTPubTopic":"dsffv","MQTTSubTopic":"fweg","MQTTLiveFreq":"2"}
							if (cJSON_HasObjectItem(Received_json, "Enable"))
							{
								const cJSON *Enable = NULL;
								Enable = cJSON_GetObjectItemCaseSensitive(Received_json, "Enable");
								EPROM_General.Mo_Comm.Mo_Co_En_Di = Enable->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "CellInterface"))
							{
								const cJSON *CellInterface = NULL;
								CellInterface = cJSON_GetObjectItemCaseSensitive(Received_json, "CellInterface");
								EPROM_General.Mo_Comm.Mo_Com_Int = CellInterface->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "APN"))
							{
								const cJSON *APN = NULL;
								APN = cJSON_GetObjectItemCaseSensitive(Received_json, "APN");
								strcpy(EPROM_General.Mo_Comm.Mo_APN , APN->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "Protocol"))
							{
								const cJSON *Protocol = NULL;
								Protocol = cJSON_GetObjectItemCaseSensitive(Received_json, "Protocol");
								EPROM_General.Mo_Comm.Mo_Proto = Protocol->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "MQTTBrokerIP"))
							{
								const cJSON *MQTTBrokerIP = NULL;
								MQTTBrokerIP = cJSON_GetObjectItemCaseSensitive(Received_json, "MQTTBrokerIP");
								strcpy(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP , MQTTBrokerIP->valuestring);
								flag_modem_MQTT_Reconnect = 1;
							}

							if (cJSON_HasObjectItem(Received_json, "MQTTBrokerPort"))
							{
								const cJSON *MQTTBrokerPort = NULL;
								MQTTBrokerPort = cJSON_GetObjectItemCaseSensitive(Received_json, "MQTTBrokerPort");
								EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_Port = MQTTBrokerPort->valueint;
								flag_modem_MQTT_Reconnect = 1;
							}

							if (cJSON_HasObjectItem(Received_json, "MQTTClientId"))
							{
								const cJSON *MQTTClientId = NULL;
								MQTTClientId = cJSON_GetObjectItemCaseSensitive(Received_json, "MQTTClientId");
								strcpy(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Cli_Id , MQTTClientId->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "MQTTUserName"))
							{
								const cJSON *MQTTUserName = NULL;
								MQTTUserName = cJSON_GetObjectItemCaseSensitive(Received_json, "MQTTUserName");
								strcpy(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Name , MQTTUserName->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "MQTTPWD"))
							{
								const cJSON *MQTTPWD = NULL;
								MQTTPWD = cJSON_GetObjectItemCaseSensitive(Received_json, "MQTTPWD");
								strcpy(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Pass , MQTTPWD->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "MQTTPubTopic"))
							{
								const cJSON *MQTTPubTopic = NULL;
								MQTTPubTopic = cJSON_GetObjectItemCaseSensitive(Received_json, "MQTTPubTopic");
								strcpy(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_PUB_Topic , MQTTPubTopic->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "MQTTSubTopic"))
							{
								const cJSON *MQTTSubTopic = NULL;
								MQTTSubTopic = cJSON_GetObjectItemCaseSensitive(Received_json, "MQTTSubTopic");
								strcpy(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Sub_Topic , MQTTSubTopic->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "MQTTLiveFreq"))
							{
								const cJSON *MQTTLiveFreq = NULL;
								MQTTLiveFreq = cJSON_GetObjectItemCaseSensitive(Received_json, "MQTTLiveFreq");
								EPROM_General.Mo_Comm.MQTT_LiveFreq = MQTTLiveFreq->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "MQTTConnectionMode"))
							{
								const cJSON *MQTTConnectionMode = NULL;
								MQTTConnectionMode = cJSON_GetObjectItemCaseSensitive(Received_json, "MQTTConnectionMode");
								EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Comm_Mode = MQTTConnectionMode->valueint;
							}

							flag_flashUpdateEPROM_General = 1;
							flag_flashUpdateEPROM_General_WaitCounter = 5;
							Config.Res_Ack=1;

//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case BLE_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
//{"CMD":1,"CMDState":7,"CMDType":1,
//"Enable":0}
							if (cJSON_HasObjectItem(Received_json, "Enable"))
							{
								const cJSON *Enable = NULL;
								Enable = cJSON_GetObjectItemCaseSensitive(Received_json, "Enable");
								EPROM_General.bleDetails.BLE_Co_En_Di = Enable->valueint;
							}
							flag_flashUpdateEPROM_General = 1;
							flag_flashUpdateEPROM_General_WaitCounter = 5;
							Config.Res_Ack=1;

//To do Write in flash
//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case GPS_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
//{"CMD":1,"CMDState":8,"CMDType":1,
//"Enable":0,"PollFreq":2}

							if (cJSON_HasObjectItem(Received_json, "Enable"))
							{
								const cJSON *Enable = NULL;
								Enable = cJSON_GetObjectItemCaseSensitive(Received_json, "Enable");
								EPROM_General.gpsDetails.GPS_Co_En_Di = Enable->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "PollFreq"))
							{
								const cJSON *PollFreq = NULL;
								PollFreq = cJSON_GetObjectItemCaseSensitive(Received_json, "PollFreq");
								EPROM_General.gpsDetails.GPS_Poll_Freq = PollFreq->valueint;
							}
							flag_flashUpdateEPROM_General = 1;
							flag_flashUpdateEPROM_General_WaitCounter = 5;
							Config.Res_Ack=1;
//To do Write in flash
//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case SD_CARD_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
//{"CMD":1,"CMDState":9,"CMDType":1,
//"Enable":0}
							if (cJSON_HasObjectItem(Received_json, "Enable"))
							{
								const cJSON *Enable = NULL;
								Enable = cJSON_GetObjectItemCaseSensitive(Received_json, "Enable");
								EPROM_General.sdCardDetails.SD_Card_Co_En_Di = Enable->valueint;
							}
							flag_flashUpdateEPROM_General = 1;
							flag_flashUpdateEPROM_General_WaitCounter = 5;
							Config.Res_Ack=1;
//To do Write in flash
//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case RTC_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
//{"CMD":1,"CMDState":10,"CMDType":1,"DD":3,"MM":2,"YY":23,"HH":18,"MN":40,"SS":10}

							const cJSON *date = NULL;
							const cJSON *month = NULL;
							const cJSON *year = NULL;
							const cJSON *hour = NULL;
							const cJSON *min = NULL;
							const cJSON *sec = NULL;

							if (cJSON_HasObjectItem(Received_json, "DD"))
							{
								date = cJSON_GetObjectItemCaseSensitive(Received_json, "DD");
								rtc.date = (unsigned char)date->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "MM"))
							{
								month = cJSON_GetObjectItemCaseSensitive(Received_json, "MM");
								rtc.month = (unsigned char)month->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "YY"))
							{
								year = cJSON_GetObjectItemCaseSensitive(Received_json, "YY");
								rtc.year = year->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "HH"))
							{
								hour = cJSON_GetObjectItemCaseSensitive(Received_json, "HH");
								rtc.hour = (unsigned char)hour->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "MN"))
							{
								min = cJSON_GetObjectItemCaseSensitive(Received_json, "MN");
								rtc.min = (unsigned char)min->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "SS"))
							{
								sec = cJSON_GetObjectItemCaseSensitive(Received_json, "SS");
								rtc.sec = (unsigned char)sec->valueint;
							}

							struct tm t, t1;

							t.tm_hour = rtc.hour;
							t.tm_min = rtc.min;
							t.tm_sec = rtc.sec;
							//t.tm_wday = (gTimeInfo.WeekDay) - 1;
							t.tm_mon = rtc.month - 1;
							t.tm_mday = rtc.date;
							t.tm_year = (rtc.year-2000) + 2000 - 1900;

							UTC = (uint64_t)mktime_new(&t);

							set_time(t1);
//							if (cJSON_HasObjectItem(Received_json, "RTCDateTime"))
//							{
//								const cJSON *Enable = NULL;
//								Enable = cJSON_GetObjectItemCaseSensitive(Received_json, "RTCDateTime");
//								strcpy(Config.RTC_D_T , Enable->valuestring);
//								//TODO: Get time ans sync
//							}
							Config.Res_Ack=1;
							Get_Astro_time();
//To do Write in flash
//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case HISTORY_LOGRATE_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
//{"CMD":1,"CMDState":11,"CMDType":1,
//"lograte":1,"Enable":0}
							if (cJSON_HasObjectItem(Received_json, "Enable"))
							{
								const cJSON *Enable = NULL;
								Enable = cJSON_GetObjectItemCaseSensitive(Received_json, "Enable");
								EPROM_General.History_En_Di = Enable->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "lograte"))
							{
								const cJSON *lograte = NULL;
								lograte = cJSON_GetObjectItemCaseSensitive(Received_json, "lograte");
								if(((lograte->valueint)<60) || ((lograte->valueint) == 60) || ((lograte->valueint) == 120) || ((lograte->valueint) == 180)
										|| ((lograte->valueint) == 240) || ((lograte->valueint) == 360) || ((lograte->valueint) == 720))
								{
									EPROM_General.LogRate = lograte->valueint;
								}
								else
								{
									EPROM_General.LogRate = DEFAULT_LOGRATE;
								}

								//maxLograteTimeSliceDelay_Second
								//gpcbplcCnfg.mLogRate = lograte->valueint;
							}
							if (cJSON_HasObjectItem(Received_json, "maxLograteTimeSliceDelayS"))
							{
								const cJSON *maxLograteTimeSliceDelayS = NULL;
								maxLograteTimeSliceDelayS = cJSON_GetObjectItemCaseSensitive(Received_json, "maxLograteTimeSliceDelayS");
								if(((maxLograteTimeSliceDelayS->valueint)<=1800)&&((maxLograteTimeSliceDelayS->valueint)>=255))
								{
									EPROM_General.maxLograteTimeSliceDelayS = maxLograteTimeSliceDelayS->valueint;
								}
								else
								{
									EPROM_General.maxLograteTimeSliceDelayS = DEFAULT_MAX_LOGRATE_TIME_SLICE_DELAY_S;
								}
								calculateLograteTimeSliceDelayS();
							}
							flag_flashUpdateEPROM_General = 1;
							flag_flashUpdateEPROM_General_WaitCounter = 5;
							Config.Res_Ack=1;
//To do Write in flash
//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case DO_MODE_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
//{"CMD":1,"CMDState":12,"CMDType":1,
//"DOMode":0,"DOValue":"1,1100000000000000000000000000000"}
							if (cJSON_HasObjectItem(Received_json, "DOMode"))
							{
								const cJSON *DOMode = NULL;
								DOMode = cJSON_GetObjectItemCaseSensitive(Received_json, "DOMode");
								EPROM_General.DoModeDetails.Do_Mode = DOMode->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "DOValue"))
							{
								const cJSON *DOValue = NULL;
								DOValue = cJSON_GetObjectItemCaseSensitive(Received_json, "DOValue");

								for(unsigned index_i=0;index_i<12;index_i++)  // TODO : replace 12 with variable
								{
									if(DOValue->valuestring[index_i] == '0')
									{
										EPROM_General.DoModeDetails.DO_Value[index_i]=0;
									}
									else if(DOValue->valuestring[index_i] == '1')
									{
										EPROM_General.DoModeDetails.DO_Value[index_i]=1;
									}
								}
								//strcpy(EPROM_General.DoModeDetails.DO_Value , DOValue->valuestring);
							}
							flag_flashUpdateEPROM_General = 1;
							flag_flashUpdateEPROM_General_WaitCounter = 5;
							Config.Res_Ack=1;

//To do Write in flash
//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case SCHEDULE_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
							int CurrentFrame_val;
//{"CMD":1,"CMDState":13,"CMDType":1,
//"TotalSch":13,"Schedule":[
//{"SchID":2,"Enable":1,"StartHH":0,"StartMin":0,"StopHH":0,"StopMin":0},
//{"SchID":3,"Enable":1,"StartHH":0,"StartMin":0,"StopHH":0,"StopMin":0},
//{"SchID":4,"Enable":1,"StartHH":0,"StartMin":0,"StopHH":0,"StopMin":0},
//{"SchID":5,"Enable":1,"StartHH":0,"StartMin":0,"StopHH":0,"StopMin":0},
//{"SchID":6,"Enable":1,"StartHH":0,"StartMin":0,"StopHH":0,"StopMin":0},
//{"SchID":7,"Enable":1,"StartHH":0,"StartMin":0,"StopHH":0,"StopMin":0},
//{"SchID":8,"Enable":1,"StartHH":0,"StartMin":0,"StopHH":0,"StopMin":0},
//{"SchID":9,"Enable":1,"StartHH":0,"StartMin":0,"StopHH":0,"StopMin":0}]}


							if (cJSON_HasObjectItem(Received_json, "TotalSch"))
							{
								const cJSON *TotalSch = NULL;
								TotalSch = cJSON_GetObjectItemCaseSensitive(Received_json, "TotalSch");
								EPROM_Schedule.Total_No_Schedule = TotalSch->valueint;
							}

							for(unsigned char sched_i = EPROM_Schedule.Total_No_Schedule ; sched_i < 84 ; sched_i++)
							{
								EPROM_Schedule.Schedule[sched_i].Sch_En_Di = CONF_DISABLE;
								EPROM_Schedule.Schedule[sched_i].Sch_Id = 0;
								EPROM_Schedule.Schedule[sched_i].Start_HH = 0;
								EPROM_Schedule.Schedule[sched_i].Start_Min = 0;
								EPROM_Schedule.Schedule[sched_i].Stop_HH = 0;
								EPROM_Schedule.Schedule[sched_i].Stop_Min = 0;
							}

							if (cJSON_HasObjectItem(Received_json, "CurrentFrame"))
							{
								const cJSON *CurrentFrame = NULL;
								CurrentFrame = cJSON_GetObjectItemCaseSensitive(Received_json, "CurrentFrame");
								CurrentFrame_val = CurrentFrame->valueint;
							}


							if (cJSON_HasObjectItem(Received_json, "Schedule"))
							{
								const cJSON *Schedule = NULL;

								Schedule = cJSON_GetObjectItemCaseSensitive(Received_json, "Schedule");
								if(Schedule)
								{
									const cJSON *a_Schedule = NULL;
									int  Schedule_length_array = 0;
									int index = 0;

									index = (8 * (CurrentFrame_val - 1));
									Schedule_length_array = cJSON_GetArraySize(Schedule);
									sprintf(print1,"Number of Schedule :%d",Schedule_length_array);
									WriteLog(1,(const char*)print,1);

									cJSON_ArrayForEach(a_Schedule, Schedule)
									{
										if (cJSON_HasObjectItem(a_Schedule, "SchID"))
										{
											const cJSON *SchID = NULL;
											SchID = cJSON_GetObjectItemCaseSensitive(a_Schedule, "SchID");
											EPROM_Schedule.Schedule[index].Sch_Id = SchID->valueint;
										}

										if (cJSON_HasObjectItem(a_Schedule, "Enable"))
										{
											const cJSON *Enable = NULL;
											Enable = cJSON_GetObjectItemCaseSensitive(a_Schedule, "Enable");
											EPROM_Schedule.Schedule[index].Sch_En_Di = Enable->valueint;
										}

										if (cJSON_HasObjectItem(a_Schedule, "StartHH"))
										{
											const cJSON *StartHH = NULL;
											StartHH = cJSON_GetObjectItemCaseSensitive(a_Schedule, "StartHH");
											EPROM_Schedule.Schedule[index].Start_HH = StartHH->valueint;
										}

										if (cJSON_HasObjectItem(a_Schedule, "StartMin"))
										{
											const cJSON *StartMin = NULL;
											StartMin = cJSON_GetObjectItemCaseSensitive(a_Schedule, "StartMin");
											EPROM_Schedule.Schedule[index].Start_Min = StartMin->valueint;
										}

										if (cJSON_HasObjectItem(a_Schedule, "StopHH"))
										{
											const cJSON *StopHH = NULL;
											StopHH = cJSON_GetObjectItemCaseSensitive(a_Schedule, "StopHH");
											EPROM_Schedule.Schedule[index].Stop_HH = StopHH->valueint;
										}

										if (cJSON_HasObjectItem(a_Schedule, "StopMin"))
										{
											const cJSON *StopMin = NULL;
											StopMin = cJSON_GetObjectItemCaseSensitive(a_Schedule, "StopMin");
											EPROM_Schedule.Schedule[index].Stop_Min = StopMin->valueint;
										}
										index++;
									}
								}
							}
							flag_flashUpdateEPROM_Schedule = 1;
							flag_flashUpdateEPROM_Schedule_WaitCounter = 5;
							Config.Res_Ack=1;
//To do Write in flash
//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;

							if (cJSON_HasObjectItem(Received_json, "ReadFrameID"))
							{
								const cJSON *ReadFrameID = NULL;
								ReadFrameID = cJSON_GetObjectItemCaseSensitive(Received_json, "ReadFrameID");
								Fream_id = ReadFrameID->valueint;
							}

							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case MODBUS_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
							int CurrentFrame_val;
//{"CMD":1,"CMDState":14,"CMDType":1,
//"TotalQuery":14,"TotalPara":16,"RetryCount":8,"ModbusQuery":[
//{"QueryID":1,"MasterPortID":1,"SlaveID":1,"StartAdd":0,"Length":0,"QueryType":0,"DataType":0},
//{"QueryID":2,"MasterPortID":1,"SlaveID":1,"StartAdd":0,"Length":0,"QueryType":0,"DataType":0},
//{"QueryID":3,"MasterPortID":1,"SlaveID":1,"StartAdd":0,"Length":0,"QueryType":0,"DataType":0},
//{"QueryID":4,"MasterPortID":1,"SlaveID":1,"StartAdd":0,"Length":0,"QueryType":0,"DataType":0},
//{"QueryID":5,"MasterPortID":1,"SlaveID":1,"StartAdd":0,"Length":0,"QueryType":0,"DataType":0},
//{"QueryID":6,"MasterPortID":1,"SlaveID":1,"StartAdd":0,"Length":0,"QueryType":0,"DataType":0},
//{"QueryID":7,"MasterPortID":1,"SlaveID":1,"StartAdd":0,"Length":0,"QueryType":0,"DataType":0},
//{"QueryID":8,"MasterPortID":1,"SlaveID":1,"StartAdd":0,"Length":0,"QueryType":0,"DataType":0},
//{"QueryID":9,"MasterPortID":1,"SlaveID":1,"StartAdd":0,"Length":0,"QueryType":0,"DataType":0},
//{"QueryID":10,"MasterPortID":1,"SlaveID":1,"StartAdd":0,"Length":0,"QueryType":0,"DataType":0}]}

							if (cJSON_HasObjectItem(Received_json, "TotalQuery"))
							{
								const cJSON *TotalQuery = NULL;
								TotalQuery = cJSON_GetObjectItemCaseSensitive(Received_json, "TotalQuery");
								if(TotalQuery->valueint > MODBUS_MASTER_MAX_TOTAL_QUERY)
								{
									EPROM_Modbus_Quary_Detail.TotalQuery = MODBUS_MASTER_MAX_TOTAL_QUERY;
								}
								else
								{
									EPROM_Modbus_Quary_Detail.TotalQuery = TotalQuery->valueint;
								}

							}

							if (cJSON_HasObjectItem(Received_json, "TotalPara"))
							{
								const cJSON *TotalPara = NULL;
								TotalPara = cJSON_GetObjectItemCaseSensitive(Received_json, "TotalPara");
								EPROM_Modbus_Quary_Detail.TotalPara = TotalPara->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "RetryCount"))
							{
								const cJSON *RetryCount = NULL;
								RetryCount = cJSON_GetObjectItemCaseSensitive(Received_json, "RetryCount");
								EPROM_Modbus_Quary_Detail.RetryCount = RetryCount->valueint;
							}
							if (cJSON_HasObjectItem(Received_json, "CurrentFrame"))
							{
								const cJSON *CurrentFrame = NULL;
								CurrentFrame = cJSON_GetObjectItemCaseSensitive(Received_json, "CurrentFrame");
								CurrentFrame_val = CurrentFrame->valueint;
							}

							if (cJSON_HasObjectItem(Received_json, "ModbusQuery"))
							{
								const cJSON *ModbusQuery = NULL;

								ModbusQuery = cJSON_GetObjectItemCaseSensitive(Received_json, "ModbusQuery");
								if(ModbusQuery)
								{
									const cJSON *a_ModbusQuery = NULL;
									int  ModbusQuery_length_array = 0;
									int index = 0;

									index = (8 * (CurrentFrame_val - 1));
									ModbusQuery_length_array = cJSON_GetArraySize(ModbusQuery);
									sprintf(print1,"Number of Modbus Quary :%d",ModbusQuery_length_array);
									WriteLog(1,(const char*)print,1);

									cJSON_ArrayForEach(a_ModbusQuery, ModbusQuery)
									{
										if (cJSON_HasObjectItem(a_ModbusQuery, "QueryID"))
										{
											const cJSON *QueryID = NULL;
											QueryID = cJSON_GetObjectItemCaseSensitive(a_ModbusQuery, "QueryID");
											EPROM_Modbus_Quary_Detail.Mod_Quary[index].Mod_Quary_ID= QueryID->valueint;
										}

										if (cJSON_HasObjectItem(a_ModbusQuery, "MasterPortID"))
										{
											const cJSON *MasterPortID = NULL;
											MasterPortID = cJSON_GetObjectItemCaseSensitive(a_ModbusQuery, "MasterPortID");
											EPROM_Modbus_Quary_Detail.Mod_Quary[index].mPortSelection = MasterPortID->valueint;
										}

										if (cJSON_HasObjectItem(a_ModbusQuery, "SlaveID"))
										{
											const cJSON *SlaveID = NULL;
											SlaveID = cJSON_GetObjectItemCaseSensitive(a_ModbusQuery, "SlaveID");
											EPROM_Modbus_Quary_Detail.Mod_Quary[index].mSlaveId = SlaveID->valueint;
										}

										if (cJSON_HasObjectItem(a_ModbusQuery, "StartAdd"))
										{
											const cJSON *StartAdd = NULL;
											StartAdd = cJSON_GetObjectItemCaseSensitive(a_ModbusQuery, "StartAdd");
											EPROM_Modbus_Quary_Detail.Mod_Quary[index].mRegStartAddr = StartAdd->valueint;
										}

										if (cJSON_HasObjectItem(a_ModbusQuery, "Length"))
										{
											const cJSON *Length = NULL;
											Length = cJSON_GetObjectItemCaseSensitive(a_ModbusQuery, "Length");
											EPROM_Modbus_Quary_Detail.Mod_Quary[index].mNoOfRegister = Length->valueint;
										}

										if (cJSON_HasObjectItem(a_ModbusQuery, "QueryType"))
										{
											const cJSON *QueryType = NULL;
											QueryType = cJSON_GetObjectItemCaseSensitive(a_ModbusQuery, "QueryType");
											EPROM_Modbus_Quary_Detail.Mod_Quary[index].mFunctionCode = QueryType->valueint;
										}

										if (cJSON_HasObjectItem(a_ModbusQuery, "DataType"))
										{
											const cJSON *DataType = NULL;
											DataType = cJSON_GetObjectItemCaseSensitive(a_ModbusQuery, "DataType");
											EPROM_Modbus_Quary_Detail.Mod_Quary[index].mDataType = DataType->valueint;
										}
										index++;
									}
								}
							}
							flag_flashUpdateEPROM_Modbus_Quary_Detail = 1;
							flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter = 5;
							BuildModbusMasterQueryTelegrams();
							Config.Res_Ack =1;
//To do Write in flash
//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;

							if (cJSON_HasObjectItem(Received_json, "ReadFrameID"))
							{
								const cJSON *ReadFrameID = NULL;
								ReadFrameID = cJSON_GetObjectItemCaseSensitive(Received_json, "ReadFrameID");
								Fream_id = ReadFrameID->valueint;
							}

							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD state %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				case LORA_INFO:
				{
					switch(Config.CMD_Type)
					{
						case WRITE_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret = JSON_SUCCESS;
//{"CMD":1,"CMDState":15,"CMDType":1,
//"lora_dev_eui":"AC1F09FFFE0D6691","lora_app_eui":"AC1F09FFF8657431","lora_app_key":"AC1F09FFFE0BA82CAC1F09FFF8657431","lora_adaptiveDataRate":0,"lora_device_serial_number":"AC1F09FFF8657431",
//"lora_network_Mode":1,"lora_dataRate":3,"lora_class":'C',"lora_power":0,"lora_active_region":3}
							/*
							 * 	unsigned char checkbyte;							//1
								unsigned char ChecksumOfStuct;						//1+1=2
								uint16_t SizeOfStuct;								//2+2=4
								unsigned char forFutureUse1[20];
								unsigned char lora_dev_eui_set[24];  	// 16byte  *
								unsigned char lora_app_eui_set[24];		// 16byte	*
								unsigned char lora_app_key_set[48];		// 32byte	*
								unsigned char lora_network_Mode_set; //the network mode (0 = ABP, 1 = OTAA) *
								unsigned char lora_adaptiveDataRate_enable_set; //the adaptive data rate setting (0 = off, 1 = on) *
								unsigned char lora_dataRate_set; //set the data rate (0,1,2,3,4,5,6,7) *
								unsigned char lora_class_set; //set the class (65 = A, 66 = B ,67 = C) *
								unsigned char lora_power_set; //set the power (0 high and 10 low) *
								unsigned char lora_active_region_set;// * AT+BAND: get or set the active region(0 = EU433, 1 = CN470, 2 = RU864, 3 = IN865, 4 = EU868,5 = US915, 6 = AU915, 7 = KR920, 8 = AS923-1, 9 = AS923-2, 10 = AS923-3, 11 = AS923-4, 12 = LA915)
								unsigned char lora_link_check_enable_set;// * AT+LINKCHECK: get or set the link check setting (0 = disabled, 1 = once, 2 = everytime)
								unsigned char lora_device_serial_number_set[24]; // * AT+SN device serial number 1-18byte
								unsigned char lora_duty_cycle_enable_set; // * the duty cycle setting (0 = off, 1 = on)
								unsigned int lora_baudrate_set; // * AT+BAUD= set the baudrate
							 */

							if (cJSON_HasObjectItem(Received_json, "lora_dev_eui"))
							{
								const cJSON *lora_dev_eui = NULL;
								lora_dev_eui = cJSON_GetObjectItemCaseSensitive(Received_json, "lora_dev_eui");
								strcpy((char *)EPROM_LoRa_Modem.lora_dev_eui_set , lora_dev_eui->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "lora_app_eui"))
							{
								const cJSON *lora_app_eui = NULL;
								lora_app_eui = cJSON_GetObjectItemCaseSensitive(Received_json, "lora_app_eui");
								strcpy((char *)EPROM_LoRa_Modem.lora_app_eui_set , lora_app_eui->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "lora_app_key"))
							{
								const cJSON *lora_app_key = NULL;
								lora_app_key = cJSON_GetObjectItemCaseSensitive(Received_json, "lora_app_key");
								strcpy((char *)EPROM_LoRa_Modem.lora_app_key_set , lora_app_key->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "lora_device_serial_number"))
							{
								const cJSON *lora_device_serial_number = NULL;
								lora_device_serial_number = cJSON_GetObjectItemCaseSensitive(Received_json, "lora_device_serial_number");
								strcpy((char *)EPROM_LoRa_Modem.lora_device_serial_number_set , lora_device_serial_number->valuestring);
							}

							if (cJSON_HasObjectItem(Received_json, "lora_network_Mode"))
							{
								const cJSON *lora_network_Mode = NULL;
								lora_network_Mode = cJSON_GetObjectItemCaseSensitive(Received_json, "lora_network_Mode");
								if((lora_network_Mode->valueint == 0)||(lora_network_Mode->valueint == 1))
								{
									EPROM_LoRa_Modem.lora_network_Mode_set = lora_network_Mode->valueint;
								}
							}

							if (cJSON_HasObjectItem(Received_json, "lora_adaptiveDataRate"))
							{
								const cJSON *lora_adaptiveDataRate = NULL;
								lora_adaptiveDataRate = cJSON_GetObjectItemCaseSensitive(Received_json, "lora_adaptiveDataRate");
								if((lora_adaptiveDataRate->valueint==0)||(lora_adaptiveDataRate->valueint==1))
								{
									EPROM_LoRa_Modem.lora_adaptiveDataRate_enable_set = lora_adaptiveDataRate->valueint;
								}
							}

							if (cJSON_HasObjectItem(Received_json, "lora_dataRate"))
							{
								const cJSON *lora_dataRate = NULL;
								lora_dataRate = cJSON_GetObjectItemCaseSensitive(Received_json, "lora_dataRate");
								if(( lora_dataRate->valueint>=0)&&( lora_dataRate->valueint<=7))
								{
									EPROM_LoRa_Modem.lora_dataRate_set = lora_dataRate->valueint;
								}
							}

							if (cJSON_HasObjectItem(Received_json, "lora_class"))
							{
								const cJSON *lora_class = NULL;
								lora_class = cJSON_GetObjectItemCaseSensitive(Received_json, "lora_class");
								if((lora_class->valueint=='A')||(lora_class->valueint=='B')||(lora_class->valueint=='C'))
								{
									EPROM_LoRa_Modem.lora_class_set = lora_class->valueint;
								}
							}

							if (cJSON_HasObjectItem(Received_json, "lora_power"))
							{
								const cJSON *lora_power = NULL;
								lora_power = cJSON_GetObjectItemCaseSensitive(Received_json, "lora_power");
								if((lora_power->valueint>=0)&&(lora_power->valueint<=10))
								{
									EPROM_LoRa_Modem.lora_power_set = lora_power->valueint;
								}
							}

							if (cJSON_HasObjectItem(Received_json, "lora_active_region"))
							{
								const cJSON *lora_active_region = NULL;
								lora_active_region = cJSON_GetObjectItemCaseSensitive(Received_json, "lora_active_region");
								if((lora_active_region->valueint>=0)&&(lora_active_region->valueint<=8))
								{
									EPROM_LoRa_Modem.lora_active_region_set = lora_active_region->valueint;
								}
							}

							if(convertHextoAsciiString((char*)EPROM_LoRa_Modem.lora_app_eui_set,(char*)&Lora_Modem_Ascii_String_app_eui,strlen((char*)EPROM_LoRa_Modem.lora_app_eui_set)*2)==0)
							{

							}
							if(convertHextoAsciiString((char*)EPROM_LoRa_Modem.lora_app_key_set,(char*)&Lora_Modem_Ascii_String_app_key,strlen((char*)EPROM_LoRa_Modem.lora_app_key_set)*2)==0)
							{

							}

							flag_flashUpdateEPROM_LORA = 1;
							flag_flashUpdateEPROM_LORA_WaitCounter = 5;
							Config.Res_Ack=1;

//send ACK of write command
							JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
							if(JSON_ret != JSON_SUCCESS)
							{
								JSON_ret = Config_response_ACK_JSON_frame(TCP , CMD_CONF, Config.CMD_State, Config.CMD_Type, Config.Res_Ack, ACK_Response);
								if(JSON_ret != JSON_SUCCESS)
								{
									sprintf(print1,"ACk of command %d Not create success ",Config.CMD_Type);
									WriteLog(1,(const char*)print,1);
								}
							}
						}
						break;
						case READ_CMD:
						{
							JSON_ERROR_RESPONSE JSON_ret;
							JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
							if(JSON_ret )
							{
								JSON_ret = Response_Config_Read_JSON_frame(TCP , CMD_CONF,Config.CMD_State, Config.CMD_Type, Config.Res_Ack,   ACK_Response);
								if(JSON_ret )
								{

								}
							}
						}
						break;
						default:
						{
							sprintf(print1,"CMD %d Not read or write command received",Config.CMD_State);
							WriteLog(1,(const char*)print,1);
						}
						break;
					}
				}
				break;
				default:
				{
					sprintf(print1,"CMD state %d not listed ",Config.CMD_State);
					WriteLog(1,(const char*)print,1);
				}
				break;
			}

			if((flag_flashUpdateEPROM_Modbus_Quary_Detail == 1)||(flag_flashUpdateEPROM_Schedule == 1) || (flag_flashUpdateEPROM_General == 1))
			{
				syncExtFlashVariableWithPCBPLCVariable();
			}

		}
		break;

		case CMD_PRODUCTION: 	//6: //Set_RTC
		{
			const cJSON *CMDState = NULL;
			if (cJSON_HasObjectItem(Received_json, "CMDState"))
			{
				CMDState = cJSON_GetObjectItemCaseSensitive(Received_json, "CMDState");
				switch(CMDState->valueint)
				{
					case 1: // ID request
					{
						/*
						 * {
								"CMD": 2,
								"CMDState": 1,
								"DeviceID": "abcd1234-12",
								"slotNo": 1,
								"HW_Version": "x.x.x",
								"DD": 9,
								"MM": 2,
								"YY": 2023,
								"HH": 10,
								"MN": 37,
								"SS": 50,
								"LORA_AppEUI":"0123456789abcdef0123456789abcdef",
								"LORA_Appkey":"0123456789abcdef"
							}
						 */
						const cJSON *DeviceID = NULL;
//						const cJSON *slotNo = NULL;
						const cJSON *HW_Version = NULL;
						const cJSON *date = NULL;
						const cJSON *month = NULL;
						const cJSON *year = NULL;
						const cJSON *hour = NULL;
						const cJSON *min = NULL;
						const cJSON *sec = NULL;
						const cJSON *LORA_AppEUI = NULL;
						const cJSON *LORA_Appkey = NULL;
						//const cJSON *MQTTBrokerIP = NULL;
						//const cJSON *MQTTBrokerPort = NULL;

						if (cJSON_HasObjectItem(Received_json, "DeviceID"))
						{
							DeviceID = cJSON_GetObjectItemCaseSensitive(Received_json, "DeviceID");
							memset(EPROM_General.DeviceID,0,sizeof(EPROM_General.DeviceID));
							strcpy((char *)EPROM_General.DeviceID , (const char *)DeviceID->valuestring);

							memset(EPROM_AI_Calibration.DeviceID,0,sizeof(EPROM_AI_Calibration.DeviceID));
							memcpy(EPROM_AI_Calibration.DeviceID,EPROM_General.DeviceID,sizeof(EPROM_General.DeviceID));

						}
//						if (cJSON_HasObjectItem(Received_json, "slotNo"))
//						{
//							slotNo = cJSON_GetObjectItemCaseSensitive(Received_json, "slotNo");
//						}
						if (cJSON_HasObjectItem(Received_json, "HW_Version"))
						{
							HW_Version = cJSON_GetObjectItemCaseSensitive(Received_json, "HW_Version");
							memset(EPROM_General.Rtu_Detail.HW_Version,0,sizeof(EPROM_General.Rtu_Detail.HW_Version));
							strcpy(EPROM_General.Rtu_Detail.HW_Version , HW_Version->valuestring);
						}
						if (cJSON_HasObjectItem(Received_json, "DD"))
						{
							date = cJSON_GetObjectItemCaseSensitive(Received_json, "DD");
							rtc.date = (unsigned char)date->valueint;
						}
						if (cJSON_HasObjectItem(Received_json, "MM"))
						{
							month = cJSON_GetObjectItemCaseSensitive(Received_json, "MM");
							rtc.month = (unsigned char)month->valueint;
						}
						if (cJSON_HasObjectItem(Received_json, "YY"))
						{
							year = cJSON_GetObjectItemCaseSensitive(Received_json, "YY");
							rtc.year = year->valueint;
						}
						if (cJSON_HasObjectItem(Received_json, "HH"))
						{
							hour = cJSON_GetObjectItemCaseSensitive(Received_json, "HH");
							rtc.hour = (unsigned char)hour->valueint;
						}
						if (cJSON_HasObjectItem(Received_json, "MN"))
						{
							min = cJSON_GetObjectItemCaseSensitive(Received_json, "MN");
							rtc.min = (unsigned char)min->valueint;
						}
						if (cJSON_HasObjectItem(Received_json, "SS"))
						{
							sec = cJSON_GetObjectItemCaseSensitive(Received_json, "SS");
							rtc.sec = (unsigned char)sec->valueint;
						}
						if (cJSON_HasObjectItem(Received_json, "LORA_AppEUI"))
						{
							LORA_AppEUI = cJSON_GetObjectItemCaseSensitive(Received_json, "LORA_AppEUI");
							strcpy((char *)EPROM_LoRa_Modem.lora_app_eui_set,(const char *)LORA_AppEUI->valuestring);
						}
						if (cJSON_HasObjectItem(Received_json, "LORA_Appkey"))
						{
							LORA_Appkey = cJSON_GetObjectItemCaseSensitive(Received_json, "LORA_Appkey");
							strcpy((char *)EPROM_LoRa_Modem.lora_app_key_set,(const char *)LORA_Appkey->valuestring);
						}
						//strcpy((char *)pro_MQTT_Client_ID,(const char *)EPROM_General.DeviceID);

						struct tm t, t1;
						t.tm_hour = rtc.hour;
						t.tm_min = rtc.min;
						t.tm_sec = rtc.sec;
						//t.tm_wday = (gTimeInfo.WeekDay) - 1;
						t.tm_mon = rtc.month - 1;
						t.tm_mday = rtc.date;
						t.tm_year = (rtc.year-2000) + 2000 - 1900;
						UTC = (uint64_t)mktime_new(&t);
						set_time(t1);

						if(com_mode == TCP)
						{
							flagTCP_ID_First=1;
						}
						else if(com_mode == MQTT)
						{
							flagMQTT_ID_First=1;
						}
						ProductionModeIDFrameReceived = 1;
						flag_flashUpdateEPROM_General = 1;
						flag_flashUpdateEPROM_General_WaitCounter = 5;
						flag_flashUpdateEPROM_AI_Calibration = 1;
						flag_flashUpdateEPROM_AI_Calibration_WaitCounter = 5;
						flag_flashUpdateEPROM_LORA = 1;
						flag_flashUpdateEPROM_LORA_WaitCounter = 5;
						flag_modem_MQTT_Reconnect = 1;
					//	flagLORAPubLogData=1;
				//		flagLORAPubLogData_fail = 255;
				//osDelay(4000);
						break;
					}
					case 2: // Test Method 1 request
					{
						/*
						 	{
								"CMD": 2,
								"CMDState": 2,
								"DeviceID": "abcd1234-12",
								"slotNo": 1,
								"TestRequest":139
							}
						 */
						const cJSON *TestRequest = NULL;
						if (cJSON_HasObjectItem(Received_json, "TestRequest"))
						{
							TestRequest = cJSON_GetObjectItemCaseSensitive(Received_json, "TestRequest");
							proTestRequest = TestRequest->valueint;
						}
						if(com_mode == TCP)
						{
							flagTCP_TestMethod_1_ACK = 1;
				//			flagLORAPubLogData=1;
						}
						else if(com_mode == MQTT)
						{
							flagMQTT_TestMethod_1_ACK = 1;
						}

						break;
					}
					case 3: // Test Method 1 result request
					{
						/*
						 * {
								"CMD": 2,
								"CMDState": 3,
								"DeviceID": "abcd1234-12",
								"slotNo": 1,
								"TestRequest":139
							}
						 */
						if(com_mode == TCP)
						{
							flagTCP_TestMethod_1_Result = 1;
					//		flagLORAPubLogData=1;
						}
						else if(com_mode == MQTT)
						{
							flagMQTT_TestMethod_1_Result = 1;
						}
						// TODO : Set flag to send Test Method 1 result
						break;
					}
					case 4: // ID request after Power Cycle
					{
						/*
						 * {
								"CMD": 2,
								"CMDState": 4,
								"DeviceID": "abcd1234-12",
								"slotNo": 1,
								"HW_Version": "x.x.x",
								"DD": 9,
								"MM": 2,
								"YY": 2023,
								"HH": 10,
								"MN": 37,
								"SS": 50,
								"LORA_AppEUI":"0123456789abcdef0123456789abcdef",
								"LORA_Appkey":"0123456789abcdef"
							}
						 */
//						const cJSON *date = NULL;
//						const cJSON *month = NULL;
//						const cJSON *year = NULL;
//						const cJSON *hour = NULL;
//						const cJSON *min = NULL;
//						const cJSON *sec = NULL;
//						const cJSON *LORA_AppEUI = NULL;
//						const cJSON *LORA_Appkey = NULL;
//						//const cJSON *MQTTBrokerIP = NULL;
//						//const cJSON *MQTTBrokerPort = NULL;
//
//
//						if (cJSON_HasObjectItem(Received_json, "DD"))
//						{
//							date = cJSON_GetObjectItemCaseSensitive(Received_json, "DD");
//							rtc.date = (unsigned char)date->valueint;
//						}
//						if (cJSON_HasObjectItem(Received_json, "MM"))
//						{
//							month = cJSON_GetObjectItemCaseSensitive(Received_json, "MM");
//							rtc.month = (unsigned char)month->valueint;
//						}
//						if (cJSON_HasObjectItem(Received_json, "YY"))
//						{
//							year = cJSON_GetObjectItemCaseSensitive(Received_json, "YY");
//							rtc.year = year->valueint;
//						}
//						if (cJSON_HasObjectItem(Received_json, "HH"))
//						{
//							hour = cJSON_GetObjectItemCaseSensitive(Received_json, "HH");
//							rtc.hour = (unsigned char)hour->valueint;
//						}
//						if (cJSON_HasObjectItem(Received_json, "MN"))
//						{
//							min = cJSON_GetObjectItemCaseSensitive(Received_json, "MN");
//							rtc.min = (unsigned char)min->valueint;
//						}
//						if (cJSON_HasObjectItem(Received_json, "SS"))
//						{
//							sec = cJSON_GetObjectItemCaseSensitive(Received_json, "SS");
//							rtc.sec = (unsigned char)sec->valueint;
//						}
//
//						struct tm t, t1;
//						t.tm_hour = rtc.hour;
//						t.tm_min = rtc.min;
//						t.tm_sec = rtc.sec;
//						//t.tm_wday = (gTimeInfo.WeekDay) - 1;
//						t.tm_mon = rtc.month - 1;
//						t.tm_mday = rtc.date;
//						t.tm_year = (rtc.year-2000) + 2000 - 1900;
//						UTC = (uint64_t)mktime_new(&t);
//						set_time(t1);

						flagTCP_ID_afterPowerCycle = 1;
						if(com_mode == TCP)
						{
							flagTCP_ID_afterPowerCycle = 1;
						}
						else if(com_mode == MQTT)
						{
							flagMQTT_ID_afterPowerCycle = 1;
						}
						// TODO : Set flag to send ID frame after power cycle
						break;
					}
					case 5: // Test Method 2 request
					{
						/*
						 * {
								"CMD": 2,
								"CMDState": 5,
								"DeviceID": "abcd1234-12",
								"slotNo": 1,
								"TestRequest":139
							}
						 */
						const cJSON *TestRequest = NULL;
						if (cJSON_HasObjectItem(Received_json, "TestRequest"))
						{
							TestRequest = cJSON_GetObjectItemCaseSensitive(Received_json, "TestRequest");
							proTestRequest = TestRequest->valueint;
						}
						if(com_mode == TCP)
						{
							flagTCP_TestMethod_2_ACK = 1;
						}
						else if(com_mode == MQTT)
						{
							flagMQTT_TestMethod_2_ACK = 1;
						}
						break;
					}
					case 6: // Test Method 2 result request
					{
						/*
						 * {
								"CMD": 2,
								"CMDState": 3,
								"DeviceID": "abcd1234-12",
								"slotNo": 1,
								"TestRequest":139
							}
						 */
						// TODO : Set flag to send Test Method 2 result
						if(com_mode == TCP)
						{
							flagTCP_TestMethod_2_Result = 1;
						}
						else if(com_mode == MQTT)
						{
							flagMQTT_TestMethod_2_Result = 1;
						}
						break;
					}
					default:
					{


					}
				}
			}
			break;
		}
		case CMD_AI_CAL: 	//6: //Set_RTC
		{


			const cJSON *CMDState = NULL;
			if (cJSON_HasObjectItem(Received_json, "CMDState"))
			{
				CMDState = cJSON_GetObjectItemCaseSensitive(Received_json, "CMDState");
				switch(CMDState->valueint)
				{
					case 1: // ID request
					{
						/*
							{
								"CMD": 3,
								"CMDState": 1,
								"slotNo": 1,
							}
						 */

//						const cJSON *slotNo = NULL;

//						if (cJSON_HasObjectItem(Received_json, "slotNo"))
//						{
//							slotNo = cJSON_GetObjectItemCaseSensitive(Received_json, "slotNo");
//						}
						if(com_mode == TCP)
						{
							flagTCP_ID_AI_CALI_App = 1;
						}
						else if(com_mode == MQTT)
						{
							flagMQTT_ID_AI_CALI_App = 1;
						}
						break;
					}
					case 2: // AI Channel Calibration Command Request
					{
						/*
							{
								"CMD": 3,
								"CMDState": 2,
								"DeviceID": "abcd1234-12",
								"slotNo": 1,
								"AI_Channel": 1,
								"AI_Type": 1, // current : 1 voltage : 2
								"AI_Point": 1, // 1 2 3 4
								"AI_Value": 1,
							}
						 */
						const cJSON *AI_Channel = NULL;
						const cJSON *AI_Type = NULL;
						const cJSON *AI_Point = NULL;
						const cJSON *AI_Value = NULL;

						if (cJSON_HasObjectItem(Received_json, "AI_Channel"))
						{
							AI_Channel = cJSON_GetObjectItemCaseSensitive(Received_json, "AI_Channel");

							if (cJSON_HasObjectItem(Received_json, "AI_Type"))
							{
								AI_Type = cJSON_GetObjectItemCaseSensitive(Received_json, "AI_Type");

								EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].AI_ch_Type = AI_Type->valueint;
								EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].AI_ch_Type = AI_Type->valueint;

								if (cJSON_HasObjectItem(Received_json, "AI_Point"))
								{
									AI_Point = cJSON_GetObjectItemCaseSensitive(Received_json, "AI_Point");
								}
								if (cJSON_HasObjectItem(Received_json, "AI_Value"))
								{
									AI_Value = cJSON_GetObjectItemCaseSensitive(Received_json, "AI_Value");
								}

								gAI_Point = AI_Point->valueint;

								if(EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].AI_ch_Type == V0TO20)
								{
									if(AI_Point->valueint == 4)
									{
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiHighCal_V = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiHighCal_V_Point = AI_Value->valuedouble;

										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiHighCal_V = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiHighCal_V_Point = AI_Value->valuedouble;
									}
									else if(AI_Point->valueint == 3)
									{
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiMidCal_V = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiMidCal_V_Point = AI_Value->valuedouble;

										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiMidCal_V = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiMidCal_V_Point = AI_Value->valuedouble;
									}
									else if(AI_Point->valueint == 2)
									{
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiLowMidCal_V = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiLowMidCal_V_Point = AI_Value->valuedouble;

										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiLowMidCal_V = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiLowMidCal_V_Point = AI_Value->valuedouble;
									}
									else if(AI_Point->valueint == 1)
									{
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiLowCal_V = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiLowCal_V_Point = AI_Value->valuedouble;

										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiLowCal_V = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiLowCal_V_Point = AI_Value->valuedouble;
									}
								}
								else
								{
									if(AI_Point->valueint == 4)
									{
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiHighCal_mA = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiHighCal_mA_Point = AI_Value->valueint;

										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiMidCal_mA = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiMidCal_mA_Point = AI_Value->valueint;
									}
									else if(AI_Point->valueint == 3)
									{
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiMidCal_mA = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiMidCal_mA_Point = AI_Value->valueint;

										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiMidCal_mA = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiMidCal_mA_Point = AI_Value->valueint;
									}
									else if(AI_Point->valueint == 2)
									{
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiLowMidCal_mA = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiLowMidCal_mA_Point = AI_Value->valueint;

										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiLowMidCal_mA = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiLowMidCal_mA_Point = AI_Value->valueint;
									}
									else if(AI_Point->valueint == 1)
									{
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiLowCal_mA = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_General.AI_DI_DO_Detail.AI_Detail[(AI_Channel->valueint)-1].mAiLowCal_mA_Point = AI_Value->valueint;

										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiLowCal_mA = ADC_VAL_RAW[(AI_Channel->valueint)-1];
										EPROM_AI_Calibration.AI_Detail[(AI_Channel->valueint)-1].mAiLowCal_mA_Point = AI_Value->valueint;
									}
								}
							}
						}
						if(com_mode == TCP)
						{
							flagTCP_AI_Channel_CaliResponse = AI_Channel->valueint;
						}
						else if(com_mode == MQTT)
						{
							flagMQTT_AI_Channel_CaliResponse = AI_Channel->valueint;
						}

						flag_flashUpdateEPROM_AI_Calibration = 1;
						flag_flashUpdateEPROM_AI_Calibration_WaitCounter=2;

						flag_flashUpdateEPROM_General = 1;
						flag_flashUpdateEPROM_General_WaitCounter = 5;

						break;
					}
					case 3: // AI Channel Test Command Request
					{
						/*
							{
								"CMD": 3,
								"CMDState": 3,
								"DeviceID": "abcd1234-12",
								"slotNo": 1,
								"AI_Channel": 1,
								"AI_Type": 1, // current : 1 voltage : 2
								"AI_Point": 1, // 1 2 3 4
								"AI_Value": 2,
							}
						 */
						const cJSON *AI_Channel = NULL;
						const cJSON *AI_Point = NULL;

						if (cJSON_HasObjectItem(Received_json, "AI_Channel"))
						{
							AI_Channel = cJSON_GetObjectItemCaseSensitive(Received_json, "AI_Channel");
						}
						if (cJSON_HasObjectItem(Received_json, "AI_Point"))
						{
							AI_Point = cJSON_GetObjectItemCaseSensitive(Received_json, "AI_Point");
						}

						gAI_Point = AI_Point->valueint;

						if(com_mode == TCP)
						{
							flagTCP_AI_Channel_Test_Result = AI_Channel->valueint;
						}
						else if(com_mode == MQTT)
						{
							flagMQTT_AI_Channel_Test_Result = AI_Channel->valueint;
						}

						break;
					}
					default:
					{


					}
				}
			}
			break;
		}
		case CMD_SET_RTC: 	//6: //Set_RTC
		{
			const cJSON *date = NULL;
			const cJSON *month = NULL;
			const cJSON *year = NULL;
			const cJSON *hour = NULL;
			const cJSON *min = NULL;
			const cJSON *sec = NULL;

			date = cJSON_GetObjectItemCaseSensitive(Received_json, "DD");
			rtc.date = (unsigned char)date->valueint;
			month = cJSON_GetObjectItemCaseSensitive(Received_json, "MM");
			rtc.month = (unsigned char)month->valueint;
			year = cJSON_GetObjectItemCaseSensitive(Received_json, "YY");
			rtc.year = (unsigned char)year->valueint;
			hour = cJSON_GetObjectItemCaseSensitive(Received_json, "HH");
			rtc.hour = (unsigned char)hour->valueint;
			min = cJSON_GetObjectItemCaseSensitive(Received_json, "MN");
			rtc.min = (unsigned char)min->valueint;
			sec = cJSON_GetObjectItemCaseSensitive(Received_json, "SS");
			rtc.sec = (unsigned char)sec->valueint;
			//TODO : SET RTC here
			break;
		}
        default :
        {
			sprintf(print1,"current_cmd %d not OTA and Config ",current_cmd);
			WriteLog(1,(const char*)print,1);
        }
        break;

    }


    const cJSON *request = NULL;
    request = cJSON_GetObjectItemCaseSensitive(Received_json, "request");
    if( cJSON_IsString(request))
    {
        /*
         * 	DO ON through web-scanet in Manual Mode
         *
    		Request:
    		{"request": "set_do_key_status","request_parameters": {"do_key_data": {"pin-no": 1,"value": 1}}}
    		Response:
    		NA

    		DO OFF through web-scanet in Manual Mode

    		Request:
    		{"request": "set_do_key_status","request_parameters": {"do_key_data": {"pin-no": 1,"value": 0}}}
    		Response:
    		NA
         */
    	if(strncmp(request->valuestring,"set_do_key_status",strlen("set_do_key_status"))==0)
    	{
    	    const cJSON *request_parameters = NULL;
    	    request_parameters = cJSON_GetObjectItemCaseSensitive(Received_json, "request_parameters");
    	    if( cJSON_IsObject(request_parameters))
    	    {
        	    const cJSON *do_key_data = NULL;
        	    do_key_data = cJSON_GetObjectItemCaseSensitive(request_parameters, "do_key_data");
        	    if( cJSON_IsObject(do_key_data))
        	    {
            	    const cJSON *pinNo = NULL;
            	    pinNo = cJSON_GetObjectItemCaseSensitive(do_key_data, "pin-no");

            	    const cJSON *pinValue = NULL;
            	    pinValue = cJSON_GetObjectItemCaseSensitive(do_key_data, "value");
            	    EPROM_General.DoModeDetails.DO_Value[pinNo->valueint-1] = pinValue->valueint;
            	    flag_flashUpdateEPROM_General = 1;
            	    flag_flashUpdateEPROM_General_WaitCounter=5;
        	    }
    	    }
    	}
    	/*
    	===================================================================
    	Set variable from webscanet
    	===================================================================
    	• Request: (Topic : v1/devices/ClientID/GroupID/RTUID  Example : v1/devices/2/3/15)

    	{"request": "set_variable","request_parameters": {"variable_data": {"data-no": 1,"value": 50}}}

    	• Response:

    	NA
		*/
    	if(strncmp(request->valuestring,"set_variable",strlen("set_variable"))==0)
    	{
    	    const cJSON *request_parameters = NULL;
    	    request_parameters = cJSON_GetObjectItemCaseSensitive(Received_json, "request_parameters");
    	    if( cJSON_IsObject(request_parameters))
    	    {
        	    const cJSON *variable_data = NULL;
        	    variable_data = cJSON_GetObjectItemCaseSensitive(request_parameters, "variable_data");
        	    if( cJSON_IsObject(variable_data))
        	    {
            	    const cJSON *dataNo = NULL;
            	    dataNo = cJSON_GetObjectItemCaseSensitive(variable_data, "data-no");

            	    const cJSON *dataValue = NULL;
            	    dataValue = cJSON_GetObjectItemCaseSensitive(variable_data, "value");
            	    EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[dataNo->valueint-1].Pulse_DI_Flow_Configured = dataValue->valueint;
            	    flag_flashUpdateEPROM_General = 1;
            	    flag_flashUpdateEPROM_General_WaitCounter=5;
        	    }
    	    }
    	}
    	else if(strncmp(request->valuestring,"set_schedule",strlen("set_schedule"))==0)
    	{
    		/*
    		 * {"request": "set_schedule",
    		 * 	"request_parameters":
    		 * 		{ "schedule_num": 1,
    		 * 			"schedule":
    		 * 			[
    		 * 				{"enable": 1,"start_hour": 1,"start_minute": 59,"stop_hour": 2,"stop_minute": 0},
    		 * 				{"enable": 1,"start_hour": 1,"start_minute": 59,"stop_hour": 2,"stop_minute": 0}
    		 * 			]
    		 * 		}
    		 * 	}
    		 *
    		 */
    	    const cJSON *request_parameters = NULL;
    	    request_parameters = cJSON_GetObjectItemCaseSensitive(Received_json, "request_parameters");
    	    if( cJSON_IsObject(request_parameters))
    	    {
        	    const cJSON *schedule_num = NULL;
        	    schedule_num = cJSON_GetObjectItemCaseSensitive(request_parameters, "schedule_num");

        	    const cJSON *schedule = NULL;
        	    schedule = cJSON_GetObjectItemCaseSensitive(request_parameters, "schedule");

        	    if( cJSON_IsArray(schedule))
        	    {
					const cJSON *schedule_element = NULL;
					const cJSON *sch_enable = NULL;
        	    	const cJSON *sch_start_hour = NULL;
        	    	const cJSON *sch_start_minute = NULL;
        	    	const cJSON *sch_stop_hour = NULL;
        	    	const cJSON *sch_stop_minute = NULL;

        	    	int n = cJSON_GetArraySize(schedule);
        	    	for (unsigned char i = 0; i < n; i++)
        	    	{
        	    		schedule_element = cJSON_GetArrayItem(schedule, i);
        	    		sch_enable = cJSON_GetObjectItem(schedule_element, "enable");
        	    		sch_start_hour = cJSON_GetObjectItem(schedule_element, "start_hour");
        	    		sch_start_minute = cJSON_GetObjectItem(schedule_element, "start_minute");
        	    		sch_stop_hour = cJSON_GetObjectItem(schedule_element, "stop_hour");
        	    		sch_stop_minute = cJSON_GetObjectItem(schedule_element, "stop_minute");

        	    		EPROM_Schedule.Schedule[((schedule_num->valueint)-1)*10+(i)].Sch_En_Di = sch_enable->valueint;
        	    		gFinalAnaValF[SCHEDULE_gFinalAnaValF + ((schedule_num->valueint)-1)*50 + (i*5) + 0 ] = sch_enable->valueint;
        	    		EPROM_Schedule.Schedule[((schedule_num->valueint)-1)*10+(i)].Start_HH = sch_start_hour->valueint;
        	    		gFinalAnaValF[SCHEDULE_gFinalAnaValF + ((schedule_num->valueint)-1)*50 + (i*5) + 1 ] = sch_start_hour->valueint;
        	    		EPROM_Schedule.Schedule[((schedule_num->valueint)-1)*10+(i)].Start_Min = sch_start_minute->valueint;
        	    		gFinalAnaValF[SCHEDULE_gFinalAnaValF + ((schedule_num->valueint)-1)*50 + (i*5) + 2 ] = sch_start_minute->valueint;
        	    		EPROM_Schedule.Schedule[((schedule_num->valueint)-1)*10+(i)].Stop_HH = sch_stop_hour->valueint;
        	    		gFinalAnaValF[SCHEDULE_gFinalAnaValF + ((schedule_num->valueint)-1)*50 + (i*5) + 3 ] = sch_stop_hour->valueint;
        	    		EPROM_Schedule.Schedule[((schedule_num->valueint)-1)*10+(i)].Stop_Min = sch_stop_minute->valueint;
        	    		gFinalAnaValF[SCHEDULE_gFinalAnaValF + ((schedule_num->valueint)-1)*50 + (i*5) + 4 ] = sch_stop_minute->valueint;

        	    	}
        	    }
        	    flag_flashUpdateEPROM_Schedule=1;
        	    flag_flashUpdateEPROM_Schedule_WaitCounter=5;
    	    }
    	}
    	else if(strncmp(request->valuestring,"get_schedule",strlen("get_schedule"))==0)
    	{
    		/*
				Request:
				{"request": "get_schedule","request_parameters": {"schedule_num": 1}}

				Response:
				{"client_id": 5, "group_id": 5, "rtu_id": 5, "schedule_num": 1,
				 "schedule":
					 [
						 {"enable": 1,"start_hour": 1,"start_minute": 59,"stop_hour": 2,"stop_minute": 0},
						 {"enable": 1,"start_hour": 1,"start_minute": 59,"stop_hour": 2,"stop_minute": 0}
					 ]
				 }

    		 */
    	    const cJSON *request_parameters = NULL;
    	    request_parameters = cJSON_GetObjectItemCaseSensitive(Received_json, "request_parameters");
    	    if( cJSON_IsObject(request_parameters))
    	    {
        	    const cJSON *schedule_num = NULL;
        	    schedule_num = cJSON_GetObjectItemCaseSensitive(request_parameters, "schedule_num");
        	    if(com_mode == MQTT)
        	    {
        	    	flagMQTTPubSchedule = 1;
        	    	pubScheduleBlock =  schedule_num->valueint;
        	    }
        	    // TODO : Set flag to send schedule json based on  schedule_num
    	    }
    	}
    	else if(strncmp(request->valuestring,"get_mode",strlen("get_mode"))==0)
    	{
    		/*
				Get PCBPLC service mode

				Request:
				{"request": "get_mode","request_parameters": "all"}

				Response:
				{"client_id": 5, "group_id": 5, "rtu_id": 5, "pcbplc_mode": 0}

    		 */
    	    const cJSON *request_parameters = NULL;
    	    request_parameters = cJSON_GetObjectItemCaseSensitive(Received_json, "request_parameters");
    	    if( cJSON_IsString(request_parameters))
    	    {
    	    	if(strncmp(request_parameters->valuestring,"all",strlen("all"))==0)
        	    {
            	    if(com_mode == MQTT)
            	    {
            	    	flagMQTTPubGetMode = 1;
            	    }
        	    }

    	    }
    	}
    	else if(strncmp(request->valuestring,"set_mode",strlen("set_mode"))==0)
    	{
    		/*
				Set PCBPLC service mode

				Request:
				{"request": "set_mode","request_parameters": {"pcbplc_mode": 0}}

				Response: NA

    		 */
    	    const cJSON *request_parameters = NULL;
    	    request_parameters = cJSON_GetObjectItemCaseSensitive(Received_json, "request_parameters");
    	    if( cJSON_IsObject(request_parameters))
    	    {
        	    const cJSON *pcbplc_mode = NULL;
        	    pcbplc_mode = cJSON_GetObjectItemCaseSensitive(request_parameters, "pcbplc_mode");
        	    // TODO : Set mode here
            	EPROM_General.DoModeDetails.Do_Mode = pcbplc_mode->valueint;
                gpcbplcCnfg.mRtuDoMode = pcbplc_mode->valueint;
                flag_flashUpdateEPROM_General = 1;
                flag_flashUpdateEPROM_General_WaitCounter=5;
    	    }
    	}
    	else if(strncmp(request->valuestring,"soft_reboot",strlen("soft_reboot"))==0)
    	{
    		/*
				Set Reboot service mode

				Request:
				{"request": "soft_reboot","request_parameters": {"reboot_value": 1}}

				Response: NA

    		 */
    	    const cJSON *request_parameters = NULL;
    	    request_parameters = cJSON_GetObjectItemCaseSensitive(Received_json, "request_parameters");
    	    if( cJSON_IsObject(request_parameters))
    	    {
        	    const cJSON *reboot_value = NULL;
        	    reboot_value = cJSON_GetObjectItemCaseSensitive(request_parameters, "reboot_value");
        	    // TODO : Set mode here
//            	EPROM_General.DoModeDetails.Do_Mode = pcbplc_mode->valueint;
//                gpcbplcCnfg.mRtuDoMode = pcbplc_mode->valueint;
//                flag_flashUpdateEPROM_General = 1;
//                flag_flashUpdateEPROM_General_WaitCounter=5;
        	    gFinalAnaValF[DEVICE_REBOOT_gFinalAnaValF] = reboot_value->valueint;
        	    if(gFinalAnaValF[DEVICE_REBOOT_gFinalAnaValF] == 1)
        	    {
        	    	reboot_device_func();
        	    }
    	    }
    	}
    	else if(strncmp(request->valuestring,"astro_offset",strlen("astro_offset"))==0)
    	{
    		/*
				Set PCBPLC service mode

				Request:
				{"request": "astro_offset","request_parameters": {"offset_value": -10}}

				Response: NA

    		 */
    	    const cJSON *request_parameters = NULL;
    	    request_parameters = cJSON_GetObjectItemCaseSensitive(Received_json, "request_parameters");
    	    if( cJSON_IsObject(request_parameters))
    	    {
        	    const cJSON *offset_value = NULL;
        	    offset_value = cJSON_GetObjectItemCaseSensitive(request_parameters, "offset_value");
        	    // TODO : Set mode here
//            	EPROM_General.DoModeDetails.Do_Mode = pcbplc_mode->valueint;
//                gpcbplcCnfg.mRtuDoMode = pcbplc_mode->valueint;
//                flag_flashUpdateEPROM_General = 1;
//                flag_flashUpdateEPROM_General_WaitCounter=5;
        	    gFinalAnaValF[ASTRO_OFFSET_gFinalAnaValF] = offset_value->valueint;
    	    }
    	}

    }

	if((flag_flashUpdateEPROM_Modbus_Quary_Detail == 1)||(flag_flashUpdateEPROM_Schedule == 1) || (flag_flashUpdateEPROM_General == 1))
	{
		syncExtFlashVariableWithPCBPLCVariable();
	}
    //end:
        cJSON_Delete(Received_json);
        return ret;
}

JSON_ERROR_RESPONSE Config_response_ACK_JSON_frame(COM_TYPE com_mode , CMD_TYPE CMD, char cmdState, char file_CMD_type, RES_ACK iACK, char * ACK_Response)
{
	cJSON *response_json_Obejct = NULL;
	JSON_ERROR_RESPONSE ret;
	ret = JSON_SUCCESS;
	response_json_Obejct = cJSON_CreateObject();
	if (response_json_Obejct == NULL)
	{
		ret = FAILED_CREATE_JSON_OBJECT;
	}


	if(!cJSON_AddNumberToObject(response_json_Obejct, "CMD", CMD))
	{
		ret = FAILED_ADD_JSON_OBJECT;
	}

	if(!cJSON_AddNumberToObject(response_json_Obejct, "CMDState", cmdState))//OTA_ACK_Data.CMDState))
	{
		ret = FAILED_ADD_JSON_OBJECT;
	}
	switch(CMD)
	{
		case CMD_OTA:
		{

			if(!cJSON_AddNumberToObject(response_json_Obejct, "FileType",file_CMD_type))
			{
				ret = FAILED_ADD_JSON_OBJECT;
			}

			if(OTA_ACK_Data.Chunk_no != 0)
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Chunk_no", OTA_ACK_Data.Chunk_no))
				{
					ret = FAILED_ADD_JSON_OBJECT;
				}
			}
		}
		break;
		case CMD_CONF:
		{
			if(!cJSON_AddNumberToObject(response_json_Obejct, "CMDType",file_CMD_type))
			{
				ret = FAILED_ADD_JSON_OBJECT;
			}
		}
		break;
		default:
		{

		}
		break;
	}
	if(!cJSON_AddNumberToObject(response_json_Obejct, "ACK", iACK))
	{
		ret = FAILED_ADD_JSON_OBJECT;
	}


	switch(com_mode)
	{
		case MQTT:
		{
	//			if(!(cJSON_PrintPreallocated(response_json_Obejct, pub_Rest_buff,sizeof(pub_Rest_buff),false)))
	//			{
	//				qt_uart_dbg(uart_conf.hdlr,"[MQTT] [JSON] [OTA_STATUS] Error at cJSON_PrintPreallocated");
	//			}
	//
	//			if(response_json_Obejct)
	//			{
	//				cJSON_Delete(response_json_Obejct);
	//				response_json_Obejct = NULL;
	//			}
	//			ret = mqtt_cli_publish((uint8 *)tPubTopic, (uint8 *)pub_Rest_buff, qos, retain);
	//			if(ret != QAPI_OK)
	//			{
	//				qt_uart_dbg(uart_conf.hdlr,"[MQTT] [JSON] [OTA_STATUS] Error at mqtt_cli_publish (ret: %d)", ret);
	//			}
	//			else
	//			{
	//				qt_uart_dbg(uart_conf.hdlr,"[MQTT] [JSON] [OTA_STATUS] published Data(%d) %s",ret , pub_Rest_buff);
	//			}
	//			return ret ;
		}
		break;
		case TCP:
		{
			memset(msg_payload,0x00,sizeof(msg_payload));
			if(!(cJSON_PrintPreallocated(response_json_Obejct, msg_payload,sizeof(msg_payload),0)))//false
			{
				ret = FAILED_TRANSFER_STRING_FROM_JSON_OBJECT;
			}
			strcpy(ACK_Response,msg_payload);

			send_OTA_status = 1;
			if(response_json_Obejct)
			{
				cJSON_Delete(response_json_Obejct);
				response_json_Obejct = NULL;
			}
			memset(msg_payload,0x00,sizeof(msg_payload));
		}
		break;
		case UART:
				{
					memset(msg_payload,0x00,sizeof(msg_payload));
					if(!(cJSON_PrintPreallocated(response_json_Obejct, msg_payload,sizeof(msg_payload),0)))//false
					{
						ret = FAILED_TRANSFER_STRING_FROM_JSON_OBJECT;
					}
					strcpy(ACK_Response,msg_payload);

					send_OTA_status = 1;
					if(response_json_Obejct)
					{
						cJSON_Delete(response_json_Obejct);
						response_json_Obejct = NULL;
					}
					memset(msg_payload,0x00,sizeof(msg_payload));
				}
				break;

		default:
		{

		}
		break;
	}
	return ret;
}

JSON_ERROR_RESPONSE response_ACK_JSON_frame(COM_TYPE com_mode ,CMD_TYPE CMD, OTA_FILE_ACK iACK,char * ACK_Response)
{
	cJSON *response_json_Obejct = NULL;
	JSON_ERROR_RESPONSE ret;
	ret = JSON_SUCCESS;
	response_json_Obejct = cJSON_CreateObject();
	if (response_json_Obejct == NULL)
	{
		ret = FAILED_CREATE_JSON_OBJECT;
	}

	switch(CMD)
	{
		case CMD_OTA:
		{

			if(!cJSON_AddNumberToObject(response_json_Obejct, "CMD", CMD))
			{
				ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "CMDState", OTA_ACK_Data.CMDState))
			{
				ret = FAILED_ADD_JSON_OBJECT;
			}

			if(OTA_ACK_Data.Chunk_no != 0)
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Chunk_no", OTA_ACK_Data.Chunk_no))
				{
					ret = FAILED_ADD_JSON_OBJECT;
				}
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "ACK", iACK))
			{
				ret = FAILED_ADD_JSON_OBJECT;
			}
		}
		break;
		case CMD_CONF:
		{

		}
		break;
		default:
		{

		}
		break;
	}

	switch(com_mode)
	{
		case MQTT:
		{
	//			if(!(cJSON_PrintPreallocated(response_json_Obejct, pub_Rest_buff,sizeof(pub_Rest_buff),false)))
	//			{
	//				qt_uart_dbg(uart_conf.hdlr,"[MQTT] [JSON] [OTA_STATUS] Error at cJSON_PrintPreallocated");
	//			}
	//
	//			if(response_json_Obejct)
	//			{
	//				cJSON_Delete(response_json_Obejct);
	//				response_json_Obejct = NULL;
	//			}
	//			ret = mqtt_cli_publish((uint8 *)tPubTopic, (uint8 *)pub_Rest_buff, qos, retain);
	//			if(ret != QAPI_OK)
	//			{
	//				qt_uart_dbg(uart_conf.hdlr,"[MQTT] [JSON] [OTA_STATUS] Error at mqtt_cli_publish (ret: %d)", ret);
	//			}
	//			else
	//			{
	//				qt_uart_dbg(uart_conf.hdlr,"[MQTT] [JSON] [OTA_STATUS] published Data(%d) %s",ret , pub_Rest_buff);
	//			}
	//			return ret ;
		}
		break;
		case TCP:
		{
			memset(msg_payload,0x00,sizeof(msg_payload));
			if(!(cJSON_PrintPreallocated(response_json_Obejct, msg_payload,sizeof(msg_payload),0)))//false
			{
				ret = FAILED_TRANSFER_STRING_FROM_JSON_OBJECT;
			}
			strcpy(ACK_Response,msg_payload);

			send_OTA_status = 1;
			if(response_json_Obejct)
			{
				cJSON_Delete(response_json_Obejct);
				response_json_Obejct = NULL;
			}
			memset(msg_payload,0x00,sizeof(msg_payload));
		}
		break;
		case UART:
				{
					memset(msg_payload,0x00,sizeof(msg_payload));
					if(!(cJSON_PrintPreallocated(response_json_Obejct, msg_payload,sizeof(msg_payload),0)))//false
					{
						ret = FAILED_TRANSFER_STRING_FROM_JSON_OBJECT;
					}
					strcpy(ACK_Response,msg_payload);

					send_OTA_status = 1;
					if(response_json_Obejct)
					{
						cJSON_Delete(response_json_Obejct);
						response_json_Obejct = NULL;
					}
					memset(msg_payload,0x00,sizeof(msg_payload));
				}
				break;

		default:
		{

		}
		break;
	}
	return ret;
}

//unsigned int
/**************************************************************************//**
 * Function name 	: buildLograteDataJson
 * arguments		: 1)
 * 		 			:
 * return			:
 * Note				:
 * 					:	{"identifier": "1D", "client_id": 5, "group_id": 5, "rtu_id": 5, "date": "19052022", "time": "112700",
 *						"mode": 1, "di_status": [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1], "ai_tag":
 *						[0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0] }
 *****************************************************************************/

//float gFinalAnaValF_temp[350];
//unsigned char DI_Final_value_temp[16];

unsigned char buildLograteDataJson(unsigned char sendPort)
{
	unsigned char result=1;//,max_DI=16;
	unsigned int i_index=0;

//	for(i_index=0;i_index<350;i_index++)
//	{
//		gFinalAnaValF_temp[i_index] = i_index*1.5;
//	}
//
//	for(i_index=0;i_index<16;i_index++)
//	{
//		if(i_index%2)
//		{
//			DI_Final_value_temp[i_index] = 0;
//		}
//		else
//		{
//			DI_Final_value_temp[i_index] = 1;
//		}
//
//	}

	cJSON *json_Full_Obejct;
	json_Full_Obejct = cJSON_CreateObject();
	cJSON *DI_Arrry;
	cJSON *AI_Tag_Arrry;

	cJSON_AddStringToObject(json_Full_Obejct, "identifier", "1D");
	cJSON_AddNumberToObject(json_Full_Obejct, "client_id", EPROM_General.Cust_Detail.Client_Id);
	cJSON_AddNumberToObject(json_Full_Obejct, "group_id", EPROM_General.Cust_Detail.Reader_Id);
	cJSON_AddNumberToObject(json_Full_Obejct, "rtu_id", EPROM_General.Rtu_Detail.RTUId);

	sprintf((char*)rtcdate, "%02d%02d%02d",gDate.Date, gDate.Month, 2000 + gDate.Year);
	sprintf((char*)rtctime, "%02d%02d%02d",gTime.Hours, gTime.Minutes, gTime.Seconds);

	cJSON_AddStringToObject(json_Full_Obejct, "date", (const char*)&rtcdate);
	cJSON_AddStringToObject(json_Full_Obejct, "time", (const char*)&rtctime);

	cJSON_AddNumberToObject(json_Full_Obejct, "mode", EPROM_General.DoModeDetails.Do_Mode);

	DI_Arrry = cJSON_CreateArray();
	for(i_index=0;i_index<EPROM_General.AI_DI_DO_Detail.Total_Di;i_index++)
	{
	  //cJSON_AddItemToArray(DI_Arrry, cJSON_CreateNumber(dig_bit_array1[i_index]));
	  cJSON_AddItemToArray(DI_Arrry, cJSON_CreateNumber(dig_bit_array[i_index]));
	}
	cJSON_AddItemToObject(json_Full_Obejct, "di_status", DI_Arrry);

	AI_Tag_Arrry = cJSON_CreateArray();
	for(i_index=0;i_index<EPROM_Modbus_Quary_Detail.mMaxDataTagEnabled;i_index++)
	{
		//gFinalAnaValF[700] = 1234567890987654321012332.1654;
	  cJSON_AddItemToArray(AI_Tag_Arrry, cJSON_CreateNumber(gFinalAnaValF[GENERAL_PURPOSE_AI_gFinalAnaValF+i_index]));
	}
	cJSON_AddItemToObject(json_Full_Obejct, "ai_tag", AI_Tag_Arrry);

	if(sendPort == 1)//MQTT_PORT)
	{
		memset(MqttPubBuf, 0x00, CIM_MAX_SIZE_OF_MQTT_PAYLOAD);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&MqttPubBuf,CIM_MAX_SIZE_OF_MQTT_PAYLOAD,false)))
		{
			result=0;
		}
	}
	else if((sendPort == 2))
	{
		memset(tcp_ResponseBuffer, 0x00, 1500);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&tcp_ResponseBuffer,1500,false)))
		{
			result=0;
		}
	}
	else
	{
		result=0;
	}

	cJSON_Delete(json_Full_Obejct);

	return result;
}
/**************************************************************************//**
 * Function name 	: buildGetScheduleJson
 * arguments		: 1)
 * 		 			:
 * return			:
 * Note				:
 * 					:	Response:
 *						{"client_id": 5, "group_id": 5, "rtu_id": 5, "schedule_num": 1,
 *						 "schedule":
 *	 						 [
 *								 {"enable": 1,"start_hour": 1,"start_minute": 59,"stop_hour": 2,"stop_minute": 0},
 *								 {"enable": 1,"start_hour": 1,"start_minute": 59,"stop_hour": 2,"stop_minute": 0}
 *							 ]
 *						 }
 *****************************************************************************/

unsigned char buildGetScheduleJson(unsigned char sendPort,unsigned scheduleBlock)
{
	unsigned char result=1,i_index=0;

	cJSON *json_Full_Obejct;
	json_Full_Obejct = cJSON_CreateObject();
	cJSON *schedule_Array;
	cJSON *schedule_Array_Element;

	cJSON_AddNumberToObject(json_Full_Obejct, "client_id", EPROM_General.Cust_Detail.Client_Id);
	cJSON_AddNumberToObject(json_Full_Obejct, "group_id", EPROM_General.Cust_Detail.Reader_Id);
	cJSON_AddNumberToObject(json_Full_Obejct, "rtu_id", EPROM_General.Rtu_Detail.RTUId);
	cJSON_AddNumberToObject(json_Full_Obejct, "schedule_num", scheduleBlock);

	schedule_Array = cJSON_CreateArray();

	for(i_index = 0;i_index<10;i_index++)
	{
		schedule_Array_Element = cJSON_CreateObject();
	    if (schedule_Array_Element == NULL)
	    {
	    	result=0;
	        goto end;
	    }
		cJSON_AddNumberToObject(schedule_Array_Element, "enable", EPROM_Schedule.Schedule[((scheduleBlock-1)*10)+i_index].Sch_En_Di);
		cJSON_AddNumberToObject(schedule_Array_Element, "start_hour", EPROM_Schedule.Schedule[((scheduleBlock-1)*10)+i_index].Start_HH);
		cJSON_AddNumberToObject(schedule_Array_Element, "start_minute", EPROM_Schedule.Schedule[((scheduleBlock-1)*10)+i_index].Start_Min);
		cJSON_AddNumberToObject(schedule_Array_Element, "stop_hour", EPROM_Schedule.Schedule[((scheduleBlock-1)*10)+i_index].Stop_HH);
		cJSON_AddNumberToObject(schedule_Array_Element, "stop_minute", EPROM_Schedule.Schedule[((scheduleBlock-1)*10)+i_index].Stop_Min);
		cJSON_AddItemToArray(schedule_Array, schedule_Array_Element);
	}
	cJSON_AddItemToObject(json_Full_Obejct, "schedule", schedule_Array);

	if(sendPort == 1)//MQTT_PORT)
	{
		memset(MqttPubBuf, 0x00, CIM_MAX_SIZE_OF_MQTT_PAYLOAD);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&MqttPubBuf,CIM_MAX_SIZE_OF_MQTT_PAYLOAD,false)))
		{
			result=0;
		}
	}
	else if((sendPort == 2))
	{
		memset(tcp_ResponseBuffer, 0x00, 1500);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&tcp_ResponseBuffer,1500,false)))
		{
			result=0;
		}
	}
	else
	{
		result=0;
	}

	end:

	cJSON_Delete(json_Full_Obejct);

	return result;
}


/**************************************************************************//**
 * Function name 	: buildGetModeResponseJson
 * arguments		: 1)
 * 		 			:
 * return			:
 * Note				: Response:
 * 					  {"client_id": 5, "group_id": 5, "rtu_id": 5, "pcbplc_mode": 0}
 *****************************************************************************/

unsigned char buildGetModeResponseJson(unsigned char sendPort)
{
	unsigned char result=1;

	cJSON *json_Full_Obejct;
	json_Full_Obejct = cJSON_CreateObject();

	cJSON_AddNumberToObject(json_Full_Obejct, "client_id", EPROM_General.Cust_Detail.Client_Id);
	cJSON_AddNumberToObject(json_Full_Obejct, "group_id", EPROM_General.Cust_Detail.Reader_Id);
	cJSON_AddNumberToObject(json_Full_Obejct, "rtu_id", EPROM_General.Rtu_Detail.RTUId);
	cJSON_AddNumberToObject(json_Full_Obejct, "pcbplc_mode", EPROM_General.DoModeDetails.Do_Mode);

	if(sendPort == 1)//MQTT_PORT)
	{
		memset(MqttPubBuf, 0x00, CIM_MAX_SIZE_OF_MQTT_PAYLOAD);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&MqttPubBuf,CIM_MAX_SIZE_OF_MQTT_PAYLOAD,false)))
		{
			result=0;
		}
	}
	else if((sendPort == 2))
	{
		memset(tcp_ResponseBuffer, 0x00, 1500);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&tcp_ResponseBuffer,1500,false)))
		{
			result=0;
		}
	}
	else
	{
		result=0;
	}

	cJSON_Delete(json_Full_Obejct);

	return result;
}

/**************************************************************************//**
 * Function name 	: buildProIdFrameJson
 * arguments		: 1)
 * 		 			:
 * return			:
 * Note				: Response:
 * 					  {
							"CMD": 2,
							"CMDState": 1,
							"DeviceID": "abcd1234-12",
							"slotNo": 1,
							"FW_Version": "x.x.x",
							"LoraDevEUI":"0123456789abcdef",
							"EthenetMAC":"ff:ef:aa:00:00:01",
							"ST_ID": "0123456789abcd"
					  }
 * After Power Cycle
 * 					  {
							"CMD": 2,
							"CMDState": 4,
							"DeviceID": "abcd1234-12",
							"slotNo": 1,
							"FW_Version": "x.x.x",
							"LoraDevEUI":"0123456789abcdef",
							"EthenetMAC":"ff:ef:aa:00:00:01",
							"ST_ID": "0123456789abcd"
					  }
 * After watchdog reset in second cycle
 * 					  {
							"CMD": 3,
							"CMDState": 1,
							"DeviceID": "abcd1234-12",
							"slotNo": 1,
							"FW_Version": "x.x.x",
							"LoraDevEUI":"0123456789abcdef",
							"EthenetMAC":"ff:ef:aa:00:00:01",
							"ST_ID": "0123456789abcd"
					  }
 *****************************************************************************/

unsigned char buildProIdFrameJson(unsigned char sendPort,unsigned char afterPowerCycle)
{
	unsigned char result=1,tempBuff[100];

	cJSON *json_Full_Obejct;
	json_Full_Obejct = cJSON_CreateObject();


	if(afterPowerCycle == 0)
	{
		cJSON_AddNumberToObject(json_Full_Obejct, "CMD",2);
		cJSON_AddNumberToObject(json_Full_Obejct, "CMDState",1);
	}
	else if(afterPowerCycle == 1)
	{
		cJSON_AddNumberToObject(json_Full_Obejct, "CMD",2);
		cJSON_AddNumberToObject(json_Full_Obejct, "CMDState",4);
	}
	else if(afterPowerCycle == 2)
	{
		cJSON_AddNumberToObject(json_Full_Obejct, "CMD",3);
		cJSON_AddNumberToObject(json_Full_Obejct, "CMDState",1);
	}
	cJSON_AddStringToObject(json_Full_Obejct, "DeviceID", (const char *)EPROM_General.DeviceID);
	cJSON_AddNumberToObject(json_Full_Obejct, "slotNo",SlotNo);
	cJSON_AddStringToObject(json_Full_Obejct, "FW_Version", DEFAULT_FV_VERSION);//EPROM_General.Rtu_Detail.Hex_Version);
	cJSON_AddStringToObject(json_Full_Obejct, "LoraDevEUI", (const char *)EPROM_LoRa_Modem.lora_dev_eui_set);

	memset(tempBuff,0,sizeof(tempBuff));
	sprintf((char *)tempBuff,"%02X:%02X:%02X:%02X:%02X:%02X",ethernetMac[5],ethernetMac[4],ethernetMac[3],ethernetMac[2],ethernetMac[1],ethernetMac[0]);
	cJSON_AddStringToObject(json_Full_Obejct, "EthenetMAC", (const char *)tempBuff);

	memset(tempBuff,0,sizeof(tempBuff));
	sprintf((char *)tempBuff,"%04X%04X%04X%04X%04X%04X",(unsigned int)(stm32deviceID[2]<<16),(unsigned int)stm32deviceID[2],(unsigned int)stm32deviceID[1]<<16,
														(unsigned int)stm32deviceID[1],(unsigned int)(stm32deviceID[0])<<16,(unsigned int)(stm32deviceID[0]));
	cJSON_AddStringToObject(json_Full_Obejct, "ST_ID", (const char *)tempBuff);

	if(sendPort == 1)//MQTT_PORT)
	{
		memset(MqttPubBuf, 0x00, CIM_MAX_SIZE_OF_MQTT_PAYLOAD);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&MqttPubBuf,CIM_MAX_SIZE_OF_MQTT_PAYLOAD,false)))
		{
			result=0;
		}
	}
	else if((sendPort == 2))
	{
		memset(tcp_ResponseBuffer, 0x00, 1500);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&tcp_ResponseBuffer,1500,false)))
		{
			result=0;
		}
	}
	else
	{
		result=0;
	}

	cJSON_Delete(json_Full_Obejct);
	return result;

}

/**************************************************************************//**
 * Function name 	: buildProIdFrameJson
 * arguments		: 1)
 * 		 			:
 * return			:
 * Note				: Response:
 * TestMethod 1
 * 					  {
							"CMD": 2,
							"CMDState": 2,
							"DeviceID": "abcd1234-12",
							"slotNo": 1,
							"ACK":1,
					  }

 * TestMethod 2
 * 					  {
							"CMD": 2,
							"CMDState": 5,
							"rebootCount" : 5,
							"DeviceID": "abcd1234-12",
							"slotNo": 1,
							"ACK":1,
					  }
 *****************************************************************************/

unsigned char buildTestMethodAckJson(unsigned char sendPort,unsigned char TestMethod)
{
	unsigned char result=1;

	cJSON *json_Full_Obejct;
	json_Full_Obejct = cJSON_CreateObject();

	cJSON_AddNumberToObject(json_Full_Obejct, "CMD",2);
	if(TestMethod == 1)
	{
		cJSON_AddNumberToObject(json_Full_Obejct, "CMDState",2);
	}
	else
	{
		cJSON_AddNumberToObject(json_Full_Obejct, "CMDState",5);
		cJSON_AddNumberToObject(json_Full_Obejct, "rebootCount",EPROM_General.rebootCount);
	}
	cJSON_AddStringToObject(json_Full_Obejct, "DeviceID", (const char *)EPROM_General.DeviceID);
	cJSON_AddNumberToObject(json_Full_Obejct, "slotNo",SlotNo);
	cJSON_AddNumberToObject(json_Full_Obejct, "ACK",1);

	if(sendPort == 1)//MQTT_PORT)
	{
		memset(MqttPubBuf, 0x00, CIM_MAX_SIZE_OF_MQTT_PAYLOAD);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&MqttPubBuf,CIM_MAX_SIZE_OF_MQTT_PAYLOAD,false)))
		{
			result=0;
		}
	}
	else if((sendPort == 2))
	{
		memset(tcp_ResponseBuffer, 0x00, 1500);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&tcp_ResponseBuffer,1500,false)))
		{
			result=0;
		}
	}
	else
	{
		result=0;
	}

	cJSON_Delete(json_Full_Obejct);
	return result;
}

/**************************************************************************//**
 * Function name 	: buildProIdFrameJson
 * arguments		: 1)
 * 		 			:
 * return			:
 * Note				: Response:
 * TestMethod 1
 					{
						"CMD": 2,
						"CMDState": 3,
						"DeviceID": "abcd1234-12",
						"slotNo": 5,
						"DOResult": "10111111111111111111111111",
						"DIResult": "10111111",
						"LORAPhysical_State": 1,
						"LORA_JOIN_State": 1,
						"LORA_RSSI": -81,
						"FW_Version": "x.x.x",
						"LCD_I2C_State": 1
					}

 * TestMethod 2
					{
						"CMD": 2,
						"CMDState": 6,
						"DeviceID": "abcd1234-12",
						"slotNo": 5,
						"DD": 9,
						"MM": 2,
						"YY": 2023,
						"HH": 10,
						"MN": 37,
						"SS": 50,
						"Flash_State":1,
						"rebootCount" : 5
					}

 *****************************************************************************/

unsigned char buildTestMethodResultJson(unsigned char sendPort,unsigned char TestMethod,unsigned char TM_slot)
{
	unsigned char result=1,tempBuff[100];

	cJSON *json_Full_Obejct;
	json_Full_Obejct = cJSON_CreateObject();

	cJSON_AddNumberToObject(json_Full_Obejct, "CMD",2);

	if(TestMethod == 1)
	{
		cJSON_AddNumberToObject(json_Full_Obejct, "CMDState",3);
		cJSON_AddStringToObject(json_Full_Obejct, "DeviceID", (const char *)EPROM_General.DeviceID);
		cJSON_AddNumberToObject(json_Full_Obejct, "slotNo",SlotNo);
		memset(tempBuff,0,sizeof(tempBuff));
		sprintf((char*)tempBuff,"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",pro_DO_State[0],pro_DO_State[1],pro_DO_State[2],pro_DO_State[3],pro_DO_State[4]
														,pro_DO_State[5],pro_DO_State[6],pro_DO_State[7],pro_DO_State[8],pro_DO_State[8],pro_DO_State[10]
                                                        ,pro_DO_State[11],pro_DO_State[12],pro_DO_State[13],pro_DO_State[14],pro_DO_State[15],pro_DO_State[16]
														,pro_DO_State[17],pro_DO_State[18],pro_DO_State[19],pro_DO_State[20],pro_DO_State[21],pro_DO_State[22],pro_DO_State[23]
													    ,pro_DO_State[24],pro_DO_State[25]);
		cJSON_AddStringToObject(json_Full_Obejct, "DOResult", (const char *)tempBuff);

		memset(tempBuff,0,sizeof(tempBuff));
		sprintf((char*)tempBuff,"%d%d%d%d%d%d%d%d",pro_DI_State[0],pro_DI_State[1],pro_DI_State[2],pro_DI_State[3],pro_DI_State[4],pro_DI_State[5],pro_DI_State[6],pro_DI_State[7]);
		cJSON_AddStringToObject(json_Full_Obejct, "DIResult", (const char *)tempBuff);

		if(lora_AT_ok_check == 1)
		{
			cJSON_AddNumberToObject(json_Full_Obejct, "LORAPhysical_State",1);
		}
		else
		{
			cJSON_AddNumberToObject(json_Full_Obejct, "LORAPhysical_State",0);
		}

		cJSON_AddNumberToObject(json_Full_Obejct, "LORA_JOIN_State",LoRa_Modem.lora_network_join_state);
		cJSON_AddNumberToObject(json_Full_Obejct, "LORA_RSSI",LoRa_Modem.lora_RSSI);

		cJSON_AddStringToObject(json_Full_Obejct, "FW_Version", ( const char * )DEFAULT_FV_VERSION);//EPROM_General.Rtu_Detail.Hex_Version);

		//cJSON_AddNumberToObject(json_Full_Obejct, "LCD_I2C_State",pro_I2C_State);

	}
	else
	{
		cJSON_AddNumberToObject(json_Full_Obejct, "CMDState",6);
		cJSON_AddStringToObject(json_Full_Obejct, "DeviceID", (const char *)EPROM_General.DeviceID);
		cJSON_AddNumberToObject(json_Full_Obejct, "slotNo",SlotNo);
	//	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BCD);
	//	HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BCD);
	//	get_time(t1);
		print_time();
		cJSON_AddNumberToObject(json_Full_Obejct, "DD",gDate.Date);
		cJSON_AddNumberToObject(json_Full_Obejct, "MM",gDate.Month);
		cJSON_AddNumberToObject(json_Full_Obejct, "YY",gDate.Year+2000);
		cJSON_AddNumberToObject(json_Full_Obejct, "HH",gTime.Hours);
		cJSON_AddNumberToObject(json_Full_Obejct, "MN",gTime.Minutes);
		cJSON_AddNumberToObject(json_Full_Obejct, "SS",gTime.Seconds);
		cJSON_AddNumberToObject(json_Full_Obejct, "Flash_State",pro_Flash_State);
		cJSON_AddNumberToObject(json_Full_Obejct, "rebootCount",EPROM_General.rebootCount);
		cJSON_AddNumberToObject(json_Full_Obejct, "LCD_I2C_State",pro_I2C_State);

	}

	if(sendPort == 1)//MQTT_PORT)
	{
		memset(MqttPubBuf, 0x00, CIM_MAX_SIZE_OF_MQTT_PAYLOAD);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&MqttPubBuf,CIM_MAX_SIZE_OF_MQTT_PAYLOAD,false)))
		{
			result=0;
		}
	}
	else if((sendPort == 2) && TM_slot== 1)
	{
		memset(tcp_ResponseBuffer, 0x00, 1500);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&tcp_ResponseBuffer,1500,false)))
		{
			result=0;
		}

	}
	else if((sendPort == 2) && TM_slot == 2)
		{
			memset(tcp_ResponseBuffer, 0x00, 1500);
			if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&tcp_ResponseBuffer,1500,false)))
			{
				result=0;
			}

			EPROM_General.slot_id=0;
			strcpy((char *)EPROM_LoRa_Modem.lora_app_eui_set,DEFAULT_LORA_APPEID);
			strcpy((char *)EPROM_LoRa_Modem.lora_app_key_set,DEFAULT_LORA_APPKEY);
			ExtFlash_update_EPROM_General();
			ExtFlash_update_EPROM_LORA();
			key_LED_flag =1;
		}

	else
	{
		result=0;
	}
	cJSON_Delete(json_Full_Obejct);
	return result;

}

/**************************************************************************//**
 * Function name 	: buildProIdFrameJson
 * arguments		: 1)
 * 		 			:
 * return			:
 * Note				: Response:
 * AI Channel Calibration Command Response
						{
							"CMD": 3,
							"CMDState": 2,
							"DeviceID": "abcd1234-12",
							"slotNo": 1,
							"AI_Channel": 1,
							"AI_Type": 1, // current : 1 voltage : 2
							"AI_Point": 1, // 1 2 3 4
							"AI_Value": 1,
							"AI_Cal_Vaule" : 345566
						}

 * AI Channel Test Command Response

						{
							"CMD": 3,
							"CMDState": 3,
							"DeviceID": "abcd1234-12",
							"slotNo": 1,
							"AI_Channel": 1,
							"AI_Type": 1, // current : 1 voltage : 2
							"AI_Point": 1, // 1 2 3 4
							"AI_Value": 2.2,
						}
 *****************************************************************************/

unsigned char buildAICaliJson(unsigned char sendPort,unsigned char ack_0_Respomse_1,unsigned int channel)
{
	unsigned char result=1;

	cJSON *json_Full_Obejct;
	json_Full_Obejct = cJSON_CreateObject();

	cJSON_AddNumberToObject(json_Full_Obejct, "CMD",3);
	if(ack_0_Respomse_1 == 0)
	{
		cJSON_AddNumberToObject(json_Full_Obejct, "CMDState",2);
	}
	else if(ack_0_Respomse_1 == 1)
	{
		cJSON_AddNumberToObject(json_Full_Obejct, "CMDState",3);
	}
	cJSON_AddStringToObject(json_Full_Obejct, "DeviceID", (const char *)EPROM_General.DeviceID);
	cJSON_AddNumberToObject(json_Full_Obejct, "slotNo",SlotNo);
	cJSON_AddNumberToObject(json_Full_Obejct, "AI_Channel",channel+1);
	cJSON_AddNumberToObject(json_Full_Obejct, "AI_Type",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].AI_ch_Type);
	if(EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].AI_ch_Type == V0TO20)
	{
		if(gAI_Point == 4)
		{
			cJSON_AddNumberToObject(json_Full_Obejct, "AI_Point",4);
			if(ack_0_Respomse_1 == 0)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiHighCal_V_Point);
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Cal_Vaule",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiHighCal_V);
			}
			else if(ack_0_Respomse_1 == 1)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",AO_VAL_float[channel]);
			}
		}
		else if(gAI_Point == 3)
		{
			cJSON_AddNumberToObject(json_Full_Obejct, "AI_Point",3);
			if(ack_0_Respomse_1 == 0)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiMidCal_V_Point);
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Cal_Vaule",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiMidCal_V);
			}
			else if(ack_0_Respomse_1 == 1)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",AO_VAL_float[channel]);
			}
		}
		else if(gAI_Point == 2)
		{
			cJSON_AddNumberToObject(json_Full_Obejct, "AI_Point",2);
			if(ack_0_Respomse_1 == 0)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiLowMidCal_V_Point);
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Cal_Vaule",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiLowMidCal_V);
			}
			else if(ack_0_Respomse_1 == 1)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",AO_VAL_float[channel]);
			}
		}
		else if(gAI_Point == 1)
		{
			cJSON_AddNumberToObject(json_Full_Obejct, "AI_Point",1);
			if(ack_0_Respomse_1 == 0)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiLowCal_V_Point);
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Cal_Vaule",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiLowCal_V);
			}
			else if(ack_0_Respomse_1 == 1)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",AO_VAL_float[channel]);
			}
		}
	}
	else
	{
		if(gAI_Point == 4)
		{
			cJSON_AddNumberToObject(json_Full_Obejct, "AI_Point",4);
			if(ack_0_Respomse_1 == 0)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiHighCal_mA_Point);
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Cal_Vaule",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiHighCal_mA);
			}
			else if(ack_0_Respomse_1 == 1)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",AO_VAL_float[channel]);
			}
		}
		else if(gAI_Point == 3)
		{
			cJSON_AddNumberToObject(json_Full_Obejct, "AI_Point",3);
			if(ack_0_Respomse_1 == 0)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiMidCal_mA_Point);
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Cal_Vaule",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiMidCal_mA);
			}
			else if(ack_0_Respomse_1 == 1)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",AO_VAL_float[channel]);
			}
		}
		else if(gAI_Point == 2)
		{
			cJSON_AddNumberToObject(json_Full_Obejct, "AI_Point",2);
			if(ack_0_Respomse_1 == 0)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiLowMidCal_mA_Point);
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Cal_Vaule",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiLowMidCal_mA);
			}
			else if(ack_0_Respomse_1 == 1)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",AO_VAL_float[channel]);
			}
		}
		else if(gAI_Point == 1)
		{
			cJSON_AddNumberToObject(json_Full_Obejct, "AI_Point",1);
			if(ack_0_Respomse_1 == 0)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiLowCal_mA_Point);
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Cal_Vaule",EPROM_General.AI_DI_DO_Detail.AI_Detail[channel].mAiLowCal_mA);
			}
			else if(ack_0_Respomse_1 == 1)
			{
				cJSON_AddNumberToObject(json_Full_Obejct, "AI_Value",AO_VAL_float[channel]);
			}
		}
	}
	if(sendPort == 1)//MQTT_PORT)
	{
		memset(MqttPubBuf, 0x00, CIM_MAX_SIZE_OF_MQTT_PAYLOAD);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&MqttPubBuf,CIM_MAX_SIZE_OF_MQTT_PAYLOAD,false)))
		{
			result=0;
		}
	}
	else if((sendPort == 2))
	{
		memset(tcp_ResponseBuffer, 0x00, 1500);
		if(!(cJSON_PrintPreallocated(json_Full_Obejct,(char *)&tcp_ResponseBuffer,1500,false)))
		{
			result=0;
		}
	}
	else
	{
		result=0;
	}

	cJSON_Delete(json_Full_Obejct);
	return result;
}
