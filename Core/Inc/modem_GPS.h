/*
 * modem_GPS.h
 *
 *  Created on: Dec 12, 2022
 *      Author: maulin
 */

#ifndef INC_MODEM_GPS_H_
#define INC_MODEM_GPS_H_

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "ATmodemTypes.h"

/**************************************************************************//**
 * Constant
 *****************************************************************************/


/**************************************************************************//**
 * Ennam / uninon / Structure
 *****************************************************************************/
typedef struct
{
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
	unsigned char miliSec;
	unsigned char date;
	unsigned char month;
	unsigned int year;
}
modem_time_t;

typedef struct
{
	modem_time_t utc;
	float latitude;
	float longitude;
	float HDOP;
	float altitude;
	unsigned char fix;
	float COG;
	float speedkm;
	float speedkn;
	unsigned char noOfSatellites;

	unsigned char GNSS_state;
}
modem_gps_loc_command_t;



/**************************************************************************//**
 * Macro
 *****************************************************************************/


/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/

extern unsigned int count_Modem_GPS, Modem_GPS_check_sec;
extern modem_gps_loc_command_t gps;
extern osThreadId Modem_MQTT_TaskHandle;

/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/

void StartModem_GPSTask(void const * argument);
void Modem_GPS_start();

unsigned char check_GPS_data();
#endif /* INC_MODEM_GPS_H_ */
