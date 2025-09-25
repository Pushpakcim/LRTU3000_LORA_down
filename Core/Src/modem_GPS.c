/*
 * modem_GPS.c
 *
 *  Created on: Dec 12, 2022
 *      Author: maulin
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "modem_GPS.h"

/**************************************************************************//**
 * Variable
 *****************************************************************************/

unsigned int count_Modem_GPS = 0 , Modem_GPS_check_sec = 0;
modem_gps_loc_command_t gps;
osThreadId Modem_GPS_TaskHandle;

/**************************************************************************//**
 * Function name 	: Modem_GPS_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/

void Modem_GPS_start()
{
	osThreadDef(Modem_GPSTask, StartModem_GPSTask, osPriorityBelowNormal, 0, 512); //512
	Modem_GPS_TaskHandle = osThreadCreate(osThread(Modem_GPSTask), NULL);
}

/**************************************************************************//**
 * Function name 	: StartModem_GPSTask
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

void StartModem_GPSTask(void const * argument)
{
	osDelay(10000);
	for(;;)
	{
//		count_Modem_GPS++;
//		Modem_GPS_check_sec++;
//
//		if(Modem_GPS_check_sec > 20)
//		{
//			Modem_GPS_check_sec = 0;
//			check_GPS_data();
//
//			gFinalAnaValF[GPS_LAT_gFinalAnaValF]= gps.latitude;
//			gFinalAnaValF[GPS_Log_gFinalAnaValF]= gps.longitude;
//			gFinalAnaValF[GPS_NO_OF_SATALITE_gFinalAnaValF]= gps.noOfSatellites;
//			gFinalAnaValF[GPS_DATE_gFinalAnaValF]= gps.utc.date;
//			gFinalAnaValF[GPS_MONTH_gFinalAnaValF]= gps.utc.month;
//			gFinalAnaValF[GPS_YEAR_gFinalAnaValF]= gps.utc.year;
//			gFinalAnaValF[GPS_HOUR_gFinalAnaValF]= gps.utc.hour;
//			gFinalAnaValF[GPS_MIN_gFinalAnaValF]= gps.utc.min;
//			gFinalAnaValF[GPS_SEC_gFinalAnaValF]= gps.utc.sec;
//			gFinalAnaValF[GPS_ALTITUDE_gFinalAnaValF]= gps.altitude;
//			gFinalAnaValF[GPS_3DFIX_gFinalAnaValF]= gps.fix;
//		}
//		if(Pro_Application_flag == 1)
//		{
//			osDelay(1000);
//		}
//		else
		{
			osDelay(5000);
		}


	}
}

/**************************************************************************//**
 * Function name 	: check_GPS_data
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char check_GPS_data()
{
	unsigned char res=0;

	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_SEQUEANCE_MQTT_PUBLISH;
		if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
		{
			Modem_AT_Command = LWGSM_CMD_GPS_QGPS_GET;
			lwgsmi_initiate_cmd(Modem_AT_Command,0);
			if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
			{
				if(gps.GNSS_state == 0)
				{
					Modem_AT_Command = LWGSM_CMD_GPS_QGPS_SET;
					lwgsmi_initiate_cmd(Modem_AT_Command,0);
					if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
					{
						xSemaphoreGive(modem_PortBlockSemaphore);
					}
				}

				Modem_AT_Command = LWGSM_CMD_GPS_QGPSLOC;
				lwgsmi_initiate_cmd(Modem_AT_Command,0);
				if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
				{
					xSemaphoreGive(modem_PortBlockSemaphore);
				}
			}
		}
		Modem_AT_Command = LWGSM_CMD_IDLE;
		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(modem_SequenceBlockSemaphore);
	}

	return res;
}
