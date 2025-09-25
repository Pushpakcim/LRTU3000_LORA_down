/*
 * RAK_Lora.c
 *
 *  Created on: Sep 16, 2025
 *      Author: Admin
 */

#include "Lora_AT_Types.h"
#include "main.h"

/**************************************************************************//**
 * Macros
 *****************************************************************************/
#define AT_PORT_SEND_STR(str)       lora_serial_send_fn((const void*)(str), (size_t)strlen(str))
#define AT_PORT_SEND_CONST_STR(str) lora_serial_send_fn((const void*)(str), (size_t)(sizeof(str) - 1))
#define AT_PORT_SEND_CHR(ch)        lora_serial_send_fn((const void*)(ch), (size_t)1)
#define AT_PORT_SEND_FLUSH()        lora_serial_send_fn(NULL, 0)
#define AT_PORT_SEND(d, l)          lora_serial_send_fn((const void*)(d), (size_t)(l))

/* Beginning and end of every AT command */
#define AT_PORT_SEND_BEGIN_AT()                                                                                        \
    do {                                                                                                               \
        AT_PORT_SEND_CONST_STR("AT");                                                                                  \
    } while (0)
#define AT_PORT_SEND_END_AT()                                                                                          \
    do {                                                                                                               \
        AT_PORT_SEND(CRLF, CRLF_LEN);                                                                                  \
        AT_PORT_SEND(NULL, 0);                                                                                         \
    } while (0)

/**************************************************************************//**
 * Variable
 *****************************************************************************/
int16_t Lora_AT_Command;
osSemaphoreId lora_PortBlockSemaphore;
osSemaphoreId lora_SequenceBlockSemaphore;
osThreadId Lora_TaskHandle;

/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/
extern uint8_t Lora_RX_Buff[50];        	//LoRA buffer to fill from Rx interrupt
extern lwrb_t lora_rx_rb;  				//LoRA Ring buffer instance for RX data
extern uint8_t lora_rx_rb_data[1000];		//LoRA Ring buffer data array for RX DMA
extern CMD_TYPE current_cmd;
extern char crcMatch;

void Lora_initiate_cmd(lora_cmd_t cmd,void *argument)
{
	switch (cmd)
	{
        // LORA AT Commands
        case RAK_LORA_DEVICE_AT:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_ATE:
		{
			AT_PORT_SEND_CONST_STR("ATE");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_ATR:
		{
			AT_PORT_SEND_CONST_STR("ATR");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_ATZ:
		{
			AT_PORT_SEND_CONST_STR("ATZ");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_SN_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+SN=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_FIRMVARE_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+VER=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_HWMODEL_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+HWMODEL=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_HWID_GET:
		{
			/* Enable detailed error messages */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+HWID=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_BLEMAC_GET:
		{
			/* Enable detailed call info */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+BLEMAC=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_BAUD_GET:
		{
			/* Get manufacturer */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+BAUD=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_BAUD_SET:
		{
			/* Get manufacturer */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+BAUD=");
//			Lora_initiate_cmd(LWGSM_U32(LoRa_Modem.lora_baudrate), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_ATM_MODE:
		{
			/* Get model */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+ATM?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_DEVEUI_GET:
		{
			/* Get serial number */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+DEVEUI=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_DEVEUI_SET:
		{
			/* Get revision */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+DEVEUI=");
			//lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_APPEUI_GET:
		{
			/* Enable +CREG message */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+APPEUI=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_APPEUI_SET:
		{
			/* Get network registration status */
			AT_PORT_SEND_BEGIN_AT();
			//AT_PORT_SEND_CONST_STR("+APPEUI=AC1F09FFF8657431"); // TODO : APPEUI as per config
			AT_PORT_SEND_CONST_STR("+APPEUI=");
//			Lora_initiate_cmd((const char*)EPROM_LoRa_Modem.lora_app_eui_set, 1, 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_APPKEY_GET:
		{
			/* Get network registration status */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+APPKEY=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_APPKEY_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			//AT_PORT_SEND_CONST_STR("+APPKEY=AC1F09FFFE0BA82CAC1F09FFF8657431"); // TODO : AppKey as per config
			AT_PORT_SEND_CONST_STR("+APPKEY=");
//			Lora_initiate_cmd((const char*)EPROM_LoRa_Modem.lora_app_key_set, 1, 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_DEVADDR_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+DEVADDR=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_DEVADDR_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+DEVADDR=");
			//lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_APPSKEY_SET:
		{
			/* Get SIM number */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+APPSKEY=");
			//lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_NWKSKEY_GET:
		{
			/* Shut down network connection and put to reset state */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+NWKSKEY=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_NWKSKEY_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+NWKSKEY=");
			//lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_NETID_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+NETID=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_MCROOTKEY_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+MCROOTKEY=?");
			AT_PORT_SEND_END_AT();
			break;
		}
//LoRaWAN Joining and Sending
		case RAK_LORA_DEVICE_CFM_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+CFM=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_CFM_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+CFM=");
			//lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_CFS_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+CFS=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_JOIN_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+JOIN=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_JOIN_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+JOIN=1:0:8:0");
			//lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_NJM_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+NJM=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_NJM_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+NJM=");
//			Lora_initiate_cmd(LWGSM_U32(EPROM_LoRa_Modem.lora_network_Mode_set), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_NJS_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+NJS=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_RECV:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+RECV=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_SEND:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+SEND=12:");
			AT_PORT_SEND_STR(lora_tx_buf);
			//lwgsmi_send_number(lora_tx_port, 0, 0);
			//lwgsmi_send_string(lora_tx_buf, 0, 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_LPSEND:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+LPSEND=");
			//lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_RETY_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+RETY=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_RETY_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+RETY=");
			//lwgsmi_send_number(LWGSM_U32(_mqtt->client_idx), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}

		// LoRaWAN Network Management
		case RAK_LORA_DEVICE_ADR_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+ADR=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_ADR_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+ADR=");
//			Lora_initiate_cmd(LWGSM_U32(EPROM_LoRa_Modem.lora_adaptiveDataRate_enable_set), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_CLASS_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+CLASS=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_CLASS_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			if(EPROM_LoRa_Modem.lora_class_set=='A')
			{
				AT_PORT_SEND_CONST_STR("+CLASS=A");
			}
			else if(EPROM_LoRa_Modem.lora_class_set == 'B')
			{
				AT_PORT_SEND_CONST_STR("+CLASS=B");
			}
			else
			{
				AT_PORT_SEND_CONST_STR("+CLASS=C");
			}
			//AT_PORT_SEND_CONST_STR("+CLASS=C"); // TODO : Class as per config
			//lwgsmi_send_number(LWGSM_U32(EPROM_LoRa_Modem.lora_class_set), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_DCS_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+DCS=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_DCS_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+DCS=");
//			Lora_initiate_cmd(LWGSM_U32(EPROM_LoRa_Modem.lora_duty_cycle_enable_set), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_DR_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+DR=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_DR_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+DR=");
//			Lora_initiate_cmd(LWGSM_U32(EPROM_LoRa_Modem.lora_dataRate_set), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_BGW_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+BGW=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_LINKCHECK:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+LINKCHECK=");
//			Lora_initiate_cmd(LWGSM_U32(EPROM_LoRa_Modem.lora_link_check_enable_set), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_BAND_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+BAND=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_BAND_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+BAND=");
//			Lora_initiate_cmd(LWGSM_U32(EPROM_LoRa_Modem.lora_active_region_set), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_P2P_MODE_SET:
		{
			/* SET modem to P2P mode */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+NWM=0");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_P2P_MODE_GET:
		{
			/* Get modem P2P mode */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+NWM=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_PRECV_SET:
		{
			char command[14] = "+PRECV=";
			if(argument != NULL)
			{
				strcat(command, (char *) argument);
			}
			else
				strcat(command, "0");

			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR(command);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_P2P_PARAM_SET:
		{
			char command[30] = "+P2P=";
			strcat(command, (char *) argument);

			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR(command);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_PSEND:
		{
			char command[] = "+PSEND=";
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR(command);
			AT_PORT_SEND_STR((const void*)lora_tx_buf);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_PENCRYPT_EN:
		{
			char command[] = "+ENCRY=0";	// Enable Encryption
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR(command);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_PENCRYPT_KEY:
		{
			char command[] = "+ENCKEY=01020304050607080102030405060708";	// Set Encryption Key
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR(command);
			AT_PORT_SEND_END_AT();
			HAL_Delay(100);
			char command1[] = "+CRYPIV=00112233445566770011223344556677";	// Set Encryption IV
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR(command1);
			AT_PORT_SEND_END_AT();
		}
		case RAK_LORA_DEVICE_CAD_EN:
		{
			char command[] = "AT+CAD=1";	// Enable Encryption
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR(command);
			AT_PORT_SEND_END_AT();
			break;
		}
	}
}

/**************************************************************************//**
 * Function name 	: lora_modem_init
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char lora_modem_init()
{
	unsigned char res=0;

	if(xSemaphoreTake(lora_SequenceBlockSemaphore, 1200000))
	{
//		modemCommandSequence = MODEM_CMD_SEQUENCE_LORA_INIT;

		osDelay(2000);
		lora_AT_check=2;
		lora_AT_ok_check=2;
		Lora_AT_Command = RAK_LORA_DEVICE_AT;
		Lora_initiate_cmd(Lora_AT_Command,0);
		osDelay(500);

		if((lora_AT_ok_check == 1)&&(lora_AT_check == 1))
		{
			Lora_AT_Command = RAK_LORA_DEVICE_ATE;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);
		}

		lora_AT_check=2;
		lora_AT_ok_check=2;
		Lora_AT_Command = RAK_LORA_DEVICE_AT;
		Lora_initiate_cmd(Lora_AT_Command,0);
		osDelay(500);

		if((lora_AT_ok_check == 1)&&(lora_AT_check == 2))
		{
			Lora_AT_Command = RAK_LORA_DEVICE_SN_GET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

/*			Lora_AT_Command = RAK_LORA_DEVICE_FIRMVARE_GET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_DEVEUI_GET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_APPEUI_GET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_APPKEY_GET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_NJM_SET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_ADR_SET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_CLASS_SET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_LINKCHECK;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_DR_SET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_BAND_SET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_ADR_GET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_CLASS_GET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_BAND_GET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			Lora_AT_Command = RAK_LORA_DEVICE_NJM_GET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);*/
		}

		Lora_AT_Command = RAK_LORA_DEVICE_AT;
//		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(lora_SequenceBlockSemaphore);
	}

	return res;
}

/**************************************************************************//**
 * Function name 	: lora_modem_network_check
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

unsigned char lora_modem_network_check()
{
	unsigned char res=0;

	if(xSemaphoreTake(lora_SequenceBlockSemaphore, 1200000))
	{
//		modemCommandSequence = MODEM_CMD_SEQUENCE_LORA_CHECK;

		osDelay(500);
		Lora_AT_Command = RAK_LORA_DEVICE_NJS_GET;
		Lora_initiate_cmd(Lora_AT_Command,0);
		osDelay(500);

		if(LoRa_Modem.lora_network_join_state == 0)
		{
			Lora_AT_Command = RAK_LORA_DEVICE_JOIN_SET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(1000);
			res=0;
		}
		else
		{
			res=1;
		}

		Lora_AT_Command = LWGSM_CMD_IDLE;
//		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(lora_SequenceBlockSemaphore);
	}

	return res;
}


/**************************************************************************//**
 * Function name 	: lora_modem_network_reconnect
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

unsigned char lora_modem_network_reconnect()
{
	unsigned char res=0;

	if(xSemaphoreTake(lora_SequenceBlockSemaphore, 1200000))
	{
//		modemCommandSequence = MODEM_CMD_SEQUENCE_LORA_CONNECT;

		if(LoRa_Modem.lora_restart_request == 1)
		{
			osDelay(100);
			Lora_AT_Command = RAK_LORA_DEVICE_ATZ;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(5000);

			Lora_AT_Command = RAK_LORA_DEVICE_JOIN_SET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);
		}
		else
		{
			osDelay(100);
			Lora_AT_Command = RAK_LORA_DEVICE_NJS_GET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);

			if(LoRa_Modem.lora_network_join_state == 0)
			{
				osDelay(100);
				Lora_AT_Command = RAK_LORA_DEVICE_ATZ;
				Lora_initiate_cmd(Lora_AT_Command,0);
				osDelay(5000);

				Lora_AT_Command = RAK_LORA_DEVICE_JOIN_SET;
				Lora_initiate_cmd(Lora_AT_Command,0);
				osDelay(500);
			}
		}

		Lora_AT_Command = LWGSM_CMD_IDLE;
//		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(lora_SequenceBlockSemaphore);
	}

	return res;
}

/**************************************************************************//**
 * Function name 	: lora_modem_send_msg
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char lora_modem_send_msg(const char *msg)
{
	unsigned char res=0;

	if(xSemaphoreTake(lora_SequenceBlockSemaphore, 1200000))
	{
//		modemCommandSequence = MODEM_CMD_SEQUENCE_LORA_SEND;

		osDelay(1000);
//		Lora_AT_Command = RAK_LORA_DEVICE_DR_SET;
		Lora_AT_Command = RAK_LORA_DEVICE_PRECV_SET;
		Lora_initiate_cmd(Lora_AT_Command,"0");
		osDelay(500);

//		Lora_AT_Command = RAK_LORA_DEVICE_SEND;
		Lora_AT_Command = RAK_LORA_DEVICE_PSEND;
		Lora_initiate_cmd(Lora_AT_Command,"0");
		osDelay(1500);

		Lora_AT_Command = RAK_LORA_DEVICE_PRECV_SET;
		Lora_initiate_cmd(Lora_AT_Command,"65534");
		osDelay(500);

		Lora_AT_Command = RAK_LORA_DEVICE_AT;
//		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(lora_SequenceBlockSemaphore);
	}

	return res;
}

/**************************************************************************//**
 * Function name 	: lora_modem_receive_msg
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char lora_modem_receive_msg()
{
	unsigned char res=0;

	if(xSemaphoreTake(lora_SequenceBlockSemaphore, 1200000))
	{
//		modemCommandSequence = MODEM_CMD_SEQUENCE_LORA_RECEIVE;
		lora_rx_port=0;

		osDelay(100);
		Lora_AT_Command = RAK_LORA_DEVICE_RECV;
		Lora_initiate_cmd(Lora_AT_Command,0);
		osDelay(500);
		res=lora_rx_port;

		Lora_AT_Command = LWGSM_CMD_IDLE;
//		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(lora_SequenceBlockSemaphore);
	}

	return res;
}

/**************************************************************************//**
 * Function name 	: lora_modem_receive_msg
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char lora_modem_update_parameter()
{
	unsigned char res=0;

	if(xSemaphoreTake(lora_SequenceBlockSemaphore, 1200000))
	{
//		modemCommandSequence = MODEM_CMD_SEQUENCE_LORA_UPDATE;


		Lora_AT_Command = LWGSM_CMD_IDLE;
//		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(lora_SequenceBlockSemaphore);
	}

	return res;
}

void StartLoraTask(void const * argument)
{
	osDelay(5000);
	unsigned int networkCheckTime = 60;
	char LED_Lora_blinking, LED_counter=0;
	HAL_UART_Receive_IT(&huart2, &Lora_RX_Buff[0], sizeof(Lora_RX_Buff));
	lora_PortBlockSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(lora_PortBlockSemaphore);

	lora_SequenceBlockSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(lora_SequenceBlockSemaphore);

	set_pulse_do_polarity();
	osDelay(10);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	lcd_clear();
	lcd_set_cursor(1,0);
	if(Pro_Application_flag == 1)
	{
		lcd_display_string("   PRODUCTION   ");
		lcd_set_cursor(2,0);
		lcd_display_string("      MODE      ");
	}

	float ADC_VAL_accumalate[MAX_AI_CHANNEL];
	unsigned char sample_count = 0;
	for(int i=0;i<MAX_AI_CHANNEL;i++)
	{
		ADC_VAL_accumalate[i] = 0;
	}

/*	Lora_AT_Command = RAK_LORA_DEVICE_APPEUI_SET; // RAK_LORA_DEVICE_APPKEY_SET
	Lora_initiate_cmd(Lora_AT_Command,0);
	osDelay(500);

	Lora_AT_Command = RAK_LORA_DEVICE_APPKEY_SET;
	Lora_initiate_cmd(Lora_AT_Command,0);
	osDelay(500);

	if(convertHextoAsciiString((char*)EPROM_LoRa_Modem.lora_app_eui_set,(char*)&Lora_Modem_Ascii_String_app_eui,strlen((char*)EPROM_LoRa_Modem.lora_app_eui_set)*2)!=0)
	{
		// Can't perform conversion
	}
	if(convertHextoAsciiString((char*)EPROM_LoRa_Modem.lora_app_key_set,(char*)&Lora_Modem_Ascii_String_app_key,strlen((char*)EPROM_LoRa_Modem.lora_app_key_set)*2)!=0)
	{
		// Can't perform conversion
	}*/

	lora_modem_init();

//	convertHextoAsciiString((char*)EPROM_LoRa_Modem.lora_dev_eui_set,(char*)&Lora_Modem_Ascii_String_dev_eui,strlen((char*)EPROM_LoRa_Modem.lora_dev_eui_set)*2);
//***************************************AN********************************//
	if(Pro_Application_flag)
		{
			while(ProductionModeIDFrameReceived == 0)
			{
				osDelay(100);
			}
		}
//***************************************AN********************************//
//	calculateLograteTimeSliceDelayS();

	for(;;)
	{
		count_EC200U=0;
		if((networkCheckTime++>30)||(LoRa_Modem.lora_restart_request == 1))
		{
			networkCheckTime = 0;
			lora_modem_network_mode_check();
			if(LoRa_Modem.lora_network_Mode != 0)
			{
				Lora_AT_Command = RAK_LORA_DEVICE_P2P_MODE_SET;
				Lora_initiate_cmd(Lora_AT_Command,0);
				osDelay(100);
				Lora_AT_Command = RAK_LORA_DEVICE_PRECV_SET;
				Lora_initiate_cmd(Lora_AT_Command,"0");
				osDelay(100);
				// 865985000 frequency, 125kHz bandwidth, spreading factor 12, coding rate 4/5, preamble length 8 and TX power of 22dBm:
				Lora_AT_Command = RAK_LORA_DEVICE_P2P_PARAM_SET;
				Lora_initiate_cmd(Lora_AT_Command,"865985000:7:0:1:8:22");
				osDelay(100);
				Lora_AT_Command = RAK_LORA_DEVICE_CAD_EN;
				Lora_initiate_cmd(Lora_AT_Command,0);
				osDelay(100);
				Lora_AT_Command = RAK_LORA_DEVICE_PENCRYPT_KEY;
				Lora_initiate_cmd(Lora_AT_Command,"0");
				osDelay(100);
				Lora_AT_Command = RAK_LORA_DEVICE_PENCRYPT_EN;
				Lora_initiate_cmd(Lora_AT_Command,"0");
				osDelay(100);
				Lora_AT_Command = RAK_LORA_DEVICE_PRECV_SET;
				Lora_initiate_cmd(Lora_AT_Command,"65534");	// continuous RX mode

				Lora_AT_Command = RAK_LORA_DEVICE_AT;
			}
		}

		if(ReceivedDataOfLoRaClient == 1)
		{
			ReceivedDataOfLoRaClient = 0;
			ProcessModbusSlave(&ModbusH[COM_LORA]);

			if(ModbusH[5].u8RxBuffer[1]==03)
			   {
				flagLORAPubLogData=1;
			   }

		}

		if(flagLORAPubLogData == 1 && Pro_Application_flag==1)
		{
			    flagLORAPubLogData = 0;
				delay_with_DevEUI();
				LoraPublish();
				lora_modem_send_msg((const char *)&lora_tx_buf);
				LED_Lora_blinking = 1;
				LED_counter = 4;
				//HAL_Delay(30000);
		}

		if(flagLORAPubLogData == 1 && Pro_Application_flag==0)
		{
			flagLORAPubLogData = 0;
			LoraPublish();
			lora_modem_send_msg((const char *)&lora_tx_buf);
//			PLC_sendcall_flag=0; //TODO: remove Akshay
			LED_Lora_blinking = 1;
			LED_counter = 4;
		}
		if(LED_counter > 0)
		{
			LED_counter--;
			if(LED_Lora_blinking == 1)
			{
				LED_Lora_blinking = 0;
				HAL_GPIO_WritePin(LED_LORA_GPIO_Port,LED_LORA_Pin , GPIO_PIN_SET);
			}
			else
			{
				LED_Lora_blinking = 1;
				HAL_GPIO_WritePin(LED_LORA_GPIO_Port,LED_LORA_Pin , GPIO_PIN_RESET);
			}
		}

		gFinalAnaValF[LORA_PHY_STATUS_gFinalAnaValF]= lora_AT_ok_check;
		gFinalAnaValF[LORA_NETWORK_JOIN_STATUS_gFinalAnaValF]= LoRa_Modem.lora_network_join_state;
		gFinalAnaValF[LORA_RSSI_gFinalAnaValF]= LoRa_Modem.lora_RSSI;
		gFinalAnaValF[LORA_CLASS_gFinalAnaValF]= LoRa_Modem.lora_class;
		//gFinalAnaValF[LORA_ADAPTIVE_DATARATE_gFinalAnaValF]= LoRa_Modem.lora_adaptiveDataRate_enable;

		gFinalAnaValF[LORA_MODE_gFinalAnaValF]= LoRa_Modem.lora_active_region;
		gFinalAnaValF[LORA_ACTIVE_REGION_gFinalAnaValF]= LoRa_Modem.lora_network_Mode;

		//osDelay(1000);


		count_DO = 0;
		flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter++;

		#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
			HAL_IWDG_Refresh(&hiwdg1);
		#endif

		//if(count_DO < 3600) // External Watch dog  //(ProTest_Request) & (0x1<<4)
		if((proTestRequest) & (0x1<<PRODUCTION_TEST_BIT_WATCHDOG))
		{

		}
		else
		{
			HAL_GPIO_TogglePin(Watchdog_GPIO_Port, Watchdog_Pin);  //hardware watchdog refresh
		}

		if(Pro_Application_flag == 1)
		{
			pro_checkDIDOState();
           if(LoRa_Modem.lora_network_join_state==1)
           {
        	   flagLORAPubLogData = 1;    //AN add
        	   osDelay(5000);			//AN ADD
           }
		}

		ScanDI();
		//set_do(); // Temp


		if(flag_flashUpdateEPROM_General == 1)
		{
			flag_flashUpdateEPROM_General_WaitCounter--;
			if(flag_flashUpdateEPROM_General_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_General = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_General();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_General = 1;
					flag_flashUpdateEPROM_General_WaitCounter = 1;
				}
			}

		}

		if(flag_flashUpdateEPROM_Frequent == 1)
		{
			flag_flashUpdateEPROM_Frequent_WaitCounter--;
			if(flag_flashUpdateEPROM_Frequent_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_Frequent = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_Frequent();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_Frequent = 1;
					flag_flashUpdateEPROM_Frequent_WaitCounter = 1;
				}
			}
		}

		if(flag_flashUpdateEPROM_AI_Calibration == 1)
		{
			flag_flashUpdateEPROM_AI_Calibration_WaitCounter--;
			if(flag_flashUpdateEPROM_AI_Calibration_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_AI_Calibration = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_AI_Calibration();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_AI_Calibration = 1;
					flag_flashUpdateEPROM_AI_Calibration_WaitCounter = 1;
				}
			}

		}

		if(flag_flashUpdateEPROM_Schedule == 1)
		{
			flag_flashUpdateEPROM_Schedule_WaitCounter--;
			if(flag_flashUpdateEPROM_Schedule_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_Schedule = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_Schedule();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_Schedule = 1;
					flag_flashUpdateEPROM_Schedule_WaitCounter = 1;
				}
			}

		}

		if(flag_flashUpdateEPROM_LORA == 1)
		{
			flag_flashUpdateEPROM_LORA_WaitCounter--;
			if(flag_flashUpdateEPROM_LORA_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_LORA = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_LORA();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_LORA = 1;
					flag_flashUpdateEPROM_LORA_WaitCounter = 1;
				}
			}

		}

		if(flag_flashUpdateEPROM_Modbus_Quary_Detail == 1)
		{
			flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter--;
			if(flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_Modbus_Quary_Detail = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_Modbus_Quary_Detail();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_Modbus_Quary_Detail = 1;
					flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter = 1;
				}
			}
		}

		if(flag_flashSaveRecipe == 1)
		{
			flag_flashSaveRecipe_WaitCounter--;
			if(flag_flashSaveRecipe_WaitCounter==0)
			{
				flag_flashSaveRecipe = 0;
		        if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
		        	WriteModifiedRecipeFile(MODIFIED_RECIPE_FILE_PATH);
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
		        else
		        {
		        	flag_flashSaveRecipe = 1;
		        	flag_flashSaveRecipe_WaitCounter = 1;
		        }
			}
		}

		if(flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter == 60)
		{
			flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter = 0;
			if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
			{
				for(int i=0;i<MAX_GENERAL_AI;i++)
				{
					EPROM_PCBPLC_General_Reg.General_Reg[i]=gFinalAnaValF[GENERAL_PURPOSE_AI_gFinalAnaValF+i];
				}
				ExtFlash_update_EPROM_PCBPLC_GENERAL_REG();

				EPROM_Frequent.DI1_Pulse = DI1_Pulse_Count;
				EPROM_Frequent.DI2_Pulse = DI2_Pulse_Count;
				ExtFlash_update_EPROM_Frequent();
				xSemaphoreGive(sendExternalFlashSemaphore);
			}
			else
			{
				flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter = 299;
			}
		}

		//count_ADC++;
		count_ADC = 0;
		sample_count++;
		print_time();

		for(int i=0;i<MAX_AI_CHANNEL;i++)
		{
			//ADC1_calibration_CH(SCALE0TO10,i);
			ADC1_calibration_CH(EPROM_General.AI_DI_DO_Detail.AI_Detail[i].AI_ch_Type,i);
			ADC_calculation(AO_VAL_float[i],i);
			ADC_VAL_accumalate[i] = (float)ADC_Temp[i]+(float)ADC_VAL_accumalate[i];
		}

		//if(sample_count >= (EPROM_General.AI_DI_DO_Detail.Sample_time_to_collect_AI)*4) // TODO: make 5 as configurable from modbus
		if(sample_count >= 1)
		{
			for(int i=0;i<MAX_AI_CHANNEL;i++)
			{
				AI_Final_value[i] = ADC_VAL_accumalate[i]/sample_count;
				ADC_VAL_accumalate[i] = 0;
			}
			sample_count = 0;
		}
		osDelay(1000);
	}
}

void Lora_start()
{
	osThreadDef(LoraTask, StartLoraTask, osPriorityHigh, 0, 512*10); //512*10
	Lora_TaskHandle = osThreadCreate(osThread(LoraTask), NULL);
}

/****************************************************************************/
//delay fuction using lora dev eui for production application

/****************************************************************************/
void delay_with_DevEUI()
{
	 // Extract the last byte of the DevEUI
	    uint8_t last_byte = LoRa_Modem.lora_dev_eui[15];

	    // Calculate additional delay based on the last byte
	    uint32_t additional_delay = (uint32_t)last_byte * 100; // Adjust the multiplier as needed

	    // Apply the combined delay
	    HAL_Delay(additional_delay);  // Delay in milliseconds
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */
  HAL_UART_ReceiverTimeout_Config(&huart2,10); // maulin change from 100 to 10
  HAL_UART_EnableReceiverTimeout(&huart2);
  /* USER CODE END USART2_Init 2 */

}

/**************************************************************************//**
 * Function name 	: AT_Serial_Send_fn
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

uint16_t lora_serial_send_fn(const void* data, size_t len)
{
    const uint8_t* d = data;

    HAL_UART_Transmit(&huart2,d,len,5000);

    return len;
}

void lora_modem_network_mode_check(void)
{
	Lora_AT_Command = RAK_LORA_DEVICE_P2P_MODE_GET;
	Lora_initiate_cmd(Lora_AT_Command,0);
}
