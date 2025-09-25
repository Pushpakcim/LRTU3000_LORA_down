/*
 * EC200U.h
 *
 *  Created on: Nov 23, 2022
 *      Author: maulin
 */

#ifndef INC_EC200U_RxRingProcess_H_
#define INC_EC200U_RxRingProcess_H_

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

/**************************************************************************//**
 * Macro
 *****************************************************************************/

/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/

extern unsigned int count_EC200U_RxRingProcess;
extern uint8_t EC200U_RX_Buff[50];        //buffer to feel from Rx intrupt
extern lwrb_t EC200U_RX_rb;  				//Ring buffer instance for RX data
extern uint8_t EC200U_RX_rb_data[1000]; 	//Ring buffer data array for RX DMA

extern char lora_AT_check;
extern char lora_AT_ok_check;
/**
 * \brief           Receive character structure to handle full line terminated with `\n` character
 */
typedef struct {
    char data[128]; /*!< Received characters */
    size_t len;     /*!< Length of valid characters */
} lwgsm_recv_t;

typedef struct {
	unsigned char model_manufacturer[100];
	unsigned char model_number[100];
	unsigned char model_serial_number[100];
	unsigned char model_revision[100];
} ModemInfo_t;

typedef struct {
	unsigned char lora_dev_eui[24];  	// 16byte
	unsigned char lora_app_eui[24];		// 16byte
	unsigned char lora_app_key[48];		// 32byte
	unsigned char lora_network_join_state;
	unsigned char lora_network_Mode; //the network mode (0 = ABP, 1 = OTAA)
	unsigned char lora_adaptiveDataRate_enable; //the adaptive data rate setting (0 = off, 1 = on)
	unsigned char lora_dataRate; //set the data rate (0,1,2,3,4,5,6,7)
	unsigned char lora_class; //set the class (0 = A, 1 = B ,2 = C)
	unsigned char lora_power; //set the power (0 high and 10 low)
	unsigned char lora_active_region;//AT+BAND: get or set the active region(0 = EU433, 1 = CN470, 2 = RU864, 3 = IN865, 4 = EU868,5 = US915, 6 = AU915, 7 = KR920, 8 = AS923-1, 9 = AS923-2, 10 = AS923-3, 11 = AS923-4, 12 = LA915)
	unsigned char lora_link_check_enable;//AT+LINKCHECK: get or set the link check setting (0 = disabled, 1 = once, 2 = everytime)
	unsigned char lora_device_serial_number[24]; //AT+SN device serial number 1-18byte
	unsigned char lora_device_firmware_version_number[24]; //AT+VER=firmware version
	signed char lora_RSSI;
	char lora_SNR;
	unsigned char lora_nbGateway;
	unsigned char lora_DemodMargin;
	unsigned char lora_duty_cycle_enable; // * the duty cycle setting (0 = off, 1 = on)
	unsigned int lora_baudrate; // * AT+BAUD= set the baudrate
	unsigned char lora_rxState;
	unsigned char lora_restart_request;
	unsigned char RETRY_LIMIT;
} lora_Modem_t;

typedef struct {
	unsigned char checkbyte;							//1
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
} lora_EPROM_Modem_t;

extern lora_EPROM_Modem_t EPROM_LoRa_Modem;
extern lora_Modem_t LoRa_Modem;
extern unsigned char lora_tx_port;
extern char lora_tx_buf[500];
//extern unsigned char lora_tx_buf_ascii[256];
extern unsigned char lora_rx_port;
extern unsigned char lora_rx_buf[250];
extern unsigned char lora_rx_buf_ascii[256];

extern ModemInfo_t ModemInfo;
extern lwgsmr_t Modem_PHY_Status;
extern lwgsmr_t Modem_PHY_Status_t;

extern unsigned char PDP_Context_ID;
extern unsigned char PDP_Context_APN[30];

extern int16_t Modem_gsm_rssi;

extern lwgsm_sim_state_t Modem_gsm_sim_state;
extern lwgsm_sim_state_t Modem_gsm_sim_state_t;

extern lwgsm_network_reg_status_t Modem_gsm_network_status;
extern lwgsm_network_reg_status_t Modem_gsm_network_status_t;

extern unsigned char Modem_gsm_network_GATT_Status;
extern unsigned char Modem_gsm_network_GATT_Status_t;
extern unsigned char Modem_gsm_network_GACT_Status;
extern unsigned char Modem_gsm_network_GACT_Status_t;


extern lwgsm_ip_t modem_ip;

extern int16_t Modem_AT_Command;
extern int16_t modemCommandSequence;

extern unsigned int rx_dataCurrentPosition;        // use for positioning for RX data of mqtt or UDU or TCP
extern unsigned char ReceivedDataOfMQTTClient;

extern unsigned int lora_rx_dataCurrentPosition;
extern unsigned char ReceivedDataOfLoRaClient;

extern osThreadId EC200U_RxRingProcess_TaskHandle;
extern char Flag_Reset_CFUN, Flag_Reset_QRESET,Flag_QPWRBACKOFF;

extern unsigned int count_1,count_2,count_3,count_4;


/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/

void MX_UART4_Init(void);
void Start_RxRingProcessTask(void const * argument);
void RxRingProcess_start();
void RxRingFiller(lwrb_t *rb, uint8_t*  data, size_t len);
void dataSendToRxRingFiller(lwrb_t *rb, uint8_t *buffer, size_t length, uint16_t RxXferCount, unsigned char callFrom);

lwgsmr_t RxRingProcess(const void* data, size_t data_len);
void RxRingProcess_buffer(lwrb_t *);
void lwgsmi_parse_received(lwgsm_recv_t* rcv);
lwgsmr_t lwgsmi_unicode_decode(lwgsm_unicode_t* s, uint8_t c);
uint8_t lwgsmi_parse_ip(const char** src, lwgsm_ip_t* ip);
uint8_t lwgsmi_parse_string(const char** src, char* dst, size_t dst_len, uint8_t trim);
//====== Network ==============================
int32_t lwgsmi_parse_number(const char** str);
float lwgsmi_parse_floatNumber(const char** str);
uint8_t lwgsmi_parse_creg(const char* str, uint8_t skip_first);
uint8_t lwgsmi_parse_csq(const char* str);
uint8_t lwgsmi_parse_cpin(const char* str, uint8_t send_evt);
uint8_t lwgsmi_parse_cgatt(const char* str);
uint8_t lwgsmi_parse_cgdcont(const char* str);
uint8_t lwgsmi_parse_cgact(const char* str, uint8_t skip_first);
uint8_t lwgsmi_parse_cgpaddr(const char* str, uint8_t skip_first);
//====== MQTT ==============================
uint8_t lwgsmi_parse_qmtopen(const char* str, uint8_t set_response);
uint8_t lwgsmi_parse_qmtclose(const char* str);
uint8_t lwgsmi_parse_qmtconn(const char* str, uint8_t set_response);
uint8_t lwgsmi_parse_qmtdisc(const char* str);
uint8_t lwgsmi_parse_qmtpubex(const char* str);
uint8_t lwgsmi_parse_qmtsub(const char* str);
uint8_t lwgsmi_parse_qmtuns(const char* str);
uint8_t lwgsmi_parse_qmtstat(const char* str);
uint8_t lwgsmi_parse_qmtrecv(const char* str);
uint8_t lwgsmi_parse_qmtping(const char* str);
//====== GPS ==============================
uint8_t lwgsmi_parse_qgpsloc(const char* str);
uint8_t lwgsmi_parse_qgps(const char* str);
//====== BLE ==============================
uint8_t lwgsmi_parse_qbtpwr(const char* str);
uint8_t lwgsmi_parse_qbtleaddr(const char* str);
uint8_t lwgsmi_parse_qbtgatscon(const char* str);
uint8_t lwgsmi_parse_qbtgatsdcon(const char* str);
uint8_t lwgsmi_parse_qbtlevaldata(const char* str);
uint8_t lwgsmi_parse_qbtgatrddataind(const char* str);
unsigned char parse_BLE_Data(unsigned short int _att_handle,unsigned short int _length,const char* _str);
//====== LORA ==============================
uint8_t parse_lora_linkCheck(const char* str);
uint8_t parse_lora_rx(const char* str);
uint8_t parse_lora_rx_event(const char* str);
uint8_t lora_device_firmware_version_number(const char* str);
uint8_t parse_lora_device_serial_number(const char* str);
uint8_t parse_lora_netork_join_state(const char* str);
uint8_t parse_lora_app_key(const char* str);
uint8_t parse_lora_app_eui(const char* str);
uint8_t parse_lora_dev_eui(const char* str);

uint8_t parse_lora_adaptive_data_rate(const char* str);
uint8_t parse_lora_class(const char* str);
uint8_t parse_lora_active_region_band(const char* str);
uint8_t parse_lora_network_mode(const char* str);

// Enhanced downlink checking functions
uint8_t check_lora_downlink_status(void);
void log_lora_downlink_diagnostics(void);

#endif /* INC_EC200U_RxRingProcess_H_ */
