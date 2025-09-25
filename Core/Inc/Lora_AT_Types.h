/*
 * Lora_AT_Types.h
 *
 *  Created on: Sep 16, 2025
 *      Author: Admin
 */

#ifndef INC_LORA_AT_TYPES_H_
#define INC_LORA_AT_TYPES_H_

/**************************************************************************//**
 * Includes
 *****************************************************************************/
#include <stddef.h>
#include <stdint.h>

/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/

typedef enum {
	///LORA AT Commands
	RAK_LORA_DEVICE_AT = 0, 				/*!< Send AT command without any specific parameters. */
	RAK_LORA_DEVICE_ATE, 			/*!< Send AT command "ATE" without any additional parameters. */
	RAK_LORA_DEVICE_ATR, 			/*!< Send AT command "ATR" without any additional parameters. */
	RAK_LORA_DEVICE_ATZ, 			/*!< Send AT command "ATZ" without any additional parameters. */
	RAK_LORA_DEVICE_SN_GET, 			/*!< Send AT command "+SN=?" to retrieve the serial number. */
	RAK_LORA_DEVICE_FIRMVARE_GET, 	/*!< Send AT command "+VER=?" to retrieve firmware version. */
	RAK_LORA_DEVICE_HWMODEL_GET, 	/*!< Send AT command "+HWMODEL=?" to retrieve hardware model. */
	RAK_LORA_DEVICE_HWID_GET, 		/*!< Send AT command "+HWID=?" to retrieve hardware ID. */
	RAK_LORA_DEVICE_BLEMAC_GET,		/*!< Send AT command "+BLEMAC=?" to retrieve Bluetooth MAC address. */
	RAK_LORA_DEVICE_BAUD_GET, 		/*!< Send AT command "+BAUD=?" to retrieve baud rate. */
	RAK_LORA_DEVICE_BAUD_SET, 		/*!< Send AT command "+BAUD=?" to retrieve baud rate. */
	RAK_LORA_DEVICE_ATM_MODE, 		/*!< Send AT command "+ATM?" to get the mode. */
	RAK_LORA_DEVICE_DEVEUI_GET, 		/*!< Send AT command "+DEVEUI=?" to retrieve the device EUI. */
	RAK_LORA_DEVICE_DEVEUI_SET, 		/*!< Send AT command "+DEVEUI=" without additional parameters. */
	RAK_LORA_DEVICE_APPEUI_GET, 		/*!< Send AT command "+APPEUI=?" to retrieve the application EUI. */
	RAK_LORA_DEVICE_APPEUI_SET, 		/*!< Send AT command "+APPEUI=" without additional parameters. */
	RAK_LORA_DEVICE_APPKEY_GET, 		/*!< Send AT command "+APPKEY=?" to retrieve the application key. */
	RAK_LORA_DEVICE_APPKEY_SET, 		/*!< Send AT command "+APPKEY=" without additional parameters. */
	RAK_LORA_DEVICE_DEVADDR_GET, 	/*!< Send AT command "+DEVADDR=?" to retrieve the device address. */
	RAK_LORA_DEVICE_DEVADDR_SET, 	/*!< Send AT command "+DEVADDR=" without additional parameters. */
	RAK_LORA_DEVICE_APPSKEY_SET, 	/*!< Send AT command "+APPSKEY=" without additional parameters. */
	RAK_LORA_DEVICE_NWKSKEY_GET, 	/*!< Send AT command "+NWKSKEY=?" to retrieve the network session key. */
	RAK_LORA_DEVICE_NWKSKEY_SET, 	/*!< Send AT command "+NWKSKEY=" without additional parameters. */
	RAK_LORA_DEVICE_NETID_GET, 		/*!< Send AT command "+NETID=?" to retrieve the network ID. */
	RAK_LORA_DEVICE_MCROOTKEY_GET, 	/*!< Send AT command "+MCROOTKEY=?" to retrieve the multicast root key. */
	RAK_LORA_DEVICE_CFM_GET, 		/*!< Send AT command "+CFM=?" to retrieve a configuration value. */
	RAK_LORA_DEVICE_CFM_SET, 		/*!< Send AT command "+CFM=" without additional parameters. */
	RAK_LORA_DEVICE_CFS_GET, 		/*!< Send AT command "+CFS=?" to retrieve a configuration value. */
	RAK_LORA_DEVICE_JOIN_GET, 		/*!< Send AT command "+JOIN=?" to retrieve join parameters. */
	RAK_LORA_DEVICE_JOIN_SET, 		/*!< Send AT command "+JOIN=" without additional parameters. */
	RAK_LORA_DEVICE_NJM_GET, 		/*!< Send AT command "+NJM=?" to retrieve network join mode. */
	RAK_LORA_DEVICE_NJM_SET, 		/*!< Send AT command "+NJM=" without additional parameters. */
	RAK_LORA_DEVICE_NJS_GET, 		/*!< Send AT command "+NJS=?" to retrieve network join settings. */
	RAK_LORA_DEVICE_RECV, 			/*!< Send AT command "+RECV=?" to retrieve received data. */
	RAK_LORA_DEVICE_SEND, 			/*!< Send AT command "+SEND=" without additional parameters. */
	RAK_LORA_DEVICE_LPSEND, 			/*!< Send AT command "+LPSEND=" without additional parameters. */
	RAK_LORA_DEVICE_RETY_GET, 		/*!< Send AT command "+RETY=?" to retrieve retry count. */
	RAK_LORA_DEVICE_RETY_SET, 		/*!< Send AT command "+RETY=" without additional parameters. */
	RAK_LORA_DEVICE_ADR_GET, 		/*!< Send AT command "+ADR=" to get ADR settings. */
	RAK_LORA_DEVICE_ADR_SET, 		/*!< Send AT command "+ADR=?" to retrieve ADR settings. */
	RAK_LORA_DEVICE_CLASS_GET, 		/*!< Send AT command "+CLASS=?" to retrieve device class. */
	RAK_LORA_DEVICE_CLASS_SET, 		/*!< Send AT command "+CLASS=" without additional parameters. */
	RAK_LORA_DEVICE_DCS_GET, 		/*!< Send AT command "+DCS=?" to retrieve data rate configuration. */
	RAK_LORA_DEVICE_DCS_SET, 		/*!< Send AT command "+DCS=" without additional parameters. */
	RAK_LORA_DEVICE_DR_GET, 			/*!< Send AT command "+DR=?" to retrieve data rate. */
	RAK_LORA_DEVICE_DR_SET, 			/*!< Send AT command "+DR=" without additional parameters. */
	RAK_LORA_DEVICE_BGW_GET, 		/*!< Send AT command "+BGW=?" to retrieve the device band. */
	RAK_LORA_DEVICE_LINKCHECK,		/*!< Send AT command "+LINKCHECK=?" to check gateway response. */
	RAK_LORA_DEVICE_BAND_GET, 			/*!< Send AT command "+BAND=?" to check gateway response. */
	RAK_LORA_DEVICE_BAND_SET, 			/*!< Send AT command "+BAND=?" to Set gateway response. */
	RAK_LORA_DEVICE_P2P_MODE_SET,	/*!< Send AT command "+NWM=0" to Set P2P mode. */
	RAK_LORA_DEVICE_P2P_MODE_GET,	/*!< Send AT command "+NWM?" to Get P2P mode. */
	RAK_LORA_DEVICE_PRECV_SET,		/*!< Send AT command "+PRECV?" to put in P2P RX mode for a period of time (ms). */
	RAK_LORA_DEVICE_P2P_PARAM_SET,	/*!< Send AT command "+P2P=" to Set P2P mode parameters. */
	RAK_LORA_DEVICE_PSEND,			/*!< Send AT command "+PSEND=" to send data in P2P mode. */
	RAK_LORA_DEVICE_PENCRYPT_EN,	/*!< Send AT command "+ENCRY=" to enable encryption in P2P mode. */
	RAK_LORA_DEVICE_PENCRYPT_KEY,	/*!< Send AT command "+ENCKEY=" to set encryption key in P2P mode. */
	RAK_LORA_DEVICE_CAD_EN			/*!< Send AT command "+CAD=1" to set P2P Channel Activity Detection. */
} lora_cmd_t;

uint16_t lora_serial_send_fn(const void* data, size_t len);
unsigned char lora_modem_init();
unsigned char lora_modem_network_check();
unsigned char lora_modem_network_reconnect();
unsigned char lora_modem_send_msg(const char *msg);
unsigned char lora_modem_update_parameter();
unsigned char lora_modem_receive_msg();
void Lora_start();

#endif /* INC_LORA_AT_TYPES_H_ */
