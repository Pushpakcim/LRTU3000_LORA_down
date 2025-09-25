/*
 * modem_BLE.c
 *
 *  Created on: Dec 13, 2022
 *      Author: maulin
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "modem_BLE.h"
#include "lwip.h"
/**************************************************************************//**
 * Variable
 *****************************************************************************/

unsigned int count_Modem_BLE = 0 , Modem_BLE_check_sec = 0 ;

modem_BLE_t ble;
modem_BLE_Characteristic_t gCharacteristic[TOTAL_CHARACTERISTIC];
modem_BLE_Service_t gService[TOTAL_SERVICES];

flag_ChangeRequired ble_para_changRequired={0};

osThreadId Modem_BLE_TaskHandle;

/**************************************************************************//**
 * Function name 	: Modem_GPS_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/

void Modem_BLE_start()
{
	osThreadDef(Modem_BLETask, StartModem_BLETask, osPriorityNormal, 0, 512); //512
	Modem_BLE_TaskHandle = osThreadCreate(osThread(Modem_BLETask), NULL);
}

/**************************************************************************//**
 * Function name 	: StartModem_GPSTask
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char bleStart = 1;

void StartModem_BLETask(void const * argument)
{
	osDelay(5000);
	float tDistance=0;
	double pi = 3.14285714;
	modem_ble_make_list_of_Service_and_Characteristic();

	for(;;)
	{
		//count_Modem_BLE++;
		count_Modem_BLE=0;
		Modem_BLE_check_sec++;



		if(Modem_BLE_check_sec > 20)
		{
			Modem_BLE_check_sec = 0;
			init_modem_BLE();
		}

		if((ble.updateCharacteristicValueOnConnect==1)&&(ble.connectionStatus==1))
		{
			ble.updateCharacteristicValueOnConnect = 0;
			checkForBleCharacteristicValueUpdateRequired(1);
		}
		else if(ble.connectionStatus==1)
		{
			checkForBleCharacteristicValueUpdateRequired(0);
		}
		gFinalAnaValF[BLE_CONEECTION_STATE_gFinalAnaValF] = ble.connectionStatus;
		Modem_GPS_check_sec++;

		if((Modem_GPS_check_sec > 120)||((Pro_Application_flag==1)&&(Modem_GPS_check_sec>20)))
		{
			Modem_GPS_check_sec = 0;
			check_GPS_data();

			if(gps.fix)
			{
				gFinalAnaValF[GPS_LAT_gFinalAnaValF]= gps.latitude;
				gFinalAnaValF[GPS_Log_gFinalAnaValF]= gps.longitude;
				gFinalAnaValF[GPS_NO_OF_SATALITE_gFinalAnaValF]= gps.noOfSatellites;
				//gFinalAnaValF[GPS_NO_OF_SATALITE_gFinalAnaValF]= EPROM_General.rebootCount;
				gFinalAnaValF[GPS_DATE_gFinalAnaValF]= gps.utc.date;
				gFinalAnaValF[GPS_MONTH_gFinalAnaValF]= gps.utc.month;
				gFinalAnaValF[GPS_YEAR_gFinalAnaValF]= gps.utc.year;
				gFinalAnaValF[GPS_HOUR_gFinalAnaValF]= gps.utc.hour;
				gFinalAnaValF[GPS_MIN_gFinalAnaValF]= gps.utc.min;
				gFinalAnaValF[GPS_SEC_gFinalAnaValF]= gps.utc.sec;
				gFinalAnaValF[GPS_ALTITUDE_gFinalAnaValF]= gps.altitude;
				gFinalAnaValF[GPS_3DFIX_gFinalAnaValF]= gps.fix;

				tDistance = acos(sin(pi*EPROM_General.Cust_Detail.Lattitude/180)*sin(pi*gps.latitude/180)+cos(pi*EPROM_General.Cust_Detail.Lattitude/180)*cos(pi*gps.latitude/180)*cos(pi*EPROM_General.Cust_Detail.Longitude/180-pi*gps.longitude/180))*6378;
				if(( (tDistance*1000) > 50) )
				{
					EPROM_General.Cust_Detail.Lattitude = gps.latitude;
					EPROM_General.Cust_Detail.Longitude = gps.longitude;
					flag_flashUpdateEPROM_General = 1;
					flag_flashUpdateEPROM_General_WaitCounter = 5;
				}
			}
		}

		osDelay(1000);
	}
}

/**************************************************************************//**
 * Function name 	: init_modem_BLE
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char init_modem_BLE()
{
	unsigned char res=0,i=0;

	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_SEQUEANCE_BLE_INT;
		if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
		{
			if(bleStart == 1)
			{
				bleStart = 0;
				Modem_AT_Command = LWGSM_CMD_BLE_QBTPWR_OFF;
				lwgsmi_initiate_cmd(Modem_AT_Command,0);
				if(xSemaphoreTake(modem_PortBlockSemaphore, 3000))
				{
					xSemaphoreGive(modem_PortBlockSemaphore);
				}
				else
				{
					xSemaphoreGive(modem_PortBlockSemaphore);
				}
				osDelay(3000);
			}

			Modem_AT_Command = LWGSM_CMD_BLE_QBTPWR_GET;
			lwgsmi_initiate_cmd(Modem_AT_Command,0);
			if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
			{
				if(ble.powerStauts == 0)
				{
					Modem_AT_Command = LWGSM_CMD_BLE_QBTPWR_ON;
					lwgsmi_initiate_cmd(Modem_AT_Command,0);
					if(xSemaphoreTake(modem_PortBlockSemaphore, 3000))
					{
						// TODO : check ok or error
						Modem_AT_Command = LWGSM_CMD_BLE_QBTLEADDR_GET;
						lwgsmi_initiate_cmd(Modem_AT_Command,&ble);
						if(xSemaphoreTake(modem_PortBlockSemaphore, 3000))
						{
							Modem_AT_Command = LWGSM_CMD_BLE_QBTNAME_SET;
							lwgsmi_initiate_cmd(Modem_AT_Command,&ble);
							if(xSemaphoreTake(modem_PortBlockSemaphore, 3000))
							{
								Modem_AT_Command = LWGSM_CMD_BLE_QBTGATADV;
								lwgsmi_initiate_cmd(Modem_AT_Command,&ble);
								if(xSemaphoreTake(modem_PortBlockSemaphore, 3000))
								{
									Modem_AT_Command = LWGSM_CMD_BLE_QBTADVDATA;
									lwgsmi_initiate_cmd(Modem_AT_Command,&ble);
									if(xSemaphoreTake(modem_PortBlockSemaphore, 3000))
									{
										Modem_AT_Command = LWGSM_CMD_BLE_QBTADVSTR;
										lwgsmi_initiate_cmd(Modem_AT_Command,&ble);

										if(xSemaphoreTake(modem_PortBlockSemaphore, 3000))
										{
											for(i=0;i<TOTAL_SERVICES;i++)
											{
												ble.Service = &gService[i];

												Modem_AT_Command = LWGSM_CMD_BLE_QBTGATSS;
												lwgsmi_initiate_cmd(Modem_AT_Command,&ble);
												if((xSemaphoreTake(modem_PortBlockSemaphore, 3000))==0)
												{
													xSemaphoreGive(modem_PortBlockSemaphore);
													xSemaphoreTake(modem_PortBlockSemaphore, 3000);
												}
											}
											xSemaphoreGive(modem_PortBlockSemaphore);

											for(i=0;i<TOTAL_CHARACTERISTIC;i++)
											{
												ble.Characteristic = &gCharacteristic[i];

												Modem_AT_Command = LWGSM_CMD_BLE_QBTGATSC;
												lwgsmi_initiate_cmd(Modem_AT_Command,&ble);
												if((xSemaphoreTake(modem_PortBlockSemaphore, 3000))==0)
												{
													xSemaphoreGive(modem_PortBlockSemaphore);
													xSemaphoreTake(modem_PortBlockSemaphore, 3000);
												}

												Modem_AT_Command = LWGSM_CMD_BLE_QBTGATSCV;
												modem_ble_make_value_String_for_Characteristic(&ble);
												lwgsmi_initiate_cmd(Modem_AT_Command,&ble);
												if((xSemaphoreTake(modem_PortBlockSemaphore, 3000))==0)
												{
													xSemaphoreGive(modem_PortBlockSemaphore);
													xSemaphoreTake(modem_PortBlockSemaphore, 3000);
												}

												Modem_AT_Command = LWGSM_CMD_BLE_QBTGATSCD;
												modem_ble_make_descriptor_String_for_Characteristic(&ble);
												lwgsmi_initiate_cmd(Modem_AT_Command,&ble);
												if((xSemaphoreTake(modem_PortBlockSemaphore, 3000))==0)
												{
													xSemaphoreGive(modem_PortBlockSemaphore);
													xSemaphoreTake(modem_PortBlockSemaphore, 3000);
												}
											}

											xSemaphoreGive(modem_PortBlockSemaphore);
											if(xSemaphoreTake(modem_PortBlockSemaphore, 3000))
											{
												Modem_AT_Command = LWGSM_CMD_BLE_QBTGATSSC;
												lwgsmi_initiate_cmd(Modem_AT_Command,0);
												if(xSemaphoreTake(modem_PortBlockSemaphore, 3000))
												{
													Modem_AT_Command = LWGSM_CMD_BLE_QBTADV;
													lwgsmi_initiate_cmd(Modem_AT_Command,0);
													if(xSemaphoreTake(modem_PortBlockSemaphore, 3000))
													{
														xSemaphoreGive(modem_SequenceBlockSemaphore);
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
				else
				{

				}
			}
		}
		Modem_AT_Command = LWGSM_CMD_IDLE;
		xSemaphoreGive(modem_PortBlockSemaphore);
		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(modem_SequenceBlockSemaphore);
	}

	return res;
}
/**************************************************************************//**
 * Function name 	: init_modem_BLE
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

void modem_ble_make_list_of_Service_and_Characteristic()
{
	////////////////////////////////  Service 0 ////////////////////////////////////////

	//==================================================================================

	gService[0].servID = 0;
	gService[0].UUID_type = 0;
	strcpy((char *)gService[0].serv_UUID_l,SERVICE_0_UUID_NAME);
	gService[0].serviceType = 1;

	//==================================================================================
	// Characteristic in Service 0
	//==================================================================================

	gCharacteristic[0].servID = 0;
	gCharacteristic[0].charaID = 0;
	strcpy((char *)gCharacteristic[0].chara_UUID_l,SERV_0_CHARA_0_UUID_NAME);
	gCharacteristic[0].property = 58;
	gCharacteristic[0].permission = 3;
	gCharacteristic[0].att_handle = 18;

	gCharacteristic[1].servID = 0;
	gCharacteristic[1].charaID = 1;
	strcpy((char *)gCharacteristic[1].chara_UUID_l,SERV_0_CHARA_1_UUID_NAME);
	gCharacteristic[1].property = 58;
	gCharacteristic[1].permission = 3;
	gCharacteristic[1].att_handle = 21;

	gCharacteristic[2].servID = 0;
	gCharacteristic[2].charaID = 2;
	strcpy((char *)gCharacteristic[2].chara_UUID_l,SERV_0_CHARA_2_UUID_NAME);
	gCharacteristic[2].property = 58;
	gCharacteristic[2].permission = 3;
	gCharacteristic[2].att_handle = 24;

	gCharacteristic[3].servID = 0;
	gCharacteristic[3].charaID = 3;
	strcpy((char *)gCharacteristic[3].chara_UUID_l,SERV_0_CHARA_3_UUID_NAME);
	gCharacteristic[3].property = 58;
	gCharacteristic[3].permission = 3;
	gCharacteristic[3].att_handle = 27;

	gCharacteristic[4].servID = 0;
	gCharacteristic[4].charaID = 4;
	strcpy((char *)gCharacteristic[4].chara_UUID_l,SERV_0_CHARA_4_UUID_NAME);
	gCharacteristic[4].property = 58;
	gCharacteristic[4].permission = 3;
	gCharacteristic[4].att_handle = 30;

	gCharacteristic[5].servID = 0;
	gCharacteristic[5].charaID = 5;
	strcpy((char *)gCharacteristic[5].chara_UUID_l,SERV_0_CHARA_5_UUID_NAME);
	gCharacteristic[5].property = 58;
	gCharacteristic[5].permission = 3;
	gCharacteristic[5].att_handle = 33;

	gCharacteristic[6].servID = 0;
	gCharacteristic[6].charaID = 6;
	strcpy((char *)gCharacteristic[6].chara_UUID_l,SERV_0_CHARA_6_UUID_NAME);
	gCharacteristic[6].property = 58;
	gCharacteristic[6].permission = 3;
	gCharacteristic[6].att_handle = 36;

	gCharacteristic[7].servID = 0;
	gCharacteristic[7].charaID = 7;
	strcpy((char *)gCharacteristic[7].chara_UUID_l,SERV_0_CHARA_7_UUID_NAME);
	gCharacteristic[7].property = 58;
	gCharacteristic[7].permission = 3;
	gCharacteristic[7].att_handle = 39;

	gCharacteristic[8].servID = 0;
	gCharacteristic[8].charaID = 8;
	strcpy((char *)gCharacteristic[8].chara_UUID_l,SERV_0_CHARA_8_UUID_NAME);
	gCharacteristic[8].property = 58;
	gCharacteristic[8].permission = 3;
	gCharacteristic[8].att_handle = 42;

	gCharacteristic[9].servID = 0;
	gCharacteristic[9].charaID = 9;
	strcpy((char *)gCharacteristic[9].chara_UUID_l,SERV_0_CHARA_9_UUID_NAME);
	gCharacteristic[9].property = 58;
	gCharacteristic[9].permission = 3;
	gCharacteristic[9].att_handle = 45;


	////////////////////////////////  Service 1 ////////////////////////////////////////

	//==================================================================================

//	gService[1].servID = 1;
//	gService[1].UUID_type = 0;
//	strcpy((char *)gService[0].serv_UUID_l,SERVICE_1_UUID_NAME);
//	gService[1].serviceType = 1;

	//==================================================================================
	// Characteristic in Service 1
	//==================================================================================

//	gCharacteristic[x].servID = 1;
//	gCharacteristic[x].charaID = 0;
//	strcpy((char *)gCharacteristic[x].chara_UUID_l,SERV_1_CHARA_0_UUID_NAME);
//	gCharacteristic[x].property = 58;
//	gCharacteristic[x].permission = 3;
//	gCharacteristic[x].att_handle = 0;

	////////////////////////////////  Service 2 ////////////////////////////////////////
}
/**************************************************************************//**
 * Function name 	: init_modem_BLE
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

int variable0=0,variable1=0,variable2=0,variable3=0,variable4=0,variable5=0,variable6=0,variable7=0,variable8=0,variable9=0;

void modem_ble_make_value_String_for_Characteristic(modem_BLE_t * _ble)
{
	char _Str[50],i;

	switch (_ble->Characteristic->servID)
	{
	    case 0:
	    {
	    	switch (_ble->Characteristic->charaID)
	    	{
	    	    case 0:
	    	    {

	    	    	lwgsm_u32_to_str(variable0, _Str);
	    	    	lwgsm_str_to_asciiStr((const char *)_Str,strlen((const char *)_Str),(const char * )_ble->msg,sizeof(_ble->msg));
	    	    	_ble->msgLen = strlen((const char *)_ble->msg)/2;
	    	    	for(i=0;i<(50-_ble->msgLen);i++)
	    	    	{
	    	    		strcat((char *)_ble->msg,"00");
	    	    	}
	    	    	break;
	    	    }
	    	    case 1:
	    	    {
	    	    	lwgsm_u32_to_str(variable0, _Str);
	    	    	lwgsm_str_to_asciiStr((const char *)_Str,strlen((const char *)_Str),(const char * )_ble->msg,sizeof(_ble->msg));
	    	    	_ble->msgLen = strlen((const char *)_ble->msg)/2;
	    	    	for(i=0;i<(50-_ble->msgLen);i++)
	    	    	{
	    	    		strcat((char *)_ble->msg,"00");
	    	    	}
	    	    	break;
	    	    }
	    	    case 2:
	    	    {
	    	    	lwgsm_u32_to_str(variable2, _Str);
	    	    	lwgsm_str_to_asciiStr((const char *)_Str,strlen((const char *)_Str),(const char * )_ble->msg,sizeof(_ble->msg));
	    	    	_ble->msgLen = strlen((const char *)_ble->msg)/2;
	    	    	for(i=0;i<(50-_ble->msgLen);i++)
	    	    	{
	    	    		strcat((char *)_ble->msg,"00");
	    	    	}
	    	    	break;
	    	    }
	    	    case 3:
	    	    {
	    	    	lwgsm_u32_to_str(variable3, _Str);
	    	    	lwgsm_str_to_asciiStr((const char *)_Str,strlen((const char *)_Str),(const char * )_ble->msg,sizeof(_ble->msg));
	    	    	_ble->msgLen = strlen((const char *)_ble->msg)/2;
	    	    	for(i=0;i<(50-_ble->msgLen);i++)
	    	    	{
	    	    		strcat((char *)_ble->msg,"00");
	    	    	}
	    	    	break;
	    	    }
	    	    case 4:
	    	    {
	    	    	lwgsm_u32_to_str(variable4, _Str);
	    	    	lwgsm_str_to_asciiStr((const char *)_Str,strlen((const char *)_Str),(const char * )_ble->msg,sizeof(_ble->msg));
	    	    	_ble->msgLen = strlen((const char *)_ble->msg)/2;
	    	    	for(i=0;i<(50-_ble->msgLen);i++)
	    	    	{
	    	    		strcat((char *)_ble->msg,"00");
	    	    	}
	    	    	break;
	    	    }
	    	    case 5:
	    	    {
	    	    	lwgsm_u32_to_str(variable5, _Str);
	    	    	lwgsm_str_to_asciiStr((const char *)_Str,strlen((const char *)_Str),(const char * )_ble->msg,sizeof(_ble->msg));
	    	    	_ble->msgLen = strlen((const char *)_ble->msg)/2;
	    	    	for(i=0;i<(50-_ble->msgLen);i++)
	    	    	{
	    	    		strcat((char *)_ble->msg,"00");
	    	    	}
	    	    	break;
	    	    }
	    	    case 6:
	    	    {
	    	    	lwgsm_u32_to_str(variable6, _Str);
	    	    	lwgsm_str_to_asciiStr((const char *)_Str,strlen((const char *)_Str),(const char * )_ble->msg,sizeof(_ble->msg));
	    	    	_ble->msgLen = strlen((const char *)_ble->msg)/2;
	    	    	for(i=0;i<(50-_ble->msgLen);i++)
	    	    	{
	    	    		strcat((char *)_ble->msg,"00");
	    	    	}
	    	    	break;
	    	    }
	    	    case 7:
	    	    {
	    	    	lwgsm_u32_to_str(variable7, _Str);
	    	    	lwgsm_str_to_asciiStr((const char *)_Str,strlen((const char *)_Str),(const char * )_ble->msg,sizeof(_ble->msg));
	    	    	_ble->msgLen = strlen((const char *)_ble->msg)/2;
	    	    	for(i=0;i<(50-_ble->msgLen);i++)
	    	    	{
	    	    		strcat((char *)_ble->msg,"00");
	    	    	}
	    	    	break;
	    	    }
	    	    case 8:
	    	    {
	    	    	lwgsm_u32_to_str(variable8, _Str);
	    	    	lwgsm_str_to_asciiStr((const char *)_Str,strlen((const char *)_Str),(const char * )_ble->msg,sizeof(_ble->msg));
	    	    	_ble->msgLen = strlen((const char *)_ble->msg)/2;
	    	    	for(i=0;i<(50-_ble->msgLen);i++)
	    	    	{
	    	    		strcat((char *)_ble->msg,"00");
	    	    	}
	    	    	break;
	    	    }
	    	    case 9:
	    	    {
	    	    	lwgsm_u32_to_str(variable9, _Str);
	    	    	lwgsm_str_to_asciiStr((const char *)_Str,strlen((const char *)_Str),(const char * )_ble->msg,sizeof(_ble->msg));
	    	    	_ble->msgLen = strlen((const char *)_ble->msg)/2;
	    	    	for(i=0;i<(50-_ble->msgLen);i++)
	    	    	{
	    	    		strcat((char *)_ble->msg,"00");
	    	    	}
	    	    	break;
	    	    }
	    	    default:
	    	    {

	    	    }
	    	}
	    	break;
	    }
	    default:
	    {

	    }
	}
}
/**************************************************************************//**
 * Function name 	: init_modem_BLE
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void modem_ble_make_descriptor_String_for_Characteristic(modem_BLE_t * _ble)
{
	char _Str[50];

	lwgsm_u32_to_str(1234, _Str);
	lwgsm_str_to_asciiStr((const char *)_Str,strlen((const char *)_Str),(const char * )_ble->msg,sizeof(_ble->msg));
	_ble->msgLen = strlen((const char *)_ble->msg)/2;
}

void checkForBleCharacteristicValueUpdateRequired(unsigned char updateAll)
{
	if((ble_para_changRequired.variable0)||(updateAll==1))
	{
		ble_para_changRequired.variable0 = 0;
		ble.Characteristic = &gCharacteristic[0];
		update_bleCharacteristicValue(&ble);
	}
	if((ble_para_changRequired.variable1)||(updateAll==1))
	{
		ble_para_changRequired.variable1 = 0;
		ble.Characteristic = &gCharacteristic[1];
		update_bleCharacteristicValue(&ble);
	}
	if((ble_para_changRequired.variable2)||(updateAll==1))
	{
		ble_para_changRequired.variable2 = 0;
		ble.Characteristic = &gCharacteristic[2];
		update_bleCharacteristicValue(&ble);
	}
	if((ble_para_changRequired.variable3)||(updateAll==1))
	{
		ble_para_changRequired.variable3 = 0;
		ble.Characteristic = &gCharacteristic[3];
		update_bleCharacteristicValue(&ble);
	}
	if((ble_para_changRequired.variable4)||(updateAll==1))
	{
		ble_para_changRequired.variable4 = 0;
		ble.Characteristic = &gCharacteristic[4];
		update_bleCharacteristicValue(&ble);
	}
}

unsigned char update_bleCharacteristicValue(modem_BLE_t * _ble)
{
	unsigned char res=0;
	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_SEQUEANCE_BLE_UPDATE;
		xSemaphoreGive(modem_PortBlockSemaphore);

		if((xSemaphoreTake(modem_PortBlockSemaphore, 3000)))
		{
			Modem_AT_Command = LWGSM_CMD_BLE_QBTGATCHSCV;
			modem_ble_make_value_String_for_Characteristic(_ble);
			lwgsmi_initiate_cmd(Modem_AT_Command,&ble);
			if((xSemaphoreTake(modem_PortBlockSemaphore, 3000)))
			{
				res = 1;
				xSemaphoreGive(modem_PortBlockSemaphore);
				xSemaphoreTake(modem_PortBlockSemaphore, 3000);
			}
			else
			{
				res = 0;
			}
		}
		Modem_AT_Command = LWGSM_CMD_IDLE;
		xSemaphoreGive(modem_PortBlockSemaphore);
		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(modem_SequenceBlockSemaphore);
	}
	return res;
}
