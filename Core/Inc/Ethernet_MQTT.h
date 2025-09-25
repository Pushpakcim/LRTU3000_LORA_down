/*
 * Ethernet_MQTT.h
 *
 *  Created on: Aug 10, 2023
 *      Author: SanketP
 */

#ifndef INC_ETHERNET_MQTT_H_
#define INC_ETHERNET_MQTT_H_


/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "ATmodemTypes.h"

/**************************************************************************//**
 * Constant
 *****************************************************************************/
extern unsigned int count_Ethernet_MQTT,ethernet_MQTT_publish_success_timer;

/**************************************************************************//**
 * Ennam / uninon / Structure
 *****************************************************************************/

extern osThreadId Ethernet_MQTT_TaskHandle;
extern unsigned char flag_EThernet_MQTT_Reconnect;
/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/


void StartEthernet_MQTTTask(void const * argument);
void Ethernet_MQTT_start();
#endif /* INC_ETHERNET_MQTT_H_ */
