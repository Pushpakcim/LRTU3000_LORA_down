/*
 * modem_BLE.h
 *
 *  Created on: Dec 13, 2022
 *      Author: maulin
 */

#ifndef INC_MODEM_BLE_H_
#define INC_MODEM_BLE_H_

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "ATmodemTypes.h"

/**************************************************************************//**
 * Constant
 *****************************************************************************/
#define TOTAL_SERVICES			1
#define SERVICE_0_UUID_NAME		"f5899b5f800000800010000000000100"

#define TOTAL_CHARACTERISTIC	10
#define SERV_0_CHARA_0_UUID_NAME		"f5899b5f800000800010000001000100"
#define SERV_0_CHARA_1_UUID_NAME		"f5899b5f800000800010000002000100"
#define SERV_0_CHARA_2_UUID_NAME		"f5899b5f800000800010000003000100"
#define SERV_0_CHARA_3_UUID_NAME		"f5899b5f800000800010000004000100"
#define SERV_0_CHARA_4_UUID_NAME		"f5899b5f800000800010000005000100"
#define SERV_0_CHARA_5_UUID_NAME		"f5899b5f800000800010000006000100"
#define SERV_0_CHARA_6_UUID_NAME		"f5899b5f800000800010000007000100"
#define SERV_0_CHARA_7_UUID_NAME		"f5899b5f800000800010000008000100"
#define SERV_0_CHARA_8_UUID_NAME		"f5899b5f800000800010000009000100"
#define SERV_0_CHARA_9_UUID_NAME		"f5899b5f800000800010000010000100"
//#define SERV_0_CHARA_10_UUID_NAME		"f5899b5f800000800010000011000100"
//#define SERV_0_CHARA_11_UUID_NAME		"f5899b5f800000800010000012000100"
//#define SERV_0_CHARA_12_UUID_NAME		"f5899b5f800000800010000013000100"
//#define SERV_0_CHARA_13_UUID_NAME		"f5899b5f800000800010000014000100"
//#define SERV_0_CHARA_14_UUID_NAME		"f5899b5f800000800010000015000100"
//#define SERV_0_CHARA_15_UUID_NAME		"f5899b5f800000800010000016000100"
//#define SERV_0_CHARA_16_UUID_NAME		"f5899b5f800000800010000017000100"
//#define SERV_0_CHARA_17_UUID_NAME		"f5899b5f800000800010000018000100"
//#define SERV_0_CHARA_18_UUID_NAME		"f5899b5f800000800010000019000100"
//#define SERV_0_CHARA_19_UUID_NAME		"f5899b5f800000800010000020000100"
//#define SERV_0_CHARA_20_UUID_NAME		"f5899b5f800000800010000021000100"
//#define SERV_0_CHARA_21_UUID_NAME		"f5899b5f800000800010000022000100"
//#define SERV_0_CHARA_22_UUID_NAME		"f5899b5f800000800010000023000100"
//#define SERV_0_CHARA_23_UUID_NAME		"f5899b5f800000800010000024000100"
//#define SERV_0_CHARA_24_UUID_NAME		"f5899b5f800000800010000025000100"
//#define SERV_0_CHARA_25_UUID_NAME		"f5899b5f800000800010000026000100"
//#define SERV_0_CHARA_26_UUID_NAME		"f5899b5f800000800010000027000100"
//#define SERV_0_CHARA_27_UUID_NAME		"f5899b5f800000800010000028000100"
//#define SERV_0_CHARA_28_UUID_NAME		"f5899b5f800000800010000029000100"
//#define SERV_0_CHARA_29_UUID_NAME		"f5899b5f800000800010000030000100"

/**************************************************************************//**
 * Ennam / uninon / Structure
 *****************************************************************************/
//<servID> 			Integer type. Service ID. Range: 0–65535.
//<UUID_type> 		Integer type. UUID type.
//					Omit <serv_UUID_s> when <UUID_type>=0.
//					Omit <serv_UUID_l> when <UUID_type>=1.
//					0 Long 128bit UUID
//					1 Short 16bit UUID
//<serv_UUID_l> 	String type. 128bit service UUID.
//<serv_UUID_s> 	String type.16bit service UUID. Range: 0–65535.
//<primary> 		Integer type. Whether it is the primary service.
//					0 Secondary service
//					1 Primary service

typedef struct
{
	uint16_t servID;
	uint8_t UUID_type;
	uint16_t serv_UUID_s;
	uint8_t serv_UUID_l[33]; //f5899b5f800000800010000000000100
	uint8_t serviceType;
}
modem_BLE_Service_t;

/**
 * <servID> 		Integer type. Service ID. Range: 0–65535.
 * <charaID> 		Integer type. Characteristic ID. Range: 0–65535.
 * <prop> 			Integer type. Characteristic property. Range: 0–255.
 * 					Bit0: Broadcast
 * 					Bit1: Read
 * 					Bit2: Write Without Response
 * 					Bit3: Write
 * 					Bit4: Notify
 * 					Bit5: Indicate
 * 					Bit6: Authenticated signed Writes
 * 					Bit7: Extended properties
 * <UUID_type> 		Integer type. UUID type.
 * 					Omit <serv_UUID_s> when <UUID_type>=0.
 * 					Omit <serv_UUID_l> when <UUID_type>=1.
 * 					0 Long 128bit UUID
 * 					1 Short 16bit UUID
 * <serv_UUID_l> 	String type. 128bit service UUID.
 * <serv_UUID_s> 	Integer type.16bit service UUID. Range: 0–65535.
 * <permission> 	Integer type. Characteristic value permission. Range: 0–1023.
 * 					Bit0: Read only, no security
 * 					Bit1: Write only, no security
 * 					Bit2: Read, authentication required
 * 					Bit3: Read, authorization required
 * 					Bit4: Read, encryption required
 * 					Bit5: Read, authorization and authentication required
 * 					Bit6: Write, authentication required
 * 					Bit7: Write, authorization required
 * 					Bit8: Write, encryption required
 * 					Bit9: Write, authorization and authentication required
 */

typedef struct
{
	uint16_t servID;
	uint16_t charaID;
	uint8_t property;
	//uint8_t UUID_type;
	//uint16_t chara_UUID_s;
	uint8_t chara_UUID_l[33]; //f5899b5f800000800010000000000100
	uint16_t permission;
	uint16_t att_handle;
}
modem_BLE_Characteristic_t;

typedef struct
{
	unsigned char powerStauts;
	lwgsm_mac_t macAddess;
	modem_BLE_Service_t *Service;
	modem_BLE_Characteristic_t *Characteristic;
	unsigned short int connection_ID;
	lwgsm_mac_t Conn_macAddess;
	unsigned char connectionStatus;
	unsigned char updateCharacteristicValueOnConnect;
	unsigned char writeDataArrive;
	unsigned char msg[512];
	uint16_t msgLen;
}
modem_BLE_t;





/**************************************************************************//**
 * Macro
 *****************************************************************************/


/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/

extern unsigned int count_Modem_BLE,Modem_BLE_check_sec;

extern modem_BLE_t ble;
extern modem_BLE_Characteristic_t gCharacteristic[TOTAL_CHARACTERISTIC];
extern modem_BLE_Service_t gService[TOTAL_SERVICES];
extern int variable0,variable1,variable2,variable3,variable4,variable5,variable6,variable7,variable8,variable9;
extern osThreadId Modem_BLE_TaskHandle;

/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/

void StartModem_BLETask(void const * argument);
void Modem_BLE_start();

unsigned char init_modem_BLE();
void modem_ble_make_list_of_Service_and_Characteristic();
void modem_ble_make_value_String_for_Characteristic(modem_BLE_t * _ble);
void modem_ble_make_descriptor_String_for_Characteristic(modem_BLE_t * _ble);
void checkForBleCharacteristicValueUpdateRequired(unsigned char updateAll);
unsigned char update_bleCharacteristicValue(modem_BLE_t * _ble);

#endif /* INC_MODEM_BLE_H_ */
