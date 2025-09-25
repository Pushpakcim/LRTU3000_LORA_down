/*
 * json_parser.h
 *
 *  Created on: Dec 15, 2022
 *      Author: maulin
 */

#ifndef INC_JSON_PARSER_H_
#define INC_JSON_PARSER_H_
/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "OTA.h"
#include "ATmodemTypes.h"
#include "cJSON.h"
#include "tcpserver.h"


/**************************************************************************//**
 * Constant
 *****************************************************************************/


/**************************************************************************//**
 * Ennam / uninon / Structure
 *****************************************************************************/
typedef enum
{
	JSON_SUCCESS = 0,
	FAILED_CREATE_JSON_OBJECT = -1,
	FAILED_ADD_JSON_OBJECT = -2,
	FAILED_TRANSFER_STRING_FROM_JSON_OBJECT = -3,
	FAILED_CREATE_ARRAY_JSON_OBJECT = -4,
}JSON_ERROR_RESPONSE;

typedef enum
{
	MQTT = 0,
	TCP,
	UART,
}COM_TYPE;

typedef enum
{
	CMD_OTA = 0, // OTA
	CMD_CONF = 1,
	CMD_PRODUCTION = 2,
	CMD_AI_CAL = 3,
	CMD_SET_RTC,
}CMD_TYPE;

typedef enum {
    ACK_FAIL = 0,
	ACK_SUCCESS,
} RES_ACK;

/**************************************************************************//**
 * extern
 *****************************************************************************/

extern unsigned char flagMQTTPubSchedule,flagMQTTPubGetMode,flagMqttPubLogData,flagMqttPubLiveData,
			flagMQTT_ID_First,flagMQTT_ID_afterPowerCycle,flagMQTT_TestMethod_1_ACK,flagMQTT_TestMethod_1_Result,
				flagMQTT_TestMethod_2_ACK,flagMQTT_TestMethod_2_Result,
				flagMQTT_ID_AI_CALI_App,flagMQTT_AI_Channel_CaliResponse,flagMQTT_AI_Channel_Test_Result;

extern unsigned char ProductionModeIDFrameReceived,flagTCP_ID_First,flagTCP_ID_afterPowerCycle,flagTCP_TestMethod_1_ACK,flagTCP_TestMethod_1_Result,
		flagTCP_TestMethod_2_ACK,flagTCP_TestMethod_2_Result,key_LED_flag,
		flagTCP_ID_AI_CALI_App,flagTCP_AI_Channel_CaliResponse,flagTCP_AI_Channel_Test_Result;

extern unsigned char flagLORAPubLogData,flagLORAPubLogData_fail;

extern unsigned char pubScheduleBlock;
extern unsigned char gAI_Point;
extern int gPlcFileLength;
extern int gRecFileLength;
extern unsigned long int ghexFileLength;
extern unsigned int OTA_START,OTA_resetCouner;
/**************************************************************************//**
 * function
 *****************************************************************************/

unsigned char check_MQTT_JSON_frame(char* jsonString);
unsigned char buildLograteDataJson(unsigned char sendPort);
unsigned char buildGetScheduleJson(unsigned char sendPort,unsigned scheduleBlock);
unsigned char buildGetModeResponseJson(unsigned char sendPort);
unsigned char buildProIdFrameJson(unsigned char sendPort,unsigned char afterPowerCycle);
unsigned char buildTestMethodAckJson(unsigned char sendPort,unsigned char TestMethod);
unsigned char buildTestMethodResultJson(unsigned char sendPort,unsigned char TestMethod,unsigned char TM_slot);
unsigned char buildAICaliJson(unsigned char sendPort,unsigned char ack_0_Respomse_1,unsigned int channel);
JSON_ERROR_RESPONSE parse_JSON_frame(COM_TYPE com_mode, char* jsonString, char * ACK_Response);
#endif /* INC_JSON_PARSER_H_ */
