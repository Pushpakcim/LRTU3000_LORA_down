/*
 * udpserver.h
 *
 *  Created on: Mar 31, 2022
 *      Author: controllerstech
 */

#ifndef INC_UDPSERVER_H_
#define INC_UDPSERVER_H_
/**************************************************************************//**
 * Includes
 *****************************************************************************/

/**************************************************************************//**
 * Constant
 *****************************************************************************/

// UDP data server - reception call back function
//
#define JJMHDP_HEADER									'H'
#define JJMHDP_GET_TOTAL_PACKETS						1
#define JJMHDP_GET_PACKET_BASED_ON_PACKET_ID			2
#define JJMHDP_HISTORY_DATA_DONE						3
#define JJMHDP_GET_RTU_ID								4
#define JJMHDP_HISTORY_DATA_ERASE						5

#define	MAX_HISTORY_DATA_PACKETS						20000 				// 2500 sector * 8 atain one sector

#define EX_FLASH_RUN_TIME_PARA_PAGE           			32760				// 32760 and 32761 Page for Runtime Array
#define EX_FLASH_FIST_PAGE_OF_LOGRATE_DATA				0					// 0 and 1st page for First Log rate data
#define EX_FLASH_LAST_PAGE_OF_LOGRATE_DATA				32758				// 32758 and 32759 pages are use to store last Data packet
#define EX_Flash_DATA_PACKET_SIZE						512					// 2 pages are use to store 1 Data packet

/**************************************************************************//**
 * Ennam / uninon / Structure
 *****************************************************************************/

typedef struct
{
	unsigned int sf_client_id;  					//4
	unsigned int sf_reader_id;  					//4+4=8
	unsigned char sf_Date;							//8+1=9
	unsigned char sf_Month;							//9+1=10
	unsigned char sf_Year;						//10+1=11
	unsigned char sf_Hour;							//11+1=12
	unsigned char sf_Min;							//12+1=13
	unsigned char sf_Sec;							//13+1=14
	unsigned char sf_NO_DI_LG;						//14+1=15
	unsigned char sf_NoofSMSTag;					//15+1=16
	unsigned int sf_Address; 						//16+4=20       //RTUD_ID address of this RTU stored in Address
	unsigned char sf_DI[2]; 						//20+2=22				//16 DI chennal in 2 byte
	float sf_AnalogValue[120];						//22+(120*4=480)=502
	unsigned char unused[6];
}flashDataSturct;

typedef struct
{
	flashDataSturct HistoryDataPacket[4];
}flashHistoryData_4PackSturct;

typedef struct
{
	unsigned short int s_ExtDataFlash_CheckByte;  							//	2
	unsigned short int s_ExtDataFlash_IsDataLogOverwritten;  		//	2
	unsigned int s_ExtDataFlash_PageCounter;  									//	4
	unsigned char s_temp_Counter;
	unsigned char unused[503];
}flashRunTimeParaSturct;

/**************************************************************************//**
 * extern
 *****************************************************************************/

extern unsigned char historyDataPacket[700];
extern unsigned char historyDataPackettemp[100];
extern unsigned char gStopHistoricalDataStoreCounter;
extern unsigned char gStopHistoricalDataStore;
extern unsigned short int tHistoriDatalength;
extern flashRunTimeParaSturct g_flashRunTimeParaSturct;

/**************************************************************************//**
 * Function proto
 *****************************************************************************/

void udpserver_init(void);
void udpserver_1_init(void);
void udp_server_1_thread(void const * argument);
unsigned char UDPFrameParser(char* data,unsigned int len);
void ExtFlash_Read_RuntimePara(unsigned char makeDefault);
void ExtFlash_ReadHistoricalDataLogFromFlash(unsigned int pageCounter,unsigned char *buffer);
void ExtFlash_WriteHistoricalData(flashDataSturct tflashDataSturct);

#endif /* INC_UDPSERVER_H_ */
